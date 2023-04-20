//
// Created by Abdelhak El moussaoui on 1/31/23.
//

#ifndef SERVER_HPP
#define SERVER_HPP
#include "../Parsing/requestParser.hpp"
#include "../httpResponse/HttpResponse.hpp"
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define GETSOCKETERRNO() (errno)
#define MAX_REQUEST_SIZE 2047 // 2g
#define TIMEOUT 5.0
#define BSIZE 1024
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <iostream>
#include <fstream>
typedef int SOCKET;
#include <poll.h>
static struct client_info *clients = nullptr;

struct client_info
{
	socklen_t address_length;
	struct sockaddr_storage address;
	SOCKET socket;

	char request[2047];
	int received;
	//this index for is the client soket have more things for read or not
	int index;
	char *name;
	int fd;
	HttpResponse	*response;
	Client	ServClient;
	requestParser *requesting;
	int size_buffer;
	char *req;
	bool headr_bool;
	int RestOfHexa;
	std::string hex;
	size_t rec;
	int maxbody;
	// ConfigParsing *parser;
	struct client_info *next;
};

class Server {
public:
	void start_event();
	Client ServClient;
    void    sentCgi(client_info *client);
	void	setConfigPath(const std::string &Path);
	struct client_info *client;
	void get_parse (client_info *client , int r);
	void parseRequest(int &r);
	void drop_client(struct client_info *client);
	void CheckForcgi(client_info *client);
	bool ChunkedCase(client_info *Client, int r);
	bool BoundryCases(client_info *client, int r);
	bool StoryTheBody(client_info *client, int r);
private:
	fd_set reads;
	fd_set writes;
	std::string	configPath;
//	fd_set reads;
//	fd_set writes;
//	int byteSent;
	std::vector<requestParser>	allRequests;
//	HttpResponse response;
	SOCKET create_socket(const char* host, const char *port);
	void wait_on_clients(SOCKET server);
	void readData();
//	void chunksSending();
	ConfigParsing *parser;
};


#endif
