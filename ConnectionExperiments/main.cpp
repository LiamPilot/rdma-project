#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <infinity/core/Context.h>
#include <infinity/queues/QueuePairFactory.h>

#include "utils.h"
#include "Server.h"
#include "RunClient.h"
#include "experiment_type.h"

#define PORT 8001

string get_arg_value(int argc, char* argv[], const string& arg_name, string default_value) {
    for (int i = 0; i < argc; i++) {
        string arg = argv[i];
        if (arg == arg_name) {
            return argv[i + 1];
        }
    }

    return default_value;
}

string get_server_ip(int argc, char* argv[]) {
    return get_arg_value(argc, argv, "-serverip", "127.0.0.1");
}

int get_tuple_size(int argc, char* argv[]) {
    string value = get_arg_value(argc, argv, "-tuplesize", to_string(TUPLE_SIZE));
    return std::stoi(value);
}

int get_num_tuples(int argc, char* argv[]) {
    string value = get_arg_value(argc, argv, "-numtuples", to_string(NUM_TUPLES));
    return std::stoi(value);
}

Metric get_test_metric(int argc, char* argv[]) {
    string value = get_arg_value(argc, argv, "-metric", "t");
    if (value == "t") {
        return Metric::throughput;
    } else {
        return Metric::latency;
    }
}

Connection get_connection_type(int argc, char* argv[]) {
    string value = get_arg_value(argc, argv, "-metric", "rdma");
    if (value == "rdma") {
        return Connection::rdma;
    } else if (value == "tcp") {
        return Connection::tcp;
    } else {
        return Connection::rpc;
    }
}

bool is_server(int argc, char* argv[]) {
    for (int i = 0; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-s") {
            return true;
        }
    }
    return false;
}

DataDirection getDirection(int argc, char* argv[]) {
    for (int i = 0; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-w") {
            return DataDirection::write;
        } else if (arg == "-r") {
            return DataDirection::read;
        } else if (arg == "-c") {
            return DataDirection::two_sided;
        }
    }
    return DataDirection::read;
}

Parrallelism get_parallelism(int argc, char* argv[]) {
    for (int i = 0; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-double") {
            return Parrallelism::double_thread;
        }
    }
    return Parrallelism::single_thread;
}

int main(int argc, char* argv[]) {
    auto context = std::make_unique<infinity::core::Context>();
    auto qp_factory = std::make_unique<infinity::queues::QueuePairFactory>(context.get());

    int tuple_size = get_tuple_size(argc, argv);
    int num_tuples = get_num_tuples(argc, argv);
    DataDirection dataDirection = getDirection(argc, argv);

    if (is_server(argc, argv)) {
        utils::print("Running Server");
        run_server(context, qp_factory, tuple_size, num_tuples, dataDirection);
    } else {
        utils::print("Running RdmaClient");
        string server_ip = get_server_ip(argc, argv);
        run_client(context, qp_factory, server_ip, tuple_size, num_tuples, dataDirection);
    }

    utils::print("Exiting");

    return 0;
}