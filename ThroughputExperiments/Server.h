//
// Created by liampilot on 05/05/2020.
//

#ifndef THROUGHPUTEXPERIMENTS_SERVER_H
#define THROUGHPUTEXPERIMENTS_SERVER_H

#include <infinity/core/Context.h>
#include <infinity/queues/QueuePairFactory.h>

#include "experiment_type.h"

void run_server(infinity::core::Context*, infinity::queues::QueuePairFactory*, int, int, DataDirection);
void serve_test_data(infinity::core::Context*, infinity::queues::QueuePairFactory*, int, int);
void receive_test_data(infinity::core::Context *context, infinity::queues::QueuePairFactory *qp_factory, int data_size);
void send_test_data(int buffer_size, infinity::core::Context *context, infinity::queues::QueuePair *qp, char *data, int data_size);

#endif //THROUGHPUTEXPERIMENTS_SERVER_H
