#include <iostream>
#include <cstring>
#include <cstdlib>
#include "epoll_server.h"

void PrintUsage(const char* prog) {
    std::cout << "Usage: " << prog << " [-p port] [-t thread_num]" << std::endl;
    std::cout << "  -p port       : listening port (default 8080)" << std::endl;
    std::cout << "  -t thread_num : number of threads (default 4)" << std::endl;
}

int main(int argc, char* argv[]) {
    int port = 8080;
    int thread_num = 4;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            thread_num = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            PrintUsage(argv[0]);
            return 0;
        }
    }

    EpollServer server(port, thread_num);
    server.Start();

    return 0;
}
