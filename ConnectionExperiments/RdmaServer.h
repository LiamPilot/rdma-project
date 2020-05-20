//
// Created by liampilot on 19/05/2020.
//

#ifndef THROUGHPUTEXPERIMENTS_RDMASERVER_H
#define THROUGHPUTEXPERIMENTS_RDMASERVER_H

#include <memory>

#include <infinity/core/Context.h>
#include <infinity/queues/QueuePair.h>
#include <infinity/queues/QueuePairFactory.h>

class RdmaServer {

public:
    RdmaServer(const infinity::core::Context& c, int port) : context(c) {
        qp_factory = std::make_unique<infinity::queues::QueuePairFactory>(&context);
        qp_factory->bindToPort(port);
    }

    void run_read_experiments(int data_size);

    void run_write_experiments(int data_size);

    void run_two_sided_experiments(int data_size);


private:
    infinity::core::Context context;
    std::unique_ptr<infinity::queues::QueuePairFactory> qp_factory;

    void write_test(int data_size);

    void two_sided_test(int buffer_size, int data_size, char *data, infinity::queues::QueuePair* qp);

    void waitForControlMessage();

};


#endif //THROUGHPUTEXPERIMENTS_RDMASERVER_H
