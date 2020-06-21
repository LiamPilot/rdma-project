//
// Created by liampilot on 22/05/2020.
//

#include "TcpClient.h"
#include "utils.h"

#include <string>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <ifaddrs.h>
#include <fstream>
#include <unistd.h>
#include <chrono>
#include <vector>


TcpClient::TcpClient(std::string ip, std::string port) {
    std::cout << "TCP Client\n";
    addrinfo hints{};
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;

    std::cout << "Getting server information\n";
    addrinfo *server_info;
    int addr_info_status = getaddrinfo(ip.data(), port.data(), &hints, &server_info);

    if (addr_info_status < 0) {
        std::cout << "Something went wrong getting address info\n";
        return;
    }

    std::cout << "Creating server socket\n";
    server_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (server_socket < 0) {
        std::cout << "Something went wrong creating the server socket\n";
        exit(-1);
    }

    std::cout << "Getting ib card\n";
    ifaddrs ib_address {};
    utils::get_ib_card_address(&ib_address);
    hints.ai_addr = ib_address.ifa_addr;

    std::cout << "Getting local address\n";
    addrinfo *client_info;
    addr_info_status = getaddrinfo(nullptr, port.data(), &hints, &client_info);
    if (addr_info_status < 0) {
        std::cout << "Something went wrong getting local address info" << std::endl;
        exit(-1);
    }

    std::cout << "Binding to ib card\n";
    int bind_status = bind(server_socket, client_info->ai_addr, client_info->ai_addrlen);
    if (bind_status < 0) {
        std::cout << "Something went wrong binding socket to local address" << std::endl;
        exit(-1);
    }

    std::cout << "Connecting to server\n";
    addr_info_status = connect(server_socket, server_info->ai_addr, server_info->ai_addrlen);
    if (addr_info_status < 0) {
        std::cout << "Something went wrong connecting to server" << std::endl;
        exit(-1);
    }

}

TcpClient::~TcpClient() {
    close(server_socket);
}

void TcpClient::run_throughput_tests(size_t data_size) {
    std::ofstream results_file;
    results_file.open(throughput_file_name);

    auto data = utils::GenerateRandomData(data_size);
    for (size_t buffer_size : utils::buffer_sizes) {
        double throughput = throughput_test(buffer_size, data);
        results_file << buffer_size << " " << throughput << std::endl;
        utils::dev_random_data(data.data(), data_size);
    }
}

double TcpClient::throughput_test(size_t buffer_size, std::vector<char> data) {
    std::cout << "Testing throughput for: " << buffer_size << '\n';

    size_t last_index = data.size() - (data.size() % buffer_size);
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t offset = 0; offset < last_index; offset += buffer_size) {
        send_tcp_message(data.data() + offset, buffer_size);
    }
    send_tcp_message(data.data() + last_index, data.size() % buffer_size);
    receive_message(1, data.data());
    auto stop = std::chrono::high_resolution_clock::now();

    return utils::calculate_throughput(start, stop, data.size());
}

void TcpClient::run_latency_tests() {
    std::ofstream results_file;
    results_file.open(latency_file_name);

    for (size_t buffer_size : utils::latency_buffer_sizes) {
        double latency = latency_test(buffer_size);
        results_file << buffer_size << " " << latency << std::endl;
    }
}

double TcpClient::latency_test(size_t buffer_size) {
    std::cout << "Testing latency for: " << buffer_size << '\n';

    int numBuffers = 40;
    std::vector<std::vector<char>> dataBuffers(numBuffers);
    for (int x = 0; x < numBuffers; x++) {
        std::vector<char> buf = utils::GenerateRandomData(buffer_size);
        dataBuffers[x] = buf;
    }

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < utils::num_loops; i++) {
        send_tcp_message(dataBuffers[i % numBuffers].data(), buffer_size);
    }
    receive_message(1, dataBuffers[0].data());
    auto stop = std::chrono::high_resolution_clock::now();

    auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
    double latency = ((double) time.count()) / (double) utils::num_loops;
    return latency;
}

void inline TcpClient::send_tcp_message(const char *message, size_t size) const {
    size_t bytes_sent = 0;
    while (bytes_sent < size) {
        int new_bytes_sent = send(server_socket, message + bytes_sent, size - bytes_sent, 0);
        if (new_bytes_sent < 0) {
            std::cout << "Something went wrong sending data: " << errno << "\n";
        }
        bytes_sent += new_bytes_sent;
    }

}

void TcpClient::receive_message(size_t buffer_size, char* data) const {
    size_t bytes_read = 0;

    while (bytes_read < buffer_size) {
        int new_bytes_read = recv(server_socket, data + bytes_read, buffer_size - bytes_read, 0);
        if (new_bytes_read <= 0) {
            std::cout << "She's going down cap'n: " << errno << std::endl;
            exit(-1);
        }
        bytes_read += new_bytes_read;
    }

}