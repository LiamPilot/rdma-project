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

#include <utils.h>

TcpServer::TcpServer(std::string port) {

    ifaddrs* ib_address;
    utils::get_ib_card_address(ib_address);

    addrinfo hints{};
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;
    hints.ai_addr = ib_address->ifa_addr;

    addrinfo *server_info;
    int status = getaddrinfo(NULL, port.data(), &hints, &server_info);

    int sock = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

    if (sock < 0) {
        std::cout << "Something went wrong creating the socket\n";
        return;
    }

    int bind_status = bind(sock, server_info->ai_addr, server_info->ai_addrlen);

    if (bind_status < 0) {
        std::cout << "Something went wrong binding to port\n";
        exit(-1);
    }

    int listen_status = listen(sock, 1);

    client_socket = accept(sock, reinterpret_cast<sockaddr *>(&client_address),
                           reinterpret_cast<socklen_t *>(sizeof(client_address)));

}


void TcpServer::run_throughput_tests() {

}

void TcpServer::run_latency_tests() {
    for (int buffer_size : utils::buffer_sizes) {
        latency_test(buffer_size);
    }
}


double TcpServer::latency_test(int buffer_size) {
    for (int i = 0; i < utils::num_loops; i++) {
        std::string data = receive_message(buffer_size);
    }
}

std::string TcpServer::receive_message(int buffer_size) const {
    int bytes_read = 0;
    char* data[buffer_size];

    while (bytes_read < buffer_size) {
        int new_bytes_read = recv(client_socket, data + bytes_read, buffer_size, 0);
        if (new_bytes_read < 0) {
            std::cout << "She's going down cap'n" << std::endl;
            exit(-1);
        }
        bytes_read += new_bytes_read;
    }

    std::string return_value = (const char*) data;
    return return_value;
}

