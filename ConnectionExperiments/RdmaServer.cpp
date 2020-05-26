//
// Created by liampilot on 19/05/2020.
//

#include "RdmaServer.h"

#include <iostream>

#include <infinity/memory/Buffer.h>

#include <utils.h>

RdmaServer::RdmaServer(std::unique_ptr<infinity::core::Context> c, const std::string& port)
: qp_factory(c.get()), context(std::move(c)) {
    qp_factory.bindToPort(std::stoi(port));
}

void RdmaServer::wait_for_control_message() {
    infinity::memory::Buffer receive_buffer(context.get(), sizeof(char));
    context->postReceiveBuffer(&receive_buffer);
    infinity::core::receive_element_t receive_elem { .buffer = &receive_buffer };
    while (!context->receive(&receive_elem));
}

void RdmaServer::run_throughput_tests(int data_size) {

}

void RdmaServer::run_read_tp_tests(int data_size) {
    std::unique_ptr<char[]> data = utils::GenerateRandomData(data_size);

    infinity::memory::Buffer data_buffer(context.get(), data.get(), data_size * sizeof(char));
    auto buffer_token = std::unique_ptr<infinity::memory::RegionToken> {data_buffer.createRegionToken()};

    std::cout << "Ready to connect\n";
    auto qp = std::unique_ptr<infinity::queues::QueuePair>
            {qp_factory.acceptIncomingConnection(buffer_token.get(), sizeof(infinity::memory::RegionToken))};

    wait_for_control_message();

    std::cout << "Done, cleaning up\n";
}

void RdmaServer::run_write_tp_tests(int data_size) {
    infinity::memory::Buffer buffer(context.get(), data_size * sizeof(char));
    auto buffer_token = std::unique_ptr<infinity::memory::RegionToken> {buffer.createRegionToken()};

    auto qp =  std::unique_ptr<infinity::queues::QueuePair>
            {qp_factory.acceptIncomingConnection(buffer_token.get(), sizeof(infinity::memory::RegionToken))};

    wait_for_control_message();
}

void RdmaServer::run_two_sided_tp_tests(int data_size) {
    std::cout << "Connecting to client\n";
    infinity::memory::Buffer buffer(context.get(), sizeof(char));
    auto buffer_token = std::unique_ptr<infinity::memory::RegionToken> {buffer.createRegionToken()};
    auto qp =  std::unique_ptr<infinity::queues::QueuePair>
            {qp_factory.acceptIncomingConnection(buffer_token.get(), sizeof(infinity::memory::RegionToken))};

    std::cout << "generating data\n";
    std::unique_ptr<char[]> data = utils::GenerateRandomData(data_size);

    std::cout << "Doing two sided test\n";
    for (int buffer_size : utils::buffer_sizes) {
        two_sided_tp_test(buffer_size, data_size, data, qp);
    }
}

void RdmaServer::two_sided_tp_test(int buffer_size, int data_size, std::unique_ptr<char[]>& data,
        std::unique_ptr<infinity::queues::QueuePair>& qp) {
    std::cout << "Sending " << data_size << " bytes in " << buffer_size << " chunks" << '\n';
    infinity::memory::Buffer buffer(context.get(), data.get(), data_size * sizeof(char));

    std::cout << "Waiting for client to be ready\n";
    wait_for_control_message();

    auto msg = data_size / buffer_size + 1;
    std::cout << "I will send " << msg << " messages" << '\n';

    auto requestToken = context->defaultRequestToken;
    auto op_flags = infinity::queues::OperationFlags();
    int last_index = data_size - (data_size % buffer_size);

    for (int offset = 0; offset < last_index; offset += buffer_size) {
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
    std::unique_ptr<char[]> data = utils::GenerateRandomData(utils::MAX_BUFFER_SIZE);

    infinity::memory::Buffer data_buffer(context.get(), data.get(), utils::MAX_BUFFER_SIZE * sizeof(char));
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
    for (int buffer_size : utils::buffer_sizes) {
        two_sided_latency_test(buffer_size);
    }
}

void RdmaServer::two_sided_latency_test(int buffer_size) {
    infinity::memory::Buffer data_buffer(context.get(), buffer_size * sizeof(char));
    auto buffer_token = std::unique_ptr<infinity::memory::RegionToken> {data_buffer.createRegionToken()};
    auto qp = std::unique_ptr<infinity::queues::QueuePair>
            {qp_factory.acceptIncomingConnection(buffer_token.get(), sizeof(infinity::memory::RegionToken))};

    wait_for_control_message();

    for (int i = 0; i < utils::num_loops; i++) {
        infinity::memory::Buffer buffer(context.get(), buffer_size);
        context->postReceiveBuffer(&buffer);
        infinity::core::receive_element_t receive_elem { .buffer = &buffer, .queuePair = qp.get() };
        while (!context->receive(&receive_elem));
        if (receive_elem.bytesWritten < buffer_size) {
            std::cout << "Not all bytes received from a send" << std::endl;
        }
    }

    wait_for_control_message();
}
