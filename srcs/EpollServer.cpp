#include "libs.hpp"
#include "EpollServer.hpp"

/*---------------------------------------------------------------------------*/
//Public functions
/*---------------------------------------------------------------------------*/
//constructor todo: take in parsed config file to setup
//one epoll multiple soxkets for multiple ports
EpollServer::EpollServer(WebServerConfig serverconfig) : epollfd(-1)
{
    config = serverconfig;
    initServer();
}

//destructor - clean up open fds
EpollServer::~EpollServer()
{
    for (size_t i = 0; i < this->config.servers.size(); ++i)
    {
        if (socketfds[i] != -1)
            close(socketfds[i]);
    }
    if (epollfd != -1)
        close(epollfd);
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
            std::cout << "Event on fd: " << events[i].data.fd << std::endl;
            if (std::find(socketfds.begin(), socketfds.end(), events[i].data.fd) != socketfds.end())
            {
                connection = accept(events[i].data.fd, reinterpret_cast<struct sockaddr*>(&client_addr), &client_len);
                if (connection == -1)
                {
                    perror("accept");
                    throw std::runtime_error("Error accepting new connection");
                }
                std::cout << "Accepted new connection: " << connection << std::endl;
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = connection;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, connection, &ev) == -1)
                {
                    perror("epoll_ctl: conn_sock");
                    throw std::runtime_error("Error adding new connection socket to epoll");
                }
                std::cout << "Ready to read from connection: " << connection << std::endl;
                //read from connection
                for (size_t j = 0; j < this->config.servers.size(); ++j)
                {
                    if (socketfds[j] == events[i].data.fd)
                    {
                        ServerConfig &server = this->config.servers[j];
                        readFromConnection(connection, server);
                        break ;
                    }
                }
            }
            else
            {
                writeToConnection(events[i].data.fd, "Hello world!\n", 14);
            }
        }
    }
}

/*---------------------------------------------------------------------------*/
//Private functions
/*---------------------------------------------------------------------------*/

void EpollServer::initServer()
{
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
    std::cout << this->config.servers.size() << std::endl;
    for (size_t i = 0; i < this->config.servers.size(); ++i)
    {
        const ServerConfig &server = this->config.servers[i];

        std::cout << "Server " << i + 1 << ":\n";
        std::cout << "  Host: " << server.host << "\n";
        std::cout << "  Port: " << server.port << "\n";
        std::cout << "  Server Names: ";
        for (size_t j = 0; j < server.server_names.size(); ++j)
        {
            std::cout << server.server_names[j] << (j + 1 < server.server_names.size() ? ", " : "\n");
        }
        std::cout << "  Client Body Size Limit: " << server.limit_client_body_size << "\n";
        std::cout << "  Default Error Pages:\n";
        for (std::map<int, std::string>::const_iterator it = server.default_error_pages.begin(); it != server.default_error_pages.end(); ++it)
        {
            std::cout << "    " << it->first << ": " << it->second << "\n";
        }
        ports.push_back(server.port);
        addSocket(server.port);
    }

}

void EpollServer::addSocket(int port)
{
    //int socket(int domain, int type, int protocol);
    //AF_INET - IPV4
    //SOCK_NONBLOCK - Set  the O_NONBLOCK file status flag on the open file descriptor. 
    //Using this flag saves extra calls to fcntl(2) to achieve the same result.
    int socketfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (socketfd == -1)
    {
        perror("socket");
        throw std::runtime_error("Failed to create socket.");
    }
    int enable = 1;
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
        throw std::runtime_error("Failed to set socket options.");
    }
    socketfds.push_back(socketfd);
    std::cout << "Socket : " << socketfd << " created." << std::endl;

    //sockaddir_in is protocol specific, in this case IPV4
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;

    //INADDR_ANY is used to instruct listening socket to bind to all
    //available interfaces, change later if necessary
    //such as addr.sin_addr.s_addr = inet_addr("localhost");
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(port);

    if (bind(socketfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == -1)
    {
        perror("bind");
        throw std::runtime_error("Failed to bind socket.");
    }
    std::cout << "Socket bound to port: " << port << std::endl;
    if (listen(socketfd, SOMAXCONN) == -1)
    {
        perror("listen");
        throw std::runtime_error("Failed to listen");
    }
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = socketfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, socketfd, &ev) == -1)
    {
        perror("epoll_ctl: listen_sock");
        throw std::runtime_error("Failed to add listening socket to epoll");
    }
    std::cout << "Listening on port: " << port << std::endl << std::endl;
}

/*---------------------------------------------------------------------------*/
//Send and receive functions
/*---------------------------------------------------------------------------*/

bool EpollServer::readFromConnection(int fd, ServerConfig &server)
{
    char buffer[1024];
    int bytesRead;
    do
    {
        //0 is a flag for no special flags
        //might consider using MSG_WAITALL flag to ensure all bytes are read
        std::cout << "Reading from fd " << fd << std::endl;
        bytesRead = recv(fd, buffer, 1023, 0);
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
            std::cout << std:: endl << "Received: " << std::endl << buffer << std::endl;
            std::cout << "Sending response to fd " << fd << std::endl;
            std::cout << "Current port: " << server.port << std::endl;
            // std::string host_directory = "./";
            // std::string cgi_directory = "../cgi-bin/";
            request_data input = request_data(buffer, config, host_directory, cgi_directory);
            respond_builder output = respond_builder(&input, host_directory);
            std::string httpResponse = output.build_respond_data();
            // std::cout << output.build_respond_data() << std::endl;
            // std::cout << httpResponse << std::endl;
            ssize_t bytesSent = send(fd, httpResponse.c_str(), httpResponse.length(), 0);
            if (bytesSent == -1) {
                perror("send");
            }
	    }
    } while (bytesRead > 0);
    return true;
}

void EpollServer::writeToConnection(int fd, const char* buffer, size_t size)
{
    size_t sentBytes = 0;
    size_t totalBytes = 0;

    while (totalBytes < size)
    {
        sentBytes = send(fd, buffer + totalBytes, size - totalBytes, 0);
        if (sentBytes == static_cast<size_t>(-1))
        {
            perror("write");
            throw std::runtime_error("Error writing to connection");
        }
        totalBytes += sentBytes;
    }
    if (totalBytes != size)
    {
        throw std::runtime_error("Failed to send all bytes to connection");
    }
}
