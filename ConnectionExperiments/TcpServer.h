//
// Created by liampilot on 22/05/2020.
//

#ifndef CONNECTIONEXPERIMENTS_TCPSERVER_H
#define CONNECTIONEXPERIMENTS_TCPSERVER_H

#include "Server.h"

#include <string>
#include <memory>
#include <sys/socket.h>

class TcpServer : public Server {
public:
    TcpServer(std::string port);

    ~TcpServer() override;

    void run_throughput_tests(int data_size) override;

    void run_latency_tests() override;

private:
    int client_socket;
    sockaddr_storage client_address;

    double latency_test(int buffer_size);

    void receive_message(int buffer_size, char* data) const;

    void throughput_test(int buffer_size, int data_size);
};


#endif //CONNECTIONEXPERIMENTS_TCPSERVER_H
