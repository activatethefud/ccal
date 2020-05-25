ccal: main.o
	gcc -Wall -Wextra -lm -o ccal main.o

main.o: main.c

.PHONY: debug

debug:
	gcc -g -o a.out main.c -lm
