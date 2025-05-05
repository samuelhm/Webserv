# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: erigonza <erigonza@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/03/25 11:46:34 by shurtado          #+#    #+#              #
#    Updated: 2025/05/05 12:18:57 by shurtado         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		=	Web_Server
CC			=	c++
CPPFLAGS	=	-std=c++98 -MMD -MP -Wshadow #-g
CPPFLAGS	+=	-Wall -Werror -Wextra -MMD -MP -pedantic #-fsanitize=address

SRC			=	src/main.cpp src/EventPool/EventPool.cpp \
					src/ConfigFile/Location.cpp src/ConfigFile/Server.cpp \
					src/HTTP/HttpRequest.cpp src/HTTP/UriParse.cpp \
					src/HTTP/HttpResponse.cpp src/HTTP/CgiExec.cpp\
					src/HTTP/AHttp.cpp src/ConfigFile/ParseConfig.cpp \
					src/ConfigFile/ParseLocation.cpp src/Utils/AutoIndexTable.cpp \
					src/Utils/Utils.cpp src/Utils/AutoIndex.cpp src/Utils/Logger.cpp

OBJS		=	$(SRC:%.cpp=obj/%.o)
DEPS		=	$(OBJS:.o=.d)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(INCLUDES) $(CPPFLAGS) -o $@ $(OBJS) $(LDFLAGS)

obj/%.o: %.cpp Makefile
	@mkdir -p $(dir $@)
	$(CC) $(INCLUDES) $(CPPFLAGS) -c $< -o $@


clean:
	rm -rf obj

f fclean: clean
	rm -f $(NAME)

re: fclean all

v: $(NAME)
	clear
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) Config_Empty.conf

r: $(NAME)
	clear
	@./$(NAME) config


-include $(DEPS)

.PHONY: all clean fclean re r f v
