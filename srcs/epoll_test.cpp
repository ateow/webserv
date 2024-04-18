#include "libs.hpp"
#define MAX_EVENTS 10

class EpollServer {
public:
    explicit EpollServer(int port) : listen_sock(-1), epollfd(-1) {
        setupServer(port);
    }

    ~EpollServer() {
        if (listen_sock != -1) close(listen_sock);
        if (epollfd != -1) close(epollfd);
    }

    void run() {
        struct epoll_event ev, events[MAX_EVENTS];
        int nfds, conn_sock;
        socklen_t addrlen;
        struct sockaddr_in addr;

        for (;;) {
            nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
            if (nfds == -1) {
                perror("epoll_wait");
                throw std::runtime_error("Error during epoll_wait");
            }

            for (int n = 0; n < nfds; ++n) {
                if (events[n].data.fd == listen_sock) {
                    addrlen = sizeof(addr);
                    conn_sock = accept(listen_sock, reinterpret_cast<struct sockaddr*>(&addr), &addrlen);
                    if (conn_sock == -1) {
                        perror("accept");
                        throw std::runtime_error("Error accepting new connection");
                    }
                    setNonBlocking(conn_sock);
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = conn_sock;
                    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
                        perror("epoll_ctl: conn_sock");
                        throw std::runtime_error("Error adding new connection socket to epoll");
                    }
                } else {
                    useFd(events[n].data.fd);
                }
            }
        }
    }

private:
    int listen_sock;
    int epollfd;

    void setupServer(int port) {
        listen_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_sock == -1) {
            perror("socket");
            throw std::runtime_error("Failed to create socket");
        }

        struct sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        if (bind(listen_sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == -1) {
            perror("bind");
            throw std::runtime_error("Failed to bind");
        }

        if (listen(listen_sock, SOMAXCONN) == -1) {
            perror("listen");
            throw std::runtime_error("Failed to listen");
        }

        epollfd = epoll_create1(0);
        if (epollfd == -1) {
            perror("epoll_create1");
            throw std::runtime_error("Failed to create epoll file descriptor");
        }

        struct epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = listen_sock;
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
            perror("epoll_ctl: listen_sock");
            throw std::runtime_error("Failed to add listening socket to epoll");
        }
    }

    void setNonBlocking(int sock) {
        int flags = fcntl(sock, F_GETFL, 0);
        if (flags == -1) {
            perror("fcntl");
            throw std::runtime_error("Failed to get flags for socket");
        }

        flags |= O_NONBLOCK;
        if (fcntl(sock, F_SETFL, flags) == -1) {
            perror("fcntl");
            throw std::runtime_error("Failed to set non-blocking socket");
        }
    }

    void useFd(int fd) {
        // Implement the logic to handle data I/O
        std::cout << "Handling fd " << fd << std::endl;
    }
};

int main() {
    std::cout << "Testing" << std::endl;
    try {
        EpollServer server(8080);
        server.run();
    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
