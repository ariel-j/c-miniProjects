all:  encoder

encoder: start.o encoder.o
	ld -m elf_i386 start.o encoder.o -o encoder

# task2: start.o task2.o Util.o
# 	ld -m elf_i386 start.o task2.o Util.o -o task2

start.o: start.s
	nasm -f elf32 start.s -o start.o

Util.o: Util.c util.h
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector Util.c -o Util.o

main.o: main.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector main.c -o main.o

encoder.o: encoder.s
	nasm -f elf32 encoder.s -o encoder.o

# task2.o: task2.c
# 	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector task2.c -o task2.o

.PHONY: clean
clean:
	rm -f *.o  task1 task2