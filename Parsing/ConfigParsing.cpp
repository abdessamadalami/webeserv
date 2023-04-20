//
// Created by Abdelhak El moussaoui on 1/4/23.
//

#include <sstream>
#include "ConfigParsing.hpp"
#include <unistd.h>
#include <cstring>
#include <cstdlib>

void ConfigParsing::setData(std::string &key, std::vector<std::string> &value) {

}
// constructors
ConfigParsing::ConfigParsing() : _file(std::string("/Users/ael-mous/Desktop/42_webserv/nginx.conf")) {
	std::cout << "Config Parsing cons"  << std::endl;
	checkClosedBrackets = 0;
}

ConfigParsing::ConfigParsing(std::string &file): _file(file) {
	checkClosedBrackets = 0;
}

ConfigParsing::~ConfigParsing() {

}

// end constructors

/*
 *  { ** } the areas called "context"
 *
 */

std::string	trim(const std::string& content)
{
	size_t first = content.find_first_not_of(" \t");
	if (first == std::string::npos)
		return "";
	return (content.substr(first,  content.size() - first));
}

bool	ConfigParsing::find_server(std::string& content)
{
	size_t pos = content.find("server ");

	if (pos == std::string::npos)
		return false;
//	std::cout << "#" << content << std::endl;
	std::string sub = content.substr(pos + std::strlen("server "));
	std::string::iterator it1 = sub.begin();
	std::string line;
	while (it1 != sub.end())
	{
		line = *it1;
		if (line == " " || line == "\t") {
			it1++;
		}
		else if (line == "{")
		{
			if (line == "{")
				checkClosedBrackets++;
			it1++;
		}
		else
		{
			std::cout << "err at server line" << std::endl;
			return false;
		}
	}
	return true;
}

void	check_semi_colon(std::string content)
{
	if (content[content.size() - 1] != ';' || content.find(';') != content.size() - 1) {
		std::cout << "no semi colon" << std::endl;
		exit(1);
		/* We Should Throw An Exception  */
	}
}

t_location ConfigParsing::check_location(std::vector<std::string>::iterator *it)
{
	t_location      location;
	std::string		content;
	size_t			posi;
	content = *(*it);
	posi = content.find_first_of('{');
	if (posi == std::string::npos)
	{
		std::cout << "error at location line" << std::endl;
		exit(1);
	}
	location.prefix = (*it)->substr(std::strlen("location "), (posi - strlen("location ") - 1));
	size_t pos = (*it)->find_first_of("}");
	(*it)++;
	location.autoIndex = 0;
	while (pos == std::string::npos)
	{
		if ((*it)->substr(0, (*it)->find_first_of(' ')) == "index")
		{
			content = *(*it);
			check_semi_colon(content);
			std::string index_file = trim(content.substr(pos + std::strlen("index ")));
			posi = index_file.find(' ');
			while ( posi != std::string::npos)
			{
				location.index_files.push_back(index_file.substr(0, posi));
				index_file = index_file.erase(0, posi + 1);
				posi = index_file.find(' ');
			}
			location.index_files.push_back(index_file.substr(0, index_file.size() - 1));
		}
		else  if ((*it)->substr(0, (*it)->find_first_of(' ')) == "root")
		{
			content = *(*it);

			check_semi_colon(content);
			std::string root_file = trim(content.substr(pos + std::strlen("root ")));
			posi = root_file.find(';');
			if (posi != std::string::npos)
				location.root_dir = root_file.substr(0, posi);
		}
		else if ((*it)->substr(0, (*it)->find_first_of(' ')) == "autoindex")
		{
			content = *(*it);
			check_semi_colon(content);
			std::string autoin = trim(content.substr(std::strlen("autoindex")));
			posi = autoin.find(';');
			if (posi != std::string::npos)
			{
				autoin = autoin.substr(0, posi);
				if (autoin == "on") {
					location.autoIndex = 1;
				} else if (autoin == "off") {
					location.autoIndex = 0;
				} else {
					std::cout << "unrecognized token at autoindex" << std::endl; // throw exception instead
					std::exit(EXIT_FAILURE);
				}
			}

		}
		else if ((*it)->substr(0, (*it)->find_first_of(' ')) == "upload")
		{
			content = *(*it);

			check_semi_colon(content);

			std::string upload_dir = trim(content.substr(std::strlen("upload")));

			posi = upload_dir.find(';');

			if (posi != std::string::npos)
				location.Upload = upload_dir.substr(0, posi);
		}
		else if ((*it)->substr(0, (*it)->find_first_of(' ')) == "allow_methods")
		{
			content = *(*it);
			check_semi_colon(content);
			content = trim(content.substr(pos + std::strlen("allow_methods ")));
			while((posi = content.find_first_of(' ')) != std::string::npos)
			{
				location.allowed_methods.push_back(content.substr(0, posi));
				content = content.erase(0, posi + 1);
			}
			posi = content.find_first_of(';');
			location.allowed_methods.push_back(content.substr(0, posi));
		}
		else if ((*it)->substr(0, (*it)->find_first_of(' ')) == "return")
		{
			content = *(*it);
			check_semi_colon(content);
			content = trim(content.substr(pos + std::strlen("return ")));
			while((posi = content.find_first_of(' ')) != std::string::npos)
			{
				location.http_redirect.push_back(content.substr(0, posi));
				content = content.erase(0, posi + 1);
			}
			posi = content.find_first_of(';');
			location.http_redirect.push_back(content.substr(0, posi));
		}
        else if ((*it)->substr(0, (*it)->find_first_of(' ')) == "cgi")
        {
            content = *(*it);
            check_semi_colon(content);
            content = trim(content.substr(pos + std::strlen("cgi ")));
            size_t position = content.find_first_of(' ');
            if (position != std::string::npos) {
                std::string extension = content.substr(0, position);
                content = content.erase(0, position + 1);
                size_t s = content.find(';');
                if ( s != std::string::npos)
                    content = content.substr(0, s);
                location._cgi[extension] = content;
            }
            else
            {
                std::cout << "ERROR IN CGI!!" << std::endl;
                exit(1);

                /* We Should Throw An Exception  */
//                throw std::exception(std::invalid_argument("Error In The ConfigFile"));
            }
        }
		content.clear();
		(*it)++;
		pos = (*it)->find_first_of("}");
	}
	return location;
}

bool		ConfigParsing::check_inside1(std::vector<std::string>::iterator vec, std::vector<std::string>::iterator vec1)
{
    size_t pos;
	t_server		server;
    while (vec != vec1)
    {
		if (*vec->begin() == '#')
			continue;
        else if ((pos = vec->find("listen ")) != std::string::npos)
        {
            std::string content = *vec;
            if (content.find(':') != std::string::npos)
            {
                std::string listen = trim(content.substr(pos + std::strlen("listen")));

                server.listen.ip_address = listen.substr(0, listen.find(':'));
                if (server.listen.ip_address.empty())
					throw std::runtime_error("Error ip address empty");
                size_t len = listen.find(';') - listen.find(':');

                server.listen.port = listen.substr(listen.find(':') + 1, len - 1);
                if (server.listen.port.empty())
					throw std::runtime_error("Error port empty");
            }
            if (content[content.size() - 1] != ';' || content.find(';') != content.size() - 1) {
				throw std::runtime_error("No semi colon");
                /* We Should Throw An Exception  */
            }
//            return true;
        }
        else if ((pos = vec->find("server_name " ) != std::string::npos))
        {
            std::string content = *vec;
            if (content[content.size() - 1] != ';' || content.find(';') != content.size() - 1) {
				throw std::runtime_error("No semi colon");
                /* We Should Throw An Exception  */
            }
            std::string server_name = trim(content.substr(pos + std::strlen("server_name")));

            size_t posi = server_name.find(' ');
            while ( posi != std::string::npos)
            {
                server.server_names.push_back(server_name.substr(0, posi));
                server_name = server_name.erase(0, posi + 1);
                posi = server_name.find(' ');
            }
            server.server_names.push_back(server_name);
//            return true;
        }
        else if ((pos = vec->find("error_page ")) != std::string::npos)
        {
            std::string content = *vec;
            if (content[content.size() - 1] != ';' || content.find(';') != content.size() - 1)
				throw std::runtime_error("No semi colon");
                /* We Should Throw An Exception  */
            std::string errors = trim(content.substr(pos + std::strlen("error_page")));

            size_t posi = errors.find(' ');
            if (posi != std::string::npos)
            {
                int error_number = std::atoi(errors.substr(0, posi).c_str());
                errors = errors.erase(0, posi + 1);
                server.error_pages[error_number] = errors.substr(0, errors.size() - 1);
            }
        }
        else if ((pos = vec->find("client_body_size ")) != std::string::npos)
        {
            std::string content = *vec;
            if (content[content.size() - 1] != ';' || content.find(';') != content.size() - 1)
				throw std::runtime_error("No semi colon");
                /* We Should Throw An Exception  */

            std::string bodySize = trim(content.substr(pos + std::strlen("client_body_size")));
            size_t posi = bodySize.find(';');
            if (posi != std::string::npos)
                server.client_bodySize = std::atoi(bodySize.substr(0, posi).c_str());

//            return true;
        }
        else if ((pos = vec->find("location ")) != std::string::npos) {
			std::string content = *vec;
			std::string substring = content.substr(pos + std::strlen("location "));
			size_t posi = substring.find_first_of('{');
			if (posi != std::string::npos) {
				server.locations.push_back(check_location(&vec));
			} else
				throw std::runtime_error("ERROR IN LOCATION BRACKETS!!!!");
		}
		else
		{
			throw std::runtime_error("Erorr in server config !!!!");
		}
        vec++;
    }
	servers.push_back(server);
    return true;
}

void ConfigParsing::parsing(std::vector<std::string> vec)
{
    std::vector<std::string>::iterator it = vec.begin();
    std::vector<std::string>::iterator ite = vec.begin();
//    size_t pos = it->find_first_not_of(" /t/n");
//    *it = it->substr(0, pos);	
	int openBrackets;
	int closedBrackets;
//	std::cout << "#" << *vec.begin() << "#"<< std::endl;
	while (it != vec.end() && find_server(*it))
	{
		closedBrackets = 0;
		openBrackets = 0;
		while (ite != vec.end())
		{
			if (ite->find_first_of('}') != std::string::npos)
			{
				++openBrackets;
			}
			else if (ite->find_first_of('{') != std::string::npos)
				closedBrackets++;
			if (openBrackets == closedBrackets)
			{
				break ;
			}
			ite++;
		}
		it++;
		check_inside1(it, ite);
		it = ite;
		++it;
		ite++;
	}
	std::vector<t_server>::iterator ss = servers.begin();
}

void ConfigParsing::parse()
{
	std::ifstream	configFile;

	configFile.open(_file);
	if (configFile.fail())
		throw std::runtime_error("Config file failed to open .");
    std::string read;
    while (std::getline(configFile, read)) {
        read = trim(read);
        File.push_back(read);
    }
    parsing(File);
	check_duplication();
	configFile.close();
}

void ConfigParsing::check_duplication(void) {
	std::vector<t_server>::iterator it = servers.begin();
	size_t  j = 0;
	if (servers.empty())
	{
		std::cout << "empty file" << std::endl;
		exit(1);
	}
	while (it != servers.end())
	{
		if (it->locations.empty())
			throw std::runtime_error("Empty location.");
		for (int i = 0; i < it->locations.size(); ++i) {
			j = i + 1;
			while (j < it->locations.size()) {
				if (it->locations[i].prefix == it->locations[j].prefix)
					throw std::runtime_error("duplicate location");
				j++;
			}
		}
		it++;
	}
}

void ConfigParsing::setFilePath(std::string &path) {
	_file = path;
}
