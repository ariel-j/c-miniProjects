all: count-words

count-words: count-words.o
	gcc -m32 -g -Wall -Wextra -o count-words count-words.o

count-words.o: count-words.c
	gcc -m32 -g -Wall -Wextra -c count-words.c

clean:
	rm -f *.o count-words
