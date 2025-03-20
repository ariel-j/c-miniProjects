all:	loader	test

test.o: test.c
	gcc	-m32		-fno-stack-protector	-c	-o	test.o	test.c 

test: test.o
	ld	-m	elf_i386	start.o	test.o	-o	test

start.o:
	nasm	-f	elf32	start.s	-o	start.o

startup.o:
	nasm	-f	elf32	startup.s	-o	startup.o

loader.o:	loader.c
	gcc	-g	-m32	-Wall	-c	-o	loader.o	loader.c

loader:	loader.o	start.o	startup.o
	ld	-o	loader	loader.o	startup.o	start.o	-L/usr/lib32	-lc	-T	linking_script	-dynamic-linker	/lib/ld-linux.so.2

.PHONY:	clean

clean:
	rm	-f	*.o	loader	test