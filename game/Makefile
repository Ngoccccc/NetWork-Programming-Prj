LIBS=-lncurses

all: test

test: test.o tetris.o
	gcc test.o tetris.o -o test $(LIBS)

tetris.o: tetris.c
	gcc -c -Wall tetris.c

test.o: test.c
	gcc -c -Wall test.c

clean:
	rm -f *.o *~