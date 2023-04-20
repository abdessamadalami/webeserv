
NAME = webserv
SRCS = webserv.cpp Parsing/ConfigParsing.cpp Parsing/requestParser.cpp \
	Server/Server.cpp httpResponse/HttpResponse.cpp httpResponse/CodeStatus.cpp   \
	httpResponse/Methods.cpp httpResponse/Response.cpp httpResponse/CgiExecution.cpp

CFLAGS =  -fsanitize=address -g  -std=c++98

all : $(NAME)

$(NAME) : $(SRCS)
	c++  $(CFLAGS) $(SRCS) -o $(NAME)
clean : fclean

fclean :
	rm -f $(NAME)

re : fclean all
.PHONY: clean all clean re fcleandd
