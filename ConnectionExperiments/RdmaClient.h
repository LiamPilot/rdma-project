//
// Created by liampilot on 19/05/2020.
//

#ifndef CONNECTIONEXPERIMENTS_RDMACLIENT_H
#define CONNECTIONEXPERIMENTS_RDMACLIENT_H

#include <memory>
#include <vector>

#include <infinity/core/Context.h>
#include <infinity/queues/QueuePair.h>
#include <infinity/queues/QueuePairFactory.h>

#include <utils.h>
#include "Client.h"

class RdmaClient : public Client{

public:
    RdmaClient(std::unique_ptr<infinity::core::Context> c, const std::string& server_ip, const std::string& port);

    void run_throughput_tests() override;

    void run_latency_tests() override;

private:
    std::unique_ptr<infinity::core::Context> context;
    std::unique_ptr<infinity::queues::QueuePair> queue_pair;

    double read_tp_test(int buffer_size, int data_size, std::unique_ptr<infinity::memory::RegionToken>& remote_buffer_token);

    double write_tp_test(int buffer_size, int data_size, std::unique_ptr<infinity::memory::RegionToken>& remote_buffer_token,
                         std::unique_ptr<infinity::memory::Buffer>& local_buffer);

    double two_sided_tp_test(int buffer_size, int data_size);

    std::vector<utils::throughput_test_result> run_read_tp_tests(int data_size);

    std::vector<utils::throughput_test_result> run_write_tp_tests(int data_size);

    std::vector<utils::throughput_test_result> run_two_sided_tp_tests(int data_size);

    double read_latency_test(int buffer_size, std::unique_ptr<infinity::memory::RegionToken>& remote_buffer_token);

    double write_latency_test(int buffer_size, std::unique_ptr<infinity::memory::RegionToken>& remote_buffer_token);

    double two_sided_latency_test(int buffer_size);

    std::vector<utils::latency_test_result> run_read_latency_tests();

    std::vector<utils::latency_test_result> run_write_latency_tests();

    std::vector<utils::latency_test_result> run_two_sided_latency_tests();
};


#endif //CONNECTIONEXPERIMENTS_RDMACLIENT_H
