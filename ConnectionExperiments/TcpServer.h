//
// Created by liampilot on 22/05/2020.
//

#ifndef CONNECTIONEXPERIMENTS_TCPSERVER_H
#define CONNECTIONEXPERIMENTS_TCPSERVER_H

#include "Server.h"

#include <string>
#include <sys/socket.h>

class TcpServer : public Server {
public:
    TcpServer(std::string port);

    ~TcpServer() override = default;

    void run_throughput_tests() override;

    void run_latency_tests() override;

private:
    int client_socket;
    sockaddr_storage client_address;

    double latency_test(int buffer_size);

    std::string receive_message(int buffer_size) const;
};


#endif //CONNECTIONEXPERIMENTS_TCPSERVER_H
