#include "libs.hpp"
#include "EpollServer.hpp"

int main()
{
	EpollServer webserv(8080);
	webserv.runServer();
	return 0;
}
