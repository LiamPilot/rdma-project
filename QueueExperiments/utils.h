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

namespace utils {
    constexpr int DATA_SIZE = 78 * 10000000;
    constexpr int NUM_TUPLES = 10000000;
    constexpr int PORT = 8001;
    constexpr int MAX_BUFFER_SIZE = 1024 << 15;

    constexpr int num_loops = 1000;

    const static int buffer_sizes[] =
    {
            1,
            4,
            16,
            32,
            32 << 1,
            32 << 2,
            32 << 3,
            32 << 4,
            32 << 5,
            32 << 6,
            32 << 7,
            32 << 8
    };


    struct throughput_test_result {
        int buffer_size;
        double throughput;
    };

    struct latency_test_result {
        int buffer_size;
        double latency;
    };

    std::unique_ptr<char[]> GenerateRandomData(int len);

    void random_data(char* s, int len);
    void dev_random_data(char* data, int size);
    std::vector<int> random_int_vector(int len);

    int get_ib_card_address(ifaddrs* address);

    template<typename Clock, typename Duration>
    double calculate_throughput(std::chrono::time_point<Clock, Duration> start, std::chrono::time_point<Clock, Duration> stop,
                                int data_size) {
        auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
        return (((double) data_size) / ((double) time.count())) * pow(10, 9);
    }
}

#endif //CONNECTIONEXPERIMENTS_UTILS_H
