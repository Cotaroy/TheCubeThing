.PHONY: all clean
FLAGS = -Wall -g

all: space

space: space.o
	gcc ${FLAGS} -o $@ $^

space.o: space.c space.h
	gcc ${FLAGS} -c $<

tags: *.c *.h
	ctags **.c **.h

clean:
	rm -f *.o space
