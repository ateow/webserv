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
        EpollServer(WebServerConfig serverconfig); //jeremy needs to break up his classes so can include and reference
        ~EpollServer();
        void runServer();
        
    private:
        //vars
        int epollfd;
        std::vector<int>socketfds;
        std::vector<int>ports;

        WebServerConfig config;

        //methods
        void initServer();
        void addSocket(int port);
        bool readFromConnection(int fd, ServerConfig &server);
        void writeToConnection(int fd, const char* buffer, size_t size);
        bool isReadingDone(std::string &request);
};

#endif
