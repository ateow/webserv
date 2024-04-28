#include "libs.hpp"
#include "EpollServer.hpp"

int main()
{
	EpollServer webserv;
	webserv.runServer();
	return 0;
}
