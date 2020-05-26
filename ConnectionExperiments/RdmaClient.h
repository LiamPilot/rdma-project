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
    RdmaClient(std::unique_ptr<infinity::core::Context> c, std::string  ip, const std::string& port);

    void run_throughput_tests(int data_size) override;

    void run_latency_tests() override;

    ~RdmaClient() override;

private:
    std::unique_ptr<infinity::core::Context> context;
    infinity::queues::QueuePairFactory qp_factory;
    int server_port;
    std::string server_ip;

    std::unique_ptr<infinity::queues::QueuePair> connect_to_remote_buffer();

    void send_control_message(std::unique_ptr<infinity::queues::QueuePair>& queue_pair);

    std::vector<utils::throughput_test_result> run_read_tp_tests(int data_size);

    double read_tp_test(int buffer_size, int data_size, std::unique_ptr<infinity::memory::RegionToken>& remote_buffer_token,
            std::unique_ptr<infinity::queues::QueuePair>& queue_pair);

    std::vector<utils::throughput_test_result> run_write_tp_tests(int data_size);

    double write_tp_test(int buffer_size, int data_size, std::unique_ptr<infinity::memory::Buffer>& local_buffer,
                         std::unique_ptr<infinity::memory::RegionToken>& remote_buffer_token,
                         std::unique_ptr<infinity::queues::QueuePair>& queue_pair);

    std::vector<utils::throughput_test_result> run_two_sided_tp_tests(int data_size);

    double two_sided_tp_test(int buffer_size, int data_size, std::unique_ptr<infinity::queues::QueuePair>& queue_pair);

    std::vector<utils::latency_test_result> run_read_latency_tests();

    double read_latency_test(int buffer_size, std::unique_ptr<infinity::memory::RegionToken>& remote_buffer_token,
                             std::unique_ptr<infinity::queues::QueuePair>& queue_pair);

    std::vector<utils::latency_test_result> run_write_latency_tests();

    double write_latency_test(int buffer_size, std::unique_ptr<infinity::memory::RegionToken>& remote_buffer_token,
                              std::unique_ptr<infinity::queues::QueuePair>& queue_pair);

    std::vector<utils::latency_test_result> run_two_sided_latency_tests();

    double two_sided_latency_test(int buffer_size);

    const char* read_throughput_file_name = "rdma_read_throughput.txt";
    const char* read_latency_file_name = "rdma_read_latency.txt";
    const char* write_throughput_file_name = "rdma_write_throughput.txt";
    const char* write_latency_file_name = "rdma_write_latency.txt";
    const char* two_sided_throughput_file_name = "rdma_two_sided_throughput.txt";
    const char* two_sided_latency_file_name = "rdma_two_sided_latency.txt";

    static void write_tp_results_to_file(const std::vector<utils::throughput_test_result>& results, std::ofstream& results_file);

    static void
    write_latency_results_to_file(const std::vector<utils::latency_test_result>& results, std::ofstream& results_file);
};


#endif //CONNECTIONEXPERIMENTS_RDMACLIENT_H
