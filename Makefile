ccal: main.o llist.o tokenizer.o
	gcc -Wall -Wextra -lm -o ccal main.o llist.o tokenizer.o

main.o:
llist.o:
tokenizer.o:

.PHONY: debug

debug:
	gcc -g -o a.out main.c -lm
