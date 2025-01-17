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

RdmaClient::RdmaClient(std::string ip, const std::string& port)
: context(new infinity::core::Context()), qp_factory(context.get()), server_ip(std::move(ip)), server_port(std::stoi(port)) {}


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

void RdmaClient::wait_for_control_message() {
    infinity::memory::Buffer receive_buffer(context.get(), sizeof(char));
    context->postReceiveBuffer(&receive_buffer);
    infinity::core::receive_element_t receive_elem { .buffer = &receive_buffer };
    while (!context->receive(&receive_elem));
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

void RdmaClient::run_throughput_tests(size_t data_size) {
    std::ofstream read_results_file;
    read_results_file.open(read_throughput_file_name);
    auto read_tp_results = run_read_tp_tests(data_size);
    write_tp_results_to_file(read_tp_results, read_results_file);

    std::ofstream write_results_file;
    write_results_file.open(write_throughput_file_name);
    auto write_tp_results = run_write_tp_tests(data_size);
    write_tp_results_to_file(write_tp_results, write_results_file);

    std::ofstream two_sided_results_file;
    two_sided_results_file.open(two_sided_throughput_file_name);
    auto two_sided_tp_results = run_two_sided_tp_tests(data_size);
    write_tp_results_to_file(two_sided_tp_results, two_sided_results_file);
}

std::vector<utils::throughput_test_result> RdmaClient::run_read_tp_tests(size_t data_size) {
    std::cout << "-- RDMA Read Throughput\n";
    auto queue_pair = connect_to_remote_buffer();
//    auto queue_pair = qp_factory.connectToRemoteHost(server_ip.data(), server_port);
    auto remote_buffer_token = (infinity::memory::RegionToken*) queue_pair->getUserData();

    std::vector<utils::throughput_test_result> results;
    for (size_t buffer_size : utils::buffer_sizes) {
        double throughput = read_tp_test(buffer_size, data_size, remote_buffer_token, queue_pair);
        utils::throughput_test_result result {buffer_size, throughput};
        results.push_back(result);
    }


    send_control_message(queue_pair);

    return results;
}

double RdmaClient::read_tp_test(size_t buffer_size, size_t data_size, infinity::memory::RegionToken* remote_buffer_token,
                                std::unique_ptr<infinity::queues::QueuePair>& queue_pair) {
    std::cout << "RDMA Read Throughput for: " << buffer_size << "\n";
    infinity::memory::Buffer local_buffer(context.get(), data_size * sizeof(char));
    infinity::queues::OperationFlags op_flags;
    auto last_index = data_size - (data_size % buffer_size);
    auto request_token = context->defaultRequestToken;

    std::cout << "Start Reading\n";
    wait_for_control_message();
    auto start = std::chrono::high_resolution_clock::now();

    for (size_t offset = 0; offset < last_index; offset += buffer_size) {
        queue_pair->read(&local_buffer,
                         offset,
                         remote_buffer_token,
                         offset,
                         buffer_size,
                         op_flags,
                         request_token);
        request_token->waitUntilCompleted();
    }
    queue_pair->read(&local_buffer,
                     last_index,
                     remote_buffer_token,
                     last_index,
                     data_size % buffer_size,
                     op_flags,
                     request_token);
    request_token->waitUntilCompleted();


    send_control_message(queue_pair);
    auto stop = std::chrono::high_resolution_clock::now();

    std::cout << "Finished Reading\n";
    return utils::calculate_throughput(start, stop, data_size);
}

std::vector<utils::throughput_test_result> RdmaClient::run_write_tp_tests(size_t data_size) {
    std::cout << "-- RDMA Write Throughput\n";
    auto data = utils::GenerateRandomData(data_size);
    auto local_buffer = std::make_unique<infinity::memory::Buffer>(context.get(),
            data.data(), data_size * sizeof(char));

    auto queue_pair = connect_to_remote_buffer();
    auto remote_buffer_token = (infinity::memory::RegionToken*) queue_pair->getUserData();

    std::vector<utils::throughput_test_result> results;
    for (size_t buffer_size : utils::buffer_sizes) {
        double throughput = write_tp_test(buffer_size, data_size, local_buffer, remote_buffer_token, queue_pair);
        utils::throughput_test_result result {buffer_size, throughput};
        results.push_back(result);
        utils::dev_random_data(data.data(), data_size);
    }

    send_control_message(queue_pair);

    return results;
}

double
RdmaClient::write_tp_test(size_t buffer_size, size_t data_size, std::unique_ptr<infinity::memory::Buffer>& local_buffer,
                          infinity::memory::RegionToken* remote_buffer_token,
                          std::unique_ptr<infinity::queues::QueuePair>& queue_pair) {
    std::cout << "RDMA Write Throughput for: " << buffer_size << "\n";
    auto requestToken = context->defaultRequestToken;
    infinity::queues::OperationFlags op_flags;
    auto last_index = data_size - (data_size % buffer_size);

    wait_for_control_message();
    auto start = std::chrono::high_resolution_clock::now();

    for (size_t offset = 0; offset < last_index; offset += buffer_size) {
        queue_pair->write(local_buffer.get(),
                          offset,
                          remote_buffer_token,
                          offset,
                          buffer_size,
                          op_flags,
                          requestToken);
        requestToken->waitUntilCompleted();
    }
    queue_pair->write(local_buffer.get(),
                      last_index,
                      remote_buffer_token,
                      last_index,
                      data_size % buffer_size,
                      op_flags,
                      requestToken);
    requestToken->waitUntilCompleted();

    send_control_message(queue_pair);
    auto stop = std::chrono::high_resolution_clock::now();

    std::cout << "Finished Reading\n";
    return utils::calculate_throughput(start, stop, data_size);
}

std::vector<utils::throughput_test_result> RdmaClient::run_two_sided_tp_tests(size_t data_size) {
    std::cout << "-- RDMA Two Sided Throughput\n";
    auto queue_pair = connect_to_remote_buffer();

    std::vector<utils::throughput_test_result> results;
    for (size_t buffer_size : utils::buffer_sizes) {
        double throughput = two_sided_tp_test(buffer_size, data_size, queue_pair);
        utils::throughput_test_result result {buffer_size, throughput};
        results.push_back(result);
    }

    return results;
}

double RdmaClient::two_sided_tp_test(size_t buffer_size, size_t data_size, std::unique_ptr<infinity::queues::QueuePair>& queue_pair) {
    send_control_message(queue_pair);
    std::cout << "RDMA Two Sided Throughput for: " << buffer_size << "\n";
//    int num_messages = (data_size / buffer_size) + 1;
    infinity::memory::Buffer buffer(context.get(), buffer_size);
    auto start = std::chrono::high_resolution_clock::now();

    for (size_t x = 0; x < data_size; x += buffer_size) {
        context->postReceiveBuffer(&buffer);
        infinity::core::receive_element_t receive_elem {};
        receive_elem.buffer = &buffer;
        receive_elem.queuePair = queue_pair.get();
        while (!context->receive(&receive_elem));
    }


    send_control_message(queue_pair);
    auto stop = std::chrono::high_resolution_clock::now();

    return utils::calculate_throughput(start, stop, data_size);
}

// =============================
// =============================
// =============================
// =============================

void RdmaClient::run_latency_tests() {
    std::cout << "---- RDMA Latency\n";
    std::ofstream read_results_file;
    read_results_file.open(read_latency_file_name);
    auto read_latency_results = run_read_latency_tests();
    write_latency_results_to_file(read_latency_results, read_results_file);

    std::ofstream write_results_file;
    write_results_file.open(write_latency_file_name);
    auto write_latency_results = run_write_latency_tests();
    write_latency_results_to_file(write_latency_results, write_results_file);

    std::ofstream two_sided_results_file;
    two_sided_results_file.open(two_sided_latency_file_name);
    auto two_sided_latency_results = run_two_sided_latency_tests();
    write_latency_results_to_file(two_sided_latency_results, two_sided_results_file);
}

std::vector<utils::latency_test_result> RdmaClient::run_read_latency_tests() {
    std::cout << "-- RDMA Read Latency\n";
    auto queue_pair = connect_to_remote_buffer();
    auto remote_buffer_token = (infinity::memory::RegionToken*) queue_pair->getUserData();

    std::vector<utils::latency_test_result> results;
    for (size_t buffer_size : utils::latency_buffer_sizes) {
        double latency = read_latency_test(buffer_size, remote_buffer_token, queue_pair);
        utils::latency_test_result result {buffer_size, latency};
        results.push_back(result);
    }

    send_control_message(queue_pair);
    return results;
}

double
RdmaClient::read_latency_test(size_t buffer_size, infinity::memory::RegionToken* remote_buffer_token,
                              std::unique_ptr<infinity::queues::QueuePair>& queue_pair) {
    std::cout << "RDMA Read Latency for: " << buffer_size << "\n";
    infinity::requests::RequestToken* request_token = context->defaultRequestToken;
    infinity::memory::Buffer local_buffer(context.get(), buffer_size);

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < utils::num_loops; i++) {
        queue_pair->read(&local_buffer, remote_buffer_token, buffer_size, request_token);
        request_token->waitUntilCompleted();
//        memset(local_buffer.getData(), 0, buffer_size);
    }
    auto stop = std::chrono::high_resolution_clock::now();

    auto total = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
    double latency = ((double) total.count()) / (double) utils::num_loops;
    return latency;
}

std::vector<utils::latency_test_result> RdmaClient::run_write_latency_tests() {
    std::cout << "-- RDMA Write Latency\n";
    auto queue_pair = connect_to_remote_buffer();
    auto remote_buffer_token = (infinity::memory::RegionToken*) queue_pair->getUserData();

    std::vector<utils::latency_test_result> results;
    for (size_t buffer_size : utils::latency_buffer_sizes) {
        double latency = write_latency_test(buffer_size, remote_buffer_token, queue_pair);
        utils::latency_test_result result {buffer_size, latency};
        results.push_back(result);
    }

    send_control_message(queue_pair);
    return results;
}

double
RdmaClient::write_latency_test(size_t buffer_size, infinity::memory::RegionToken*  remote_buffer_token,
                               std::unique_ptr<infinity::queues::QueuePair>& queue_pair) {
    std::cout << "RDMA Write Latency for: " << buffer_size << "\n";

    infinity::queues::OperationFlags op_flags;
    auto data = utils::GenerateRandomData(buffer_size);
    infinity::memory::Buffer local_buffer(context.get(), data.data(), buffer_size * sizeof(char));
    auto request_token = context->defaultRequestToken;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < utils::num_loops; i++) {
        queue_pair->write(&local_buffer, remote_buffer_token, buffer_size, request_token);
        request_token->waitUntilCompleted();
//        memset(local_buffer.getData(), 0, buffer_size);
    }
    auto stop = std::chrono::high_resolution_clock::now();

    auto total = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
    double latency = ((double) total.count()) / (double) utils::num_loops;
    return latency;
}

std::vector<utils::latency_test_result> RdmaClient::run_two_sided_latency_tests() {
    std::cout << "-- RDMA Two Sided Latency\n";
    std::vector<utils::latency_test_result> results;

    auto queue_pair = connect_to_remote_buffer();

    for (size_t buffer_size : utils::latency_buffer_sizes) {
        double latency = two_sided_latency_test(buffer_size, queue_pair);
        utils::latency_test_result result {buffer_size, latency};
        results.push_back(result);
    }

    return results;
}

double RdmaClient::two_sided_latency_test(size_t buffer_size,
        std::unique_ptr<infinity::queues::QueuePair>& queue_pair) {
    std::cout << "RDMA Two Sided Latency for: " << buffer_size << "\n";

    send_control_message(queue_pair);
    auto request_token = context->defaultRequestToken;
    infinity::memory::Buffer buffer(context.get(), buffer_size);

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < utils::num_loops; i++) {
        queue_pair->send(&buffer, buffer_size, request_token);
        request_token->waitUntilCompleted();
    }
    send_control_message(queue_pair);
    auto stop = std::chrono::high_resolution_clock::now();

    auto total = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);

    return ((double) total.count()) / ((double) utils::num_loops);
}



