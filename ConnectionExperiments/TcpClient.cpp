//
// Created by liampilot on 22/05/2020.
//

#include "TcpClient.h"
#include "utils.h"

#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <iostream>
#include <ifaddrs.h>
#include <fstream>
#include <unistd.h>
#include <chrono>
#include <numeric>
#include <vector>

void TcpClient::run_throughput_tests() {

}

void TcpClient::run_latency_tests() {
    std::ofstream results_file;
    results_file.open("results.txt");

    for (int buffer_size : utils::buffer_sizes) {
        double throughput = latency_test(buffer_size);
        results_file << buffer_size << throughput << std::endl;
    }
}

double TcpClient::latency_test(int buffer_size) {
    std::vector<double> results{};

    for (int i = 0; i < utils::num_loops; i++) {
        char* data = utils::GenerateRandomData(buffer_size);

        auto start = std::chrono::high_resolution_clock::now();
        send_tcp_message(data, buffer_size);
        auto stop = std::chrono::high_resolution_clock::now();

        auto time = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
        results.push_back(time.count());

        free(data);
    }

    double sum = std::accumulate(results.begin(), results.end(), 0.0);
    return sum / num_loops;
}

void inline TcpClient::send_tcp_message(const char *message, int size) {
    int bytes_sent = 0;
    while (bytes_sent < size) {
        int new_bytes_sent = send(server_socket, message + bytes_sent, size, 0);
        if (new_bytes_sent < 0) {
            std::cout << "Something went wrong sending data\n";
        }
        bytes_sent += new_bytes_sent;
    }

}

TcpClient::TcpClient(std::string ip, std::string port) {
    addrinfo hints{};
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    addrinfo *server_info;
    int status = getaddrinfo(ip.data(), port.data(), &hints, &server_info);

    if (status < 0) {
        std::cout << "Something went wrong getting address info\n";
        return;
    }

    server_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (server_socket < 0) {
        std::cout << "Something went wrong creating the socket\n";
        exit(-1);
    }

    ifaddrs* ib_address;
    utils::get_ib_card_address(ib_address);
    hints.ai_addr = ib_address->ifa_addr;
    addrinfo *client_info;
    status = getaddrinfo(NULL, port.data(), &hints, &client_info);
    if (status < 0) {
        std::cout << "Something went wrong getting local address\n";
        exit(-1);
    }

    status = bind(server_socket, client_info->ai_addr, client_info->ai_addrlen);
    if (status < 0) {
        std::cout << "Something went wrong binding socket to local address\n";
        exit(-1);
    }

    status = connect(server_socket, server_info->ai_addr, server_info->ai_addrlen);
    if (status < 0) {
        std::cout << "Something went wrong connecting to server";
        exit(-1);
    }

}

TcpClient::~TcpClient() {
    close(server_socket);
}

