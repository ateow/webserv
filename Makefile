NAME = webserv
COMPILER = g++
CFLAGS = -Wall -Wextra -Werror -g -Wshadow -Wno-shadow -std=c++98
INCLUDES = -Iincludes

SRCS_DIR = srcs/
BUILD_DIR = build/
SRCS := $(wildcard $(SRCS_DIR)*.cpp)
# SRCS := srcs/main.cpp srcs/EpollServer.cpp
OBJS := $(SRCS:$(SRCS_DIR)%.cpp=$(BUILD_DIR)%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(COMPILER) $(CFLAGS) $(INCLUDES) -o $(NAME) $(OBJS)

$(BUILD_DIR)%.o: $(SRCS_DIR)%.cpp
	 mkdir -p $(BUILD_DIR)
	$(COMPILER) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(NAME)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
