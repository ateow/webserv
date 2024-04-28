#pragma once
#ifndef _EPOLLSERVER_HPP_
# define _EPOLLSERVER_HPP_

# define MAX_EVENTS 10
# include "libs.hpp"

class EpollServer
{
    public:
        EpollServer(); //jeremy needs to break up his classes so can include and reference
        ~EpollServer();
        void runServer();

    private:
        //vars
        int epollfd;
        std::vector<int>socketfds;

        //methods
        void initServer();
        void addSocket(int port);
        bool readFromConnection(int fd);
        void writeToConnection(int fd, const char* buffer, size_t size);
};

#endif
