all: pipeline

pipeline: pipeline.o
	gcc	-g	-m32	-Wall	-o	pipeline	pipeline.o


pipeline.o: pipeline.c
	gcc	-g	-m32	-Wall	-c	-o	pipeline.o	pipeline.c 

.PHONY: clean

clean:
	rm -f *.o pipeline
