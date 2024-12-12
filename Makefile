CC := clang
CFLAGS := -g -fsanitize=thread

all: test

clean:
	rm -f test

test: main.c inventory.c inventory.h
	$(CC) $(CFLAGS) -o test main.c inventory.c -lpthread