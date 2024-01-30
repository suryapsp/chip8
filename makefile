CFLAGS=-std=c17 -Wall -Wextra -Werror

all:
	gcc src/chip8.c -o bin/chip8 $(CFLAGS) `sdl2-config --cflags --libs`

debug:
	gcc src/chip8.c -o bin/chip8 $(CFLAGS) `sdl2-config --cflags --libs` -DDEBUG