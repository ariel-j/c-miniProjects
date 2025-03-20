all: myshell looper

myshell: myshell.o ../LineParser.o
	gcc -m32 -g -Wall -o myshell myshell.o ../LineParser.o

myshell.o: myshell.c ../LineParser.h
	gcc -m32 -g -Wall -I.. -c -o myshell.o myshell.c

../LineParser.o: ../LineParser.c ../LineParser.h
	gcc -m32 -g -Wall -I.. -c -o ../LineParser.o ../LineParser.c

looper: ../Looper.o
	gcc -m32 -g -Wall -o looper ../Looper.o

../Looper.o: ../Looper.c 
	gcc -m32 -g -Wall -I.. -c -o ../Looper.o ../Looper.c

clean:
	rm -f *.o ../LineParser.o ../Looper.o myshell looper
