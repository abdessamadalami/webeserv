//
// Created by Abdelhak El moussaoui on 1/31/23.
//


#include <sys/select.h>
#include "Server.hpp"


SOCKET Server::create_socket(const char* host, const char *port)
{
	std::cout << "starting the server ....\n" << std::endl;
	struct addrinfo hints;
	struct addrinfo *bind_address;
	int		ecode;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = IPPROTO_TCP;

	if ((ecode = getaddrinfo(host, port, &hints, &bind_address)) != 0)
		throw std::runtime_error("Error in getaddrinfo()");
//	getaddrinfo(host, port, &hints, &bind_address);
	SOCKET socket_listen;
	socket_listen = socket(bind_address->ai_family,
						   bind_address->ai_socktype, bind_address->ai_protocol);
	if (!ISVALIDSOCKET(socket_listen))
		throw std::runtime_error("socket() failed");
	int optval = 1;
	if (setsockopt(socket_listen, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) == -1)
		throw std::runtime_error("setsockopt() failed");
	if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen))
		throw std::runtime_error("bind() failed.");
	freeaddrinfo(bind_address);
	std::cout << "listening ......" << std::endl;
	if (listen(socket_listen, 100000) < 0)
		throw std::runtime_error("listen() failed.");
	return socket_listen;
}

//!!!! An In-Depth Overview of TCP Connections,
void Server::wait_on_clients(SOCKET server)
{
	timeval listening_timeout;
	listening_timeout.tv_sec = 2;
	listening_timeout.tv_usec = 0;
//	fd_set readFd, writeFD; // for
	FD_ZERO(&reads);
	FD_SET(server, &reads);

	FD_ZERO(&writes);
	//FD_SET(server, &writes);
	SOCKET max_socket = server;
	struct client_info *ci = clients;
	while(ci)
	{
		FD_SET(ci->socket, &writes);
		FD_SET(ci->socket, &reads);
		if (ci->socket > max_socket)
			max_socket = ci->socket;
		ci = ci->next;
	}
	std::cout << "in select.........\n";
	if (select(max_socket+1, &reads, &writes, 0, &listening_timeout) < 0)
		throw std::runtime_error("select() failed.");
	std::cout << " selected.........\n";
//	return reads;
}

const char *get_client_address(struct client_info *ci)
{
	static char address_buffer[100]; // thread here is danger to use -> static
	getnameinfo((struct sockaddr*)&ci->address,
				ci->address_length,
				address_buffer, sizeof(address_buffer), 0, 0,
				NI_NUMERICHOST);
	return address_buffer;
}

//!for searching for a client if is exiest ->return it esle creat a new cleant info and add it (front)to the liked list
struct client_info *get_client(SOCKET s)
{
	struct client_info *ci = clients;
	while(ci)
	{
		if (ci->socket == s)
			break;
		ci = ci->next;
	}
	if (ci) return ci;
	struct client_info *n = (struct client_info*) calloc(1, sizeof(struct client_info));
	if (!n)
		throw std::runtime_error("Out of memory.");
	n->address_length = sizeof(n->address);
	n->next = clients;
	n->index = 0;
	n->name = 0;
	n->requesting = new requestParser();
	bzero(n->request, 2047);
	n->RestOfHexa = 110;
	bzero(n->request, 2048);
	n->requesting = new requestParser();
	clients = n;
	return n;
}

//! for remove a client in liskedlist of cliensts
void Server::drop_client(struct client_info *client)
{
	CLOSESOCKET(client->socket);
	struct client_info **p = &clients;

	while(*p)
	{
		if (*p == client) {
			std::cout << " I AM DROPING A CLIENT it have this ip" << get_client_address(client) << std::endl;
//			FD_CLR(client->socket, &reads);
			*p = client->next;
//			close(client->socket);
			free(client);
			return;
		}
		p = &(*p)->next;
	}
	fprintf(stderr, "drop_client not found %d.\n" , client->socket);
}

void Server::parseRequest(int &r)
{
	client->requesting = client->requesting->requestParsing(client->request);
	if( client->requesting->headers["Content-Length"].length() != 0 && client->maxbody < atoi(client->requesting->headers["Content-Length"].c_str()))
	{
		std::cout <<client->maxbody  << std::endl;
		std::cout << "exit from parce req \n";
		exit(0);
	}
	client->requesting->req = client->request;
	client->response->bites = r;
	client->response->setRequest(*(client->requesting));
	client->response->response();
	client->ServClient = client->response->getServClient();
	std::cout << "....................." << std::endl;
	//std::cout << "Ser" << client->ServClient.it->first << std::endl;
}


void writeInFile(char *request , int r , char *name)
{
	FILE *P;
	P = fopen(name, "w");
	fwrite(request, 1 , r ,P);
	fclose(P);
}

int getTheRestofhexa(client_info *Client, int i , int r)
{
	if(Client->RestOfHexa == -2)
		Client->RestOfHexa = 2;
	int size = 0;
	while ((i) < r)
	{
		if(isxdigit(Client->request[i]) && Client->RestOfHexa != 4)
		{
			Client->hex+=Client->request[i];
			size++;
		
		}
		else if (Client->request[i] == '\r' && Client->request[(i) + 1] == '\n')
		{
			Client->RestOfHexa+=2;
			i++;
			size+=2;
		}
		else if (Client->RestOfHexa == 4)
		{
			Client->response->SizeOfChunked = hexadecimalToDecimal(Client->hex);
			Client->response->BytesWritten = 0;
			Client->RestOfHexa = 0;
			if(Client->hex.length() == 0)
			{
				std::cout << "I am exiting from the length == 0\n";
				exit(0);
			}
			return size;
		}
		(i)++;
	}
	return size;
}

void GetTheHex(client_info *Client , int& r , int i) // o(n)
{
	int index = 0;
	int HexaIndex = 0;
	int rest = 0;
	bool HexaForm = false;

	Client->RestOfHexa = 0;
	while ((i) < r)
	{
		if ( (i < r - 1) && Client->RestOfHexa == 0 &&  Client->request[i] == '\r' && Client->request[i + 1] == '\n')
		{
			HexaIndex = i;
			HexaIndex+=2;
			index = 0;
			Client->RestOfHexa = 2;
			Client->hex= "";
			while(Client->request[HexaIndex] != '\r') 
			{
				if (HexaIndex > (r - 1)) //! the case if the rest of case in the next chunked 
				{
					Client->RestOfHexa = -2;
					return;
				}
				if(!isxdigit(Client->request[HexaIndex]) || index > 6) //! the case when we have \r\n in the file 
				{
					std::cout << " x second : "<< HexaIndex <<  " r " << index << std::endl;
					Client->RestOfHexa = 0;
					break;
				}
				Client->hex+=Client->request[HexaIndex];
				HexaIndex++;
				index++;
			}
			if ((!Client->hex.empty()) && Client->RestOfHexa == 2 && Client->request[HexaIndex] == '\r'
					&&  Client->request[HexaIndex + 1] == '\n') //? normal case 
			{
				if (HexaIndex >= r - 1)
				{
					std::cout << "this is x: " << HexaIndex << " this  is:  " << r << std::endl;
					std::cout << "exption  look to case2 file ";
					exit(0);
				}
				Client->RestOfHexa +=2;
				HexaIndex +=2;
				HexaForm = true;     
			}
		}
		if(HexaForm == true)
		{
			Client->response->SizeOfChunked = hexadecimalToDecimal(Client->hex);
			std::cout << "this is chunked size: "  << Client->response->SizeOfChunked << std::endl;
			if(Client->response->SizeOfChunked == 0)
                return;
			Client->response->BytesWritten = 0;
			Client->RestOfHexa = 0;
			index = 0;
			HexaForm = false;
			Client->hex = "";
			std::cout << "this is size of hex: " << HexaIndex << std::endl;
			i += (HexaIndex - i);
		}
		fwrite(&Client->request[i], 1, 1, Client->response->fp);
		if(Client->RestOfHexa == 0)
		{
			rest++;
		}
		i++;
	}
	Client->response->BytesWritten = rest;
}

void Server::CheckForcgi(client_info *client)
{
    if (client->response->fp)
	    fclose(client->response->fp);
	if (client->response->IsCgi)
	{
		client->ServClient.isPost = false;
		client->response->BodyCgi = true;
        client->response->cgiContentSent = false;
		client->response->setSocketFd(client->socket);
		client->response->post_file();
		client->ServClient = client->response->getServClient();
		client->response->chunksSending();
		client->ServClient = client->response->getServClient();
        if (client->response->getByteSent() == client->ServClient.length)
            drop_client(client);
	}
	else
	{
		client->ServClient.buildHeader(client->socket);
		drop_client(client);
	}
}

bool Server::ChunkedCase(client_info *Client , int r)
{
	int i = 0;
	const char *str = "\r\n";
	std::cout << "BytesWritten " << client->RestOfHexa << " SizeOfChunked "
				<< client->response->SizeOfChunked << std::endl;
	if (client->RestOfHexa == -2)
		i = getTheRestofhexa(client, i, r);
	if (((client->response->BytesWritten + r) < client->response->SizeOfChunked) && r == 2047)
	{
		client->RestOfHexa = 0;
		char *found = std::search(client->request, &client->request[r], str, str + strlen(str)) + 2;
		if (found < &client->request[r])
		{
			GetTheHex(client, r, i);
			return true;
		}
		fwrite(client->request, 1, r, client->response->fp);
		client->response->BytesWritten += r;
	}
	else
	{
		client->hex = "";
		GetTheHex(client, r, i);
		if (client->RestOfHexa == 4 && client->response->SizeOfChunked == 0)
			CheckForcgi(client);
	}
	return false;
}

bool Server::BoundryCases(client_info *client, int r)
{
	std::string last_b = client->response->request.boundary + "--";
	if (client->response->BytesWritten + r >=
		atoi(client->requesting->headers["Content-Length"].c_str()))
	{
		if (client->response->IsCgi == false)
		{
			/* std::cout << r  << std::endl;
			std::cout << r - last_b.length() - 4 << std::endl;
			exit(1); */
			fwrite(client->request, 1, r - last_b.length() - 4, client->response->fp);
		}
		else
			fwrite(client->request, 1, r, client->response->fp);
		fclose(client->response->fp);
		client->response->BytesWritten += r - last_b.length() - 4;
		CheckForcgi(client);
		return true;
	}
	fwrite(client->request, 1, r, client->response->fp);
	client->response->BytesWritten += r;
	client->RestOfHexa = -1;
	return false;
}

bool Server::StoryTheBody(client_info *client, int r)
{
	if (r != 0 && client->response->request.boundary.length() > 0) //! the case of boundary
	{
		if(BoundryCases(client,r))
			return true;
	}
	else if (r != 0 && client->requesting->headers["Transfer-Encoding"].length() != 0)//!chunked
	{
		if(ChunkedCase(client,r))
			return true;
	}
	else
	{
		fwrite(client->request, 1, r, client->response->fp);
		client->response->BytesWritten += r;
		if (client->response->BytesWritten == atoi(client->requesting->headers["Content-Length"].c_str()))
			CheckForcgi(client);
	}
	return false;
}

void Server::readData()
{
	while (client)
	{
		int r = 0;
		struct client_info *next = client->next;
		std::cout << FD_ISSET(client->socket, &reads) << std::endl;
		std::cout << " write " << FD_ISSET(client->socket, &writes) << std::endl;
		if (FD_ISSET(client->socket, &reads)) {
			std::cout << "reading state ~~~\n";
			r = recv(client->socket, client->request, MAX_REQUEST_SIZE, 0);
			//std::cout << client->request << std::endl;
			client->ServClient.isHeaderSent = false;
			if (r < 1) {
				std::cout << "Unexpected disconnect from" << std::endl;
				drop_client(client);
				client = next;
				continue;
			}
			client->received += r;
			if (client->received >= client->maxbody)
			{
				std::cout << "max body request\n";
				if(client->response->fp)
				{
					fclose(client->response->fp);// remove file
					std::cout << "remove file\n";
					exit(1);
				}
			}
			client->response->setFd(-1);
			client->received = r;
		}
		if (FD_ISSET(client->socket, &writes) && client->received != 0)
		{
			std::cout << "write state " << std::endl;
            if (!client->response->cgiContentSent)
                sentCgi(client);
            else
			{
                if (client->index == 0)
				{
                    if (FD_ISSET(client->socket, &reads) && client->response->getByteSent() == 0)
                        parseRequest(r); // ? add a container for multiple request
                    if (!client->ServClient.isPost)
					{
                        client->response->setSocketFd(client->socket);
                        client->response->chunksSending();

						client->ServClient = client->response->getServClient();

                        if (client->response->getByteSent() == client->ServClient.length && !client->response->isFirstOfbody) {
                            std::cout << "\033[1;31m:::::: the body has been successfully sent :::::\033[0m"
                                      << std::endl;
                            close(client->response->getFd());
                            if (client->response->IsCgi)
                                std::remove(client->ServClient.path.c_str());
                            client->response->setFd(-1);
                            client->response->resetByteSent();
                            FD_CLR(client->socket, &writes);
                            FD_CLR(client->socket, &reads);
                            std::cout << client->response->getFd() << std::endl;
                            drop_client(client);
                        }
                        client = next;
                        continue;
                    }
                    client->index = 1;
                }
				else
				{
					if(StoryTheBody(client , r))
					{
						client = next;
						continue;
					}
				}
				 // //! check for the type of post method
               /*  else if (r != 0 && client->response->request.boundary.length() > 0) //! the case of boundary
                {
					if(BoundryCases(client,r))
					{
						client = next;
                        continue;
					}
                }
				else if (r != 0 && client->requesting->headers["Transfer-Encoding"].length() != 0)//!chunked
                {
					if(ChunkedCase(client,r))
					{
						client = next;
                        continue;
					}
                }
				else
				{
                    fwrite(client->request, 1, r, client->response->fp);
                    client->response->BytesWritten += r;
                    if (client->response->BytesWritten == atoi(client->requesting->headers["Content-Length"].c_str()))
                        CheckForcgi(client);
                } */
            }
		}
		client = next;
	}
}

void Server::start_event() {
	parser = new ConfigParsing;
	if (!configPath.empty())
		parser->setFilePath(this->configPath);
	parser->parse();
	size_t	sizeOfServer = (*parser).servers.size();
	int *server = new int [sizeOfServer];
	for (size_t i = 0; i < sizeOfServer; ++i) {
		server[i] = create_socket((*parser).servers[i].listen.ip_address.c_str(), (*parser).servers[i].listen.port.c_str());

	}
	while (true)
	{
		for (size_t i = 0; i < sizeOfServer; ++i) {
			wait_on_clients(server[i]);
			if (FD_ISSET(server[i], &reads))
			{
				client = get_client(-1);
				/*  At this point, a connection has been accepted and a new socket */
				std::cout << "Accept the connection " << std::endl;
				client->socket = accept(server[i], (struct sockaddr *) &(client->address), &(client->address_length));
				if (client->ServClient.isBodySent()) {
					delete client->response;
					client->response = new HttpResponse();
				}
				client->response->ConfigFile = *parser;
				client->maxbody = client->response->ConfigFile.servers[i].client_bodySize;
				if (fcntl(client->socket, F_SETFL, O_NONBLOCK) == -1) {
					perror("fcntl");
					throw std::runtime_error("fcntl failed.");
				}
                client->response->cgiContentSent = true;
				std::cout << " We add a new client here this clinet socket \n";
//				std::cout << client->socket << " have this ip: " << get_client_address(client) << std::endl;
				if (!ISVALIDSOCKET(client->socket))
					throw std::runtime_error("accept() failed ");
				continue;
			}
			client = clients;
			readData();
		}
	}
	printf("\n Closing socket...\n");
//	CLOSESOCKET(server); todo loop for every socket
}

void Server::setConfigPath(const std::string &Path) {
	this->configPath = Path;
	std::cout << configPath << std::endl;
}

void Server::sentCgi(client_info *client) {
    client->ServClient = client->response->getServClient();
    client->response->chunksSending();
	client->ServClient = client->response->getServClient();
    if (client->response->getByteSent() == client->ServClient.length)
        drop_client(client);
}
