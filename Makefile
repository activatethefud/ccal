objects = main.o tokenizer.o random.o
ccal: main.o tokenizer.o random.o
	gcc -o ccal $(objects) ../libs/libalg.a -Wall -Wextra -lm

main.o:
tokenizer.o:
random.o:

.PHONY: debug
debug:
	gcc -g -o a.out main.c -lm

.PHONY: clean
clean:
	rm ccal $(objects)
