#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#include <cstring>
#include <iostream>
#include <string>

#define BUFFER_SIZE 8192
#define TIMEOUT_SEC 10

int listen_fd = -1;

void handle_signal(int sig) {
    if (listen_fd != -1)
        close(listen_fd);
    exit(0);
}

bool valid_port(const std::string &p) {
    try {
        int port = std::stoi(p);
        return port > 1023 && port <= 65535;
    } catch (...) {
        return false;
    }
}

int main(int argc, char* argv[]) {

    if (argc != 3) {
        std::cerr << "ERROR: Invalid arguments\n";
        return 1;
    }

    std::string port_str = argv[1];
    std::string directory = argv[2];

    if (!valid_port(port_str)) {
        std::cerr << "ERROR: Invalid port\n";
        return 1;
    }

    int port = std::stoi(port_str);

    signal(SIGTERM, handle_signal);
    signal(SIGQUIT, handle_signal);

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        std::cerr << "ERROR: socket failed\n";
        return 1;
    }

    int yes = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listen_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "ERROR: bind failed\n";
        return 1;
    }

    if (listen(listen_fd, SOMAXCONN) < 0) {
        std::cerr << "ERROR: listen failed\n";
        return 1;
    }

    int connection_id = 0;

    while (true) {

        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);

        int clientSockfd = accept(listen_fd,
                                  (sockaddr*)&clientAddr,
                                  &clientLen);

        if (clientSockfd < 0)
            continue;

        connection_id++;

        // Set 10 second timeout
        struct timeval timeout;
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;
        setsockopt(clientSockfd,
                   SOL_SOCKET,
                   SO_RCVTIMEO,
                   &timeout,
                   sizeof(timeout));

        std::string filename =
            directory + "/" +
            std::to_string(connection_id) +
            ".file";

        int file_fd = open(filename.c_str(),
                           O_WRONLY | O_CREAT | O_TRUNC,
                           0644);

        if (file_fd < 0) {
            close(clientSockfd);
            continue;
        }

        char buffer[BUFFER_SIZE];
        bool timeout_occured = false;

        while (true) {
            ssize_t bytes =
                recv(clientSockfd,
                     buffer,
                     BUFFER_SIZE,
                     0);

            if (bytes > 0) {
                write(file_fd, buffer, bytes);
            }
            else if (bytes == 0) {
                break; // client closed normally
            }
            else {
                if (errno == EWOULDBLOCK ||
                    errno == EAGAIN) {
                    timeout_occured = true;
                }
                break;
            }
        }

        close(clientSockfd);

        if (timeout_occured) {
            close(file_fd);
            file_fd = open(filename.c_str(),
                           O_WRONLY | O_TRUNC);
            write(file_fd, "ERROR", 5);
        }

        close(file_fd);
    }

    return 0;
}