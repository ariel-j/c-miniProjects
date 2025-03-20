all: myshell

myshell: myshell.o ../LineParser.o
	gcc -m32 -g -Wall -o myshell myshell.o ../LineParser.o

myshell.o: myshell.c ../LineParser.h
	gcc -m32 -g -Wall -I.. -c -o myshell.o myshell.c

../LineParser.o: ../LineParser.c ../LineParser.h
	gcc -m32 -g -Wall -I.. -c -o ../LineParser.o ../LineParser.c

clean:
	rm -f *.o ../LineParser.o myshell
