cmake_minimum_required(VERSION 3.5)
project(ThroughputExperiments)

set(CMAKE_CXX_STANDARD 14)

# Add -O0 to remove optimizations when using gcc
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O0")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O0")

add_executable(ThroughputExperiments main.cpp
        infinity/core/Context.cpp
        infinity/memory/Atomic.cpp
        infinity/memory/Buffer.cpp
        infinity/memory/Region.cpp
        infinity/memory/RegionToken.cpp
        infinity/memory/RegisteredMemory.cpp
        infinity/queues/QueuePair.cpp
        infinity/queues/QueuePairFactory.cpp
        infinity/requests/RequestToken.cpp
        infinity/utils/Address.cpp
        Server.cpp
        Server.h
        utils.cpp
        utils.h
        Client.cpp
        Client.h experiment_type.h)

include_directories(/usr/include .)

find_library(IBVERBS ibverbs /usr/lib)
target_link_libraries(ThroughputExperiments ${IBVERBS})
