//
// Created by liampilot on 19/05/2020.
//

#ifndef CONNECTIONEXPERIMENTS_RDMASERVER_H
#define CONNECTIONEXPERIMENTS_RDMASERVER_H

#include <memory>

#include <infinity/core/Context.h>
#include <infinity/queues/QueuePair.h>
#include <infinity/queues/QueuePairFactory.h>

#include "Server.h"

class RdmaServer : public Server {

public:
    RdmaServer(std::unique_ptr<infinity::core::Context> c, const std::string& port) {
        context = std::move(c);
        qp_factory = std::make_unique<infinity::queues::QueuePairFactory>(context.get());
        qp_factory->bindToPort(std::stoi(port));
    }

    void run_read_experiments(int data_size);

    void run_write_experiments(int data_size);

    void run_two_sided_experiments(int data_size);

    void run_throughput_tests() override;

    void run_latency_tests() override;

private:
    std::unique_ptr<infinity::core::Context> context;
    std::unique_ptr<infinity::queues::QueuePairFactory> qp_factory;

    void write_test(int data_size);

    void two_sided_test(int buffer_size, int data_size, char *data, infinity::queues::QueuePair* qp);

    void waitForControlMessage();

};


#endif //CONNECTIONEXPERIMENTS_RDMASERVER_H
