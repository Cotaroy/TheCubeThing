.PHONY: all clean
FLAGS = -Wall -g

all: test_space test_raycast test_camera

test_space: test_space.o space.o
	gcc ${FLAGS} -o $@ $^ -lm

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

renderer.o: renderer.c renderer.h
	gcc ${FLAGS} -c $<

test_camera: test_camera.c camera.c raycast.c space.c renderer.c camera.h raycast.h space.h renderer.h
	gcc -Wall -g -o test_camera test_camera.c camera.c raycast.c space.c renderer.c -lm

manager: manager.c camera.c raycast.c space.c renderer.c camera.h raycast.h space.h renderer.h
	gcc -Wall -Wextra -g -o manager manager.c camera.c raycast.c space.c renderer.c -lm

tags: *.c *.h
	ctags **.c **.h

clean:
	rm -f *.o test_space test_raycast test_renderer
