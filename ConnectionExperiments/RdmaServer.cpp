//
// Created by liampilot on 19/05/2020.
//

#include "RdmaServer.h"

#include <iostream>

#include <infinity/memory/Buffer.h>

#include <utils.h>

void RdmaServer::run_read_experiments(int data_size) {
    utils::print("Creating random data");
    char *data = utils::GenerateRandomData(data_size);

    utils::print("Creating Buffer with Data");
    auto data_buffer = new infinity::memory::Buffer(&context, data, data_size * sizeof(char));
    auto buffer_token = data_buffer->createRegionToken();
    auto qp = qp_factory->acceptIncomingConnection(buffer_token, sizeof(infinity::memory::RegionToken));

    utils::print("Waiting for client to finish");
    waitForControlMessage();

    utils::print("Done, cleaning up");
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
    utils::print("Connecting to client");
    auto buffer = new infinity::memory::Buffer(&context, sizeof(char));
    auto buffer_token = buffer->createRegionToken();
    auto qp = qp_factory->acceptIncomingConnection(buffer_token, sizeof(infinity::memory::RegionToken));

    utils::print("generating data");
    char *data = utils::GenerateRandomData(tuple_size * num_tuples);

    utils::print("Doing two sided test");
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

    utils::print("Ready for test");
    auto qp = qp_factory->acceptIncomingConnection(buffer_token, sizeof(infinity::memory::RegionToken));

    utils::print("Waiting for client to finish");
    waitForControlMessage();

    delete data_buffer;
    delete qp;
}

void RdmaServer::two_sided_test(int buffer_size, int data_size, char *data, infinity::queues::QueuePair *qp) {
    std::cout << "Sending " << data_size << " bytes in " << buffer_size << " chunks" << '\n';
    auto data_buffer = new infinity::memory::Buffer(&context, data, data_size * sizeof(char));

    utils::print("Waiting for client to be ready");
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

    utils::print("Done sending, waiting for confirmation from client");
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
