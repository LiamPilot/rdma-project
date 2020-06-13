#define GASNET_PAR

#include <string>
#include <chrono>
#include <thread>
#include <vector>
#include <cmath>
#include <memory>
#include <cstring>

#include <bcl/bcl.hpp>
#include <bcl/containers/CircularQueue.hpp>
#include <sstream>

#include "../utils.h"

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

void master(BCL::CircularQueue<int>& queue, int buf_size) {
//    prints("Running Master!");

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
    BCL::barrier();
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_pushes; i++) {
        queue.push(vs[i % num_vecs]);
    }
    BCL::barrier();
    auto stop = std::chrono::high_resolution_clock::now();

    double duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    double throughput = (((double) (buf_size * sizeof(int) * num_pushes)) / duration) * 1e9;
    double latency = duration / (double) num_pushes;

    std::stringstream message;
    message << "PUSH | size: " << buf_size * sizeof(int) << " bytes, "
            << "latency: " << latency << "ns, "
            << "throughput: " << throughput << " bytes/second";
    prints(message.str().c_str());
}

void worker(BCL::CircularQueue<int>& queue, int buf_size) {
//    prints("Running Worker!");

    auto data = std::make_unique<int[]>(buf_size * num_vecs);
    std::vector<int> value(buf_size);

    int num_pops = std::floor((double) queue_size / (double) buf_size);
    BCL::barrier();
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_pops; i++) {
        while (!queue.pop(value, buf_size)) {
        }
        std::memcpy(data.get() + ((i * buf_size) % num_vecs), value.data(), buf_size);
        value.clear();
    }
    BCL::barrier();
    auto stop = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    double throughput = (((double) (buf_size * sizeof(int) * num_pops)) / duration) * 1e9;
    double latency = duration / (double) num_pops;

    std::stringstream message;
    message << "POP  | size: " << buf_size * sizeof(int) << " bytes, "
            << "latency: " << latency << "ns, "
            << "throughput: " << throughput << " bytes/second";
    prints(message.str().c_str());

}

int main() {
    BCL::init();

    srand48(BCL::rank());
    if (BCL::nprocs() < 2) {
        BCL::print("must run with at least 2 processes");
        exit(1);
    }

    BCL::CircularQueue<int> queue(1, queue_size);

    // warmup
    srand48(BCL::rank());
    for (int i = 0; i < queue_size / 2; i++) {
        queue.push(lrand48());
    }
    int x;
    for (int i = 0; i < queue_size / 2; i++) {
        queue.pop(x);
    }

    prints(std::to_string(queue.size()).c_str());

    BCL::barrier();

    for (int buffer_size : utils::buffer_sizes) {
        if (BCL::rank() % 2 == 0) {
            master(queue, buffer_size);
            if (buffer_size == utils::buffer_sizes[sizeof(utils::buffer_sizes) - 1]) BCL::barrier();
        } else {
            if (buffer_size == utils::buffer_sizes[0]) BCL::barrier();
            worker(queue, buffer_size);
        }
    }

    prints("Done, cleaning up");

    BCL::finalize();
    return 0;
}
