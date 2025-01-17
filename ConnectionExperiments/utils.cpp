//
// Created by liampilot on 05/05/2020.
//
#include "utils.h"

#include <iostream>
#include <random>
#include <regex>
#include <fcntl.h>
#include <unistd.h>

namespace utils {
    std::vector<char> GenerateRandomData(size_t len) {
        std::vector<char> s(len);
        dev_random_data(s.data(), len);
//        std::default_random_engine rand {};
//        std::uniform_int_distribution<int> distribution(65, 90);
//        std::cout << "Finished filling data\n";
        return s;
    }

    void random_data(char* s, size_t len) {
        static std::default_random_engine rand {};
        static std::uniform_int_distribution<int> distribution(65, 90);

        for (int i = 0; i < len; ++i) {
            s[i] = (char) distribution(rand);
        }

        s[len] = 0;
    }


    void dev_random_data(char* data, size_t size) {
        int fd = open("/dev/urandom", O_RDONLY);
        read(fd, data, size);
    }

    int get_ib_card_address(ifaddrs* out_address) {
        ifaddrs* address;
        int status = getifaddrs(&address);

        if (status < 0) {
            std::cout << "Could not get NIC address\n" << std::endl;
            return status;
        }

        bool found = false;
        for (; address != nullptr; address = address->ifa_next) {
            if (!found && address->ifa_name[2] != '\0' && address->ifa_name[3] == '1' && address->ifa_name[4] == '0') {
                found = true;
                *out_address = *address;
            }
        }

        if (!found) {
            return -1;
        } else {
            return 0;
        }
    }

    std::unique_ptr<infinity::memory::Buffer> create_large_buffer(size_t data_size,
            std::unique_ptr<infinity::core::Context>& context) {
        auto data = utils::GenerateRandomData(data_size);
        return std::make_unique<infinity::memory::Buffer>(context.get(), data.data(), data_size * sizeof(char));
    }
}
