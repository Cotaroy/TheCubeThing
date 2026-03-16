.PHONY: all clean
FLAGS = -Wall -g

all: space

space: space.o
	gcc ${FLAGS} -o $@ $^

space.o: space.c space.h
	gcc ${FLAGS} -c $<

clean:
	rm -f *.o space
