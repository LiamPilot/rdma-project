//
// Created by liampilot on 05/05/2020.
//

#include "RunClient.h"

#include "utils.h"
#include <string>
#include <chrono>
#include <iostream>
#include <infinity/queues/QueuePair.h>
#include <fstream>

using namespace std;


void run_client(infinity::core::Context *context, infinity::queues::QueuePairFactory *qp_factory,
        const string server_ip, int tuple_size, int num_tuples, DataDirection dataDirection) {
    std::ofstream results_file;
    results_file.open("results.txt");
    auto *qp = qp_factory->connectToRemoteHost(server_ip.data(), utils::PORT);
    auto remote_buffer_token = (infinity::memory::RegionToken*) qp->getUserData();
    infinity::requests::RequestToken requestToken(context);


    switch (dataDirection) {
        case DataDirection::write: {
            int data_size = tuple_size * num_tuples;
            auto data = utils::GenerateRandomData(data_size);
            auto local_buffer = new infinity::memory::Buffer(context, data.data), data_size * sizeof(char));

            for (int buffer_size : utils::buffer_sizes) {
                double throughput = run_write_test(buffer_size, context, qp, remote_buffer_token, &requestToken,
                                                   data_size, local_buffer);
                results_file << buffer_size << " " << throughput << endl;
            }

            break;
        }
        case DataDirection::read: {
            for (int buffer_size : utils::buffer_sizes) {
                double throughput = run_read_test(buffer_size, context, qp, remote_buffer_token, &requestToken,
                                                  tuple_size * num_tuples);
                results_file << buffer_size << " " << throughput << endl;
            }
            break;
        }
        case DataDirection::two_sided: {
            for (int buffer_size : utils::buffer_sizes) {
                double throughput = run_twosided_test(buffer_size, context, qp, &requestToken, tuple_size * num_tuples);
                results_file << buffer_size << " " << throughput << endl;
            }
            break;
        }
    }

    auto done_buffer = new infinity::memory::Buffer(context, 1);
    qp->send(done_buffer, &requestToken);
    delete done_buffer;
}

double run_read_test(int buffer_size, infinity::core::Context *context, infinity::queues::QueuePair *qp,
        infinity::memory::RegionToken* remote_buffer_token, infinity::requests::RequestToken *requestToken, int data_size) {
    auto local_buffer = new infinity::memory::Buffer(context, data_size * sizeof(char));

    std::cout << "Start Reading\n";

    auto read_size = buffer_size * sizeof(char);
    auto op_flags = infinity::queues::OperationFlags();
    auto rem = data_size - (data_size % read_size);
    auto start = chrono::high_resolution_clock::now();
    for (int offset = 0; offset < rem; offset += buffer_size) {
        qp->read(local_buffer,
                offset,
                remote_buffer_token,
                offset,
                read_size,
                op_flags,
                requestToken);
        requestToken->waitUntilCompleted();
    }
    qp->read(local_buffer,
             rem,
             remote_buffer_token,
             rem,
             data_size - rem,
             op_flags,
             requestToken);
    requestToken->waitUntilCompleted();
    auto stop = chrono::high_resolution_clock::now();
    std::cout << "Finished Reading\n";
    chrono::duration<double, std::ratio<1, 1>> time = stop - start;
    cout << time.count() << endl;
    double throughput = ((double) data_size) / time.count();
    cout << throughput << endl;
    delete local_buffer;
    return throughput;
}

double run_write_test(int buffer_size, infinity::core::Context *context, infinity::queues::QueuePair *qp,
                     infinity::memory::RegionToken* remote_buffer_token, infinity::requests::RequestToken *requestToken,
                     int data_size, infinity::memory::Buffer* local_buffer) {

    std::cout << "Start Writing\n";
    requestToken = context->defaultRequestToken;
    auto read_size = buffer_size * sizeof(char);
    auto op_flags = infinity::queues::OperationFlags();
    auto rem = data_size - (data_size % read_size);
    auto start = chrono::high_resolution_clock::now();

    for (int offset = 0; offset < rem; offset += buffer_size) {
        qp->write(local_buffer,
                 offset,
                 remote_buffer_token,
                 offset,
                 read_size,
                 op_flags,
                 requestToken);
        requestToken->waitUntilCompleted();
    }
    qp->write(local_buffer,
             rem,
             remote_buffer_token,
             rem,
             data_size - rem,
             op_flags,
             requestToken);
    requestToken->waitUntilCompleted();

    auto stop = chrono::high_resolution_clock::now();
    std::cout << "Finished Reading\n";
    chrono::duration<double, std::ratio<1, 1>> time = stop - start;
    cout << time.count() << endl;
    double throughput = ((double) data_size) / time.count();
    cout << throughput << endl;
    return throughput;
}


double run_twosided_test(int buffer_size, infinity::core::Context *context, infinity::queues::QueuePair *qp,
                         infinity::requests::RequestToken *requestToken, int data_size) {
    std::cout << "Receiving " << data_size << " bytes in " << buffer_size << " byte chunks" << '\n';

    auto buffer = new infinity::memory::Buffer(context, buffer_size);
    std::cout << "Sending start signal\n";
    auto *control_buffer = new infinity::memory::Buffer(context, 1);
    qp->send(control_buffer, requestToken);
    requestToken->waitUntilCompleted();

    int num_messages = (data_size / buffer_size) + 1;
    std::cout << "I expect to receive " << num_messages << " messages" << std::endl;

    auto start = chrono::high_resolution_clock::now();
    for (int x = 0; x < data_size; x += buffer_size) {
        context->postReceiveBuffer(buffer);
        auto *receive_elem = new infinity::core::receive_element_t;
        while (!context->receive(receive_elem));
    }
    auto stop = chrono::high_resolution_clock::now();

    std::cout << "Received all messages, now sending finish signal\n";
    qp->send(control_buffer, requestToken);
    requestToken->waitUntilCompleted();

    delete buffer;

    chrono::duration<double, std::ratio<1, 1>> time = stop - start;
    cout << time.count() << endl;
    double throughput = ((double) data_size) / time.count();
    cout << throughput << endl;
    return throughput;
}
