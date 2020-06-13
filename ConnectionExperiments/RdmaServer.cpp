//
// Created by liampilot on 19/05/2020.
//

#include "RdmaServer.h"

#include <iostream>
#include <infinity/memory/Buffer.h>

#include "utils.h"

RdmaServer::RdmaServer(const std::string& port)
: context(new infinity::core::Context()), qp_factory(context.get()) {
    qp_factory.bindToPort(std::stoi(port));
}

RdmaServer::~RdmaServer() = default;

void RdmaServer::wait_for_control_message() {
    infinity::memory::Buffer receive_buffer(context.get(), sizeof(char));
    context->postReceiveBuffer(&receive_buffer);
    infinity::core::receive_element_t receive_elem { .buffer = &receive_buffer };
    while (!context->receive(&receive_elem));
}

void RdmaServer::send_control_message(std::unique_ptr<infinity::queues::QueuePair>& queue_pair) {
    infinity::memory::Buffer control_buffer(context.get(), 1);
    auto request_token = context->defaultRequestToken;
    queue_pair->send(&control_buffer, request_token);
    request_token->waitUntilCompleted();
}

void RdmaServer::run_throughput_tests(size_t data_size) {
    std::cout << "Throughput tests ready\n";
    run_read_tp_tests(data_size);
    run_write_tp_tests(data_size);
    run_two_sided_tp_tests(data_size);
}

void RdmaServer::run_read_tp_tests(size_t data_size) {
    std::vector<char> data(data_size);

    infinity::memory::Buffer data_buffer(context.get(), data.data(), data_size * sizeof(char));
    auto buffer_token = std::unique_ptr<infinity::memory::RegionToken> {data_buffer.createRegionToken()};

    std::cout << "Ready to connect\n";
    auto qp = std::unique_ptr<infinity::queues::QueuePair>
            {qp_factory.acceptIncomingConnection(buffer_token.get(), sizeof(infinity::memory::RegionToken))};

    for (size_t unused : utils::buffer_sizes) {
        utils::dev_random_data(data.data(), data_size);
        send_control_message(qp);
        wait_for_control_message();
        std::cout << "refilling data\n";
    }

    wait_for_control_message();

    std::cout << "Done, cleaning up\n";
}

void RdmaServer::run_write_tp_tests(size_t data_size) {
    infinity::memory::Buffer buffer(context.get(), data_size * sizeof(char));
    auto buffer_token = std::unique_ptr<infinity::memory::RegionToken> {buffer.createRegionToken()};

    auto qp =  std::unique_ptr<infinity::queues::QueuePair>
            {qp_factory.acceptIncomingConnection(buffer_token.get(), sizeof(infinity::memory::RegionToken))};

    for (size_t unused : utils::buffer_sizes) {
        memset(buffer.getData(), 0, data_size);
        send_control_message(qp);
        wait_for_control_message();
        std::cout << "refilling data\n";
    }

    wait_for_control_message();
}

void RdmaServer::run_two_sided_tp_tests(size_t data_size) {
    std::cout << "Connecting to client\n";
    infinity::memory::Buffer buffer(context.get(), sizeof(char));
    auto buffer_token = std::unique_ptr<infinity::memory::RegionToken> {buffer.createRegionToken()};
    auto qp =  std::unique_ptr<infinity::queues::QueuePair>
            {qp_factory.acceptIncomingConnection(buffer_token.get(), sizeof(infinity::memory::RegionToken))};

    std::cout << "generating data\n";
    std::vector<char> data = utils::GenerateRandomData(data_size);

    std::cout << "Doing two sided test\n";
    for (size_t buffer_size : utils::buffer_sizes) {
        two_sided_tp_test(buffer_size, data_size, data, qp);
    }
}

void RdmaServer::two_sided_tp_test(size_t buffer_size, size_t data_size, std::vector<char> data,
        std::unique_ptr<infinity::queues::QueuePair>& qp) {
    std::cout << "Sending " << data_size << " bytes in " << buffer_size << " chunks" << '\n';
    infinity::memory::Buffer buffer(context.get(), data.data(), data_size * sizeof(char));

    std::cout << "Waiting for client to be ready\n";
    wait_for_control_message();

    auto msg = data_size / buffer_size + 1;
    std::cout << "I will send " << msg << " messages" << '\n';

    auto requestToken = context->defaultRequestToken;
    auto op_flags = infinity::queues::OperationFlags();
    size_t last_index = data_size - (data_size % buffer_size);

    for (size_t offset = 0; offset < last_index; offset += buffer_size) {
        qp->send(&buffer,
                 offset,
                 buffer_size,
                 op_flags,
                 requestToken);
        requestToken->waitUntilCompleted();
    }
    // Send final chunks
    qp->send(&buffer,
             last_index,
             data_size % buffer_size,
             op_flags,
             requestToken);
    requestToken->waitUntilCompleted();

    std::cout << "Done sending, waiting for confirmation from client\n";
    wait_for_control_message();
}

void RdmaServer::run_latency_tests() {
    run_read_latency_tests();
    run_write_latency_tests();
    run_two_sided_latency_tests();
}

void RdmaServer::run_read_latency_tests() {
    std::vector<char> data = utils::GenerateRandomData(utils::MAX_BUFFER_SIZE);

    infinity::memory::Buffer data_buffer(context.get(), data.data(), utils::MAX_BUFFER_SIZE * sizeof(char));
    auto buffer_token = std::unique_ptr<infinity::memory::RegionToken> {data_buffer.createRegionToken()};

    std::cout << "Ready to connect\n";
    auto qp = std::unique_ptr<infinity::queues::QueuePair>
            {qp_factory.acceptIncomingConnection(buffer_token.get(), sizeof(infinity::memory::RegionToken))};

    wait_for_control_message();

    std::cout << "Done, cleaning up\n";
}

void RdmaServer::run_write_latency_tests() {
    infinity::memory::Buffer data_buffer(context.get(), utils::MAX_BUFFER_SIZE * sizeof(char));
    auto buffer_token = std::unique_ptr<infinity::memory::RegionToken> {data_buffer.createRegionToken()};

    std::cout << "Ready to connect\n";
    auto qp = std::unique_ptr<infinity::queues::QueuePair>
            {qp_factory.acceptIncomingConnection(buffer_token.get(), sizeof(infinity::memory::RegionToken))};

    wait_for_control_message();
}


void RdmaServer::run_two_sided_latency_tests() {
    infinity::memory::Buffer data_buffer(context.get(), utils::MAX_BUFFER_SIZE * sizeof(char));
    auto buffer_token = std::unique_ptr<infinity::memory::RegionToken> {data_buffer.createRegionToken()};

    std::cout << "Ready to connect\n";
    auto qp = std::unique_ptr<infinity::queues::QueuePair>
            {qp_factory.acceptIncomingConnection(buffer_token.get(), sizeof(infinity::memory::RegionToken))};

    for (size_t buffer_size : utils::latency_buffer_sizes) {
        two_sided_latency_test(buffer_size, qp);
    }
}

void RdmaServer::two_sided_latency_test(size_t buffer_size, std::unique_ptr<infinity::queues::QueuePair>& queue_pair) {

    wait_for_control_message();
    infinity::memory::Buffer buffer(context.get(), buffer_size);
    infinity::core::receive_element_t receive_elem {};
    receive_elem.buffer = &buffer;
    receive_elem.queuePair = queue_pair.get();

    for (int i = 0; i < utils::num_loops; i++) {
        context->postReceiveBuffer(&buffer);
        while (!context->receive(&receive_elem));
        if (receive_elem.bytesWritten < buffer_size) {
            std::cout << "Not all bytes received from a send" << std::endl;
        }
    }

    wait_for_control_message();
}

