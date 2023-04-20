//
// Created by Abdelhak El moussaoui on 1/14/23.
//

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP
#include <cstring>
#include "../Parsing/requestParser.hpp"
#include "../Parsing/ConfigParsing.hpp"
#include "CodeStatus.hpp"
#include <sys/stat.h>
#include <dirent.h>
// no index file and autoindex on
#define NI_ON 1
// no index file and autoindex off
#define NI_OF 2
// reset NI_ON
#define NI_OFF 0
#include <fcntl.h>

class	Client {
public:
	std::string resourcePath;
	std::string path;
	MapIterator it;
	char 		*body;
    void buildHeader(int &socket);
	std::string	buff;
	int			is_autoIndexOn;
	size_t 		lengthOfbody;
	bool 			isHeaderSent;
	size_t		byteSent; // ? if sending by chunks
	unsigned long		 length;
	bool			isPost;
    int             BodyOnly;
	Client() {
		is_autoIndexOn = NI_OFF;
	}
	~Client()
	{
		path.clear();
		length = 0;
	}
	bool isBodySent() const {
		return (byteSent == length);
	}
};

int hexadecimalToDecimal(std::string hexVal);

class HttpResponse : public StatusCodes {
public:

    //! serv
//	std::ofstream *MyF;
	bool 	CheckForLastboundary();
	void	GetBody();
	void	CheckForCgi();
	bool	FirstChunked(char *part);
	bool	FristBoundry(char *body);
	void	CheckBoundary();
	int		GetContentType(char *part);
	void	generat_name(std::string type);
	int		bites;
	FILE	*fp;
	bool	find_del(char *part,int r);
	int 	SizeOfChunked;
	int 	BytesWritten;


	//!
	HttpResponse();
	~HttpResponse();
	/* Handling The Errors Of The *** REQUEST ***  */
	void		Get();
    void  	    Delete();
	void		Post();
	void		chunksSending();
	void		get_file();
	std::string retriveResourceType();
	void		post_dir();
	void		post_file();
	void		get_dir();
	void 		response();
	void 		sendResponse(Client &Serve);
	std::string	GetContentTypeOfContent(std::string path);
	bool		 check_path();

	bool		getLocation(std::string url, std::vector<t_server>::iterator sIt);
	bool 		get_autoindex(Client &Serve);
	void 		fill_body(Client &Serve);
	bool		delete_folder(std::string& path);
	bool		delete_file(std::string& path);
	void		setRequest(requestParser &requestParser);
	void 		is_req_wellFormed(requestParser& request, Client& ErrorCodes);
	bool 		get_match_location(std::vector<t_server>::iterator sIt);
	bool 		checkURI(std::string& uri, Client& ErrorCodes);
	bool 		checkRedirection() const;
	void		readFile(std::ifstream &file, Client &Serve);
	bool 		get_match_server();
	bool		isMethodAllowed();
	/*
	 *  Setters and Getters
	 */
	size_t 		getByteSent() const;
	void 		resetByteSent();
	void		setSocketFd(int socketFd);
	size_t		getFileSize() const;
	void		setFd(int fd);
	int			getFd() const;
	Client		&getServClient();
	// CGI

	bool isCgiScript(std::string& path);
    std::string GenerateRandNames();
	void SetEnvironments();
	char** ConvertEnvironment();
	std::string& getQueryString();
	std::string executeCgi();
	void CgiOutputParser(const std::string& filePath);
	std::string readingParsingFile(const std::string& filePath);
    void freeEnv(char** env);
    std::string generateResponse(std::string& Response);

public:
	std::vector<t_location>::iterator locationI;
	std::vector<t_server>::iterator serverI;
    Client 		DeleteHandling;
    t_server	*ServerErrors;
	requestParser request;
	// requestParser request;
	ConfigParsing ConfigFile;

	bool BodyCgi;
	bool IsCgi;
    bool cgiContentSent;
	bool	status;
	bool		isFirstOfbody;
private:
	Client 	ServClient;
	int			fd;
	int			socketFd;
	size_t		byteSent;
	size_t		fileSize;
	std::string	fullUrl;

private:
	// requestParser request

    // CGI

    std::map<std::string, std::string> env;
    std::map<std::string, std::string> _CgiHeaders;
    s_location locationconfig;
    std::string Querystring;
    std::string CgiFilePath;
    std::string randomCgiFile;
    char* args[3];
    int fileFd;
    int BodyFileFd;
    std::string PostCgiFile;
};

/*
 *
*/


#endif