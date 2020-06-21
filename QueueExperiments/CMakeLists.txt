cmake_minimum_required(VERSION 3.11)
project(QueueExperiments)

set(CMAKE_CXX_STANDARD 17)
set(Boost_VERBOSE 1)

include_directories(BEFORE include /home/lp616/.local/include/)

#set(Boost_NO_BOOST_CMAKE ON)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_INCLUDEDIR /home/lp616/.local/include)
set(Boost_LIBRARYDIR /home/lp616/.local/lib)
find_package(Boost 1.73.0)
#if (Boost_FOUND)
message(STATUS "Boost Inc Dirs: ${Boost_INCLUDE_DIR}")
add_executable(BoostExperiments boost_tests/BoostTests.cpp utils.h utils.cpp)
include_directories(${Boost_INCLUDE_DIR})
target_link_libraries(BoostExperiments boost_graph boost_graph_parallel boost_mpi boost_regex boost_serialization pthread rt mpi stdc++)
#endif()

add_executable(BCLExperiments bcl_tests/BCLTests.cpp utils.h utils.cpp)
target_include_directories(BCLExperiments BEFORE PRIVATE /home/lp616/.local/include /home/lp616/.local/include/ibv-conduit lib)
target_link_libraries(BCLExperiments /home/lp616/.local/lib/libgasnet-ibv-seq.a ibverbs pthread rt mpi)

## see https://cmake.org/cmake/help/latest/module/FindBoost.html
#find_package(Boost REQUIRED)
#
