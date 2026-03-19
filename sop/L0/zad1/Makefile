all: prog1b
prog1b:
	gcc -Wall -fsanitize=address,undefined -o prog1b prog1b.c

.PHONY: clean all
clean:
	-rm -f prog1b