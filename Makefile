# Compiler and flags
NAME = webserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

# Include directories
INCLUDES = -I./includes -I./includes/config -I./includes/server -I./includes/utils -I./includes/http -I./includes/http/httpRequest

# Directories
OBJ_DIR = obj

# Find all .cpp files recursively in src/
SRCS = $(shell find src -name '*.cpp')

# Map each .cpp file in src/ to a .o file in obj/ with the same subdirectory structure
OBJS = $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

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
