NAME = rpirtscts
DESTDIR ?= /usr/local/bin

$(NAME): $(NAME).c
	cc -o $(NAME) $(NAME).c -std=gnu99 -Wno-declaration-after-statement

clean:
	rm -f $(NAME)

install: $(NAME)
	cp $(NAME) $(DESTDIR)
