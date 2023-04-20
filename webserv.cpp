#include <iostream>
#include "Server/Server.hpp"
/*
	- ! bug Report
   todo :
		!404 with cgi;
		!check byte recieve when no content-length 
 	  	add mime type for .cpp
        when autoIndex is on or off and there is a CGI

 */

int main(int ac, char **av)
{
	Server virtualServer;

	try {
		if (ac == 2)
		{
			std::cout << av[1] << std::endl;
			virtualServer.setConfigPath(av[1]);
		}
		virtualServer.start_event();
	}
	catch (const std::exception &e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}