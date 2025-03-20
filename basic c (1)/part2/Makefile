all: base
base: base.o
	gcc -m32 -g -Wall  -o base base.o

base.o: base.c
	gcc -m32 -g -Wall  -c base.o base.c

clean:
	rm -f *.o base