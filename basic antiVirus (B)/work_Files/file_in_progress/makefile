all:	part1

part1:	part1.o
	gcc	-g	-m32	-Wall	-o	part1	part1.o

part1.o:	part1.c
	gcc	-g	-m32	-Wall	-c	-o	part1.o	part1.c

.PHONY: clean

clean:
	rm -f *.o part1