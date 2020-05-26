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
    RdmaServer(std::unique_ptr<infinity::core::Context> c, const std::string& port);

    void run_throughput_tests(int data_size) override;

    void run_latency_tests() override;

private:
    std::unique_ptr<infinity::core::Context> context;
    infinity::queues::QueuePairFactory qp_factory;

    void run_read_tp_tests(int data_size);

    void run_write_tp_tests(int data_size);

    void run_two_sided_tp_tests(int data_size);

    void two_sided_tp_test(int buffer_size, int data_size, std::unique_ptr<char[]>& data, std::unique_ptr<infinity::queues::QueuePair>& qp);

    void run_read_latency_tests();

    void run_write_latency_tests();

    void run_two_sided_latency_tests();

    void wait_for_control_message();

    void two_sided_latency_test(int buffer_size);
};


#endif //CONNECTIONEXPERIMENTS_RDMASERVER_H
