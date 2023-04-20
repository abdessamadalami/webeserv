#include "HttpResponse.hpp"


std::string	checkStateOfPath(const std::string &filePath)
{
	struct stat st;

	if (stat(filePath.c_str(), & st) == 0)
	{
		if (st.st_mode != S_IRUSR)
			return "Read";
	}
	return "Not Found";
}
void 	HttpResponse::Get() {
	ServClient.is_autoIndexOn = NI_OFF;
	std::cout << "----------------url------------------" << std::endl;
	std::cout << this->fullUrl << std::endl;
	std::cout << locationI->prefix << std::endl;
	std::cout << "----------------------------------" << std::endl;
	if (!check_path()) // check if exist in root !!!
	{
		// requested resource not found
		std::cerr << "the requested  resource  not accessible or because of permissions" << std::endl;
		if (checkStateOfPath(fullUrl) == "Not Found") {
			ServClient.it = get_code("404");
			ServClient.path = serverI->error_pages[404];
		}
		else
		{
			ServClient.it = get_code("401");
			ServClient.path = serverI->error_pages[401];
		}
		return  ;
	}
	// Directory
	if (retriveResourceType() == "DIR") {
		if (*(request.url.end() - 1) == '/') {
			 get_dir();
		}
		else
		{
			std::cout << "moved perm" << std::endl;
			ServClient.it = get_code("301");
		}
	}
	else if (retriveResourceType() == "FILE")
	{
		get_file();
	}
	// request an uri with / at the end

}

void HttpResponse::generat_name(std::string type)
{
	std::string name;

	if(this->locationI->Upload.length() != 0)
	{
		name = this->locationI->Upload + "/";
	}
	else {
        name = "uploads/";
    }
	time_t now = time(0);
	char* dt = ctime(&now);
	dt[strlen(dt) - 1] = '\0';
	//std::cout << "this is time:"<<  dt <<"_"<< std::endl;
	name += dt;
	std::replace(name.begin(), name.end(), ' ', '_');
	name+=type;
    PostCgiFile = name;
	this->fp = fopen(name.c_str() , "w");
	std::cout << "this is fp"<< fp << " " << name << std::endl;
	if (fp == NULL)
		throw  std::runtime_error("fopen failed");
}

void HttpResponse::CheckBoundary()
{
	size_t startIndex = this->request.headers["Content-Type"].find("boundary=");
	if (startIndex < this->request.headers["Content-Type"].length())
	{
		this->request.boundary = "--" + this->request.headers["Content-Type"].substr(startIndex + 9,request.headers["Content-Type"].length()) ;
	}
	else
		this->request.boundary="";
}

int HttpResponse::GetContentType(char *body)
{
	char *content = (char *)strstr(body, "Content-Type: ");
	if(!content)
	{
		std::cerr << "Not enaght headr \n";
		exit(0);
	}
	char *slash = (char *)strstr(content, "\n");
	int pos = slash - content;
	content[pos - 1] = '\0';
	content += 14;
	this->request.headers["Content-Type"] = content;
	// std::cout << "this is the type of-"<< this->request.headers["Content-Type"] <<"-"<< std::endl
	this->request.body_start = body + ((content - body) + strlen(content) + 4); //!  1 \0\r\n
	this->bites -= (this->request.body_start - body);
	//!bites upload
	return 0;
}

int hexadecimalToDecimal(std::string hexVal)
{
    int len = hexVal.size();
	std::cerr<< hexVal << std::endl;
    // Initializing base value to 1, i.e 16^0
    int base = 1;
 
    int dec_val = 0;
    for (int i = len - 1; i >= 0; i--)
	{
		if(hexVal[i] >= 'a' && hexVal[i] <= 'f')
			hexVal[i]-= 32;
        if (hexVal[i] >= '0' && hexVal[i] <= '9') {
            dec_val += (int(hexVal[i]) - 48) * base;
            base = base * 16;
        }
        else if (hexVal[i] >= 'A' && hexVal[i] <= 'F') {
            dec_val += (int(hexVal[i]) - 55) * base;
            base = base * 16;
        }
    }
    return dec_val;
}
/*  */
bool HttpResponse::CheckForLastboundary()
{
	std::string last_b =  this->request.boundary + "--";
	std::cout << this->bites << std::endl;
	char *found = std::search(this->request.body_start , this->request.body_start + this->bites, last_b.c_str(), last_b.c_str() + last_b.length());
	if(found < this->request.body_start + this->bites)
	{
		fwrite(this->request.body_start, 1 , this->bites -  last_b.length() - 4, this->fp);
		fclose(this->fp);
		return true;
	}
	return false;
}

void HttpResponse::CheckForCgi()
{
	if (IsCgi)
	{
		BodyCgi = true;
		ServClient.isPost = false;
		this->post_file();
		return;
	}
	ServClient.it = get_code("201");
	ServClient.path = serverI->error_pages[201];
	ServClient.isPost = false;
}

bool HttpResponse::FirstChunked(char *body)
{
	int i = 0;
	char* found;
	const char* str = "\r\n";;
	std::string hexa;
	
	if(this->IsCgi)
		generat_name(".txt");
	else
		generat_name(GetContentTypeOfContent(this->request.headers["Content-Type"]));
	found = std::search(body, body + this->bites, str, str + strlen(str)) + 2;
	i = 2;
	if (found[0] == '\r' && found[1] == '\n')
	{
		while (found[i] != '\r')
		{
			hexa += found[i];
			i++;
		}
		std::cerr << "this is the hexa: " <<  hexa << std::endl;
		this->SizeOfChunked = hexadecimalToDecimal(hexa);
		if(this->SizeOfChunked == 0) //! just for the case of  empty file 
		{
			CheckForCgi();
			return true;
		}
	}
	found += i + 2;
	if(this->SizeOfChunked < this->bites) // ! case of have two hex in our body
	{
		int n = this->bites - (found - this->request.req);
		std::cout << "bites wrote:  " << n  << " this size of chunked " << this->SizeOfChunked  << std::endl;
		if(n > this->SizeOfChunked)
			fwrite(found , 1 , this->SizeOfChunked, this->fp);//! write the body without the zero hex
		else
			fwrite(found , 1 , n, this->fp);
			
		if(n < this->SizeOfChunked)
			return true;
		else
		{
			fclose(fp); //! more test about for this case??::  
			CheckForCgi();
			return true;
		}
	}
	else  //! this for normal case if we have just one hex
	{
		this->BytesWritten = this->bites - (found - this->request.req);
		fwrite(found, 1, this->BytesWritten, this->fp);
	}
	return false;
}
bool HttpResponse:: FristBoundry(char *body)
{
	std::cerr <<  " \n FristBoundry function\n\n";
	if(this->IsCgi)
	{
		generat_name(".txt");
		body+=4;
		this->BytesWritten = this->bites - (body - this->request.req);
		fwrite(body, 1 , this->BytesWritten , this->fp);
		if (this->BytesWritten >= atoi(this->request.headers["Content-Length"].c_str()))
		{
			this->BodyCgi = true;
			ServClient.isPost = false;
			fclose(this->fp);
			post_file();
		}
		return true;
	}
	this->BytesWritten = this->bites - (body - this->request.req);
	this->bites -= body - this->request.req;
	this->GetContentType(body); //! parce the many headr and change the content type 
	generat_name(GetContentTypeOfContent(this->request.headers["Content-Type"]));
	if (CheckForLastboundary())
	{
		CheckForCgi(); //! check for cgi or send response if not exist 
		return true;
	}
	fwrite(this->request.body_start, 1 , this->bites , this->fp);
	return false;
}

void HttpResponse::GetBody()
{
		ServClient.isPost = true;
		CheckBoundary(); //! check if the boundary is exist or not 
		char *body = ( char *)strstr(this->request.req, "\r\n\r\n");
        this->request.body = body + 5;
		std::cout<< "Content-Type " << this->request.headers["Content-Type"] << std::endl;
		if ((this->request.boundary.length() > 0 || IsCgi) && this->request.headers["Transfer-Encoding"].length() == 0)
		{
			if(FristBoundry(body))
				return;
		}
		else if (this->request.headers["Transfer-Encoding"].length() != 0 )//? chunked
		{
			if(FirstChunked(body))
				return;
		}
		else
		{
			generat_name(GetContentTypeOfContent(this->request.headers["Content-Type"]));
			body = body + 4;
			this->bites = this->bites - ((body) - this->request.req);
			fwrite(body , 1 , this->bites, this->fp);
			this->BytesWritten = this->bites;
			if(this->BytesWritten == atoi(this->request.headers["Content-Length"].c_str()))
			{
				fclose(this->fp);
				CheckForCgi();
			}
		}
}

void HttpResponse::Post()
{
    char *body = ( char *)strstr(this->request.req, "\r\n\r\n");
	std::cout << request.url << std::endl;
	std::cout << "post called" << std::endl;
	if (locationI->Upload.empty()) //! edited !!!!!
	{
		// get requested resource
		if (!check_path())
		{
			ServClient.it = get_code("404");
			ServClient.path = serverI->error_pages[404];
			std::cout << "the path not exist ." << ServClient.path << std::endl;
		} else // found
		{
			if (retriveResourceType() == "DIR")
				post_dir();
			else if (retriveResourceType() == "FILE")
				post_file();
		}
	}
	else
		GetBody();
}


// ********************* Deleting The Folder ********************* //

bool HttpResponse::delete_folder(std::string& path) {
	DIR *dir = opendir(path.c_str());

	if (dir == NULL) {
		std::cout << "Error: Unable to open directory" << std::endl;
		return false;
	}
	struct dirent *entry;

	while ((entry = readdir(dir)) != NULL) {
		std::string appendingPath = path + "/" + entry->d_name;

		if (entry->d_type == DT_DIR) {
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
				continue;
			}
			if (!delete_folder(appendingPath)) {
				closedir(dir);
				return false;
			}
		} else {
			if (std::remove(appendingPath.c_str()) != 0) {
				closedir(dir);
				return false;
			}
		}
	}
	closedir(dir);
	if (rmdir(path.c_str()) != 0) {
		if (errno == EACCES) {

			DeleteHandling.it = get_code("403");
			DeleteHandling.path = serverI->error_pages[403];

			std::cout << "Error: No write access on folder " << path << std::endl;
			exit(1);

			// We Should Make An Exception //
		} else
			perror("Error: Unable to delete folder");
		return false;
	}
	return true;
}

bool HttpResponse::delete_file(std::string& path) {
	if (std::remove(path.c_str()) == 0)
		return true;
	std::cout << "failed to delete" << std::endl;
	return false;
}

// ************** Delete Request Method *************** //

void HttpResponse::Delete() {


	if (!check_path()) // check if exist in root !!!
	{
		ServClient.it = get_code("404");
		ServClient.path = serverI->error_pages[404];
		return  ;
	}
	if (*(this->request.url.end() - 1) == '/')
	{
		if (!delete_folder(this->fullUrl))
		{
			ServClient.it = get_code("500");
			ServClient.path = serverI->error_pages[500];
			return  ;
		}
		else {
			ServClient.it = get_code("204");
		}
	}
	else { // Delete File
		this->fullUrl.erase(0, 1);
        std::cout << this->fullUrl << "[]" << std::endl;
        if (delete_file(this->fullUrl))
            ServClient.it = get_code("204");
        else
            ServClient.it = get_code("404");
	}
}
