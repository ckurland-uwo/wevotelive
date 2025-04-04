#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocketServer.h>
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>
#include <algorithm>
#include <functional>
#include <random>
#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include <deque>
#include <memory>

#include "json.hpp"

// TODO:
// check duplicarte ids
// fix threads

using json = nlohmann::json;

using namespace std;

mutex g_coutMutex;
mutex roomExpiryMutex;

unordered_map<string, chrono::steady_clock::time_point> expiringRooms;

unordered_map<string, function<void(const json &, shared_ptr<ix::WebSocket>)>> handlers;

unordered_map<string, unordered_map<string, string>> roomParticipantMap;

void print(const string &message)
{
    lock_guard<mutex> lock(g_coutMutex);
    cout << message << endl;
}

string generateRandomID()
{
    static const string chars =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789";

    static thread_local mt19937 rng{random_device{}()};
    static uniform_int_distribution<size_t> dist(0, chars.size() - 1);

    string code;
    for (int i = 0; i < 4; ++i)
    {
        code += chars[dist(rng)];
    }

    return code;
}

struct Poll
{
    Poll(const string &newQuestion, const string &newDescription, const string &newOptionA, const string &newOptionB)
    {
        id = generateRandomID();
        question = newQuestion;
        description = newDescription;
        optionA = newOptionA;
        optionB = newOptionB;
        predictionsA = 0;
        predictionsB = 0;
        votesA = 0;
        votesB = 0;
        revealed = false;
    };

    string id;
    string question;
    string description;
    string optionA;
    string optionB;
    int predictionsA;
    int predictionsB;
    int votesA;
    int votesB;
    bool revealed;
};

void to_json(json &j, const Poll &p)
{
    j = json{
        {"id", p.id},
        {"question", p.question},
        {"description", p.description},
        {"optionA", p.optionA},
        {"optionB", p.optionB},
        {"predictionsA", p.predictionsA},
        {"predictionsB", p.predictionsB},
        {"votesA", p.votesA},
        {"votesB", p.votesB},
        {"revealed", p.revealed}};
}
unordered_map<string, vector<Poll>> allPolls; // roomId, polls

struct UserVote
{
    optional<bool> votedA;
    optional<bool> predictedA;
};

void to_json(json &j, const UserVote &u)
{
    j = json{
        {"votedA", u.votedA},
        {"predictedA", u.predictedA},
    };
}

struct Message;

struct ConnectionThreadInfo
{
    ConnectionThreadInfo(const string &type,
                         const string &identity,
                         const string &roomCode,
                         shared_ptr<ix::WebSocket> socket)
        : type(type), identity(identity), roomCode(roomCode), socket(socket), running(true)
    {
    }

    string type;
    string identity;
    string roomCode;
    shared_ptr<ix::WebSocket> socket;
    thread threadHandle;
    deque<Message> personalQueue;
    condition_variable personalCond;
    mutex personalMutex;
    bool running;
};

struct Message
{
    json payload;
    shared_ptr<ix::WebSocket> socket;
    int priority; // note the lower the higher the priority (0 most priority, 5 least priority)
    string roomCode;
};

mutex wsQueueMutex;
condition_variable queueCond;
deque<Message> globalMessageQueue;

unordered_map<shared_ptr<ix::WebSocket>, unique_ptr<ConnectionThreadInfo>> allConnectionThreads;

unordered_map<string, unordered_map<string, unordered_map<string, UserVote>>> allVotes;
//<roomId, <identity, <pollId, uservote>>>

Message buildErrorMessage(shared_ptr<ix::WebSocket> ws, const string &error, const bool &retry)
{
    Message message;
    message.socket = ws;
    message.payload = json{
        {"type", "error"},
        {"error", error},
        {"retry", retry}};
    return message;
}

// Message callback function.
void onMessage(shared_ptr<ix::WebSocket> webSocket, const ix::WebSocketMessagePtr &msg)
{
    string received;
    if (msg->type == ix::WebSocketMessageType::Message)
    {
        received = msg->str;
        auto it = allConnectionThreads.find(webSocket);
        if (it == allConnectionThreads.end())
        {
            json message = json::parse(received);
            print("Received from " + message["identity"].get<string>() + ": " + received);
        }
        else
        {
            print("Received from " + it->second->identity + ": " + received);
        }
    }
    else if (msg->type == ix::WebSocketMessageType::Open)
    {
        print("A client connected!");
    }
    else if (msg->type == ix::WebSocketMessageType::Close)
    {
        auto it = allConnectionThreads.find(webSocket);
        if (it == allConnectionThreads.end())
        {
            print("A client has disconnected.");
        }
        else
        {
            string type = it->second->type;
            if (type == "host")
            {
                print("The host " + it->second->identity + " of room " + it->second->roomCode + " has disconnected.");
                {
                    lock_guard<mutex> lock(roomExpiryMutex);
                    expiringRooms[it->second->roomCode] = chrono::steady_clock::now() + chrono::minutes(1);
                }
                print("Room " + it->second->roomCode + " is expiring in 1 minute.");
            }
            else
            {
                print("The participant " + it->second->identity + " of room " + it->second->roomCode + " has disconnected.");
            }
        }
    }
    else if (msg->type == ix::WebSocketMessageType::Error)
    {
        print("Error: " + msg->errorInfo.reason);
    }

    if (msg->type == ix::WebSocketMessageType::Message)
    {
        json message = json::parse(received);
        if (!message.contains("type") || !message["type"].is_string())
        {
            Message message = buildErrorMessage(webSocket, "missing-type", true);
            message.priority = 0;
            globalMessageQueue.push_back(message);
            queueCond.notify_one();
            return;
        }

        string action = message["type"];
        auto it = handlers.find(action);
        if (it != handlers.end())
        {
            it->second(message, webSocket);
        }
        else
        {
            Message message = buildErrorMessage(webSocket, "invalid-type", true);
            message.priority = 0;
            globalMessageQueue.push_back(message);
            queueCond.notify_one();
            return;
        }
    }
}

// Connection callback function.
void onConnection(weak_ptr<ix::WebSocket> weakWebSocket,
                  shared_ptr<ix::ConnectionState> connectionState)
{
    // Obtain a shared pointer to the WebSocket.
    shared_ptr<ix::WebSocket> webSocket = weakWebSocket.lock();
    if (!webSocket)
    {
        return;
    }
    // Set the message callback using bind.
    webSocket->setOnMessageCallback(bind(onMessage, webSocket, placeholders::_1));
}

void createRoomExpiryThread()
{
    while (true)
    {
        this_thread::sleep_for(chrono::seconds(1));
        auto now = chrono::steady_clock::now();

        vector<string> toRemove;

        {
            lock_guard<mutex> lock(roomExpiryMutex);
            for (auto &[room, expiryTime] : expiringRooms)
            {
                if (now >= expiryTime)
                {
                    toRemove.push_back(room);
                }
            }
        }

        for (const auto &room : toRemove)
        {
            {
                lock_guard<mutex> lock(wsQueueMutex);
                allPolls.erase(room);
                roomParticipantMap.erase(room);
                allVotes.erase(room);

                for (auto it = allConnectionThreads.begin(); it != allConnectionThreads.end();)
                {
                    if (it->second->roomCode == room)
                    {
                        auto ws = it->first;
                        try
                        {
                            Message message = buildErrorMessage(ws, "room-closed", false);
                            message.priority = 0;
                            ws->send(message.payload.dump());
                        }
                        catch (const std::exception &e)
                        {
                            print("[ERROR] Exception while sending room-closed error: " + string(e.what()));
                        }
                        catch (...)
                        {
                            print("[ERROR] Unknown exception while sending room-closed error");
                        }

                        it->second->running = false;
                        it->second->personalCond.notify_all();

                        if (it->second->threadHandle.joinable())
                        {
                            it->second->threadHandle.join();
                        }

                        it = allConnectionThreads.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }
            }
            {
                lock_guard<mutex> lock(roomExpiryMutex);
                if (expiringRooms.count(room) == 0)
                    continue;
                expiringRooms.erase(room);
            }
            print("Room " + room + " has expired and has been deleted.");
        }
    }
}

void repairHostThread(shared_ptr<ix::WebSocket> newSocket, ConnectionThreadInfo &oldCTI)
{
    {
        lock_guard<mutex> lock(roomExpiryMutex);
        expiringRooms.erase(oldCTI.roomCode);
    }

    auto oldSocket = oldCTI.socket;
    oldCTI.socket = newSocket;

    oldCTI.running = false;
    oldCTI.personalCond.notify_all();
    if (oldCTI.threadHandle.joinable())
    {
        oldCTI.threadHandle.join();
    }

    oldCTI.threadHandle = thread([cti_ptr = &oldCTI]()
                                 {
            while (true)
            {
            unique_lock<mutex> lock(cti_ptr->personalMutex);
            cti_ptr->personalCond.wait(lock, [cti_ptr]() {
                return !cti_ptr->personalQueue.empty() || !cti_ptr->running;
            });
            if (!cti_ptr->running)
            {
                break;
            }

            auto msg = cti_ptr->personalQueue.front();
            cti_ptr->personalQueue.pop_front();
            lock.unlock();

            cti_ptr->socket->send(msg.payload.dump());

            print("[Thread] Reconnected host thread for " + cti_ptr->identity + " in room " + cti_ptr->roomCode);
            } });

    {
        lock_guard<mutex> lock(wsQueueMutex);
        allConnectionThreads[newSocket] = move(allConnectionThreads[oldSocket]);
        allConnectionThreads.erase(oldSocket);
    }

    newSocket->send(json{
        {"polls", allPolls[oldCTI.roomCode]},
        {"type", "host"},
        {"hostingRoomCode", oldCTI.roomCode}}
                        .dump());
}

void createHostThread(shared_ptr<ix::WebSocket> socket, const string &identity)
{
    auto it = find_if(allConnectionThreads.begin(), allConnectionThreads.end(),
                      [&](const auto &pair)
                      {
                          return pair.second->identity == identity;
                      });

    if (it != allConnectionThreads.end() && it->second->type == "host")
    {
        auto &threadInfo = *it->second;
        print("Found host in room: " + threadInfo.roomCode);
        repairHostThread(socket, threadInfo);
        return;
    }
    string roomCode = generateRandomID();
    roomParticipantMap[roomCode]["host"] = identity;
    allPolls[roomCode] = {};

    auto cti = make_unique<ConnectionThreadInfo>("host", identity, roomCode, socket);
    ConnectionThreadInfo *cti_ptr = cti.get();

    cti_ptr->threadHandle = thread([cti_ptr]()
                                   {
        while (true) {
            unique_lock<mutex> lock(cti_ptr->personalMutex);
            cti_ptr->personalCond.wait(lock, [cti_ptr]() {
                return !cti_ptr->personalQueue.empty() || !cti_ptr->running;
            });
            if (!cti_ptr->running)
            {
                break;
            }
    
            auto msg = cti_ptr->personalQueue.front();
            cti_ptr->personalQueue.pop_front();
            lock.unlock();
    
            cti_ptr->socket->send(msg.payload.dump());
    
            print("[Thread] Sent to " + cti_ptr->identity + " (" + cti_ptr->type + ") in room " + cti_ptr->roomCode);
        } });
    allConnectionThreads[socket] = move(cti);
    print("[Thread] Created host thread for " + identity + " in room " + roomCode);

    socket->send(json{
        {"polls", allPolls[roomCode]},
        {"type", "host"},
        {"hostingRoomCode", roomCode}}
                     .dump());
}

void repairParticipantThread(shared_ptr<ix::WebSocket> newSocket, ConnectionThreadInfo &oldCTI)
{
    auto oldSocket = oldCTI.socket;
    oldCTI.socket = newSocket;

    oldCTI.running = false;
    oldCTI.personalCond.notify_all();
    if (oldCTI.threadHandle.joinable())
    {
        oldCTI.threadHandle.join();
    }

    oldCTI.threadHandle = thread([cti_ptr = &oldCTI]()
                                 {
            while (true)
            {
            unique_lock<mutex> lock(cti_ptr->personalMutex);
            cti_ptr->personalCond.wait(lock, [cti_ptr]() {
                return !cti_ptr->personalQueue.empty() || !cti_ptr->running;
            });
            if (!cti_ptr->running)
            {
                break;
            }

            auto msg = cti_ptr->personalQueue.front();
            cti_ptr->personalQueue.pop_front();
            lock.unlock();

            cti_ptr->socket->send(msg.payload.dump());

            print("[Thread] Reconnected host thread for " + cti_ptr->identity + " in room " + cti_ptr->roomCode);
            } });

    {
        lock_guard<mutex> lock(wsQueueMutex);
        allConnectionThreads[newSocket] = move(allConnectionThreads[oldSocket]);
        allConnectionThreads.erase(oldSocket);
    }

    newSocket->send(json{
        {"polls", allPolls[oldCTI.roomCode]},
        {"type", "participant"},
        {"votes", allVotes[oldCTI.roomCode][oldCTI.identity]}}
                        .dump());
}

void createParticipantThread(shared_ptr<ix::WebSocket> socket, const string &roomCode, const string &identity)
{
    auto it = find_if(allConnectionThreads.begin(), allConnectionThreads.end(),
                      [&](const auto &pair)
                      {
                          return pair.second->identity == identity;
                      });

    if (it != allConnectionThreads.end())
    {
        auto &threadInfo = *it->second;
        print("Found participant in room: " + threadInfo.roomCode);
        repairParticipantThread(socket, threadInfo);
        return;
    }
    auto cti = make_unique<ConnectionThreadInfo>("participant", identity, roomCode, socket);
    ConnectionThreadInfo *cti_ptr = cti.get();

    cti_ptr->threadHandle = thread([cti_ptr]()
                                   {
        while (true) {
            unique_lock<mutex> lock(cti_ptr->personalMutex);
            cti_ptr->personalCond.wait(lock, [cti_ptr]() {
                return !cti_ptr->personalQueue.empty() || !cti_ptr->running;
            });
            if (!cti_ptr->running)
            {
                break;
            }
    
            auto msg = cti_ptr->personalQueue.front();
            cti_ptr->personalQueue.pop_front();
            lock.unlock();
    
            cti_ptr->socket->send(msg.payload.dump());
    
            print("[Thread] Sent to " + cti_ptr->identity + " (" + cti_ptr->type + ") in room " + cti_ptr->roomCode);
        } });
    allConnectionThreads[socket] = move(cti);
    print("[Thread] Created participant thread for " + identity + " in room " + roomCode);

    socket->send(json{
        {"polls", allPolls[roomCode]},
        {"type", "participant"},
        {"votes", allVotes[roomCode][identity]}}
                     .dump());
}

Message buildMessageForClient(shared_ptr<ix::WebSocket> ws, const string &roomCode)
{
    ConnectionThreadInfo &cti = *allConnectionThreads[ws];

    Message message;
    message.socket = ws;
    message.roomCode = roomCode;

    if (cti.type == "host")
    {
        message.payload = json{
            {"type", "host"},
            {"polls", allPolls[roomCode]},
            {"hostingRoomCode", roomCode}};
    }
    else
    {
        message.payload = json{
            {"type", "participant"},
            {"polls", allPolls[roomCode]},
            {"votes", allVotes[roomCode][cti.identity]}};
    }

    return message;
}

void dispatcherThreadFunction()
{
    while (true)
    {
        unique_lock<mutex> lock(wsQueueMutex);
        queueCond.wait(lock, []
                       { return !globalMessageQueue.empty(); });

        sort(globalMessageQueue.begin(), globalMessageQueue.end(),
             [](const Message &m1, const Message &m2)
             {
                 return m1.priority < m2.priority;
             });
        auto message = globalMessageQueue.front();
        globalMessageQueue.pop_front();
        lock.unlock();

        if (message.payload["type"] == "error")
        {
            message.socket->send(message.payload.dump());
            continue;
        }

        for (auto &[ws, ctiPtr] : allConnectionThreads)
        {
            if (ctiPtr->roomCode == message.roomCode)
            {
                Message tailoredMsg = buildMessageForClient(ws, message.roomCode);
                tailoredMsg.priority = message.priority;
                lock_guard<mutex> threadLock(ctiPtr->personalMutex);
                ctiPtr->personalQueue.push_back(tailoredMsg);
                ctiPtr->personalCond.notify_one();
            }
        }
    }
}

void registerHandlers()
{
    handlers["join-hello"] = [](const json &msg, shared_ptr<ix::WebSocket> ws)
    {
        string roomCode = msg["roomCode"];
        string identity = msg["identity"];
        if (roomParticipantMap[roomCode].empty())
        {
            Message message = buildErrorMessage(ws, "invalid-roomCode", false); // dont retry as the room is wrong
            message.priority = 0;
            globalMessageQueue.push_back(message);
            queueCond.notify_one();
            return;
        }
        roomParticipantMap[roomCode].insert(make_pair("participant", identity));
        createParticipantThread(ws, roomCode, identity);
    };

    handlers["host-hello"] = [](const json &msg, shared_ptr<ix::WebSocket> ws)
    {
        string identity = msg["identity"];
        createHostThread(ws, identity);
    };

    handlers["vote"] = [](const json &msg, shared_ptr<ix::WebSocket> ws)
    {
        lock_guard<mutex> lock(wsQueueMutex);
        string poll = msg["for"];
        bool votedA = msg["votedA"];
        bool predictedA = msg["predictedA"];

        if (allConnectionThreads.find(ws) == allConnectionThreads.end())
        {
            Message message = buildErrorMessage(ws, "thread-error", false); // dont retry as the thread wont be created if u try again lol
            message.priority = 0;
            globalMessageQueue.push_back(message);
            queueCond.notify_one();
            return;
        }

        ConnectionThreadInfo &cti = *allConnectionThreads.at(ws);
        string type = cti.type;
        string identity = cti.identity;
        string roomCode = cti.roomCode;

        if (type != "participant")
        {
            Message message = buildErrorMessage(ws, "host-vote", false);
            message.priority = 0;
            globalMessageQueue.push_back(message);
            queueCond.notify_one();
            return;
        }

        auto &polls = allPolls[roomCode];
        auto it = find_if(polls.begin(), polls.end(), [&](const Poll &p)
                          { return p.id == poll; });

        if (it == polls.end())
        {
            Message message = buildErrorMessage(ws, "invalid-poll-vote", false);
            message.priority = 0;
            globalMessageQueue.push_back(message);
            queueCond.notify_one();
            return;
        }

        auto &votes = allVotes[roomCode][identity][poll];
        votes.votedA = votedA;
        votes.predictedA = predictedA;

        if (votedA)
        {
            it->votesA++;
        }
        else
        {
            it->votesB++;
        }

        if (predictedA)
        {
            it->predictionsA++;
        }
        else
        {
            it->predictionsB++;
        }

        Message message = buildMessageForClient(ws, roomCode);
        message.priority = 3;
        globalMessageQueue.push_back(message);

        queueCond.notify_one();
    };

    handlers["create-poll"] = [](const json &msg, shared_ptr<ix::WebSocket> ws)
    {
        lock_guard<mutex> lock(wsQueueMutex);
        string question = msg["question"];
        string description = msg["description"];
        string optionA = msg["optionA"];
        string optionB = msg["optionB"];

        if (allConnectionThreads.find(ws) == allConnectionThreads.end())
        {

            Message message = buildErrorMessage(ws, "thread-error", true);
            message.priority = 0;
            globalMessageQueue.push_back(message);
            queueCond.notify_one();
            return;
        }

        ConnectionThreadInfo &cti = *allConnectionThreads.at(ws);
        string type = cti.type;
        string identity = cti.identity;
        string roomCode = cti.roomCode;

        if (type != "host")
        {
            Message message = buildErrorMessage(ws, "participant-poll-creation", true);
            message.priority = 0;
            globalMessageQueue.push_back(message);
            queueCond.notify_one();
            return;
        }

        Poll createdPoll = Poll(question, description, optionA, optionB);
        allPolls[roomCode].push_back(createdPoll);

        Message message = buildMessageForClient(ws, roomCode);
        message.priority = 0;
        globalMessageQueue.push_back(message);

        queueCond.notify_one();
    };
    handlers["reveal-poll"] = [](const json &msg, shared_ptr<ix::WebSocket> ws)
    {
        lock_guard<mutex> lock(wsQueueMutex);
        string pollId = msg["id"];

        if (allConnectionThreads.find(ws) == allConnectionThreads.end())
        {
            Message message = buildErrorMessage(ws, "thread-error", true);
            message.priority = 0;
            globalMessageQueue.push_back(message);
            queueCond.notify_one();
            return;
        }

        ConnectionThreadInfo &cti = *allConnectionThreads.at(ws);
        string type = cti.type;
        string identity = cti.identity;
        string roomCode = cti.roomCode;

        if (type != "host")
        {
            Message message = buildErrorMessage(ws, "participant-reveal", true);
            message.priority = 0;
            globalMessageQueue.push_back(message);
            queueCond.notify_one();
            return;
        }

        auto &polls = allPolls[roomCode];
        auto it = find_if(polls.begin(), polls.end(), [&](const Poll &poll)
                          { return poll.id == pollId; });

        if (it != polls.end())
        {
            int index = distance(polls.begin(), it);
            allPolls[roomCode][index].revealed = true;
        }
        else
        {
            Message message = buildErrorMessage(ws, "invalid-poll-reveal", true);
            message.priority = 0;
            globalMessageQueue.push_back(message);
            queueCond.notify_one();
            return;
        }

        Message message = buildMessageForClient(ws, roomCode);
        message.priority = 1;
        globalMessageQueue.push_back(message);

        queueCond.notify_one();
    };
    handlers["delete-poll"] = [](const json &msg, shared_ptr<ix::WebSocket> ws)
    {
        lock_guard<mutex> lock(wsQueueMutex);
        string pollId = msg["id"];

        if (allConnectionThreads.find(ws) == allConnectionThreads.end())
        {
            Message message = buildErrorMessage(ws, "thread-error", true);
            message.priority = 0;
            globalMessageQueue.push_back(message);
            queueCond.notify_one();
            return;
        }

        ConnectionThreadInfo &cti = *allConnectionThreads.at(ws);
        string type = cti.type;
        string identity = cti.identity;
        string roomCode = cti.roomCode;

        if (type != "host")
        {
            Message message = buildErrorMessage(ws, "participant-poll-delete", true);
            message.priority = 0;
            globalMessageQueue.push_back(message);
            queueCond.notify_one();
            return;
        }

        auto &polls = allPolls[roomCode];
        bool found = false;

        for (auto it = polls.begin(); it != polls.end();)
        {
            if (it->id == pollId)
            {
                it = polls.erase(it);
                found = true;
            }
            else
            {
                ++it;
            }
        }

        if (!found)
        {
            Message message = buildErrorMessage(ws, "invalid-poll-delete", true);
            message.priority = 0;
            globalMessageQueue.push_back(message);
            queueCond.notify_one();
            return;
        }
        for (auto &[participant, voteMap] : allVotes[roomCode])
        {
            voteMap.erase(pollId);
        }

        Message message = buildMessageForClient(ws, roomCode);
        message.priority = 0;
        globalMessageQueue.push_back(message);

        queueCond.notify_one();
    };
}

int main()
{
    registerHandlers();
    // initialize the network system (for windows only :))))))
    ix::initNetSystem();

    // create a localhost websocket on 8080
    ix::WebSocketServer server(8080, "0.0.0.0");

    // set a connection callback
    server.setOnConnectionCallback(onConnection);

    // start listening!
    auto res = server.listen();
    if (!res.first)
    {
        print("Error starting server: " + res.second);
        return 1;
    }

    // start the server in a background thread
    server.start();
    thread(dispatcherThreadFunction).detach();
    thread(createRoomExpiryThread).detach();
    print("WebSocket server started on ws://localhost:8080");

    // keep the server running.
    while (true)
    {
        this_thread::sleep_for(chrono::seconds(1));
    }

    return 0;
}
