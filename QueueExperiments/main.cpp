#define GASNET_PAR

#include <string>
#include <chrono>
#include <thread>
#include <vector>
#include <cmath>
#include <memory>
#include <cstring>

#include <bcl/bcl.hpp>
#include <bcl/containers/FastQueue.hpp>
#include <bcl/containers/CircularQueue.hpp>
#include <sstream>

#include "utils.h"

constexpr int num_ops = 10000;
constexpr int queue_size = num_ops;
constexpr int num_vecs = 24;
static int barrier_num = 0;

void prints(const char message[]) {
    printf("[%s]: %s\n", BCL::hostname().c_str(), message);
}

void loud_barrier() {
    printf("[%s]: Entering %d\n", BCL::hostname().c_str(), barrier_num);
    BCL::barrier();
    printf("[%s]: Exiting %d\n", BCL::hostname().c_str(), barrier_num++);
}

void master(BCL::CircularQueue<int>& queue, BCL::CircularQueue<int>& control_queue, int buf_size) {
    prints("Running Master!");

    // Random int vectors to push
    std::vector<std::vector<int>> vs(num_vecs);
    for (int i = 0; i < num_vecs; i++) {
        vs[i] = utils::random_int_vector(buf_size);
    }

//    int data_buffer_size = buf_size * 24;
//    auto data = utils::GenerateRandomData(data_buffer_size);
//    std::string data_string(data.release(), data_buffer_size);
//    printf("%d : %d : %lu\n", buf_size, data_buffer_size, data_string.size());

    int num_pushes = std::floor((double) queue_size / (double) buf_size);
    loud_barrier();
    BCL::barrier_num = barrier_num;
    auto start = std::chrono::high_resolution_clock::now();
    printf("[%s]: Size before pushing: %lu\n", BCL::hostname().c_str(), queue.size());
    for (int i = 0; i < num_pushes; i++) {
        if (barrier_num == 6) {
            printf("[%s]: Trying to push when queue size is: %lu\n", BCL::hostname().c_str(), queue.size());
//            printf("[%s]: Args for push: string: %s, index: %d, buffer size: %d\n",
//                    BCL::hostname().c_str(), data.get() + ((i * buf_size) % data_buffer_size), (i * buf_size) % data_buffer_size, buf_size);
        }
        queue.push(vs[i % num_vecs]);
//        BCL::barrier();
    }
    loud_barrier();
    auto stop = std::chrono::high_resolution_clock::now();

    double duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    double throughput = (((double) (buf_size * sizeof(int) * num_pushes)) / duration) * 1e9;
    double latency = duration / (double) num_pushes;

    std::stringstream message;
    message << "PUSH latency: " << latency << "ns, throughput: " << throughput << " bytes/second";
    prints(message.str().c_str());
}

void worker(BCL::CircularQueue<int>& queue, BCL::CircularQueue<int>& control_queue, int buf_size) {
    prints("Running Worker!");
//    int x;
//    while (!control_queue.pop(x));

    prints("mallocing data");
    auto data = std::make_unique<int[]>(buf_size * num_vecs);
    std::vector<int> value(buf_size);

    loud_barrier();
    prints("starting");
    int num_pops = std::floor((double) queue_size / (double) buf_size);
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_pops; i++) {
        while (!queue.pop(value, buf_size)) {
//            prints("pop failed");
//            BCL::barrier();

        }
        std::memcpy(data.get() + ((i * buf_size) % num_vecs), value.data(), buf_size);
        value.clear();
    }
    loud_barrier();
    auto stop = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    double throughput = (((double) (buf_size * num_ops)) / duration) * 1e9;
    double latency = duration / (double) num_pops;

    std::stringstream message;
    message << "POP latency: " << latency << "ns, throughput: " << throughput << " bytes/second";

    prints(message.str().c_str());

}

int main() {
    BCL::init();

    srand48(BCL::rank());
    if (BCL::nprocs() < 2) {
        BCL::print("must run with at least 2 processes");
        exit(1);
    }

    BCL::CircularQueue<int> queue(0, queue_size);
    BCL::CircularQueue<int> control_queue(0, 1);

    loud_barrier();
//    printf("Hello, BCL! I am rank %lu/%lu on host %s.\n",
//           BCL::rank(), BCL::nprocs(), BCL::hostname().c_str());

    for (int buffer_size : utils::buffer_sizes) {
        if (BCL::rank() % 2 == 0) {
            master(queue, control_queue, buffer_size);
            if (buffer_size == utils::buffer_sizes[sizeof(utils::buffer_sizes) - 1]) loud_barrier();
        } else {
            if (buffer_size == utils::buffer_sizes[0]) loud_barrier();
            worker(queue, control_queue, buffer_size);
        }
    }


    BCL::finalize();
    return 0;
}
