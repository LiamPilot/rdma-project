//
// Created by liampilot on 19/05/2020.
//

#include "RdmaClient.h"

#include <memory>
#include <vector>
#include <chrono>
#include <iostream>

#include <infinity/memory/Buffer.h>

std::vector<test_result> RdmaClient::run_read_experiments(int data_size) {
    auto pntr = (infinity::memory::RegionToken*) queue_pair->getUserData();
    auto remote_buffer_token = std::unique_ptr<infinity::memory::RegionToken>(pntr);

    std::vector<test_result> results;
    for (int buffer_size : utils::buffer_sizes) {
        double throughput = read_test(buffer_size, data_size, remote_buffer_token.get());
        test_result result {buffer_size, throughput};
        results.push_back(result);
    }

    return results;
}

std::vector<test_result> RdmaClient::run_write_experiments(int data_size) {
    char *data = utils::GenerateRandomData(data_size);
    auto *local_buffer = new infinity::memory::Buffer(&context, data, data_size * sizeof(char));
    auto pntr = (infinity::memory::RegionToken*) queue_pair->getUserData();
    auto remote_buffer_token = std::unique_ptr<infinity::memory::RegionToken>(pntr);

    std::vector<test_result> results;
    for (int buffer_size : utils::buffer_sizes) {
        double throughput = write_test(buffer_size, data_size, remote_buffer_token.get(), local_buffer);
        test_result result {buffer_size, throughput};
        results.push_back(result);
    }

    free(data);
    delete local_buffer;
    return results;
}

std::vector<test_result> RdmaClient::run_two_sided_experiments(int data_size) {
    auto pntr = (infinity::memory::RegionToken*) queue_pair->getUserData();
    auto remote_buffer_token = std::unique_ptr<infinity::memory::RegionToken>(pntr);

    std::vector<test_result> results;
    for (int buffer_size : utils::buffer_sizes) {
        double throughput = two_sided_test(buffer_size, data_size);
        test_result result {buffer_size, throughput};
        results.push_back(result);
    }

    return results;
}

double RdmaClient::read_test(int buffer_size, int data_size, infinity::memory::RegionToken *remote_buffer_token) {
    auto local_buffer = std::make_unique<infinity::memory::Buffer>(&context, data_size * sizeof(char));

    utils::print("Start Reading");

    auto op_flags = infinity::queues::OperationFlags();
    auto rem = data_size - (data_size % buffer_size);

    auto request_token = context.defaultRequestToken;
    auto start = std::chrono::high_resolution_clock::now();

    for (int offset = 0; offset < rem; offset += buffer_size) {
        queue_pair->read(local_buffer.get(),
                offset,
                remote_buffer_token,
                offset,
                buffer_size,
                op_flags,
                request_token);
        request_token->waitUntilCompleted();
    }
    queue_pair->read(local_buffer.get(),
             rem,
             remote_buffer_token,
             rem,
             data_size - rem,
             op_flags,
             request_token);
    request_token->waitUntilCompleted();

    auto stop = std::chrono::high_resolution_clock::now();

    utils::print("Finished Reading");
    auto time = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
    double throughput = ((double) data_size) / time.count();
    std::cout << throughput << '\n';
    return throughput;
}

double RdmaClient::write_test(int buffer_size, int data_size, infinity::memory::RegionToken *remote_buffer_token,
                              infinity::memory::Buffer *local_buffer) {
    utils::print("Start Writing");
    auto requestToken = context.defaultRequestToken;
    auto read_size = buffer_size * sizeof(char);
    auto op_flags = infinity::queues::OperationFlags();
    auto rem = data_size - (data_size % read_size);
    auto start = std::chrono::high_resolution_clock::now();

    for (int offset = 0; offset < rem; offset += buffer_size) {
        queue_pair->write(local_buffer,
                  offset,
                  remote_buffer_token,
                  offset,
                  read_size,
                  op_flags,
                  requestToken);
        requestToken->waitUntilCompleted();
    }
    queue_pair->write(local_buffer,
              rem,
              remote_buffer_token,
              rem,
              data_size - rem,
              op_flags,
              requestToken);
    requestToken->waitUntilCompleted();

    auto stop = std::chrono::high_resolution_clock::now();
    utils::print("Finished Reading");
    auto time = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
    double throughput = ((double) data_size) / time.count();
    return throughput;
}

double RdmaClient::two_sided_test(int buffer_size, int data_size) {
    std::cout << "Receiving " << data_size << " bytes in " << buffer_size << " byte chunks" << '\n';

    auto buffer = new infinity::memory::Buffer(&context, buffer_size);
    utils::print("Sending start signal");
    auto *control_buffer = new infinity::memory::Buffer(&context, 1);
    auto request_token = context.defaultRequestToken;
    queue_pair->send(control_buffer, request_token);
    request_token->waitUntilCompleted();

    int num_messages = (data_size / buffer_size) + 1;
    std::cout << "I expect to receive " << num_messages << " messages" << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    for (int x = 0; x < data_size; x += buffer_size) {
        context.postReceiveBuffer(buffer);
        auto *receive_elem = new infinity::core::receive_element_t;
        while (!context.receive(receive_elem));
    }
    auto stop = std::chrono::high_resolution_clock::now();

    utils::print("Received all messages, now sending finish signal");
    queue_pair->send(control_buffer, request_token);
    request_token->waitUntilCompleted();

    delete buffer;
    auto time = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
    std::cout << time.count() << '\n';
    double throughput = ((double) data_size) / time.count();
    std::cout << throughput << '\n';
    return throughput;
}
