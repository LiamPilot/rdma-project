cmake_minimum_required(VERSION 3.5.1)
project(RDMAHelloWorldC)

set(CMAKE_CXX_STANDARD 14)

add_executable(RDMAHelloWorldC main.cpp
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
        )

include_directories(/usr/include .)

find_library(IBVERBS ibverbs /usr/lib)
target_link_libraries(RDMAHelloWorldC ${IBVERBS})
