#ifndef CHIP8_H
#define CHIP8_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include "SDL2/SDL.h"


typedef struct {
	uint32_t window_width;
	uint32_t window_height;
	uint32_t foreground_color; // foreground color R-8 G-8 B-8 A-8
	uint32_t background_color; // bg color R-8 G-8 B-8 A-8

	uint32_t scale_factor; // Scale CHIP-8 px

	bool pixel_outlines; // does the user want pixel outlines or not

	uint32_t inst_per_sec; // cpu clock rate
}config_t;

typedef struct 
{
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_AudioSpec want, have;
	SDL_AudioDeviceID dev;
}sdl_t;

typedef struct{
	uint16_t opcode;
	uint16_t NNN;
	uint8_t NN;
	uint8_t N;
	uint8_t X;
	uint8_t Y;
}instruction_t;

// Emulator States
typedef enum {
	QUIT,
	RUNNING,
	PAUSED
} emulator_state_t;

// CHIP8 Obj
typedef struct{
	emulator_state_t state;
	uint8_t ram[4096];
	bool display[64*32]; // CHIP-8 resolution pixels
	uint16_t stack[12]; // CHIP-8 Stack
	uint16_t *SP;
	uint8_t V[16]; // CHIP-8 Registers V0-VF
	uint16_t I; // Memory Address Register
	uint8_t delay_timer; //subtract 1 from the value of DT(Delay Timer Register) at a rate of 60Hz
	uint8_t sound_timer; //subtract 1 from the value of ST(Sound Timer Register) at a rate of 60Hz
	bool keypad[16]; //0-F
	const char *rom_name; //Name of ROM
	uint16_t PC; //Program Counter
	instruction_t inst; //instruction currently executing
} chip8_t;



// Initialize CHIP8 machine
bool init_chip8(chip8_t *chip8, const char rom_name[]);

bool init_sdl(sdl_t *sdl, config_t config);

bool set_config(config_t *config, int argc, char **argv);

void final_cleanup(sdl_t sdl);

void update_timers(chip8_t *chip8);

#endif
