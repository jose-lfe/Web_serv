C++ = c++
C++_FLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic

SRCS =	main.cpp\
		autoindex.cpp\
		HandleRequest.cpp\
		HttpServer.cpp\
		Parser.cpp\
		SimpleRouter.cpp\
		Socket.cpp\
		Struct.cpp\

OBJDIR = build
OBJS = $(SRCS:.cpp=.o)
OBJFILES = $(addprefix $(OBJDIR)/, $(OBJS))

NAME = webserv

all: $(NAME)

$(NAME): $(OBJFILES)
	$(C++) $(C++_FLAGS) $(OBJFILES) -o $(NAME)

# règle pour créer le dossier et compiler les .o dedans
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)
	$(C++) $(C++_FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
