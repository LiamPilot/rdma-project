#define GASNET_PAR

#include <iostream>

#include <bcl/bcl.hpp>

int main() {
    BCL::init();
    printf("Hello, BCL! I am rank %lu/%lu on host %s.\n",
           BCL::rank(), BCL::nprocs(), BCL::hostname().c_str());
    BCL::finalize();
    return 0;
}
