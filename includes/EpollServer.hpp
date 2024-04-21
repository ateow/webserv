#pragma once
#ifndef _EPOLLSERVER_HPP_
# define _EPOLLSERVER_HPP_

# define MAX_EVENTS 10

class EpollServer
{
    public:
        explicit EpollServer(int port);
        ~EpollServer();
        void runServer();

    private:
        //vars
        int socketfd;
        int epollfd;

        //methods
        void initServer(int port);
        void readFromConnection(int fd);
        void writeToConnection(int fd, const char* buffer, size_t size);
};

#endif
