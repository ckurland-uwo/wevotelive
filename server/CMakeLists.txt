cmake_minimum_required(VERSION 3.5)
project(WebSocketTestServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the path to your IXWebSocket repository.
# Here, we assume your directory structure is:
# server/
# ├── IXWebSocket/   <-- cloned IXWebSocket repository
# ├── server.cpp
# └── CMakeLists.txt
set(IXWS_PATH "${CMAKE_SOURCE_DIR}/IXWebSocket")

# Include the IXWebSocket headers. Since the headers are in IXWebSocket/ixwebsocket,
# we add the top-level folder so that #include <ixwebsocket/IXWebSocketServer.h> works.
include_directories(${IXWS_PATH})

# Link to the IXWebSocket build output.
link_directories(${IXWS_PATH}/build)

add_executable(server server.cpp)

# Link against ixwebsocket, the Windows sockets library, and zlib.
target_link_libraries(server ixwebsocket ws2_32 z)
