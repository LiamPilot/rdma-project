//
// Created by liampilot on 05/05/2020.
//

#ifndef CONNECTIONEXPERIMENTS_RUNCLIENT_H
#define CONNECTIONEXPERIMENTS_RUNCLIENT_H


#include <infinity/core/Context.h>
#include <infinity/queues/QueuePairFactory.h>

#include "experiment_type.h"

using namespace std;

void run_client(infinity::core::Context*, infinity::queues::QueuePairFactory*, string, int, int, DataDirection);

double run_read_test(int, infinity::core::Context*, infinity::queues::QueuePair*, infinity::memory::RegionToken*,
        infinity::requests::RequestToken*, int);


double run_write_test(int, infinity::core::Context*, infinity::queues::QueuePair*, infinity::memory::RegionToken*,
                     infinity::requests::RequestToken*, int, infinity::memory::Buffer*);

double run_twosided_test(int buffer_size, infinity::core::Context *context, infinity::queues::QueuePair *qp,
                         infinity::requests::RequestToken *requestToken, int data_size);

#endif //CONNECTIONEXPERIMENTS_RUNCLIENT_H
