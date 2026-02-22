#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <cstring>
#include <iostream>
#include <fstream>

#define BUFFER_SIZE 8192
#define TIMEOUT_SEC 10

bool valid_port(const std::string &p) {
    try {
        int port = std::stoi(p);
        return port > 1023 && port <= 65535;
    } catch (...) {
        return false;
    }
}

int main(int argc, char* argv[]) {

    if (argc != 4) {
        std::cerr << "ERROR: Invalid arguments\n";
        return 1;
    }

    std::string hostname = argv[1];
    std::string port_str = argv[2];
    std::string filename = argv[3];

    if (!valid_port(port_str)) {
        std::cerr << "ERROR: Invalid port\n";
        return 1;
    }

    addrinfo hints{}, *res;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(hostname.c_str(),
                    port_str.c_str(),
                    &hints,
                    &res) != 0) {
        std::cerr << "ERROR: Hostname failed\n";
        return 1;
    }

    int sockfd =
        socket(res->ai_family,
               res->ai_socktype,
               res->ai_protocol);

    if (sockfd < 0) {
        std::cerr << "ERROR: socket failed\n";
        return 1;
    }

    // 10 second timeout
    struct timeval timeout;
    timeout.tv_sec = TIMEOUT_SEC;
    timeout.tv_usec = 0;
    setsockopt(sockfd,
               SOL_SOCKET,
               SO_SNDTIMEO,
               &timeout,
               sizeof(timeout));

    if (connect(sockfd,
                res->ai_addr,
                res->ai_addrlen) < 0) {
        std::cerr << "ERROR: connect failed\n";
        return 1;
    }

    freeaddrinfo(res);

    std::ifstream file(filename,
                       std::ios::binary);

    if (!file) {
        std::cerr << "ERROR: file open failed\n";
        return 1;
    }

    char buffer[BUFFER_SIZE];

    while (file.read(buffer, BUFFER_SIZE) ||
           file.gcount() > 0) {

        ssize_t sent =
            send(sockfd,
                 buffer,
                 file.gcount(),
                 0);

        if (sent < 0) {
            std::cerr << "ERROR: send failed\n";
            close(sockfd);
            return 1;
        }
    }

    close(sockfd);
    return 0;
}