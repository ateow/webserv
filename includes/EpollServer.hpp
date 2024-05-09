#pragma once
#ifndef _EPOLLSERVER_HPP_
# define _EPOLLSERVER_HPP_

# define MAX_EVENTS 10
# include "libs.hpp"
# include "load_config.hpp"
# include "request_parser.hpp"
# include "respond_builder.hpp"

class WebServerConfig;
class ServerConfig;

class EpollServer
{
    public:
        EpollServer(WebServerConfig serverconfig);
        ~EpollServer();
        void runServer();

        int signalHandler(int signum);
        
    private:
        //vars
        int epollfd;
        int shutdownfd;
        std::set<int> clientfds;
        std::vector<int>socketfds;
        std::vector<int>ports;

        WebServerConfig config;

        //methods
        void initServer();
        void addSocket(int port);
        void receiveData(int fd, std::vector<char> &buffer, size_t &totalBytes);
        bool readFromConnection(int fd);
        void writeToConnection(int fd, const char* buffer, size_t size);
};

#endif
