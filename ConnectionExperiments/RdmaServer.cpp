//
// Created by liampilot on 19/05/2020.
//

#include "RdmaServer.h"

#include <iostream>

#include <infinity/memory/Buffer.h>

#include <utils.h>

void RdmaServer::run_read_experiments(int data_size) {
    std::cout << "Creating random data\n";
    char *data = utils::GenerateRandomData(data_size);

    std::cout << "Creating Buffer with Data\n";
    auto data_buffer = new infinity::memory::Buffer(&context, data, data_size * sizeof(char));
    auto buffer_token = data_buffer->createRegionToken();
    auto qp = qp_factory->acceptIncomingConnection(buffer_token, sizeof(infinity::memory::RegionToken));

    std::cout << "Waiting for client to finish\n";
    waitForControlMessage();

    std::cout << "Done, cleaning up\n";
    free(data);
    delete data_buffer;
    delete qp;
}

void RdmaServer::run_write_experiments(int data_size) {
    for (int buffer_size : utils::buffer_sizes) {
        write_test(data_size);
    }

}

void RdmaServer::run_two_sided_experiments(int data_size) {
    std::cout << "Connecting to client\n";
    auto buffer = new infinity::memory::Buffer(&context, sizeof(char));
    auto buffer_token = buffer->createRegionToken();
    auto qp = qp_factory->acceptIncomingConnection(buffer_token, sizeof(infinity::memory::RegionToken));

    std::cout << "generating data\n";
    char *data = utils::GenerateRandomData(tuple_size * num_tuples);

    std::cout << "Doing two sided test\n";
    for (int buffer_size : utils::buffer_sizes) {
        two_sided_test(buffer_size, data_size, data, qp);
    }

    free(data);
    delete qp;
    delete buffer;
    delete buffer_token;
}

void RdmaServer::write_test(int data_size) {
    auto data_buffer = new infinity::memory::Buffer(&context, data_size * sizeof(char));
    auto buffer_token = data_buffer->createRegionToken();

    std::cout << "Ready for test\n";
    auto qp = qp_factory->acceptIncomingConnection(buffer_token, sizeof(infinity::memory::RegionToken));

    std::cout << "Waiting for client to finish\n";
    waitForControlMessage();

    delete data_buffer;
    delete qp;
}

void RdmaServer::two_sided_test(int buffer_size, int data_size, char *data, infinity::queues::QueuePair *qp) {
    std::cout << "Sending " << data_size << " bytes in " << buffer_size << " chunks" << '\n';
    auto data_buffer = new infinity::memory::Buffer(&context, data, data_size * sizeof(char));

    std::cout << "Waiting for client to be ready\n";
    waitForControlMessage();

    auto requestToken = context.defaultRequestToken;
    auto op_flags = infinity::queues::OperationFlags();

    auto msg = data_size / buffer_size + 1;
    int final = data_size - (data_size % buffer_size);
    std::cout << "I will send " << msg << " messages" << '\n';
    for (int offset = 0; offset < final; offset += buffer_size) {
        qp->send(data_buffer,
                 offset,
                 buffer_size,
                 op_flags,
                 requestToken);
        requestToken->waitUntilCompleted();
    }
    // Send final chunks
    qp->send(data_buffer,
             final,
             data_size % buffer_size,
             op_flags,
             requestToken);
    requestToken->waitUntilCompleted();

    std::cout << "Done sending, waiting for confirmation from client\n";
    waitForControlMessage();

    delete data_buffer;
}

void RdmaServer::waitForControlMessage() {
    auto receive_buffer = new infinity::memory::Buffer(&context, sizeof(char));
    context.postReceiveBuffer(receive_buffer);
    auto receive_elem = new infinity::core::receive_element_t;
    while (!context.receive(receive_elem));
    delete receive_elem;
    delete receive_buffer;
}

void RdmaServer::run_throughput_tests() {

}

void RdmaServer::run_latency_tests() {

}
