//
// Created by liampilot on 05/05/2020.
//

#ifndef CONNECTIONEXPERIMENTS_UTILS_H
#define CONNECTIONEXPERIMENTS_UTILS_H

#include <string>
#include <memory>
#include <chrono>
#include <ifaddrs.h>
#include <random>
#include <cmath>
#include <infinity/memory/Buffer.h>
#include <infinity/core/Context.h>

namespace utils {
    constexpr int DATA_SIZE = 78 * 10000000;
    constexpr int NUM_TUPLES = 10000000;
    constexpr int PORT = 8001;
    constexpr int MAX_BUFFER_SIZE = 1024 << 15;

    constexpr int num_loops = 100000;

    const static size_t latency_buffer_sizes[] = {8,
                                                  16,
                                                  32,
                                                  64,
                                                  128,
                                                  256,
                                                  512,
                                                  1024,
                                                  1024ul << 1u,
                                                  1024ul << 2u,
                                                  1024ul << 3u,
                                                  1024ul << 4u,
                                                  1024ul << 5u,
                                                  1024ul << 6u,
                                                  1024ul << 7u,
                                                  1024ul << 8u,
                                                  1024ul << 9u};

    const static size_t buffer_sizes[] = {1024,
                                          1024ul << 1u,
                                          1024ul << 2u,
                                          1024ul << 3u,
                                          1024ul << 4u,
                                          1024ul << 5u,
                                          1024ul << 6u,
                                          1024ul << 7u,
                                          1024ul << 8u,
                                          1024ul << 9u,
                                          1024ul << 10u,
                                          1024ul << 11u,
                                          1024ul << 12u,
                                          1024ul << 13u,
                                          1024ul << 14u,
                                          1024ul << 15u};


    struct throughput_test_result {
        size_t buffer_size;
        double throughput;
    };

    struct latency_test_result {
        size_t buffer_size;
        double latency;
    };

    std::vector<char> GenerateRandomData(size_t len);

    void random_data(char* s, size_t len);
    void dev_random_data(char* data, size_t size);

    int get_ib_card_address(ifaddrs* address);

    template<typename Clock, typename Duration>
    double calculate_throughput(std::chrono::time_point<Clock, Duration> start, std::chrono::time_point<Clock, Duration> stop,
                                size_t data_size) {
        auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
        return (((double) data_size) / ((double) time.count())) * pow(10, 9);
    }
    std::unique_ptr<infinity::memory::Buffer> create_large_buffer(size_t data_size,
            std::unique_ptr<infinity::core::Context>& context);
}

#endif //CONNECTIONEXPERIMENTS_UTILS_H
