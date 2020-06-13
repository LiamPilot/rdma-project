cmake_minimum_required(VERSION 3.5)
project(ConnectionExperiments)

set(CMAKE_CXX_STANDARD 14)

# Add -O0 to remove optimizations when using gcc
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O0")
#set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O0")

add_subdirectory(infinity)

add_executable(ConnectionExperiments main.cpp
        utils.cpp
        utils.h
        experiment_type.h
        RdmaClient.cpp
        RdmaClient.h
        RdmaServer.cpp
        RdmaServer.h
        Client.h
        Server.h
        TcpClient.cpp
        TcpClient.h
        TcpServer.cpp
        TcpServer.h)

include_directories(/usr/include /usr/local/include ./)

find_library(IBVERBS ibverbs /usr/lib)
target_link_libraries(ConnectionExperiments ${IBVERBS} Infinity)
