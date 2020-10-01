ccal: main.o llist.o tokenizer.o random.o
	gcc -Wall -Wextra -lm -o ccal main.o llist.o tokenizer.o random.o

main.o:
llist.o:
tokenizer.o:
random.o:

.PHONY: debug

debug:
	gcc -g -o a.out main.c -lm
