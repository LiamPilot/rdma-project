#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <infinity/core/Context.h>
#include <infinity/queues/QueuePairFactory.h>

#include "utils.h"
#include "RunServer.h"
#include "RunClient.h"
#include "experiment_type.h"

#include "Client.h"
#include "Server.h"
#include "TcpServer.h"
#include "TcpClient.h"
#include "RdmaClient.h"
#include "RdmaServer.h"

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

int get_data_size(int argc, char** argv) {
    string value = get_arg_value(argc, argv, "-datasize", to_string(utils::DATA_SIZE));
    return std::stoi(value);
}

int get_num_tuples(int argc, char* argv[]) {
    string value = get_arg_value(argc, argv, "-numtuples", to_string(utils::NUM_TUPLES));
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
    } else if (value == "rpc") {
        return Connection::rpc;
    }

    return Connection::rdma;
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

DataDirection get_direction(int argc, char **argv) {
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

std::unique_ptr<Server> make_server(Connection connection) {
    std::string port = to_string(utils::PORT);
    switch (connection) {
        case Connection::rdma: {
            auto context = std::make_unique<infinity::core::Context>();
            return make_unique<RdmaServer>(std::move(context), port);
        }
        case Connection::tcp: {
            return make_unique<TcpServer>(port);
        }
        case Connection::rpc: {
            std::cout << "Path not made yet" << std::endl;
            exit(-1);
        }
    }
}

std::unique_ptr<Client> make_client(Connection connection, std::string ip) {
    std::string port = to_string(utils::PORT);
    switch (connection) {
        case Connection::rdma: {
            auto context = std::make_unique<infinity::core::Context>();
            return make_unique<RdmaClient>(std::move(context), ip, port);
        }
        case Connection::tcp:
            return make_unique<TcpClient>(ip, port);
        case Connection::rpc:
            std::cout << "Path not made yet" << std::endl;
            exit(-1);
    }
}

int main(int argc, char* argv[]) {
//    auto context = std::make_unique<infinity::core::Context>();
//    auto qp_factory = std::make_unique<infinity::queues::QueuePairFactory>(context.get());

    int data_size = get_data_size(argc, argv);
    int num_tuples = get_num_tuples(argc, argv);
    DataDirection dataDirection = get_direction(argc, argv);
    Metric metric = get_test_metric(argc, argv);
    Connection connection = get_connection_type(argc, argv);


    if (is_server(argc, argv)) {
        std::cout << "Running Server\n";
        auto server = make_server(connection);

        server->run_latency_tests();
        server->run_throughput_tests(data_size);
    } else {
        std::cout << "Running RdmaClient\n";
        string server_ip = get_server_ip(argc, argv);

        auto client = make_client(connection, server_ip);

        client->run_latency_tests();
        client->run_throughput_tests(data_size);
    }

    std::cout << "Exiting\n";

//    run_client(context, qp_factory, server_ip, tuple_size, num_tuples, dataDirection);
//    run_server(context, qp_factory, tuple_size, num_tuples, dataDirection);
    return 0;
}