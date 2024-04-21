#include "libs.hpp"
#include "EpollServer.hpp"

/*---------------------------------------------------------------------------*/
//Public functions
/*---------------------------------------------------------------------------*/

//constructor
EpollServer::EpollServer(int port) : socketfd(-1), epollfd(-1)
{
    initServer(port);
}

//destructor - clean up open fds
EpollServer::~EpollServer()
{
    if (socketfd != -1)
        close(socketfd);
    if (epollfd != -1)
        close(epollfd);
}

/*---------------------------------------------------------------------------*/
//Private functions
/*---------------------------------------------------------------------------*/

void EpollServer::initServer(int port)
{
    //int socket(int domain, int type, int protocol);
    //AF_INET - IPV4
    //SOCK_NONBLOCK - Set  the O_NONBLOCK file status flag on the open file descriptor. 
    //Using this flag saves extra calls to fcntl(2) to achieve the same result.
    socketfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (socketfd == -1)
    {
        perror("socket");
        throw std::runtime_error("Failed to create socket.");
    }

    //sockaddir_in is protocol specific, in this case IPV4
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;

    //INADDR_ANY is used to instruct listening socket to bind to all
    //available interfaces, change later if necessary
    //such as addr.sin_addr.s_addr = inet_addr("localhost");
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    //not sure why htons vs htonl
    //host to network short vs host to network long
    //try with short first
    addr.sin_port = htons(port);

    if (bind(socketfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == 1)
    {
        perror("bind");
        throw std::runtime_error("Failed to bind socket.");
    }

    if (listen(socketfd, SOMAXCONN) == -1)
    {
        perror("listen");
        throw std::runtime_error("Failed to listen");
    }

    //input to epoll_create is a hint to the number of added file descriptors
    //for the kernel to decide how much space to initially allocate
    //kernel will allocate more if necessary, but still has to be greater than 0
    //for backwards compatibility
    epollfd = epoll_create(10);
    if (epollfd == -1)
    {
        perror("epoll_create1");
        throw std::runtime_error("Failed to create epoll file descriptor");
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = socketfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, socketfd, &ev) == -1)
    {
        perror("epoll_ctl: listen_sock");
        throw std::runtime_error("Failed to add listening socket to epoll");
    }
    std::cout << "Starting server on port: " << port << std::endl;
}

bool EpollServer::readFromConnection(int fd)
{
    char buffer[1024];
    int bytesRead;
    do
    {
        //0 is a flag for no special flags
        //might consider using MSG_WAITALL flag to ensure all bytes are read
        bytesRead = recv(fd, buffer, 1024, 0);
        if (bytesRead < 1)
        {
            if (bytesRead == -1)
            {
                perror("read");
                throw std::runtime_error("Error reading from connection");
            }
            close(fd);
            return false;
        }
        else
        {
            buffer[bytesRead] = '\0';
            std::cout << "Received: " << buffer << std::endl;
            //send to other functions to parse and respond
            //processRequest(buffer);
	    }
        const char* httpResponse = "HTTP/1.1 200 OK\r\n"
                                    "Content-Type: text/plain\r\n"
                                    "Content-Length: 13\r\n"
                                    "\r\n"
                                    "Hello, World!";
        std::cout << "Sending response to fd " << fd << std::endl;
        ssize_t bytesSent = send(fd, httpResponse, strlen(httpResponse), 0);
        if (bytesSent == -1) {
            perror("send");
        }
    } while (bytesRead > 0);
    return true;
}

void EpollServer::writeToConnection(int fd, const char* buffer, size_t size)
{
    // size_t sentBytes = 0;
    // size_t totalBytes;

    // while (totalBytes < size)
    // {
    //     sentBytes = send(fd, buffer + totalBytes, size - totalBytes, 0);
    //     if (sentBytes == static_cast<size_t>(-1))
    //     {
    //         perror("write");
    //         throw std::runtime_error("Error writing to connection");
    //     }
    //     totalBytes += sentBytes;
    // }
    // if (totalBytes != size)
    // {
    //     throw std::runtime_error("Failed to send all bytes to connection");
    // }
    (void)buffer, (void)size;
    const char* httpResponse = "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/plain\r\n"
                                "Content-Length: 13\r\n"
                                "\r\n"
                                "Hello, World!";
    std::cout << "Sending response to fd " << fd << std::endl;
    ssize_t bytesSent = send(fd, httpResponse, strlen(httpResponse), 0);
    if (bytesSent == -1) {
        perror("send");
    }
}

void EpollServer::runServer()
{
    struct epoll_event ev, events[MAX_EVENTS];
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int numfds, connection;

    while (1)
    {
        numfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (numfds == -1)
        {
            perror("epoll_wait");
            throw std::runtime_error("Error in epoll_wait");
        }

        for (int i = 0; i < numfds; i++)
        {
            if (events[i].data.fd == socketfd)
            {
                connection = accept(socketfd, reinterpret_cast<struct sockaddr*>(&client_addr), &client_len);
                if (connection == -1)
                {
                    perror("accept");
                    throw std::runtime_error("Error accepting new connection");
                }

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = connection;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, connection, &ev) == -1)
                {
                    perror("epoll_ctl: conn_sock");
                    throw std::runtime_error("Error adding new connection socket to epoll");
                }
            }
            else
            {
                if (events[i].events & EPOLLIN)
                {
                    //read from connection
                    bool readyToSend = readFromConnection(events[i].data.fd);
                    if (readyToSend)
                    {
                        // If ready to send, enable EPOLLOUT event for this fd
                        ev.events = EPOLLOUT | EPOLLET;
                        ev.data.fd = events[i].data.fd;
                        epoll_ctl(epollfd, EPOLL_CTL_MOD, events[i].data.fd, &ev);
                    }
                }
                if (events[i].events & EPOLLOUT)
                {
                    const char *httpResponse = "bla";
                    writeToConnection(events[i].data.fd, httpResponse, strlen(httpResponse));
                }
            }
        }
    }
}
