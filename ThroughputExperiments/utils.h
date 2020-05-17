//
// Created by liampilot on 05/05/2020.
//

#ifndef THROUGHPUTEXPERIMENTS_UTILS_H
#define THROUGHPUTEXPERIMENTS_UTILS_H

#endif //THROUGHPUTEXPERIMENTS_UTILS_H

#include <string>

#define TUPLE_SIZE 78
#define NUM_TUPLES 10000000
#define PORT 8001
#define TEST_LENGTH 100000
namespace utils {
    void print(const std::string&);
    char* GenerateRandomData(int len);
    const static int buffer_sizes[] = {1024,
                                       1024 << 1,
                                       1024 << 2,
                                       1024 << 3,
                                       1024 << 4,
                                       1024 << 5,
                                       1024 << 6,
                                       1024 << 7,
                                       1024 << 8,
                                       1024 << 9,
                                       1024 << 10,
                                       1024 << 11,
                                       1024 << 12,
                                       1024 << 13,
                                       1024 << 14,
                                       1024 << 15};
}
