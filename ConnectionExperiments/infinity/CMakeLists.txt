add_library(Infinity
        core/Context.h
        core/Context.cpp
        core/Configuration.h
        memory/Atomic.h
        memory/Atomic.cpp
        memory/Buffer.h
        memory/Buffer.cpp
        memory/Region.h
        memory/Region.cpp
        memory/RegionToken.h
        memory/RegionToken.cpp
        memory/RegionType.h
        memory/RegisteredMemory.h
        memory/RegisteredMemory.cpp
        queues/QueuePair.h
        queues/QueuePair.cpp
        queues/QueuePairFactory.h
        queues/QueuePairFactory.cpp
        requests/RequestToken.h
        requests/RequestToken.cpp
        utils/Address.h
        utils/Address.cpp
        utils/Debug.h
        )

include_directories(/usr/include /usr/lib ../)
find_library(IBVERBS ibverbs /usr/lib)
target_link_libraries(Infinity ${IBVERBS})
