all:myshell looper mypipe

myshell:	LineParser.o	myshell.o
	gcc	-g	-m32	-Wall	 -o	myshell myshell.o LineParser.o

myshell.o:	myshell.c	LineParser.h
	gcc	-g	-m32	-Wall	 -c	-o myshell.o myshell.c


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
	rm -f *.o looper myshell
