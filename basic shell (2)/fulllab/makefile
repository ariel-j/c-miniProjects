all:myshell looper mypipe

myshell:	LineParser.o	task2.o
	gcc	-g	-m32	-Wall	 -o	myshell task2.o LineParser.o

task2.o:	task2.c	LineParser.h
	gcc	-g	-m32	-Wall	 -c	-o task2.o task2.c


looper: looper.o
	gcc	-g	-m32	-Wall	 -o	looper looper.o

LineParser.o: LineParser.c LineParser.h
	gcc -g -m32 -Wall	-c	-o	LineParser.o	LineParser.c

looper.o: looper.c
	gcc -g -m32 -Wall	-c	-o	looper.o	looper.c

mypipe:	mypipe.o
	gcc -g -m32 -Wall	-o	mypipe	mypipe.o

mypipe.o: mypipe.c
	gcc -g -m32 -Wall	-c	-o	mypipe.o	mypipe.c


.PHONY: clean

clean:
	rm -f *.o looper task2
