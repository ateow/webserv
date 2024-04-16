NAME = interfaces
COMPILER = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -g -Wshadow -Wno-shadow
INCLUDES = -Iincludes

SRCS_DIR = srcs/
BUILD_DIR = build/
SRCS = *.cpp
OBJS = $(SRCS:.cpp=.o)
OBJS_PREFIXED = $(addprefix $(BUILD_DIR), $(OBJS))

all: $(NAME)

$(NAME): $(OBJS_PREFIXED)
	$(COMPILER) $(CFLAGS) $(INCLUDES) -o $(NAME) $(OBJS_PREFIXED)

$(BUILD_DIR)%.o: $(SRCS_DIR)%.cpp
	mkdir -p $(BUILD_DIR)
	$(COMPILER) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS_PREFIXED)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
