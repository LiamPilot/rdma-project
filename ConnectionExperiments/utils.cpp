//
// Created by liampilot on 05/05/2020.
//
#include "utils.h"

#include <iostream>
#include <random>
#include <regex>

namespace utils {
    std::unique_ptr<char[]> GenerateRandomData(int len) {
        char* s = new char[len];
        std::default_random_engine rand {};
        std::uniform_int_distribution<int> distribution(65, 90);

        for (int i = 0; i < len; ++i) {
            s[i] = (char) distribution(rand);
        }

        std::cout << "Finished filling data\n";

        s[len] = 0;
        return std::unique_ptr<char []> {s};
    }

    int get_ib_card_address(ifaddrs* address) {
        int status = getifaddrs(&address);

        if (status < 0) {
            std::cout << "Could not get NIC address\n" << std::endl;
            return status;
        }

        for (; address->ifa_next != NULL; address = address->ifa_next) {
            std::regex pattern {R"(ib\d{1})"};
            if (std::regex_match(address->ifa_name, pattern)) {
                return 0;
            }
            address = address->ifa_next;
        }

        // No ib device found
        return -1;
    }

    std::unique_ptr<infinity::memory::Buffer> create_large_buffer(int data_size,
            std::unique_ptr<infinity::core::Context>& context) {
        auto data = utils::GenerateRandomData(data_size);
        return std::make_unique<infinity::memory::Buffer>(context.get(), data.get(), data_size * sizeof(char));
    }
}
