all: hello

hello: hello.o
	ld	-m	elf_i386	hello.o	-o	hello

hello.o: hello.s
	nasm	-f	elf32	hello.s	-o	hello.o

.PHONY: clean

clean:
	rm -f *.o hello