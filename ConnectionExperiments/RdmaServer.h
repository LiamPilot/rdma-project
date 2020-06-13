//
// Created by liampilot on 19/05/2020.
//

#ifndef CONNECTIONEXPERIMENTS_RDMASERVER_H
#define CONNECTIONEXPERIMENTS_RDMASERVER_H

#include <memory>

#include <infinity/core/Context.h>
#include <infinity/queues/QueuePair.h>
#include <infinity/queues/QueuePairFactory.h>
#include <vector>

#include "Server.h"

class RdmaServer : public Server {

public:
    explicit RdmaServer(const std::string& port);

    void run_throughput_tests(size_t data_size) override;

    void run_latency_tests() override;

    ~RdmaServer() override;

private:
    std::unique_ptr<infinity::core::Context> context;
    infinity::queues::QueuePairFactory qp_factory;

    void run_read_tp_tests(size_t data_size);

    void run_write_tp_tests(size_t data_size);

    void run_two_sided_tp_tests(size_t data_size);

    void two_sided_tp_test(size_t buffer_size, size_t data_size, std::vector<char> data,
            std::unique_ptr<infinity::queues::QueuePair>& qp);

    void run_read_latency_tests();

    void run_write_latency_tests();

    void run_two_sided_latency_tests();

    void wait_for_control_message();

    void two_sided_latency_test(size_t buffer_size, std::unique_ptr<infinity::queues::QueuePair>& queue_pair);

    void send_control_message(std::unique_ptr<infinity::queues::QueuePair>& queue_pair);
};


#endif //CONNECTIONEXPERIMENTS_RDMASERVER_H
