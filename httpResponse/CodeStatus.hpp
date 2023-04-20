#ifndef STATUSCODE_HPP
#define STATUSCODE_HPP
#include <map>
#include <iostream>
typedef std::map<std::string, std::string>::iterator MapIterator;

class StatusCodes {
public:
	StatusCodes();
	MapIterator get_code(std::string key);
	void		setCode(const std::string& key, std::string value);
	std::map<std::string, std::string>  statusCode;
	~StatusCodes();
};

#endif