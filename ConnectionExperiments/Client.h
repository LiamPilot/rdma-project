//
// Created by liampilot on 22/05/2020.
//

#ifndef CONNECTIONEXPERIMENTS_CLIENT_H
#define CONNECTIONEXPERIMENTS_CLIENT_H

#include <cstddef>

class Client {
public:
    virtual void run_throughput_tests(size_t data_size) = 0;
    virtual void run_latency_tests() = 0;
    virtual ~Client() = default;
};


#endif //CONNECTIONEXPERIMENTS_CLIENT_H
