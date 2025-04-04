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

using json = nlohmann::json;

using namespace std;

mutex g_coutMutex;
mutex globalStorageMutex;

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
    Poll(const string &pollId, const string &newQuestion, const string &newDescription, const string &newOptionA, const string &newOptionB)
    {
        id = pollId;
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

string generateUniqueRoomID()
{
    string id;
    while (true)
    {
        id = generateRandomID();
        {
            lock_guard<mutex> lock(globalStorageMutex);
            if (roomParticipantMap.find(id) == roomParticipantMap.end() &&
                allPolls.find(id) == allPolls.end() &&
                expiringRooms.find(id) == expiringRooms.end())
            {
                break;
            }
        }
    }
    return id;
}

string generateUniquePollID(const string &roomCode)
{
    string id;
    bool collision;
    do
    {
        id = generateRandomID();
        collision = false;
        {
            lock_guard<mutex> lock(globalStorageMutex);
            if (allPolls.find(roomCode) != allPolls.end())
            {
                for (const auto &poll : allPolls[roomCode])
                {
                    if (poll.id == id)
                    {
                        collision = true;
                        break;
                    }
                }
            }
        }
    } while (collision);
    return id;
}

string generateUniqueSessionID()
{
    string id;
    while (id.length() < 16)
    {
        string temp = generateRandomID();
        id += temp;
    }
    return (id);
}

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
            string roomCode = it->second->roomCode;
            string identity = it->second->identity;
            if (type == "host")
            {
                print("A host session for " + identity + " in room " + roomCode + " has disconnected.");

                it->second->running = false;
                it->second->personalCond.notify_all();
                if (it->second->threadHandle.joinable())
                {
                    it->second->threadHandle.join();
                }
                allConnectionThreads.erase(it);

                bool otherHostSessions = false;
                {
                    lock_guard<mutex> lock(wsQueueMutex);
                    for (auto &pair : allConnectionThreads)
                    {
                        if (pair.second->roomCode == roomCode &&
                            pair.second->type == "host" &&
                            pair.second->identity == identity)
                        {
                            otherHostSessions = true;
                            break;
                        }
                    }
                }
                if (!otherHostSessions)
                {
                    lock_guard<mutex> lock(globalStorageMutex);
                    expiringRooms[roomCode] = chrono::steady_clock::now() + chrono::minutes(1);
                    print("No more host sessions remain for room " + roomCode + ". Room is expiring in 1 minute.");
                }
            }
            else
            {
                print("The participant " + identity + " of room " + roomCode + " has disconnected.");
                it->second->running = false;
                it->second->personalCond.notify_all();
                if (it->second->threadHandle.joinable())
                {
                    it->second->threadHandle.join();
                }
                allConnectionThreads.erase(it);
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
            lock_guard<mutex> lock(globalStorageMutex);
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
                lock_guard<mutex> lock(globalStorageMutex);
                allPolls.erase(room);
                roomParticipantMap.erase(room);
                allVotes.erase(room);
            }
            vector<thread> threadsToJoin;
            {
                lock_guard<mutex> lock(wsQueueMutex);
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
                            threadsToJoin.push_back(move(it->second->threadHandle));
                        }
                        it = allConnectionThreads.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }
            }
            for (auto &t : threadsToJoin)
            {
                t.join();
            }
            {
                lock_guard<mutex> lock(globalStorageMutex);
                if (expiringRooms.count(room) > 0)
                    expiringRooms.erase(room);
            }
            print("Room " + room + " has expired and has been deleted.");
        }
    }
}

void createHostThread(shared_ptr<ix::WebSocket> socket, const string &identity)
{
    string roomCode;
    bool roomExists = false;
    {
        lock_guard<mutex> lock(globalStorageMutex);
        for (const auto &entry : roomParticipantMap)
        {
            auto it = entry.second.find("host");
            if (it != entry.second.end() && it->second == identity)
            {
                roomCode = entry.first;
                roomExists = true;
                break;
            }
        }
    }

    if (!roomExists)
    {
        lock_guard<mutex> lock(wsQueueMutex);
        for (const auto &pair : allConnectionThreads)
        {
            if (pair.second->identity == identity && pair.second->type == "host")
            {
                roomCode = pair.second->roomCode;
                roomExists = true;
                break;
            }
        }
    }

    if (roomExists)
    {
        lock_guard<mutex> lock(globalStorageMutex);
        if (expiringRooms.find(roomCode) != expiringRooms.end())
        {
            expiringRooms.erase(roomCode);
            print("Host " + identity + " for room " + roomCode + " has rejoined, cancelling room expiry.");
        }
    }

    if (!roomExists)
    {
        roomCode = generateUniqueRoomID();
        {
            lock_guard<mutex> lock(globalStorageMutex);
            roomParticipantMap[roomCode]["host"] = identity;
            allPolls[roomCode] = {};
        }
        print("No existing room found. Creating new room " + roomCode + " for host " + identity);
    }

    string sessionId = generateUniqueSessionID();

    auto cti = make_unique<ConnectionThreadInfo>("host", identity, roomCode, socket);
    ConnectionThreadInfo *cti_ptr = cti.get();

    cti_ptr->threadHandle = thread([cti_ptr]()
                                   {
        while (true)
        {
            unique_lock<mutex> lock(cti_ptr->personalMutex);
            cti_ptr->personalCond.wait(lock, [cti_ptr]()
            {
                return !cti_ptr->personalQueue.empty() || !cti_ptr->running;
            });
            if (!cti_ptr->running)
                break;

            auto msg = cti_ptr->personalQueue.front();
            cti_ptr->personalQueue.pop_front();
            lock.unlock();

            cti_ptr->socket->send(msg.payload.dump());
            print("[Thread] Sent to " + cti_ptr->identity + " (" + cti_ptr->type + ") in room " + cti_ptr->roomCode);
        } });

    {
        lock_guard<mutex> lock(wsQueueMutex);
        allConnectionThreads[socket] = move(cti);
    }

    print("[Thread] Created new host session for " + identity + " (session " + sessionId + ") in room " + roomCode);

    {
        lock_guard<mutex> lock(globalStorageMutex);
        socket->send(json{
            {"polls", allPolls[roomCode]},
            {"type", "host"},
            {"hostingRoomCode", roomCode}}
                         .dump());
    }
}

void createParticipantThread(shared_ptr<ix::WebSocket> socket, const string &roomCode, const string &identity)
{
    string sessionId = generateUniqueSessionID();
    string compositeId = identity + "_" + sessionId;

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
                break;

            auto msg = cti_ptr->personalQueue.front();
            cti_ptr->personalQueue.pop_front();
            lock.unlock();

            cti_ptr->socket->send(msg.payload.dump());
            print("[Thread] Sent to " + cti_ptr->identity + " (" + cti_ptr->type + ") in room " + cti_ptr->roomCode);
        } });

    {
        lock_guard<mutex> lock(wsQueueMutex);
        allConnectionThreads[socket] = move(cti);
    }

    print("[Thread] Created new participant session for " + identity + " (session " + sessionId + ") in room " + roomCode);

    {
        lock_guard<mutex> lock(globalStorageMutex);
        socket->send(json{
            {"polls", allPolls[roomCode]},
            {"type", "participant"},
            {"votes", allVotes[roomCode][identity]}}
                         .dump());
    }
}

Message buildMessageForClient(shared_ptr<ix::WebSocket> ws, const string &roomCode)
{
    ConnectionThreadInfo &cti = *allConnectionThreads[ws];

    Message message;
    message.socket = ws;
    message.roomCode = roomCode;

    {
        lock_guard<mutex> lock(globalStorageMutex);
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
        {
            lock_guard<mutex> lock(globalStorageMutex);
            if (roomParticipantMap[roomCode].empty())
            {
                Message message = buildErrorMessage(ws, "invalid-roomCode", false); // dont retry as the room is wrong
                message.priority = 0;
                globalMessageQueue.push_back(message);
                queueCond.notify_one();
                return;
            }
            roomParticipantMap[roomCode][identity] = identity;
        }
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

        {
            lock_guard<mutex> lock(globalStorageMutex);
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

        string pollId = generateUniquePollID(roomCode);
        Poll createdPoll = Poll(pollId, question, description, optionA, optionB);
        {
            lock_guard<mutex> gLock(globalStorageMutex);
            allPolls[roomCode].push_back(createdPoll);
        }
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

        {
            lock_guard<mutex> gLock(globalStorageMutex);
            auto &polls = allPolls[roomCode];
            auto it = find_if(polls.begin(), polls.end(), [&](const Poll &poll)
                              { return poll.id == pollId; });

            if (it != polls.end())
            {
                int index = distance(polls.begin(), it);
                polls[index].revealed = true;
            }
            else
            {
                Message message = buildErrorMessage(ws, "invalid-poll-reveal", true);
                message.priority = 0;
                globalMessageQueue.push_back(message);
                queueCond.notify_one();
                return;
            }
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

        bool found = false;
        {
            lock_guard<mutex> gLock(globalStorageMutex);
            auto &polls = allPolls[roomCode];
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
