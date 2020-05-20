//
// Created by liampilot on 05/05/2020.
//

#include "Server.h"

#include "utils.h"

#include <infinity/queues/QueuePair.h>
#include <iostream>

void run_server(infinity::core::Context *context, infinity::queues::QueuePairFactory *qp_factory, int tuple_size,
        int num_tuples, DataDirection dataDirection) {

    if (dataDirection == DataDirection::write) {
        receive_test_data(context, qp_factory, tuple_size * num_tuples);
    } else if (dataDirection == DataDirection::read) {
        serve_test_data(context, qp_factory, tuple_size, num_tuples);
    } else if (dataDirection != DataDirection::two_sided) {
        utils::print("Connecting to client");
        qp_factory->bindToPort(PORT);
        auto buffer = new infinity::memory::Buffer(context, sizeof(char));
        auto buffer_token = buffer->createRegionToken();
        auto qp = qp_factory->acceptIncomingConnection(buffer_token, sizeof(infinity::memory::RegionToken));

        utils::print("generating data");
        char *data = utils::GenerateRandomData(tuple_size * num_tuples);

        utils::print("Doing two sided test");
        for (int buffer_size : utils::buffer_sizes) {
            send_test_data(buffer_size, context, qp, data, tuple_size * num_tuples);
        }

        free(data);
        delete qp;
        delete buffer;
        delete buffer_token;
    }
}

void waitForControlMessage(infinity::core::Context *context) {
    auto receive_buffer = new infinity::memory::Buffer(context, sizeof(char));
    context->postReceiveBuffer(receive_buffer);
    auto receive_elem = new infinity::core::receive_element_t;
    while (!context->receive(receive_elem));
    delete receive_elem;
    delete receive_buffer;
}

void serve_test_data(infinity::core::Context *context, infinity::queues::QueuePairFactory *qp_factory, int tuple_size,
                     int num_tuples) {
    utils::print("Creating random data");
    auto data_size = tuple_size * num_tuples;
    char *data = utils::GenerateRandomData(data_size);

    utils::print("Creating Buffer with Data");
    auto data_buffer = new infinity::memory::Buffer(context, data, data_size * sizeof(char));
    auto buffer_token = data_buffer->createRegionToken();
    qp_factory->bindToPort(PORT);
    auto qp = qp_factory->acceptIncomingConnection(buffer_token, sizeof(infinity::memory::RegionToken));

    utils::print("Waiting for client to finish");
    waitForControlMessage(context);

    utils::print("Done, cleaning up");
    free(data);
    delete data_buffer;
    delete qp;
}

void receive_test_data(infinity::core::Context *context, infinity::queues::QueuePairFactory *qp_factory, int data_size) {
    auto data_buffer = new infinity::memory::Buffer(context, data_size * sizeof(char));
    auto buffer_token = data_buffer->createRegionToken();
    qp_factory->bindToPort(PORT);

    utils::print("Ready for test");
    auto qp = qp_factory->acceptIncomingConnection(buffer_token, sizeof(infinity::memory::RegionToken));

    utils::print("Waiting for client to finish");
    waitForControlMessage(context);

    delete data_buffer;
    delete qp;
}


void send_test_data(int buffer_size, infinity::core::Context *context, infinity::queues::QueuePair *qp, char *data, int data_size) {

    std::cout << "Sending " << data_size << " bytes in " << buffer_size << " chunks" << std::endl;
    auto data_buffer = new infinity::memory::Buffer(context, data, data_size * sizeof(char));

    utils::print("Waiting for client to be ready");
    waitForControlMessage(context);

    auto requestToken = context->defaultRequestToken;
    auto op_flags = infinity::queues::OperationFlags();

    auto msg = data_size / buffer_size + 1;
    int final = data_size - (data_size % buffer_size);
    std::cout << "I will send " << msg << " messages" << std::endl;
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
    waitForControlMessage(context);

    delete data_buffer;
}