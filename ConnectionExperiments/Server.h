//
// Created by liampilot on 22/05/2020.
//

#ifndef CONNECTIONEXPERIMENTS_SERVER_H
#define CONNECTIONEXPERIMENTS_SERVER_H


class Server {
public:
    virtual void run_throughput_tests(int data_size) = 0;
    virtual void run_latency_tests() = 0;
    virtual ~Server() = default;
};


#endif //CONNECTIONEXPERIMENTS_SERVER_H
