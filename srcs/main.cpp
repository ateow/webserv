#include "libs.hpp"
#include "EpollServer.hpp"

int main()
{
	EpollServer webserv(8081);
	webserv.runServer();
	return 0;
}
