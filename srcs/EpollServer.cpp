#include "libs.hpp"
#include "EpollServer.hpp"

/*---------------------------------------------------------------------------*/
//Public functions
/*---------------------------------------------------------------------------*/

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
            if (errno == EINTR)
            {
                std::cout << "Shutting down server" << std::endl;
                return ;
            } 
            perror("epoll_wait");
            throw std::runtime_error("Error in epoll_wait");
        }

        for (int i = 0; i < numfds; i++)
        {
            std::cout << "====================================" << std::endl;
            std::cout << "Event on fd: " << events[i].data.fd << std::endl;
            if (std::find(socketfds.begin(), socketfds.end(), events[i].data.fd) != socketfds.end())
            {
                connection = accept(events[i].data.fd, reinterpret_cast<struct sockaddr*>(&client_addr), &client_len);
                if (connection == -1)
                {
                    perror("accept");
                    throw std::runtime_error("Error accepting new connection");
                }
                //setting timeout for connection
                struct timeval tv;
                tv.tv_sec = 10;
                tv.tv_usec = 0;
                if (setsockopt(connection, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
                {
                    perror("setsockopt");
                    throw std::runtime_error("Error setting timeout for connection");
                }
                std::cout << "Accepted new connection: " << connection << std::endl;
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = connection;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, connection, &ev) == -1)
                {
                    perror("epoll_ctl: conn_sock");
                    throw std::runtime_error("Error adding new connection socket to epoll");
                }
                std::cout << "Ready to read from connection: " << connection  << "on port: " << std::endl;
                clientfds.insert(connection);
                //read from connection
                try
                {
                    readFromConnection(connection);
                }
                catch(const std::exception& e)
                {
                    std::cerr << e.what() << '\n';
                }

            }
            else if (events[i].events & EPOLLIN)
            {
                std::cout << "Existing connection!" << std::endl;
                try
                {
                    readFromConnection(events[i].data.fd);
                }
                catch(const std::exception& e)
                {
                    close(events[i].data.fd);
                    std::cerr << e.what() << '\n';
                }
                
            }

            std::cout << "====================================" << std::endl << std::endl;
        }
    }
}

/*---------------------------------------------------------------------------*/
//Initialising functions
/*---------------------------------------------------------------------------*/

EpollServer::EpollServer(WebServerConfig serverconfig) : epollfd(-1)
{
    config = serverconfig;
    initServer();
}

//destructor - clean up open fds
EpollServer::~EpollServer()
{
    uint64_t u = 1;
    write(shutdownfd, &u, sizeof(uint64_t));
    for (size_t i = 0; i < this->config.servers.size(); ++i)
    {
        if (socketfds[i] != -1)
        {
            std::cout << "Closing socket: " << socketfds[i] << std::endl;
            close(socketfds[i]);
        }
    }
    for (std::set<int>::iterator it = clientfds.begin(); it != clientfds.end(); ++it)
    {
        if (*it != -1)
        {
            std::cout << "Closing client connection: " << *it << std::endl;
            close(*it);
        }
    }
    if (epollfd != -1)
    {
        std::cout << "Closing epoll file descriptor: " << epollfd << std::endl;
        close(epollfd);
    }
}

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
    shutdownfd = eventfd(0, EFD_NONBLOCK);
    if (shutdownfd == -1)
    {
        perror("eventfd");
        throw std::runtime_error("Failed to create shutdown file descriptor");
    }
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = shutdownfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, shutdownfd, &ev) == -1)
    {
        perror("epoll_ctl: shutdownfd");
        throw std::runtime_error("Failed to add shutdown file descriptor to epoll");
    }

    size_t serversstarted = 0;
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
        try
        {
            addSocket(server.port);
            ports.push_back(server.port);
            serversstarted++;
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    }
    if (serversstarted == 0)
    {
        throw std::runtime_error("Failed to start any servers");
    }
    else
    {
        std::cout << "Started " << serversstarted << " servers" << std::endl;
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
        throw std::runtime_error("Failed to create socket.\n");
    }
    int enable = 1;
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
        throw std::runtime_error("Failed to set socket options.\n");
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
        throw std::runtime_error("Failed to bind socket.\n");
    }
    std::cout << "Socket bound to port: " << port << std::endl;
    if (listen(socketfd, SOMAXCONN) == -1)
    {
        perror("listen");
        throw std::runtime_error("Failed to listen.\n");
    }
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = socketfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, socketfd, &ev) == -1)
    {
        perror("epoll_ctl: listen_sock");
        throw std::runtime_error("Failed to add listening socket to epoll.\n");
    }
    std::cout << "Listening on port: " << port << std::endl << std::endl;
}

/*---------------------------------------------------------------------------*/
//Send and receive functions
/*---------------------------------------------------------------------------*/

static bool isReadingDone(std::string &request)
{
    //if content-length present, read that many bytes
    if (request.find("Content-Length: ") != std::string::npos)
    {
        size_t pos = request.find("Content-Length: ") + 16;
        size_t end = request.find("\r\n", pos);
        size_t length = atoi(request.substr(pos, end - pos).c_str());
        if (request.length() == length)
            return true;
    }
    //if transfer-encoding present, read until 0\r\n\r\n
    else if (request.find("Transfer-Encoding: chunked") != std::string::npos)
    {
        size_t pos = request.find("\r\n") + 2;
        size_t end = request.find("\r\n", pos);
        size_t length = strtol(request.substr(pos, end - pos).c_str(), NULL, 16);
        if (length == 0)
        {
            request = request.substr(0, pos);
            return true;
        }
    }
    else if (request.find("\r\n\r\n") != std::string::npos)
    {
        return true;
    }
    return false;
}

void EpollServer::receiveData(int fd, std::vector<char> &buffer, size_t &totalBytes)
{
    size_t bytesExpected = 4;
    int bytesRead = 0;

    do
    {
        buffer.resize(totalBytes + bytesExpected);
        bytesRead = recv(fd, buffer.data() + totalBytes, bytesExpected, 0);
        if (bytesRead < 1)
        {
            if (bytesRead == -1)
            {
                close(fd);
                clientfds.erase(fd);
                perror("read");
                throw std::runtime_error("Error reading from connection");
            }
            break ;
        }
        totalBytes += bytesRead;
        std::string tmp(buffer.begin(), buffer.end());
        if (isReadingDone(tmp))
        {
            break ;
        }
    } while (static_cast<size_t>(bytesRead) == bytesExpected);
    buffer.resize(totalBytes);
}

static void extractFormData(std::vector<char> buffer, std::map<std::string, std::vector<char> > &files, std::string boundary)
{
    size_t boundaryLength = boundary.length();
    std::string body;
    std::vector<char> fileBuffer;
    
    //append to body until blank line is found
    while (body.find("\r\n\r\n") == std::string::npos)
    {
        body += buffer[0];
        buffer.erase(buffer.begin());
    }
    //add to fileBuffer until boundary is found
    while (fileBuffer.size() < boundaryLength || std::string(fileBuffer.end() - boundaryLength, fileBuffer.end()) != boundary)
    {
        fileBuffer.push_back(buffer[0]);
        buffer.erase(buffer.begin());
    }
    //remove boundary from fileBuffer and add to body
    body += std::string(fileBuffer.end() - boundaryLength, fileBuffer.end());
    fileBuffer.erase(fileBuffer.end() - boundaryLength, fileBuffer.end());
    if (buffer.size() == 4)
    {
        body += std::string(buffer.begin(), buffer.end());
        buffer.clear();
    }
    files[body] = fileBuffer;
    return (!buffer.empty() ? extractFormData(buffer, files, boundary) : void());
}

static void writeToFile(std::string filename, std::vector<char> fileBuffer)
{
    // std::ofstream file(filename.c_str(), std::ios::binary);
    std::ofstream file("testfile.jpeg", std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file" << std::endl;
        return ;
    }
    std::cout << "Writing to file: " << filename << std::endl;
    for (size_t i = 0; i < fileBuffer.size(); ++i)
    {
        file << fileBuffer[i];
    }
    file.close();
    std::cout << "File written" << std::endl;
}

static ServerConfig &getServer(int port, WebServerConfig &config)
{
    for (size_t i = 0; i < config.servers.size(); ++i)
    {
        if (config.servers[i].port == port)
        {
            return config.servers[i];
        }
    }
    throw std::runtime_error("Server not found or header mismatch");
}

bool EpollServer::readFromConnection(int fd)
{
    size_t totalBytes = 0;
    std::vector<char> buffer;
    std::map<std::string, std::vector<char> > files;
    // bool fileUpload;

    std::cout << "Reading from fd " << fd << std::endl;
    std::string header;
    // std::cout << "? " << std::endl;
    receiveData(fd, buffer, totalBytes);
    //headers are always separated from the body by \r\n\r\n
    //so we can split the buffer at that point
    // std::cout << "? " << std::endl;
    while (header.find("\r\n\r\n") == std::string::npos && !buffer.empty())
    {
        // std::cout << "! " << std::endl;
        header += buffer[0];
        buffer.erase(buffer.begin());
    }
    //look for host in header to find the port
    std::string _server = header.substr(header.find("Host: ") + 6, header.find("\r\n", header.find("Host: ")) - header.find("Host: ") - 6);
    int port = std::atoi(_server.substr(_server.find(":") + 1).c_str());
    std::cout << "Port: " << port << std::endl;
    ServerConfig &server = getServer(port, this->config);

    // std::cout << "Header: " << header << std::endl;
    //look through buffer for "Content-Type: multipart/form-data"
    if (header.find("Content-Type: multipart/form-data") != std::string::npos)
    {
        // fileUpload = true;
        std::cout << "File upload detected" << std::endl;
        size_t boundaryStart = header.find("boundary=");
        if (boundaryStart == std::string::npos)
        {
            std::cerr << "No boundary found in header" << std::endl;
            return false;
        }
        std::string boundary = "\r\n--" + header.substr(boundaryStart + 9, header.find("\r\n", boundaryStart) - boundaryStart - 9);
        std::cout << "Boundary: " << boundary << std::endl;
        extractFormData(buffer, files, boundary);
        for (std::map<std::string, std::vector<char> >::iterator it = files.begin(); it != files.end(); ++it)
        {
            size_t pos = it->first.find("filename=\"") + 10;
            size_t end = it->first.find("\"", pos);
            std::string filename = it->first.substr(pos, end - pos);
            if (!filename.empty())
                writeToFile(filename, it->second);
        }
    }
    else
    {
        //whole buffer can be turned into string
        std::string body(buffer.begin(), buffer.end());
        header += body;
    } 
    // std::cout << "------------------------------------" << std::endl;
    // std::cout << "Received: " << std::endl << header << std::endl;
    // std::cout << "------------------------------------" << std::endl;
    // std::cout << "Sending response to fd " << fd << std::endl;
    // if (fileUpload)
    // {
    //     do something
    // }
    // else
    request_data input = request_data(header.c_str(), server, files);
    // request_data input = request_data(buffer, config, host_directory, cgi_directory);
    respond_builder output = respond_builder(&input);
    std::string httpResponse = output.build_respond_data();
    // std::cout << output.build_respond_data() << std::endl;
    std::cout << httpResponse << std::endl;
    ssize_t bytesSent = send(fd, httpResponse.c_str(), httpResponse.length(), 0);
    if (bytesSent == -1) {
        perror("send");
    }
    if (header.find("Connection: close") != std::string::npos)
    {
        close(fd);
        std::cout << "Closing connection to fd " << fd << std::endl;
        clientfds.erase(fd);
        return true;
    }
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
