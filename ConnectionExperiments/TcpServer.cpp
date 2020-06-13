//
// Created by liampilot on 22/05/2020.
//

#include "TcpServer.h"

#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <iostream>
#include <ifaddrs.h>
#include <regex>
#include <unistd.h>

#include <utils.h>

TcpServer::TcpServer(std::string port) {
//    std::cout << "TCP Server" << std::endl;

//    std::cout << "Getting infiniband card\n" << std::endl;
    ifaddrs ib_address {};
    utils::get_ib_card_address(&ib_address);
//    std::cout << ib_address->ifa_name << std::endl;

    addrinfo hints{};
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_addr = ib_address.ifa_addr;

//    std::cout << "Getting local server info\n" << std::endl;
    addrinfo *server_info;
    int addr_info_status = getaddrinfo(nullptr, port.data(), &hints, &server_info);
    if (addr_info_status < 0) {
        std::cout << "Couldn't get address information for my socket" << std::endl;
        exit(-1);
    }

//    std::cout << "Creating socket\n";
    int sock = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (sock < 0) {
        std::cout << "Something went wrong creating the socket\n";
        exit(-1);
    }

//    std::cout << "Binding socket\n";
    int bind_status = bind(sock, server_info->ai_addr, server_info->ai_addrlen);
    if (bind_status < 0) {
        std::cout << "Something went wrong binding to port\n";
        exit(-1);
    }

//    std::cout << "Listening on socket\n";
    int listen_status = listen(sock, 10);
    if (listen_status < 0) {
        std::cout << "I'm not listening";
        exit(-1);
    }

    std::cout << "Waiting for client" << std::endl;
    sockaddr_storage client_address {};
    socklen_t addr_size = sizeof(client_address);
    client_socket = accept(sock, (sockaddr*) &client_address, &addr_size);
    if (client_socket < 0) {
        std::cout << "Couldn't accept client socket\n";
        exit(-1);
    }
}

TcpServer::~TcpServer() {
    close(client_socket);
}


void TcpServer::run_throughput_tests(size_t data_size) {
    for (size_t buffer_size : utils::buffer_sizes) {
        throughput_test(buffer_size, data_size);
    }
}

void TcpServer::throughput_test(size_t buffer_size, size_t data_size) {
    size_t last_index = data_size - (data_size % buffer_size);
    std::vector<char> data(data_size);
    std::vector<char> little_buffer(buffer_size);
    for (size_t offset = 0; offset < last_index; offset += buffer_size) {
        receive_message(buffer_size, data.data() + offset);
        std::copy(data.begin(), data.end(), little_buffer.begin());
    }
    receive_message(data_size % buffer_size, data.data() + last_index);
    std::copy(data.begin(), data.end(), little_buffer.begin());
}

void TcpServer::run_latency_tests() {
    for (size_t buffer_size : utils::latency_buffer_sizes) {
        latency_test(buffer_size);
    }
    std::cout << "Finished latency test" << std::endl;
}


double TcpServer::latency_test(size_t buffer_size) {
    std::vector<char> data(buffer_size);
    std::vector<char> little_buffer(buffer_size);
    for (int i = 0; i < utils::num_loops; i++) {
        memset(data.data(), 0, buffer_size);
        receive_message(buffer_size, data.data());
//        memcpy(little_buffer, data, buffer_size);
    }
}

void TcpServer::receive_message(size_t buffer_size, char* data) const {
    int bytes_read = 0;

    while (bytes_read < buffer_size) {
        int new_bytes_read = recv(client_socket, data + bytes_read, buffer_size - bytes_read, 0);
        if (new_bytes_read <= 0) {
            std::cout << "She's going down cap'n: " << errno << std::endl;
            exit(-1);
        }
        bytes_read += new_bytes_read;
    }

}


