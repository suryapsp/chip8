CFLAGS=-std=c17 -Wall -Wextra -Werror

all:
	gcc -o bin/chip8 $(CFLAGS) src/chip8.c src/debug.c src/instructions.c src/keyboard.c src/main.c src/screen.c `sdl2-config --cflags --libs`

