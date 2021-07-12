objects = main.o random.o tokenizer.o llist.o
ccal: main.o random.o tokenizer.o llist.o
	gcc -o ccal $(objects) -Wall -Wextra -lm
	#gcc -o ccal main.c random.c -lalg -Wall -Wextra -lm

main.o:
random.o:
tokenizer.o:
llist.o:

.PHONY: debug
debug:
	gcc -g main.c random.c -lalg -Wall -Wextra -lm

.PHONY: clean
clean:
	rm ccal $(objects)
