cmake_minimum_required(VERSION 3.5)
project(WebSocketTestServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Base path to IXWebSocket source
set(IXWS_PATH "${CMAKE_SOURCE_DIR}/IXWebSocket")

# Include the IXWebSocket headers
include_directories(${IXWS_PATH})

# Choose the correct build output folder depending on platform
if(WIN32)
    set(IXWS_LIB_PATH "${IXWS_PATH}/build-win") # You said you already built it for Windows
elseif(UNIX)
    set(IXWS_LIB_PATH "${IXWS_PATH}/build") # You’ll build this next on Ubuntu/WSL
endif()

link_directories(${IXWS_LIB_PATH})

add_executable(server server.cpp)

# Platform-specific linking
if(WIN32)
    target_link_libraries(server ixwebsocket ws2_32 z)
elseif(UNIX)
    target_link_libraries(server ixwebsocket z pthread)
endif()