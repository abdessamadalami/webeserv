//
// Created by Abdelhak El moussaoui on 1/14/23.
//

#include <sstream>
#include <dirent.h>
#include "HttpResponse.hpp"
#include <unistd.h>
#include <cstring>

#include <sys/socket.h>

void HttpResponse::response()
{
	/* Handling The Errors Of The Request */
	/* Matching The Request With The Configuration File */
//	std::cout << request.url << "this is the url u req" << std::endl;
	ServClient.isPost = false;
	if (this->get_match_server())
	{
		is_req_wellFormed(request, ServClient);
		if (!get_match_location(serverI))
		{
			status = false;
			ServClient.path = serverI->error_pages[404];
			ServClient.it = get_code("404");
			std::cout << "location not matched" << std::endl;
			return ;
		}
		if (checkRedirection())
		{
			status = false;
			ServClient.it = get_code("301");
			ServClient.path = serverI->error_pages[301];
			request.url = locationI->http_redirect[1];
			ServClient.is_autoIndexOn = NI_OF;
			std::cout << "a return found====" << std::endl;
			return ;
		}
		if (!this->isMethodAllowed())
		{
			status = false;
			ServClient.it = get_code("405");
			ServClient.path = serverI->error_pages[405];
			return ;
		}
		if (request.url != "/")
		{
			std::string requestUrl = request.url;
			this->fullUrl = locationI->root_dir + requestUrl.erase(0, locationI->prefix.length());
		}
		else
			fullUrl = locationI->root_dir;
		std::ifstream infile;
		for (int i = 0; i < (locationI)->index_files.size(); ++i) {
			ServClient.resourcePath = fullUrl + locationI->index_files[i];
			infile.open(ServClient.resourcePath);
			if (infile.good()){
				break ;
				infile.close();
			}
			infile.close();
		}
		if (locationI->index_files.empty())
			ServClient.resourcePath = fullUrl;
		if (request.Method == "GET")
		{
			Get();
			return;
		}
		else if (request.Method == "POST")
		{
			Post();
			return;
		}
		else if (request.Method == "DELETE")
		{
			std::cout << "Delete" << std::endl;
			Delete();
			return;
		}
	}
	ServClient.it = get_code("404");
	ServClient.path = serverI->error_pages[404];
}

bool HttpResponse::getLocation(std::string url, std::vector<t_server>::iterator sIt)
{
	std::vector<t_location>::iterator locationIt = sIt->locations.begin();
	std::vector<t_location>::iterator defaultLocation = sIt->locations.end();

	while (locationIt != sIt->locations.end())
	{

		if (locationIt->prefix == url)
		{
			std::cout << "found location" << locationIt->prefix << std::endl;
			this->locationI = locationIt;
			return true;
		}
		locationIt++;
	}
	locationI = sIt->locations.end();
	return false;
}

bool	HttpResponse::get_match_location(std::vector<t_server>::iterator sIt)
{
	std::string url = request.url;// = request.url.substr(0, request.url.find_last_of('/'));
	// !  example of url          - /home/test /test.html
	// * break it to / the  /home/ then /home/test/
	while (true)
	{
		if (getLocation(url, sIt) || url == "/" || url.empty()) {
			break ;
		}
		url = url.substr(0, url.find_last_of('/'));
	}
	if (this->locationI == sIt->locations.end())
	{
		if (!getLocation("/", sIt))
			return false;
	}
	return true;
}

/*
 ? 1 - looks for listen directive that matches ip and port of the request
 *  	- if no matching listen directive found
 *  		then looks for server_name if match is found, the request handled by that server block
 *      - If neither the listen nor the server_name directive match the request,
 *  			Nginx uses the default server block to handle the request.
 *	2 - Once the server block is determined, Nginx will look for the most specific
 *				location block to handle the request by matching the uri requested
 */
bool HttpResponse::get_match_server() {
	std::vector<t_server>::iterator sIt = ConfigFile.servers.begin();
	
	std::map<std::string, std::string>::iterator rIt = request.headers.find("Host");

	if (rIt != request.headers.end())
	{
		for (sIt = ConfigFile.servers.begin(); sIt != ConfigFile.servers.end(); sIt++)
		{
//			std::cout << sIt->listen.ip_address << std::endl;
			if (sIt->listen.ip_address == rIt->second.substr(0, rIt->second.find_first_of(':')) && \
                sIt->listen.port == rIt->second.substr(rIt->second.find_first_of(':') + 1))
			{
				std::cout << "found the server lets keep the that server" << std::endl;
				serverI = sIt;
				return true;
			}
		}
		if (sIt == ConfigFile.servers.end())
		{
			std::vector<std::string>::iterator server_nameIt;

			for (sIt = ConfigFile.servers.begin(); sIt != ConfigFile.servers.end(); sIt++)
			{
				if (std::find(sIt->server_names.begin(), sIt->server_names.end(), rIt->second) !=
					sIt->server_names.end())
				{
					std::cout << "found the server server_name lets keep the that server" << std::endl;
					serverI = sIt;
					return true;
				}
			}
		}
	}
	std::cout << "Host not found " << std::endl;

	return false;
}


bool HttpResponse::checkRedirection() const {
	if (locationI->http_redirect.empty()) {
		return false;
	}
	return true;
}

/* ------------- Checking The Errors Handling Of The Upcoming Request ---------------- */

bool HttpResponse::checkURI(std::string& uri, Client& ErrorCodes) {
    std::string allowed = "-._~:/?#[]@!$&'()*+,;=%";

    for (std::string::const_iterator it = uri.begin(); it != uri.end(); it++)
    {
        if (!std::isalnum(*it))
            if (allowed.find(*it) == std::string::npos)
                return (false);
    }

    // ------ Checking The Max Size Of "URI" -------- //
    if (uri.size() > 2048)
    {
        ErrorCodes.path = serverI->error_pages[414];
        ErrorCodes.it = get_code("414");
        /* << 414 Request-URI Too Long >> */
    }
    /***//***//***//***//***//***//***//***/

    return (true);
}

void HttpResponse::is_req_wellFormed(requestParser& request, Client& ErrorCodes) {

    /*
        -> Checking If "Transfer-Encoding" Exists And Is Different From "Chunked"
        -> If The Method Is "POST" And The "Content-Length", "Transfer-Encoding" Does Not Exist
        -> If "URI" Contains A Character That Is Not Allowed
        -> If "URI" Contains More Than 2048 Characters
        -> If Request Body Larger Than "Client-Body-Size" In Config File.
     */

    std::map<std::string, std::string>::iterator It = request.headers.find("Transfer-Encoding");
    if (It != request.headers.end())
    {
        if (It->second != "chunked") {
            ErrorCodes.path = serverI->error_pages[501];
            ErrorCodes.it = get_code("501");
        }
            /* Redirect To Error-Page << 501 Not Implemented >> */
    }
    else if ((It == request.headers.end() && request.headers.find("Content-Length") == request.headers.end()
    		&& request.Method == "POST"))
    {
        ErrorCodes.path = serverI->error_pages[400];
        ErrorCodes.it = get_code("400");
        /* << 400 Bad Request >> */
    }
    else if (!checkURI(request.url, ErrorCodes))
    {
        ErrorCodes.path = serverI->error_pages[400];
        ErrorCodes.it = get_code("400");
        /* 400 Bad Request */
    }
    else if ((It = request.headers.find("Content-Length")) != request.headers.end())
    {
        if (std::atoi(It->second.c_str()) > serverI->client_bodySize) {
			ErrorCodes.path = serverI->error_pages[413];
			ErrorCodes.it = get_code("413");
            /* 413 Request Entity Too Large */
        }
    }
}


/* ---------- The End Of The Handling -------------- */

bool HttpResponse::check_path()
{
	std::string path = this->fullUrl;
	std::ifstream infile(path.c_str());
	return infile.good();
}

void 	HttpResponse::get_dir()
{
	std::cout << " get dir()" << this->fullUrl << std::endl;
//	! check if uri ends with slash
	if (locationI->index_files.empty() && locationI->autoIndex == 1)
	{
		std::cout << "1" << std::endl;
		// ? auto index ON and there is no index file
		// .. return autoindex of the directory
		ServClient.is_autoIndexOn = NI_ON;
		std::cout << "on dir listening" << this->fullUrl << std::endl;
		ServClient.it = get_code("200");
		ServClient.path =  this->fullUrl;
	}
	else if (locationI->index_files.empty() && locationI->autoIndex == 0)
	{
		std::cout << "wa hmadi" << std::endl;
		// ? autoindex off and index file not exist
		ServClient.is_autoIndexOn = NI_OFF;
		ServClient.it = get_code("403");
		ServClient.path = serverI->error_pages[403];
	}
	else if (!locationI->index_files.empty())
	{
		std::cout << "2" << std::endl;
		ServClient.it = get_code("200");
		std::ifstream infile;
		for (int i = 0; i < (locationI)->index_files.size(); ++i) {
			ServClient.path = locationI->root_dir + locationI->index_files[i];
            this->fullUrl = ServClient.path;
            std::cout << "Warning ~~~~~~~~~~~" << std::endl;
			if (isCgiScript(ServClient.path) && !locationI->_cgi.empty())
			{
				std::string cgiExec = executeCgi();
                std::cout << "INDEX FILES CGI" << std::endl;
				CgiOutputParser(cgiExec);
				return ;
			}
			infile.open(ServClient.path);
			if (infile.good())
		 		return ;
			infile.close();
		}
		ServClient.it = get_code("404");
		ServClient.path = serverI->error_pages[404];
	}
	std::cout << locationI->index_files.empty() << "|" << locationI->autoIndex << std::endl;

}

void HttpResponse::get_file() {

	if (locationI->_cgi.empty() || !isCgiScript(this->fullUrl))
	{
		// no cgi
		ServClient.it = get_code("200");
		ServClient.path = this->fullUrl;
		return  ;
	}
	 /*? I have To Keep The Body In The File and Doing "fread" To Send It 1024 By 1024 */
	 std::cout << "*** ENTERED TO CGI ***" << std::endl;
	 std::string cgiExec = executeCgi();
	 CgiOutputParser(cgiExec);
	 std::cout << "*** FINISHED FROM CGI ***" << std::endl;
}

bool HttpResponse::get_autoindex(Client &Serve)
{
	const char* PATH = Serve.path.c_str();
	std::string body;

	DIR *dir = opendir(PATH);
	if (!dir)
	{
		Serve.it = get_code("404");
		Serve.path = serverI->error_pages[404];
		return false;
	}
	struct dirent *entry = readdir(dir);
	body = "<html>\n"
					  "<head><title>Index of " + Serve.path + "</title></head>\n"
					  "<body>\n";
	std::string name;
	body.append("<h1>Index Of The Directory</h1><hr><pre>\n");
	while (entry != NULL)
	{
		name = entry->d_name;
		if (entry->d_type == DT_DIR)
		{
			name.append("/");
			body += "<a href=\"" + name + "\">" + name + "</a>\n";
		}
		else if (entry->d_type == DT_REG) // file
		{
			body += "<a href=\"" + name + "\">" + name + "</a>\n";
		}
		body += "<br>";
		entry = readdir(dir);
	}
	body.append("</pre><hr></body>\n</html>");
	closedir(dir);
	Serve.body = new char [body.length() + 1];

	Serve.body = strcpy(Serve.body, body.c_str());

	Serve.length = body.length();
	byteSent = Serve.length;
	Serve.lengthOfbody  = byteSent;
	return true;
}

std::string HttpResponse:: GetContentTypeOfContent(std::string path)
{
    std::ifstream mimeType("httpResponse/extensions.type");
    std::string    str;
    std::string sub;
    std::string next;
    if (path.find('.') != std::string::npos)
    {
        path.erase(0, path.find('.'));
    }
    if (mimeType.is_open())
    {
        int index = 0;
        while (std::getline(mimeType, str, '\n'))
        {
            sub = str.substr(0, str.find(' '));
            next = str.erase(0, str.find(' ') + 1);
            std::cout << "a[sub:" << sub << "]" << std::endl;
            std::cout << "a[path:" <<  "]" << std::endl;
			
            if (sub == path)
			{
                return next;
			}
			if (path == next)
			{
                return sub;
			}
			str.clear();
            sub.clear();
            next.clear();
        }
    }
    return "";
}

 
/*  
std::string HttpResponse::GetContentTypeOfContent(std::string& path)

{
	std::cout <<"path *****:"<<  path << std::endl;

	if (path == "video/mp4")     return ".mp4";
     if (path == "text/css")     return ".css";
	 if (path == "text/plain")     return ".txt";
     if (path == "text/csv")     return ".csv";
     if (path == "image/gif")     return ".gif";
     if (path == "text/html")     return ".html";
     if (path == "image/x-icon")     return ".ico";
     if (path ==  "image/jpeg")     return ".jpeg";
     if (path == "image/jpeg")     return ".jpg";
     if (path == "application/javascript")      return ".js";
     if (path == "application/json")      return ".json";
     if (path == "image/png")      return ".png";
     if (path == "application/pdf")     return ".pdf";
     if (path == "image/svg+xml")     return ".svg";

     // ***********************************************************
	 std::string a = "video/mp4";
	 const char *last_dot = strrchr(path.c_str(), '.');
     if (last_dot)
     {
		 if (strcmp(last_dot, ".mp4") == 0) return a;
         if (strcmp(last_dot, ".aac") == 0) return "audio/aac";
         if (strcmp(last_dot, ".abw") == 0) return "application/x-abiword";
         if (strcmp(last_dot, ".arc") == 0) return "application/x-freearc";
         if (strcmp(last_dot, ".avif") == 0) return "image/avif";
         if (strcmp(last_dot, ".avi") == 0) return "video/x-msvideo";
         if (strcmp(last_dot, ".azw") == 0) return "application/vnd.amazon.ebook";
         if (strcmp(last_dot, ".bin") == 0) return "application/octet-stream";
         if (strcmp(last_dot, ".bmp") == 0) return "image/bmp";
         if (strcmp(last_dot, ".bz") == 0) return "application/x-bzip";
         if (strcmp(last_dot, ".bz2") == 0) return "application/x-bzip2";
         if (strcmp(last_dot, ".cda") == 0) return "application/x-cdf";
         if (strcmp(last_dot, ".csh") == 0) return "application/x-csh";
         if (strcmp(last_dot, ".doc") == 0) return "application/msword";
         if (strcmp(last_dot, ".docx") == 0) return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
         if (strcmp(last_dot, ".eot") == 0) return "application/vnd.ms-fontobject";
         if (strcmp(last_dot, ".epub") == 0) return "application/epub+zip";
         if (strcmp(last_dot, ".gz") == 0) return "application/gzip";
         if (strcmp(last_dot, ".ics") == 0) return "text/calendar";
         if (strcmp(last_dot, ".jar") == 0) return "application/java-archive";
         if (strcmp(last_dot, ".mp3") == 0) return "audio/mpeg";
         if (strcmp(last_dot, ".mpeg") == 0) return "video/mpeg";
         if (strcmp(last_dot, ".mpkg") == 0) return "application/vnd.apple.installer+xml";
         if (strcmp(last_dot, ".odp") == 0) return "application/vnd.oasis.opendocument.presentation";
         if (strcmp(last_dot, ".ods") == 0) return "application/vnd.oasis.opendocument.spreadsheet";
         if (strcmp(last_dot, ".oga") == 0) return "audio/ogg";
         if (strcmp(last_dot, ".ogv") == 0) return "video/ogg";
         if (strcmp(last_dot, ".php") == 0) return "application/x-httpd-php";
         if (strcmp(last_dot, ".ppt") == 0) return "application/vnd.ms-powerpoint";
         if (strcmp(last_dot, ".rar") == 0) return "application/vnd.rar";
         if (strcmp(last_dot, ".sh") == 0) return "application/x-sh";
         if (strcmp(last_dot, ".tar") == 0) return "application/x-tar";
         if (strcmp(last_dot, ".ts") == 0) return "video/mp2t";
         if (strcmp(last_dot, ".ttf") == 0) return "font/ttf";
         if (strcmp(last_dot, ".webm") == 0) return "video/webm";
         if (strcmp(last_dot, ".webp") == 0) return "image/webp";
         if (strcmp(last_dot, ".woff") == 0) return "font/woff";
         if (strcmp(last_dot, ".xhtml") == 0) return "application/xhtml+xml";
         if (strcmp(last_dot, ".xml") == 0) return "application/xml";
         if (strcmp(last_dot, ".zip") == 0) return "application/zip";
         if (strcmp(last_dot, ".mp3") == 0) return "audio/mpeg";
         if (strcmp(last_dot, ".mp3") == 0) return "audio/mpeg";
         if (strcmp(last_dot, ".css") == 0) return "text/css";
         if (strcmp(last_dot, ".csv") == 0) return "text/csv";
         if (strcmp(last_dot, ".gif") == 0) return "image/gif";
         if (strcmp(last_dot, ".htm") == 0) return "text/html";
         if (strcmp(last_dot, ".html") == 0) return "text/html";
         if (strcmp(last_dot, ".ico") == 0) return "image/vnd.microsoft.icon";
         if (strcmp(last_dot, ".jpeg") == 0) return "image/jpeg";
         if (strcmp(last_dot, ".jpg") == 0) return "image/jpeg";
         if (strcmp(last_dot, ".js") == 0) return "application/javascript";
         if (strcmp(last_dot, ".json") == 0) return "application/json";
         if (strcmp(last_dot, ".png") == 0) return "image/png";
         if (strcmp(last_dot, ".pdf") == 0) return "application/pdf";
         if (strcmp(last_dot, ".svg") == 0) return "image/svg+xml";
         if (strcmp(last_dot, ".txt") == 0) return "text/plain";
         //if (strcmp(last_dot, ".mkv") == 0) return "video/x-matroska";
     }
     return ""; 
}

*/
std::string HttpResponse::retriveResourceType() {
	struct stat st;

	if (stat(this->fullUrl.c_str(), &st) == 0)
	{
		if (S_ISDIR(st.st_mode))
			return "DIR";
		else if (S_ISREG(st.st_mode))
			return "FILE";
	}
	return "";
}

void HttpResponse::post_file() {
	std::string	resourcePath;
	if (!locationI->index_files.empty())
	{
		std::ifstream s;
		for (int i = 0; i < locationI->index_files.size(); ++i) {
			resourcePath = this->fullUrl + locationI->index_files[i];
			s.open(resourcePath);
			if (s.is_open())
				break ;
		}
	}
	else
		resourcePath = fullUrl;
	if (locationI->_cgi.empty() || !isCgiScript(resourcePath))
	{
		ServClient.it = get_code("403");
		ServClient.path = serverI->error_pages[403];
	}
	else if (BodyCgi)
	{
		std::cout << "CgI_POST_REQUEST" << std::endl;
		std::string cgiExec = executeCgi();
		CgiOutputParser(cgiExec);
	}
	else
	{
		this->IsCgi=true;
		GetBody();
	}
}

void HttpResponse::post_dir() {
	if (*(request.url.end() - 1) != '/')
	{
		ServClient.it = get_code("301");
		ServClient.path = this->fullUrl + '/';
		return ;
	}
	// no index file
	if (locationI->index_files.empty())
	{
		ServClient.it = get_code("403");
		ServClient.path = serverI->error_pages[403];
		return ;
	}
	else
		post_file();
}

HttpResponse::HttpResponse() {
	status = true;
	fileSize = 0;
	byteSent = 0;
//	this->ConfigFile = ConfigFile;
	fd = -1;
	std::cout << "Http constructor" << std::endl;
}

bool HttpResponse::isMethodAllowed() {
	return std::find(locationI->allowed_methods.begin(), locationI->allowed_methods.end(), request.Method) !=
		   locationI->allowed_methods.end();
}

HttpResponse::~HttpResponse() {
	fd = -1;
}


void HttpResponse::setRequest(requestParser &requestParser) {
	this->request = requestParser;
}

void HttpResponse::setFd(int fileD) {
	this->fd = fileD;
}

int HttpResponse::getFd() const
{
	return fd;
}

size_t HttpResponse::getByteSent() const {
	return byteSent;
}

void	HttpResponse::setSocketFd(const int sFD) {
	this->socketFd = sFD;
}

size_t HttpResponse::getFileSize() const
{
	return fileSize;
}

void HttpResponse::resetByteSent() {
	byteSent = 0;
}

Client &HttpResponse::getServClient() {
	return ServClient;
}


void Client::buildHeader(int &socket) {
   std::string s = "<html>\n<head><title> 201 Created </title></head>\n"
             "<body>\n"
             "<center><h1>201 Created</h1></center>\n"
             "<hr><center> Webserv/0.1 </center>\n"
             "</body>\n"
             "</html>";
    std::string  header = "HTTP/1.1 201 Created\r\nServer: Webserv\r\nContent-Type: text/html\r\nContent-Length: 144\r\n\r\n";
    header += s;
    if (send(socket, header.c_str(), header.length(), 0) < 0)
        std::cout << "err in send.." << std::endl;
}
