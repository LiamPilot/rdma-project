//
// Created by liampilot on 19/05/2020.
//

#include "RdmaClient.h"

#include <memory>
#include <utility>
#include <vector>
#include <chrono>
#include <iostream>
#include <numeric>
#include <fstream>
#include <infinity/memory/Buffer.h>

#include "utils.h"

RdmaClient::RdmaClient(std::unique_ptr<infinity::core::Context> c, std::string ip, const std::string& port)
: context(std::move(c)), qp_factory(context.get()), server_ip(std::move(ip)), server_port(std::stoi(port)) {}


RdmaClient::~RdmaClient() = default;

std::unique_ptr<infinity::queues::QueuePair> RdmaClient::connect_to_remote_buffer() {
    auto queue_pair = std::unique_ptr<infinity::queues::QueuePair>
            {qp_factory.connectToRemoteHost(server_ip.data(), server_port)};

    return queue_pair;
}

void RdmaClient::send_control_message(std::unique_ptr<infinity::queues::QueuePair>& queue_pair) {
    infinity::memory::Buffer control_buffer(context.get(), 1);
    auto request_token = context->defaultRequestToken;
    queue_pair->send(&control_buffer, request_token);
    request_token->waitUntilCompleted();
}

void RdmaClient::write_tp_results_to_file(const std::vector<utils::throughput_test_result>& results, std::ofstream& results_file) {
    for (auto result : results) {
        results_file << result.buffer_size << " " << result.throughput << std::endl;
    }
}

void RdmaClient::write_latency_results_to_file(const std::vector<utils::latency_test_result>& results, std::ofstream& results_file) {
    for (auto result : results) {
        results_file << result.buffer_size << " " << result.latency << std::endl;
    }
}

void RdmaClient::run_throughput_tests(int data_size) {
    std::ofstream read_results_file;
    read_results_file.open(read_throughput_file_name);
    auto read_tp_results = run_read_tp_tests(data_size);
    write_tp_results_to_file(read_tp_results, read_results_file);

    std::ofstream write_results_file;
    write_results_file.open(write_throughput_file_name);
    auto write_tp_results = run_write_tp_tests(data_size);
    write_tp_results_to_file(write_tp_results, write_results_file);

    std::ofstream two_sided_results_file;
    two_sided_results_file.open(two_sided_latency_file_name);
    auto two_sided_tp_results = run_two_sided_tp_tests(data_size);
    write_tp_results_to_file(two_sided_tp_results, two_sided_results_file);
}

std::vector<utils::throughput_test_result> RdmaClient::run_read_tp_tests(int data_size) {
    auto queue_pair = connect_to_remote_buffer();
    auto remote_buffer_token = std::unique_ptr<infinity::memory::RegionToken>
            {(infinity::memory::RegionToken*) queue_pair->getUserData()};

    std::vector<utils::throughput_test_result> results;
    for (int buffer_size : utils::buffer_sizes) {
        double throughput = read_tp_test(buffer_size, data_size, remote_buffer_token, queue_pair);
        utils::throughput_test_result result {buffer_size, throughput};
        results.push_back(result);
    }

    send_control_message(queue_pair);

    return results;
}

double RdmaClient::read_tp_test(int buffer_size, int data_size, std::unique_ptr<infinity::memory::RegionToken>& remote_buffer_token,
                                std::unique_ptr<infinity::queues::QueuePair>& queue_pair) {
    infinity::memory::Buffer local_buffer(context.get(), data_size * sizeof(char));
    infinity::queues::OperationFlags op_flags;
    auto last_index = data_size - (data_size % buffer_size);
    auto request_token = context->defaultRequestToken;

    std::cout << "Start Reading\n";
    auto start = std::chrono::high_resolution_clock::now();

    for (int offset = 0; offset < last_index; offset += buffer_size) {
        queue_pair->read(&local_buffer,
                         offset,
                         remote_buffer_token.get(),
                         offset,
                         buffer_size,
                         op_flags,
                         request_token);
        request_token->waitUntilCompleted();
    }
    queue_pair->read(&local_buffer,
                     last_index,
                     remote_buffer_token.get(),
                     last_index,
                     data_size % buffer_size,
                     op_flags,
                     request_token);
    request_token->waitUntilCompleted();

    auto stop = std::chrono::high_resolution_clock::now();

    std::cout << "Finished Reading\n";
    return utils::calculate_throughput(start, stop, data_size);
}

std::vector<utils::throughput_test_result> RdmaClient::run_write_tp_tests(int data_size) {
    auto data = utils::GenerateRandomData(data_size);
    auto local_buffer = std::make_unique<infinity::memory::Buffer>(context.get(), data.get(), data_size * sizeof(char));

    auto queue_pair = connect_to_remote_buffer();
    auto remote_buffer_token = std::unique_ptr<infinity::memory::RegionToken>
            {(infinity::memory::RegionToken*) queue_pair->getUserData()};

    std::vector<utils::throughput_test_result> results;
    for (int buffer_size : utils::buffer_sizes) {
        double throughput = write_tp_test(buffer_size, data_size, local_buffer, remote_buffer_token, queue_pair);
        utils::throughput_test_result result {buffer_size, throughput};
        results.push_back(result);
    }

    send_control_message(queue_pair);

    return results;
}

double
RdmaClient::write_tp_test(int buffer_size, int data_size, std::unique_ptr<infinity::memory::Buffer>& local_buffer,
                          std::unique_ptr<infinity::memory::RegionToken>& remote_buffer_token,
                          std::unique_ptr<infinity::queues::QueuePair>& queue_pair) {
    auto requestToken = context->defaultRequestToken;
    infinity::queues::OperationFlags op_flags;
    auto last_index = data_size - (data_size % buffer_size);
    auto start = std::chrono::high_resolution_clock::now();

    for (int offset = 0; offset < last_index; offset += buffer_size) {
        queue_pair->write(local_buffer.get(),
                          offset,
                          remote_buffer_token.get(),
                          offset,
                          buffer_size,
                          op_flags,
                          requestToken);
        requestToken->waitUntilCompleted();
    }
    queue_pair->write(local_buffer.get(),
                      last_index,
                      remote_buffer_token.get(),
                      last_index,
                      data_size % buffer_size,
                      op_flags,
                      requestToken);
    requestToken->waitUntilCompleted();

    auto stop = std::chrono::high_resolution_clock::now();
    std::cout << "Finished Reading\n";
    return utils::calculate_throughput(start, stop, data_size);
}

std::vector<utils::throughput_test_result> RdmaClient::run_two_sided_tp_tests(int data_size) {
    auto queue_pair = connect_to_remote_buffer();

    std::vector<utils::throughput_test_result> results;
    for (int buffer_size : utils::buffer_sizes) {
        double throughput = two_sided_tp_test(buffer_size, data_size, queue_pair);
        utils::throughput_test_result result {buffer_size, throughput};
        results.push_back(result);
    }

    return results;
}

double RdmaClient::two_sided_tp_test(int buffer_size, int data_size, std::unique_ptr<infinity::queues::QueuePair>& queue_pair) {
    send_control_message(queue_pair);

//    int num_messages = (data_size / buffer_size) + 1;
    infinity::memory::Buffer buffer(context.get(), buffer_size);
    auto start = std::chrono::high_resolution_clock::now();

    for (int x = 0; x < data_size; x += buffer_size) {
        context->postReceiveBuffer(&buffer);
        infinity::core::receive_element_t receive_elem {};
        receive_elem.buffer = &buffer;
        receive_elem.queuePair = queue_pair.get();
        while (!context->receive(&receive_elem));
    }

    auto stop = std::chrono::high_resolution_clock::now();

    send_control_message(queue_pair);

    return utils::calculate_throughput(start, stop, data_size);
}

// =============================
// =============================
// =============================
// =============================

void RdmaClient::run_latency_tests() {
    std::ofstream read_results_file;
    read_results_file.open(read_latency_file_name);
    auto read_latency_results = run_read_latency_tests();
    write_latency_results_to_file(read_latency_results, read_results_file);

    std::ofstream write_results_file;
    write_results_file.open(read_latency_file_name);
    auto write_latency_results = run_write_latency_tests();
    write_latency_results_to_file(write_latency_results, write_results_file);

    std::ofstream two_sided_results_file;
    two_sided_results_file.open(two_sided_latency_file_name);
    auto two_sided_latency_results = run_two_sided_latency_tests();
    write_latency_results_to_file(two_sided_latency_results, two_sided_results_file);
}

std::vector<utils::latency_test_result> RdmaClient::run_read_latency_tests() {
    auto queue_pair = connect_to_remote_buffer();
    auto remote_buffer_token = std::unique_ptr<infinity::memory::RegionToken>
            {(infinity::memory::RegionToken*) queue_pair->getUserData()};

    std::vector<utils::latency_test_result> results;
    for (int buffer_size : utils::buffer_sizes) {
        double latency = read_latency_test(buffer_size, remote_buffer_token, queue_pair);
        utils::latency_test_result result {buffer_size, latency};
        results.push_back(result);
    }

    send_control_message(queue_pair);
    return results;
}

double
RdmaClient::read_latency_test(int buffer_size, std::unique_ptr<infinity::memory::RegionToken>& remote_buffer_token,
                              std::unique_ptr<infinity::queues::QueuePair>& queue_pair) {
    std::vector<double> latencies;

    for (int i = 0; i < utils::num_loops; i++) {
        infinity::memory::Buffer local_buffer(context.get(), buffer_size * sizeof(char));
        infinity::requests::RequestToken* request_token = context->defaultRequestToken;

        auto start = std::chrono::high_resolution_clock::now();
        queue_pair->read(&local_buffer, remote_buffer_token.get(), request_token);
        request_token->waitUntilCompleted();
        auto stop = std::chrono::high_resolution_clock::now();
        auto latency = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        latencies.push_back(latency.count());
    }

    double sum = std::accumulate(latencies.begin(), latencies.end(), 0.0);
    return sum / utils::num_loops;
}

std::vector<utils::latency_test_result> RdmaClient::run_write_latency_tests() {
    auto queue_pair = connect_to_remote_buffer();
    auto remote_buffer_token = std::unique_ptr<infinity::memory::RegionToken>
            {(infinity::memory::RegionToken*) queue_pair->getUserData()};

    std::vector<utils::latency_test_result> results;
    for (int buffer_size : utils::buffer_sizes) {
        double latency = write_latency_test(buffer_size, remote_buffer_token, queue_pair);
        utils::latency_test_result result {buffer_size, latency};
        results.push_back(result);
    }

    return results;
}

double
RdmaClient::write_latency_test(int buffer_size, std::unique_ptr<infinity::memory::RegionToken>& remote_buffer_token,
                               std::unique_ptr<infinity::queues::QueuePair>& queue_pair) {
    std::vector<double> latencies;

    infinity::queues::OperationFlags op_flags;
    auto data = utils::GenerateRandomData(buffer_size);
    for (int i = 0; i < utils::num_loops; i++) {
        infinity::memory::Buffer local_buffer(context.get(), data.get(), buffer_size * sizeof(char));
        auto request_token = context->defaultRequestToken;

        auto start = std::chrono::high_resolution_clock::now();
        queue_pair->write(&local_buffer, remote_buffer_token.get(), request_token);
        request_token->waitUntilCompleted();
        auto stop = std::chrono::high_resolution_clock::now();

        auto latency = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        latencies.push_back(latency.count());
    }

    auto sum = std::accumulate(latencies.begin(), latencies.end(), 0.0);
    return sum / utils::num_loops;
}

std::vector<utils::latency_test_result> RdmaClient::run_two_sided_latency_tests() {
    std::vector<utils::latency_test_result> results;

    for (int buffer_size : utils::buffer_sizes) {
        double latency = two_sided_latency_test(buffer_size);
        utils::latency_test_result result {buffer_size, latency};
        results.push_back(result);
    }

    return results;
}

double RdmaClient::two_sided_latency_test(int buffer_size) {
    auto queue_pair = connect_to_remote_buffer();
    send_control_message(queue_pair);

    auto request_token = context->defaultRequestToken;
    std::vector<double> latencies;
    for (int i = 0; i < utils::num_loops; i++) {
        infinity::memory::Buffer buffer(context.get(), buffer_size);

        auto start = std::chrono::high_resolution_clock::now();
        queue_pair->send(&buffer, request_token);
        request_token->waitUntilCompleted();
        auto stop = std::chrono::high_resolution_clock::now();
        auto latency = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        latencies.push_back(latency.count());
    }

    auto start = std::chrono::high_resolution_clock::now();

    send_control_message(queue_pair);

    double sum = std::accumulate(latencies.begin(), latencies.end(), 0.0);
    return sum / utils::num_loops;
}



