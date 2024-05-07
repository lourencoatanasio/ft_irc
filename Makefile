GREEN		=	\033[0;32m
RED			=	\033[0;31m
YELLOW	=	\033[0;33m
END			=	\033[0m

NAME		=	ft_irc

SRCS		=	$(wildcard src/*.cpp)

OBJ_DIR =	obj
OBJS    =	$(addprefix $(OBJ_DIR)/,$(notdir $(SRCS:.cpp=.o)))

CC			=	c++
CFLAGS	=	-Wall -Wextra -Werror -I./inc -std=c++98
RM			= rm -rf

all:			$(NAME)
					@echo "$(GREEN)ft_irc successfully compiled!$(END)"
					@sleep 0.5
					@clear

$(OBJ_DIR)/%.o:	src/%.cpp | $(OBJ_DIR)
								@$(CC) $(CFLAGS) -c $< -o $@

$(NAME):	$(OBJS)
					@clear
					@echo "$(YELLOW)Compiling ft_irc...$(END)"
					@$(CC) $(OBJS) $(CFLAGS) -o $(NAME)

$(OBJ_DIR):
					@mkdir -p $(OBJ_DIR)

clean:
					@$(RM) $(OBJ_DIR)
					@clear
					@echo "$(RED)All objects deleted!$(END)"

fclean:		clean
					@$(RM) $(NAME)
					@echo "$(RED)All executables deleted!$(END)"
					@sleep 0.5
					@clear

re:				fclean all

.PHONY:		all clean fclean re