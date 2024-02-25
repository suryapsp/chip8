#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "chip8.h"

// User Input

/*
CHIP-8 KEYPAD		QWERTY

1 2 3 C 			1 2 3 4
4 5 6 D				Q W E R
7 8 9 E				A S D F
A 0 B F				Z X C V

*/


void handle_input(chip8_t *chip8);

#endif