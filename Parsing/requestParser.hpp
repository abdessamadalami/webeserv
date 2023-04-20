
#ifndef WEBSERV_REQUESTPARSER_HPP
#define WEBSERV_REQUESTPARSER_HPP

#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <vector>

//typedef enum Method {GET, POST, DELETE} Method;

class requestParser {
public :
    std::string url;
    std::string Method;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
    std::string boundary;
    int n_boundary;
    char* req; //! this for the body chunked 
    char* body_start;
    std::string Cookies;

    /* Functions */

    requestParser *requestParsing(const std::string& request);
};


#endif
