.PHONY: all clean
FLAGS = -Wall -g

all: test_space test_raycast

test_space: test_space.o space.o
	gcc ${FLAGS} -o $@ $^

test_raycast: test_raycast.o raycast.o space.o
	gcc ${FLAGS} -o $@ $^ -lm

space.o: space.c space.h
	gcc ${FLAGS} -c $<

test_space.o: test_space.c space.h
	gcc ${FLAGS} -c $<

raycast.o: raycast.c raycast.h space.h
	gcc ${FLAGS} -c $<
	
test_raycast.o: test_raycast.c raycast.h space.h
	gcc ${FLAGS} -c $<

tags: *.c *.h
	ctags **.c **.h

renderer: renderer.c
	gcc ${FLAGS} -o $@ $<

clean:
	rm -f *.o test_space test_raycast
