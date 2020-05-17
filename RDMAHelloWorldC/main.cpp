#include <iostream>
#include <string>
#include <vector>
#include "infinity/core/Context.h"
#include "infinity/queues/QueuePairFactory.h"
#include "infinity/queues/QueuePair.h"
#include "infinity/memory/Buffer.h"
using namespace std;

#define PORT 8001

void print(const string& str) {
    cout << str << '\n';
}

void run_server(infinity::core::Context *context, infinity::queues::QueuePairFactory *qp_factory) {
    string data = "Some data to read. Also some other things to make up t";

    print("Creating Read Buffer");
    auto buffer = new infinity::memory::Buffer(context, (void *) data.data(), 64 * sizeof(char));
    qp_factory->bindToPort(PORT);
    auto qp = qp_factory->acceptIncomingConnection(buffer->createRegionToken(), sizeof(infinity::memory::RegionToken));

    infinity::requests::RequestToken requestToken(context);
    print("Sending message");
    qp->send(buffer, &requestToken);

    delete buffer;

}

void run_client(infinity::core::Context *context, infinity::queues::QueuePairFactory *qp_factory, const string server_ip) {
    print("Creating Buffer");
    auto buffer = new infinity::memory::Buffer(context, 64 * sizeof(char));
    context->postReceiveBuffer(buffer);

    print("Connecting To Host");
    auto qp = qp_factory->connectToRemoteHost(server_ip.data(), PORT);

    print("Set up receive element");
    auto receive_elem = new infinity::core::receive_element_t;
    print("Waiting to receive");
    while (!context->receive(receive_elem));
    print("Received a message");
    cout << (char *) buffer->getData() << endl;

    delete buffer;
    delete qp;
}

string get_server_ip(int argc, char **argv) {
    for (int i = 0; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-serverip") {
            return argv[i + 1];
        }
    }

    return "127.0.0.1";
}

bool is_server(int argc, char **argv) {
    for (int i = 0; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-s") {
            return true;
        }
    }
    return false;
}

int main(int argc, char **argv) {
    cout << argc << '\n';
    print("Creating Context");
    auto context = new infinity::core::Context();

    print("Creating Queue Pair Factory");
    auto qp_factory = new infinity::queues::QueuePairFactory(context);

    if (is_server(argc, argv)) {
        print("Running Server");
        run_server(context, qp_factory);
    } else {
        print("Running Client");
        string server_ip = get_server_ip(argc, argv);
        run_client(context, qp_factory, server_ip);
    }

    print("Exiting");
    delete qp_factory;
    delete context;

    return 0;
}


