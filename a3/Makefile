.PHONY: all clean
FLAGS = -Wall -g

all: test_space test_raycast test_renderer

test_space: test_space.o space.o
	gcc ${FLAGS} -o $@ $^

test_raycast: test_raycast.o raycast.o space.o
	gcc ${FLAGS} -o $@ $^ -lm

test_renderer: test_renderer.o renderer.o raycast.o space.o
	gcc ${FLAGS} -o $@ $^ -lm

space.o: space.c space.h
	gcc ${FLAGS} -c $<

test_space.o: test_space.c space.h
	gcc ${FLAGS} -c $<

raycast.o: raycast.c raycast.h space.h
	gcc ${FLAGS} -c $<
	
test_raycast.o: test_raycast.c raycast.h space.h
	gcc ${FLAGS} -c $<

renderer.o: renderer.c renderer.h
	gcc ${FLAGS} -c $<


test_renderer.o: test_renderer.c renderer.h raycast.h space.h
	gcc ${FLAGS} -c $<

# temporary
camera: camera.c raycast.c space.c renderer.c camera.h raycast.h space.h renderer.h
	gcc -Wall -g -o camera camera.c raycast.c space.c renderer.c -lm

tags: *.c *.h
	ctags **.c **.h

clean:
	rm -f *.o test_space test_raycast test_renderer
