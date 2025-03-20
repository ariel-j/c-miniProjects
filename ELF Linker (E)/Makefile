all: setup myELF 

myELF: myELF.o
	gcc	-g	-m32	-Wall	-o	myELF	myELF.o

myELF.o: myELF.c
	gcc	-g	-m32	-Wall	-c	-o	myELF.o	myELF.c 

# Setup target to copy ELF files
setup:
	cp ELF_file/* . 2>/dev/null || :

.PHONY: clean setup

clean:
	rm -f *.o myELF
	rm -f *.ro
	rm -f F1*.o F2*.o  # Remove copied ELF files