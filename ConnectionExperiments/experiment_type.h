//
// Created by liampilot on 13/05/2020.
//

#ifndef CONNECTIONEXPERIMENTS_EXPERIMENT_TYPE_H
#define CONNECTIONEXPERIMENTS_EXPERIMENT_TYPE_H

enum class DataDirection {write, read, two_sided};
enum class Parrallelism {single_thread, double_thread};
enum class Metric {throughput, latency, all};
enum class Connection {rdma, rpc, tcp};

#endif //CONNECTIONEXPERIMENTS_EXPERIMENT_TYPE_H
