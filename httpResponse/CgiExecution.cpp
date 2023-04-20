
#include <sys/fcntl.h>
#include "HttpResponse.hpp"

std::string HttpResponse::GenerateRandNames() {
    std::string name;
    std::string str = "abcdefghABCDEFGH1234567890";

    int length = rand() % 6 + 5; // Generate a random name length between 5 and 10 characters

    // Generate random characters for the name
    for (int i = 0; i < length; i++) {
        name += str[rand() % 26];
    }
    return name;
}
bool HttpResponse::isCgiScript(std::string& path) {
    if (!locationI->_cgi.empty()) {
        size_t fpos = path.find('?');
        if (fpos != std::string::npos) {
            std::cout << "IM HERE1" << std::endl;

            CgiFilePath = path.substr(0, fpos);
            Querystring = path.substr(fpos + 1);
            std::cout << path << std::endl;
            std::cout << Querystring << std::endl;

            size_t pos = CgiFilePath.rfind('.');
            if (pos != std::string::npos) {
                std::string extension = CgiFilePath.substr(pos + 1);

                if (extension == locationI->_cgi.begin()->first)
                    return true;
            }
        } else if (fpos == std::string::npos) {
            size_t pos = path.rfind('.');
            CgiFilePath = path;
            if (pos != std::string::npos) {
                std::string extension = path.substr(pos + 1);
                std::cout << "HAHA  " << extension << std::endl;
                std::cout << "Ext :" << extension << "$" << locationI->_cgi.begin()->first << std::endl;
                if (extension == locationI->_cgi.begin()->first)
                    return true;
            }
        }
    }
    return false;
}

std::string& HttpResponse::getQueryString() {
    return Querystring;
}

void HttpResponse::SetEnvironments() {
    if (request.Method == "POST") {
        this->env["CONTENT_LENGTH"] = request.headers["Content-Length"];
    }
    if (request.Method == "GET") {
        this->env["QUERY_STRING"] = getQueryString();
    }
    this->env["CONTENT_TYPE"] = request.headers["Content-Type"];
    this->env["REQUEST_METHOD"] = request.Method;
    this->env["GATEWAY_INTERFACE"] = "CGI/1.1";
    this->env["PATH_INFO"] = request.url;
    this->env["SERVER_NAME"] = serverI->listen.ip_address;
    this->env["SERVER_PORT"] = serverI->listen.port;
    this->env["SERVER_PROTOCOL"] = request.version;
    this->env["SCRIPT_NAME"] = CgiFilePath;
    this->env["HTTP_HOST"] = request.headers["Host"];
    this->env["PATH_TRANSLATED"] = CgiFilePath;
    this->env["REDIRECT_STATUS"] = "200";
    this->env["SCRIPT_FILENAME"] = CgiFilePath;
    this->env["HTTP_COOKIE"] = this->request.Cookies;
//    this->env["AUTH_TYPE"] = ;
//    this->env["REMOTE_ADDR"] = request.headers["Host"];
//    this->env["REMOTE_HOST"] = request.headers["Host"];

    std::string cgiString = locationI->_cgi.begin()->second;

    args[0] = strdup((char *) cgiString.c_str());
    args[1] = strdup((char *)CgiFilePath.c_str());
    args[2] = NULL;

    /*?  I Need To Free "args"  */
}

char** HttpResponse::ConvertEnvironment() {
    SetEnvironments();
    size_t  i = 0;
    char **_env = new char*[this->env.size() + 1];
    std::string elements;

    for (std::map<std::string, std::string>::iterator it = env.begin(); it != env.end(); it++)
    {
        elements = it->first + "=" + it->second;
        _env[i] = strdup((char *)elements.c_str());
        i++;
    }
    _env[i++] = NULL;

    return _env;
}

std::string HttpResponse::readingParsingFile(const std::string& filePath)
{
    std::string line;
    std::string content;
    std::cout <<"\n"<< filePath << std::endl;
    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cout << "Unable To Open The File !" << std::endl;
        exit(1);
    }
    while (std::getline(file, content))
        line += content + '\n';
    file.close();
    return (line);
}

void HttpResponse::freeEnv(char** env) {
    for (size_t i = 0; env[i]; i++)
        delete[] env[i];
    delete[] env;
}

std::string HttpResponse::executeCgi()
{
    srand(time(NULL));
    char** _env = ConvertEnvironment();
    int i = 0;

    randomCgiFile = "CgiFileTemp/" + GenerateRandNames();
    std::cout << "***  CgiEnvironments  ***" << std::endl;

    while (_env[i])
        std::cout << _env[i++] << std::endl;
    std::cout << "IMHERETOO" << std::endl;
    int fdPipe[2], pid;

    /* I Should Read The File Of The Requested Body */
    std::cout << "POSTFILES :    " << PostCgiFile << std::endl;
    BodyFileFd = open(PostCgiFile.c_str(), O_RDONLY);
//    if (BodyFileFd == -1)
//        std::cout << "IMHERETOO2" << std::endl;
    pipe(fdPipe);

    pid = fork();
    if (pid == 0)
    {
        fileFd = open(randomCgiFile.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
//        if (fileFd == -1)
//            std::cout << "ERROR!!!" << std::endl;
        dup2(BodyFileFd, 0);
        dup2(fileFd, 1);
        execve(args[0], args, _env);
        close(fileFd);

        exit(1);
    }
    else {
		close(fdPipe[1]);
		close(fdPipe[0]);
		waitpid(-1, NULL, 0);
        close(BodyFileFd);
//        close(fileFd);
    }
	std::cout << "POSTFILES :    " << PostCgiFile << std::endl;
    freeEnv(_env);
    return (randomCgiFile);
}
std::string HttpResponse::generateResponse(std::string& bodyString)
{
    std::string response;

    if (!_CgiHeaders["Status"].empty())
    {
        std::cout << "sssssssss" << std::endl;
        if (!_CgiHeaders["Location"].empty()) {
            response = "HTTP/1.1 302 Found\r\nContent-Type: "
                           + _CgiHeaders["Content-type"].substr(0, _CgiHeaders["Content-type"].find(";")) + "\r\nContent-Length: "
                           + std::to_string(bodyString.size()) + "\r\n" + "Location: " + _CgiHeaders["Location"] + "\r\n";
        }
        else {
            response = "HTTP/1.1 " + _CgiHeaders["Status"] + "\r\n" + "Content-Type: "
                           + _CgiHeaders["Content-type"].substr(0, _CgiHeaders["Content-type"].find(";")) +
                           "\r\nContent-Length: "
                           + std::to_string(bodyString.size()) + "\r\n";
        }
    }
    else if (_CgiHeaders.find("Status: ") == _CgiHeaders.end()) {
        response = "HTTP/1.1 200 OK\r\nContent-Type: "
                       + _CgiHeaders["Content-type"].substr(0, _CgiHeaders["Content-type"].find(";")) +
                       "\r\nContent-Length: "
                       + std::to_string(bodyString.size()) + "\r\n";
    }
    return response;
}

void HttpResponse::CgiOutputParser(const std::string& filePath)
{
    std::cout << "Calling CgiOutputParser Function()" << std::endl;
   std::string ResponseString;
   std::string headerString;
   std::string bodyString;
    std::string Key, Value;

    ResponseString = readingParsingFile(filePath);
   std::string copyString = ResponseString;
   
   std::ifstream file(filePath);

//   std::getline(file, contentLine);
   while (copyString != "\r")
   {
       size_t pos1 = copyString.find(":", 0);
       if (pos1 != std::string::npos) {
            Key = copyString.substr(0, pos1);
            Value = copyString.substr(pos1 + 2, (copyString.size() - 1) - pos1 - 2);
            _CgiHeaders[Key] = Value;
       }
       std::getline(file, copyString);
   }
   std::streampos position = file.tellg();

   file.seekg(0, std::ios_base::end);
   
   size_t pos = ResponseString.find("\r\n\r\n");
   if (pos != std::string::npos)
        bodyString = ResponseString.substr(pos + 1);
   
    headerString = generateResponse(bodyString);
    ServClient.buff = headerString;

    std::string fileName = "CgiFileTemp/" + GenerateRandNames();
    ServClient.BodyOnly = open(fileName.c_str(), O_CREAT | O_TRUNC | O_RDWR, 0777);
    if (ServClient.BodyOnly == -1)
        throw std::runtime_error("BodyOnlyFile Error");

    if (write(ServClient.BodyOnly, bodyString.c_str(), bodyString.length()) == -1)
        throw std::runtime_error("Write Error");
    std::cout << fileName << std::endl;
    ServClient.path = fileName; 
//	ServClient.body = new char [bodyString.length() + 1];
////    ServClient.body = (char *)bodyString.c_str();
//	ServClient.body = strcpy(ServClient.body, bodyString.c_str());
//	ServClient.lengthOfbody = bodyString.length();
//	byteSent = ServClient.lengthOfbody;
//    ServClient.length = ServClient.lengthOfbody;
   file.close();
    close(ServClient.BodyOnly);
   std::remove(randomCgiFile.c_str());
   IsCgi = true;
//   std::cout << "cgi " << std::endl;
//   std::remove(PostCgiFile.c_str());
}


