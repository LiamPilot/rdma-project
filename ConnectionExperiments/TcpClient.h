//
// Created by liampilot on 22/05/2020.
//

#ifndef CONNECTIONEXPERIMENTS_TCPCLIENT_H
#define CONNECTIONEXPERIMENTS_TCPCLIENT_H

#include "Client.h"

#include <string>
#include <memory>

class TcpClient : public Client {
public:
    TcpClient(std::string ip, std::string port);

    ~TcpClient() override;

    void run_throughput_tests(int data_size) override;

    void run_latency_tests() override;

private:
    const char* throughput_file_name = "tcp_throughput.txt";
    const char* latency_file_name = "tcp_latency.txt";

    int server_socket;

    double latency_test(int buffer_size);

    void inline send_tcp_message(const char* message, int size) const;

    double throughput_test(int buffer_size, std::unique_ptr<char[]>& data, int data_size);
};


#endif //CONNECTIONEXPERIMENTS_TCPCLIENT_H
