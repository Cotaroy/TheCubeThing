.PHONY: all clean
FLAGS = -Wall -Wextra -g

all: test/test_space test/test_raycast TheCubeThing 

test/test_space: test/test_space.c space.c
	gcc ${FLAGS} -o $@ $^ -lm

test/test_raycast: test/test_raycast.c raycast.c space.c
	gcc ${FLAGS} -o $@ $^ -lm

TheCubeThing: manager.c camera.c raycast.c space.c renderer.c controller.c parser.c camera.h raycast.h space.h renderer.h manager.h controller.h parser.h command_handlers.c
	gcc ${FLAGS} -o TheCubeThing manager.c camera.c raycast.c space.c renderer.c controller.c parser.c command_handlers.c -lm

tags: *.c *.h
	ctags **.c **.h

clean:
	rm -f test/test_space test/test_raycast TheCubeThing
