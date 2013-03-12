NAME = rpirtscts

$(NAME): $(NAME).c
	cc -o $(NAME) $(NAME).c -std=gnu99 -Wno-declaration-after-statement

clean:
	rm -f $(NAME)
