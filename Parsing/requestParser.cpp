
#include "requestParser.hpp"
#include <sstream>
#include <cstring>
requestParser* requestParser::requestParsing(const std::string& request) {
    
    requestParser* pRequestParser = new requestParser();
    char* parser = std::strtok((char *)request.c_str(), "\r\n");
	if (!parser)
		throw std::runtime_error("Error in parse request");
    std::string stringParser = parser;
    size_t lpost;
    size_t position = stringParser.find(' ');
    pRequestParser->Method = stringParser.substr(0, position);
    position++; //Skip character ' '

    std::string token = request;


    // Find path

    lpost = stringParser.find(' ', position);
    pRequestParser->url = stringParser.substr(position, (lpost - position));
    position++; //Skip character ' '

    // Find HTTP version

    position = stringParser.find(' ', position);
    pRequestParser->version = stringParser.substr(position + 1);

    // HTTP Headers

    token = token.substr(token.find_first_of("\r\n") + 1);

    std::istringstream file(token);
    std::string header;
    size_t pos;
    std::string key, value;

    while (std::getline(file, header) && header != "\r")
    {
        pos = header.find(':');
        if (pos != std::string::npos)
        {
            key = header.substr(0, pos);
            pos++;
            value = header.substr(pos + 1);
            pRequestParser->headers[key] = value.substr(0, value.size() - 1);
			key.clear();
			value.clear();
        }
    }
    /* Adding Cookies In Order To Work With it In "CGI" */
    if (!pRequestParser->headers["Cookie"].empty())
        this->Cookies = pRequestParser->headers["Cookie"];
    return (pRequestParser);
}

