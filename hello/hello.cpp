#include <iostream>

#include <kos.h>

#define PROJECT_NAME "hello"

KOS_INIT_FLAGS(INIT_DEFAULT);

int main(int argc, char **argv) {
    std::cout << "argc is " << argc << "\n";
    std::cout << "This is project " << PROJECT_NAME << ".\n";
    return 0;
}
