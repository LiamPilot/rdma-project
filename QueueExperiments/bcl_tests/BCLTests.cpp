#define GASNET_SEQ

#include <string>
#include <chrono>
#include <thread>
#include <vector>
#include <cmath>
#include <memory>
#include <cstring>
#include <fstream>
#include <sstream>

#include <bcl/bcl.hpp>
#include <bcl/containers/CircularQueue.hpp>

#include "../utils.h"

constexpr int num_ops = 100000;
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

void master(BCL::CircularQueue<int>& queue, int buf_size, std::ofstream& results_file) {
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
    double ops = (num_pushes / duration) * 1e9;

    std::stringstream message;
    message << "PUSH | size: " << buf_size * sizeof(int) << " bytes, "
            << "latency: " << latency << "ns, "
            << "throughput: " << throughput << " bytes/second, "
            << "ops/s: " << ops << " operations/second";
    prints(message.str().c_str());
    results_file << message.str() << '\n';
}

void worker(BCL::CircularQueue<int>& queue, int buf_size, std::ofstream& results_file) {
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
    double ops = (num_pops / duration) * 1e9;
    double latency = duration / (double) num_pops;

    std::stringstream message;
    message << "POP  | size: " << buf_size * sizeof(int) << " bytes, "
            << "latency: " << latency << "ns, "
            << "throughput: " << throughput << " bytes/second, "
            << "ops/s: " << ops << " operations/second";
    prints(message.str().c_str());
    results_file << message.str() << '\n';
}

int main() {
    BCL::init();

    if (BCL::nprocs() < 2) {
        BCL::print("must run with at least 2 processes");
        exit(1);
    }

    BCL::CircularQueue<int> queue(1, queue_size);
    BCL::barrier();

    srand48(BCL::rank());
    for (int i = 0; i < 5; i++) {
        queue.push(BCL::rank());
        std::cout << BCL::rank() << ": Pushed to queue, i = " << i << "\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    BCL::barrier();

    int x;
    for (int i = 0; i < 5; i++) {
        queue.pop(x);
        std::cout << BCL::rank() << ": Popped " << x << " from queue, i = " << i << "\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    BCL::barrier();

//    prints(std::to_string(queue.size()).c_str());
//    std::ofstream push_results;
//    push_results.open("mpi_ib_remote_push.txt");
//    std::ofstream pop_results;
//    pop_results.open("mpi_ib_local_pop.txt");
//
//    BCL::barrier();
//
//    for (int buffer_size : utils::buffer_sizes) {
//        if (BCL::rank() % 2 == 0) {
//            master(queue, buffer_size, push_results);
//            if (buffer_size == utils::buffer_sizes[sizeof(utils::buffer_sizes) - 1]) BCL::barrier();
//        } else {
//            if (buffer_size == utils::buffer_sizes[0]) BCL::barrier();
//            worker(queue, buffer_size, pop_results);
//        }
//    }
//
//    prints("Done, cleaning up");
//    push_results.close();
//    pop_results.close();
    BCL::finalize();
    return 0;
}
