NAME = ft_nm

CC = gcc

FLAGS = -Wall -Wextra -Werror

SRC = main.c utils.c

INC = ft_nm.h

OBJ = $(SRC:.c=.o)

DEP = $(OBJ:.o=.d)

all:	$(NAME)

-include $(DEP)

$(NAME):	$(OBJ) $(SRC) $(INC)
			$(CC) $(FLAGS) -o $(NAME) $(OBJ)

clean :
			rm -rf $(OBJ)

fclean :	clean
			rm -rf $(NAME)

re :		fclean all

.PHONY: 	all clean fclean re

