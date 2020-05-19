//
// Created by liampilot on 19/05/2020.
//

#ifndef THROUGHPUTEXPERIMENTS_RDMACLIENT_H
#define THROUGHPUTEXPERIMENTS_RDMACLIENT_H

#include <memory>
#include <vector>

#include <infinity/core/Context.h>
#include <infinity/queues/QueuePair.h>
#include <infinity/queues/QueuePairFactory.h>

#include <utils.h>

class RdmaClient {

public:
    RdmaClient(const infinity::core::Context& c, const std::string& server_ip, int port) : context{c} {
        auto qp_factory = std::make_unique<infinity::queues::QueuePairFactory>(&context);
        queue_pair = std::unique_ptr<infinity::queues::QueuePair>{qp_factory->connectToRemoteHost(server_ip.data(), port)};
    }

    std::vector<test_result> run_read_experiments(int data_size);

    std::vector<test_result> run_write_experiments(int data_size);

    std::vector<test_result> run_two_sided_experiments(int data_size);


private:
    infinity::core::Context context;
    std::unique_ptr<infinity::queues::QueuePair> queue_pair;

    double read_test(int buffer_size, int data_size, infinity::memory::RegionToken* remote_buffer_token);

    double write_test(int buffer_size, int data_size, infinity::memory::RegionToken* remote_buffer_token,
            infinity::memory::Buffer* local_buffer);

    double two_sided_test(int buffer_size, int data_size);
};


#endif //THROUGHPUTEXPERIMENTS_RDMACLIENT_H
