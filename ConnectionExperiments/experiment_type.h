//
// Created by liampilot on 13/05/2020.
//

#ifndef THROUGHPUTEXPERIMENTS_EXPERIMENT_TYPE_H
#define THROUGHPUTEXPERIMENTS_EXPERIMENT_TYPE_H

enum class DataDirection {write, read, two_sided};
enum class Parrallelism {single_thread, double_thread};
enum class Metric {throughput, latency};
enum class Connection {rdma, rpc, tcp};

#endif //THROUGHPUTEXPERIMENTS_EXPERIMENT_TYPE_H
