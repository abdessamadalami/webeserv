
#ifndef PARSER_HPP
# define PARSER_HPP

# include <map>
# include <iostream>
# include <utility>
# include <vector>
# include <fstream>
# include <exception>
# include <fstream>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
# include <list>


typedef struct s_listen {
    std::string ip_address;
    std::string 		port;
} t_listen;

typedef struct s_location {
	std::string root_dir;
    std::string                 prefix;
    std::vector<std::string>    allowed_methods;
    int                         autoIndex;
    std::vector<std::string>    index_files;
    std::vector<std::string>    http_redirect;
	std::string 				Upload;

    /* Inserting The 'CGI' INFOS */
    std::map<std::string, std::string> _cgi;
}	t_location;

typedef struct s_server {
	t_listen					listen;
	std::vector<std::string>	server_names;
	std::map<int, std::string>	error_pages;
	size_t						client_bodySize;
	std::vector<t_location>		locations; // * multiple  locations
}	t_server;



class ConfigParsing {
	public:
    std::vector<std::string> File;
    std::vector<t_server>	servers; //   * multiple servers
    ConfigParsing();
    explicit ConfigParsing(std::string& file);
    ~ConfigParsing();
	void	parse();
	void 	setFilePath(std::string &path);
private:
//	void check_server_dup();
//	void checkLocationDup();
	void check_duplication(void);
	void setData(std::string& key, std::vector<std::string> &value);
    bool		find_server(std::string& content);
//    bool		check_inside(const std::string& content);
    bool		check_inside1(std::vector<std::string>::iterator vec, std::vector<std::string>::iterator vec1);
    t_location       check_location(std::vector<std::string>::iterator *it);
    void        parsing(std::vector<std::string> vec);
    int 		checkClosedBrackets;
    std::string		_file;

    /*    CGI    */

};

/* Probably USING std::list */

#endif