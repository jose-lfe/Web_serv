C++ = c++
C++_FLAGS = -Wall -Wextra -Werror -std=c++98

# dossier sources et build
SRC_DIR = src
OBJDIR = build

# liste des sources
SRCS =	main.cpp \
		autoindex.cpp \
		buildResponse.cpp \
		CGI.cpp \
		Client.cpp \
		handleGallery.cpp \
		handleMethod.cpp \
		handleRequest.cpp \
		HttpServer.cpp \
		matchConfig.cpp \
		Parser.cpp \
		SimpleRouter.cpp \
		Socket.cpp \
		Struct.cpp \
		utils.cpp

# ajout du chemin src/
SRCFILES = $(addprefix $(SRC_DIR)/, $(SRCS))

# objets dans build/
OBJS = $(SRCS:.cpp=.o)
OBJFILES = $(addprefix $(OBJDIR)/, $(OBJS))

NAME = webserv

all: $(NAME)

$(NAME): $(OBJFILES)
	$(C++) $(C++_FLAGS) $(OBJFILES) -o $(NAME)

# règle pour compiler les .o dans build à partir de src
$(OBJDIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(C++) $(C++_FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
