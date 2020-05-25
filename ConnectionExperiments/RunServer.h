//
// Created by liampilot on 05/05/2020.
//

#ifndef CONNECTIONEXPERIMENTS_RUNSERVER_H
#define CONNECTIONEXPERIMENTS_RUNSERVER_H

#include <infinity/core/Context.h>
#include <infinity/queues/QueuePairFactory.h>

#include "experiment_type.h"

std::unique_ptr<infinity::core::Context>
run_server(infinity::core::Context*, infinity::queues::QueuePairFactory*, int, int, DataDirection);
void serve_test_data(infinity::core::Context*, infinity::queues::QueuePairFactory*, int, int);
void receive_test_data(infinity::core::Context *context, infinity::queues::QueuePairFactory *qp_factory, int data_size);
void send_test_data(int buffer_size, infinity::core::Context *context, infinity::queues::QueuePair *qp, char *data, int data_size);

#endif //CONNECTIONEXPERIMENTS_RUNSERVER_H
