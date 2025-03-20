all: menu

menu: menu.o
	gcc -m32 -g -Wall  -o menu menu.o

menu.o: menu.c
	gcc -m32 -g -Wall  -c -o menu.o menu.c

.PHONY: clean

clean:
	rm -f *.o menu