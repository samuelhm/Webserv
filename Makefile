# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/03/25 11:46:34 by shurtado          #+#    #+#              #
#    Updated: 2025/03/25 11:58:02 by shurtado         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		=	PmergeMe
CC			=	c++
CPPFLAGS	=	-std=c++98 -g -Wall -Werror -Wextra -MMD -MP -fsanitize=address -pedantic

SRC			=	src/main.cpp \
				src/ConfigFile/Location.cpp src/ConfigFile/Server.cpp \
				src/HTTP/HttpRequest.cpp

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

fclean: clean
	rm -f $(NAME)

re: fclean all

-include $(DEPS)

.PHONY: all clean fclean re
