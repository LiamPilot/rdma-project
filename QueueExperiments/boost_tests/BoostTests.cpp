//
// Created by liampilot on 06/06/2020.
//


//#define DEV

#include <unistd.h>
#include <deque>
#include <queue>

#ifdef DEV
#include "../include/boost/graph/use_mpi.hpp"
#include "../include/boost/mpi.hpp"
#include "../include/boost/graph/distributed/mpi_process_group.hpp"
#include "../include/boost/graph/distributed/queue.hpp"
#include "../include/boost/pending/queue.hpp"
#include "../include/boost/container/pmr/deque.hpp"
#include "../include/boost/property_map/property_map.hpp"
#else
#include <boost/graph/use_mpi.hpp>
#include <boost/mpi.hpp>
#include <boost/graph/distributed/mpi_process_group.hpp>
#include <boost/graph/distributed/queue.hpp>
#include <boost/pending/queue.hpp>
#include <boost/property_map/property_map.hpp>
#endif

#include <iostream>

using namespace boost::graph::distributed;
using boost::graph::distributed::mpi_process_group;

struct global_value
{
    global_value(int p = -1, std::size_t l = 0) : processor(p), value(l) {}

    int processor;
    std::size_t value;

    template<typename Archiver>
    void serialize(Archiver& ar, const unsigned int /*version*/)
    {
        ar & processor & value;
    }
};

struct global_value_owner_map
{
    typedef int value_type;
    typedef value_type reference;
    typedef global_value key_type;
    typedef boost::readable_property_map_tag category;
};

int get(global_value_owner_map, global_value k)
{
    return k.processor;
}

std::string hostname() {
    constexpr size_t BUF_SIZE = 2048;
    char buf[BUF_SIZE + 1];
    gethostname(buf, BUF_SIZE);
    return std::string(buf);
}

int main(int argc, char* argv[]) {
    boost::mpi::environment env(argc, argv);

    mpi_process_group pg;

//    typedef boost::queue<global_value> local_queue;

//    typedef distributed_queue<mpi_process_group, global_value_owner_map, local_queue> distributed_queue;

    mpi_process_group::process_id_type id = process_id(pg);

    mpi_process_group::process_id_type n = num_processes(pg);

    std::cout << "Host: " << hostname() << ", " << "ID: " << id << ", N:" << n << "\n";

    distributed_queue<mpi_process_group, global_value_owner_map, boost::queue<global_value>> q(pg, global_value_owner_map());

    synchronize(pg);

    global_value v(0, 0);

    if (id == 0) {
        std::cout << "[" << hostname() << "]: Wrote " << v.value << "\n";
        q.push(v);
    }

    synchronize(pg);

    std::cout << "[" << hostname() << "]: check in\n";

    synchronize(pg);

    int size = q.size();
    std::cout << "[" << hostname() << "]: queue size " << size << "\n";

    while (!q.empty()) {
        if (id == 1) {
            std::cout << "[" << hostname() << "]: About to Read " << v.value << "\n";
            v = q.top();
            q.pop();
            std::cout << "[" << hostname() << "]: Read " << v.value << "\n";
        }
    }

    return 0;
}
