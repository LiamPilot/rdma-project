cmake_minimum_required(VERSION 3.11)
project(QueueExperiments)

set(CMAKE_CXX_STANDARD 17)


add_executable(QueueExperiments main.cpp BCLQueue.cpp BCLQueue.h TCPQueue.cpp TCPQueue.h utils/PaddedLong.h QueryBuffer.h)

set_property(TARGET QueueExperiments PROPERTY CXX_STANDARD 17)

include_directories(/home/lp616/.local/c++/9.3.1 /usr/include/mpi /home/lp616/.local/include /home/lp616/.local/include/ibv-conduit lib)

target_link_libraries(QueueExperiments /home/lp616/.local/lib/libgasnet-ibv-par.a ibverbs pthread rt mpi)