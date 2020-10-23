ccal: main.o llist.o tokenizer.o random.o
	gcc -o ccal main.o llist.o tokenizer.o random.o -Wall -Wextra -lm 

main.o:
llist.o:
tokenizer.o:
random.o:

.PHONY: debug

debug:
	gcc -g -o a.out main.c -lm
