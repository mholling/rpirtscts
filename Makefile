NAME = rpirtscts
PREFIX ?= /usr
BINDIR = $(DESTDIR)$(PREFIX)/bin/

$(NAME): $(NAME).c
	cc -o $(NAME) $(NAME).c -std=gnu99 -Wno-declaration-after-statement

clean:
	rm -f $(NAME)

install: $(NAME)
	mkdir -p "$(BINDIR)"
	cp $(NAME) "$(BINDIR)"
