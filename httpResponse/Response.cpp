#include "HttpResponse.hpp"
#include <sys/socket.h>
#include <sstream>

void	ft_deplay(Client &Serv)
{
	std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~`" << std::endl;
	std::cout << "header length: " << Serv.buff.length() << std::endl;
	std::cout << "body length: " << Serv.lengthOfbody << std::endl;
	std::cout << "length of the file: " << Serv.length << std::endl;
	std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~`" << std::endl;
}
void HttpResponse::chunksSending() {
	if (fd == -1 && !ServClient.path.empty() && ServClient.is_autoIndexOn != NI_ON ) {
		fd = open(ServClient.path.c_str(), O_RDONLY);
		if (fd < 0) perror("open");
		// todo see what to do with open fail to open file
	}
	if (!isFirstOfbody)
	{
		std::cout << "~~~~~~sendResponse~~~~~~" << std::endl;
		sendResponse(ServClient);
	}
	if (!ServClient.isHeaderSent)
	{
		std::cout << "headerrrrr****~~~~" << std::endl;
		isFirstOfbody = true;
		ServClient.isHeaderSent = true;
		if (send(socketFd, ServClient.buff.c_str(), ServClient.buff.length(), 0) < 0)
			std::cout << "err send()" << std::endl;
	}
	else {
		if (ServClient.lengthOfbody > 0 && ServClient.length > 0) {
			std::cerr << "bodddddyyyyy****~~~~" << std::endl;
			isFirstOfbody = false;
			if (send(socketFd, ServClient.body, ServClient.lengthOfbody, 0) < 0)
				std::cout << "body err send()" << std::endl;
			if (ServClient.body) {
				delete[] ServClient.body;
				ServClient.body = NULL;
			}
		}
		else
			isFirstOfbody = false;
	}
	std::cout << isFirstOfbody << std::endl;
}

std::string	intToString(long a)
{
	std::stringstream len;

	len << a;
	return len.str();
}

void	HttpResponse::readFile(std::ifstream &file, Client &Serve)
{
	file.seekg(0, std::ios::end);
	long length = file.tellg();
	file.seekg(0, std::ios::beg);
	Serve.length = length;
	if (length == 0)
	{
		Serve.body = NULL;
		byteSent = 0;
		return;
	}
	char	*bu = new char [2025];
	bzero(bu, 2025);
	ssize_t 	re;
	if ((re = read(this->fd, bu, 2025)) > 0) {
		Serve.lengthOfbody = re;
		Serve.body = bu;
	}
	else if (re == -1)
	{
		std::cout << "could not " << re << "open file " << fd << std::endl;
		exit(1);
	}
	byteSent += re;
}

void HttpResponse::fill_body(Client &Serve)
{
	std::string body;
	std::ifstream file;
	if (Serve.is_autoIndexOn == NI_ON) {
		if (get_autoindex(Serve))
			return ;
	}
    std::cout << "here i am here "  << std::endl;
	file.open(Serve.path.c_str(), std::ios::binary);
	std::cout << Serve.path << " <path> open state <fail> " << file.fail() << std::endl;
	if (!file.fail()) {
		readFile(file, Serve);
		file.close();
		return;
	}
	body = "<html>\n"
		   "<head><title>" + Serve.it->first + " " + Serve.it
				   ->second
		   + "</title></head>\n"
			 "<body>\n"
			 "<center><h1>" + ServClient.it->first + " " + ServClient.it
				   ->second
		   + "</h1></center>\n"
			 "<hr><center> Webserv/0.1 </center>\n"
			 "</body>\n"
			 "</html>";
	Serve.body = new char [body.length() + 1];
	Serve.body = strcpy(Serve.body, body.c_str());
	std::cout << "the content len" << body.length() << std::endl;
	Serve.length = body.length();
	byteSent = Serve.length;
	Serve.lengthOfbody = byteSent;
}


void HttpResponse::sendResponse(Client &Serve)
{
	char buf[1000];
	time_t now = time(NULL);
	struct tm tm = *gmtime(&now);
	strftime(buf, sizeof buf, "%a, %d %b %Y %T %Z", &tm);

	fill_body(Serve);
    if (!IsCgi) {
        Serve.buff = "HTTP/1.1 " + Serve.it->first + " " + Serve.it->second;
        Serve.buff += "\r\nServer: Webserv\r\n";
        Serve.buff += "Last-Modified: ";
        Serve.buff += buf;
        if (Serve.it->first == "301") {
            Serve.buff += "\r\nLocation: " + request.url + "/";
            Serve.length = 0;
            Serve.lengthOfbody = 0;
            byteSent = 0;
        }
        Serve.buff += "\r\nConnection: keep-alive\r\nContent-Length: ";
        Serve.buff += intToString(Serve.length);
        std::cout << Serve.it->first << "****" << std::endl;
        Serve.buff += "\r\nContent-Type: " + GetContentTypeOfContent(Serve.path) + "\r\n\r\n";
    }
}