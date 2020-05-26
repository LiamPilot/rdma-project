//
// Created by liampilot on 05/05/2020.
//

#include "RunServer.h"

#include "utils.h"

#include <infinity/queues/QueuePair.h>
#include <iostream>


void run_server(std::unique_ptr<infinity::core::Context> context, infinity::queues::QueuePairFactory *qp_factory, int tuple_size,
           int num_tuples, DataDirection dataDirection) {

    if (dataDirection == DataDirection::write) {
        receive_test_data(context.get(), qp_factory, tuple_size * num_tuples);
    } else if (dataDirection == DataDirection::read) {
        serve_test_data(context.get(), qp_factory, tuple_size, num_tuples);
    } else if (dataDirection != DataDirection::two_sided) {
        std::cout << "Connecting to client\n";
        qp_factory->bindToPort(utils::PORT);
        auto buffer = new infinity::memory::Buffer(context.get(), sizeof(char));
        auto buffer_token = buffer->createRegionToken();
        auto qp = qp_factory->acceptIncomingConnection(buffer_token, sizeof(infinity::memory::RegionToken));

        std::cout << "generating data\n";
        auto data = utils::GenerateRandomData(tuple_size * num_tuples);

        std::cout << "Doing two sided test\n";
        for (int buffer_size : utils::buffer_sizes) {
            send_test_data(buffer_size, context.get(), qp, data.get(), tuple_size * num_tuples);
        }

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
    std::cout << "Creating random data\n";
    auto data_size = tuple_size * num_tuples;
    auto data = utils::GenerateRandomData(data_size);

    std::cout << "Creating Buffer with Data\n";
    auto data_buffer = new infinity::memory::Buffer(context, data.get(), data_size * sizeof(char));
    auto buffer_token = data_buffer->createRegionToken();
    qp_factory->bindToPort(utils::PORT);
    auto qp = qp_factory->acceptIncomingConnection(buffer_token, sizeof(infinity::memory::RegionToken));

    std::cout << "Waiting for client to finish\n";
    waitForControlMessage(context);

    std::cout << "Done, cleaning up\n";
    delete data_buffer;
    delete qp;
}

void receive_test_data(infinity::core::Context *context, infinity::queues::QueuePairFactory *qp_factory, int data_size) {
    auto data_buffer = new infinity::memory::Buffer(context, data_size * sizeof(char));
    auto buffer_token = data_buffer->createRegionToken();
    qp_factory->bindToPort(utils::PORT);

    std::cout << "Ready for test\n";
    auto qp = qp_factory->acceptIncomingConnection(buffer_token, sizeof(infinity::memory::RegionToken));

    std::cout << "Waiting for client to finish\n";
    waitForControlMessage(context);

    delete data_buffer;
    delete qp;
}


void send_test_data(int buffer_size, infinity::core::Context *context, infinity::queues::QueuePair *qp, char *data, int data_size) {

    std::cout << "Sending " << data_size << " bytes in " << buffer_size << " chunks" << std::endl;
    auto data_buffer = new infinity::memory::Buffer(context, data, data_size * sizeof(char));

    std::cout << "Waiting for client to be ready\n";
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

    std::cout << "Done sending, waiting for confirmation from client\n";
    waitForControlMessage(context);

    delete data_buffer;
}
