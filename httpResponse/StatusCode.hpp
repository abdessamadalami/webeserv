#ifndef STATUSCODE_HPP
#define STATUSCODE_HPP
#include <map>

class StatusCodes {
public:
    StatusCodes();
    std::string get_code(std::string key);
    std::map<std::string, std::string>  statusCode;
};
#endif
