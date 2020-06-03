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


void TcpServer::run_throughput_tests(int data_size) {
    for (int buffer_size : utils::buffer_sizes) {
        throughput_test(buffer_size, data_size);
    }
}

void TcpServer::throughput_test(int buffer_size, int data_size) {
    int last_index = data_size - (data_size % buffer_size);
    std::unique_ptr<char[]> data(new char[data_size]);
    char* little_buffer = new char[buffer_size];
    for (int offset = 0; offset < last_index; offset += buffer_size) {
        receive_message(buffer_size, data.get() + offset);
//        memcpy(little_buffer, data.get(), buffer_size);
    }
    receive_message(data_size % buffer_size, data.get() + last_index);
    memcpy(little_buffer, data.get(), buffer_size);
    delete[] little_buffer;
}

void TcpServer::run_latency_tests() {
    for (int buffer_size : utils::buffer_sizes) {
        latency_test(buffer_size);
    }
    std::cout << "Finished latency test" << std::endl;
}


double TcpServer::latency_test(int buffer_size) {
    char* data = new char[buffer_size];
    char* little_buffer = new char[buffer_size];
    for (int i = 0; i < utils::num_loops; i++) {
        memset(data, 0, buffer_size);
        receive_message(buffer_size, data);
//        memcpy(little_buffer, data, buffer_size);
    }
    delete[] little_buffer;
    delete[] data;
}

void TcpServer::receive_message(int buffer_size, char* data) const {
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


