.PHONY: all clean
FLAGS = -Wall -g

all: space

space: space.o
	gcc ${FLAGS} -o $@ $^

raycast: raycast.o
	gcc ${FLAGS} -o $@ $^

space.o: space.c space.h
	gcc ${FLAGS} -c $<

raycast.o: raycast.c raycast.h space.h
	gcc ${FLAGS} -c $<
	
clean:
	rm -f *.o space raycast
