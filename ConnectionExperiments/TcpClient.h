//
// Created by liampilot on 22/05/2020.
//

#ifndef CONNECTIONEXPERIMENTS_TCPCLIENT_H
#define CONNECTIONEXPERIMENTS_TCPCLIENT_H

#include "Client.h"

#include <string>

class TcpClient : public Client {
public:
    TcpClient(std::string ip, std::string port);

    ~TcpClient() override;

    void run_throughput_tests() override;

    void run_latency_tests() override;

private:
    int server_socket;

    double latency_test(int buffer_size);

    void inline send_tcp_message(const char* message, int size);
};


#endif //CONNECTIONEXPERIMENTS_TCPCLIENT_H
