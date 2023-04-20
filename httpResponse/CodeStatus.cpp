
#include "CodeStatus.hpp"

void StatusCodes::setCode(const std::string& key, std::string value) {
	statusCode[key] = value;
}

StatusCodes::~StatusCodes() {

}

StatusCodes::StatusCodes() {
	std::cout << "Status codes constrictor" << std::endl;
	setCode("501", "Not Implemented");
    setCode("500", "Internal Server Error");
    setCode("204", "No Content");
	setCode("400", "Bad Request");
	setCode("414", "Request-URI Too Long");
	setCode("413", "Request Entity Too Large");
	setCode("404", "Not Found");
	setCode("301", "Moved Permanently");
	setCode("405", "Method Not Allowed");
	setCode("405", "Method Not Allowed");
	setCode("403", "Forbidden");
	setCode("200", "OK");
	setCode("409", "Conflict");
	setCode("401", "Unauthorized");
	setCode("201", "Created");
}

MapIterator StatusCodes::get_code(const std::string key)
{
	std::cout << statusCode[key] << std::endl;
	return statusCode.find(key);
}