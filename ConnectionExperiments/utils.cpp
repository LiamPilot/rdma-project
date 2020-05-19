//
// Created by liampilot on 05/05/2020.
//
#include "utils.h"

#include <iostream>
#include <random>

namespace utils {
    void print(const std::string &str) {
        std::cout << str << '\n';
    }

    char *GenerateRandomData(int len) {
        char *s = (char *) malloc(sizeof(char) * len);
        std::default_random_engine rand;
        std::uniform_int_distribution<int> distribution(65, 90);

        for (int i = 0; i < len; ++i) {
            s[i] = (char) distribution(rand);
        }

        utils::print("Finished filling data");

        s[len] = 0;
        return s;
    }
}
