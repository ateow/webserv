#include "libs.hpp"

class EpollServer
{
    public:
        EpollServer(int port);
        ~EpollServer();
        void runServer();

    private:
        //vars
        int socketfd;
        int epollfd;

        //methods
        void initServer(int port);
}
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
    socketfd = socket(AF_INET, SOCK_NONBLOCK, 0);
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
    addr.sin_addr.s_addr = INADDR_ANY;
    
    //not sure why htons vs htonl
    //host to network short vs host to network long
    //try with short first
    addr.sin_port = htons(port);

    if (bind(socketfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == 1)
    {
        perror("bind");
        throw std::runtime_error("Failed to bind socket.");
    }

    if (listen(listen_sock, SOMAXCONN) == -1)
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
}