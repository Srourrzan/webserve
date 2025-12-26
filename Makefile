# Compiler and flags
NAME = webserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

# Include directories
INCLUDES = -I./includes -I./includes/config -I./includes/server -I./includes/utils -I./includes/http

# Directories
OBJ_DIR = obj

# Source files
SRCS = src/webserv.cpp \
       src/config/ConfigTokenizer.cpp \
       src/config/ConfigValidator.cpp \
       src/config/ConfigParser.cpp \
       src/config/ConfigStructures.cpp \
       src/server/Server.cpp \
       src/http/HttpRequest.cpp \
       src/http/HttpResponse.cpp \
       src/utils/utils.cpp

# Object files (in obj directory)
OBJS = $(SRCS:src/%.cpp=$(OBJ_DIR)/%.o)

# Default target
all: $(NAME)

# Build
$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

# Compile .cpp to .o in obj directory
$(OBJ_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean object files
clean:
	rm -rf $(OBJ_DIR)

# Full clean: remove objects and binary
fclean: clean
	rm -f $(NAME)

# Rebuild everything
re: fclean all

# Phony targets
.PHONY: all clean fclean re
