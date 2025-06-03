NAME    = webserv

CC      = c++
CFLAGS  = -Wall -Wextra -Werror -std=c++98 -fsanitize=address -fno-omit-frame-pointer -g
CPPFLAGS= -Iinc

SRC_DIR = src
INC_DIR = inc

SRCS    = main.cpp \
		Config.cpp \
		ConfigParser.cpp \
		Server.cpp \
		ServerManager.cpp \
		Location.cpp \
		Utils.cpp \
		# Connection.cpp \
		Request.cpp \
		Response.cpp \


OBJS    = $(SRCS:.cpp=.o)

DEPS    = $(OBJS:.o=.d)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -c $< -o $@

-include $(DEPS)

clean:
	rm -f $(OBJS) $(DEPS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

# -----------------------------
# Unit test target
# -----------------------------
test:
	$(CC) $(CFLAGS) $(CPPFLAGS) \
		tests/test_server.cpp \
		$(SRC_DIR)/Location.cpp \
		$(SRC_DIR)/Server.cpp \
		-I$(INC_DIR) \
		-o test_server
	./test_server
