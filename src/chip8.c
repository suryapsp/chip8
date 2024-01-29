#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "SDL.h"

typedef struct {
	uint32_t window_width;
	uint32_t window_height;
	uint32_t foreground_color; // foreground color R-8 G-8 B-8 A-8
	uint32_t background_color; // bg color R-8 G-8 B-8 A-8

	uint32_t scale_factor; // Scale CHIP-8 px
}config_t;

typedef struct 
{
	SDL_Window *window;
	SDL_Renderer *renderer;
}sdl_t;

// Emulator States
typedef enum {
	QUIT,
	RUNNING,
	PAUSED
} emulator_state_t;


typedef struct{
	uint16_t opcode;
	uint16_t NNN;
	uint8_t NN;
	uint8_t N;
	uint8_t X;
	uint8_t Y;
}instruction_t;

// CHIP-8 Obj
typedef struct{
	emulator_state_t state;
	uint8_t ram[4096];
	bool display[64*32]; // CHIP-8 resolution pixels
	uint16_t stack[12]; // CHIP-8 Stack
	uint8_t V[16]; // CHIP-8 Registers V0-VF
	uint16_t I; // Memory Address Register
	uint8_t delay_timer; //subtract 1 from the value of DT(Delay Timer Register) at a rate of 60Hz
	uint8_t sound_timer; //subtract 1 from the value of ST(Sound Timer Register) at a rate of 60Hz
	bool keypad[16]; //0-F
	const char *rom_name; //Name of ROM
	uint16_t PC; //Program Counter
	instruction_t inst; //instruction currently executing
} chip8_t;

bool init_sdl(sdl_t *sdl, config_t config){
	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER) != 0){
		SDL_Log("Can't Initialize SDL Subsystem %s \n", SDL_GetError());
		return false; // Initialization Failed
	}


	sdl -> window = SDL_CreateWindow("CHIP-8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, config.window_width * config.scale_factor, config.window_height * config.scale_factor, 0);

	if(!sdl -> window){
		SDL_Log("could not create window %s\n", SDL_GetError());
	}

	sdl -> renderer = SDL_CreateRenderer(sdl -> window, -1, SDL_RENDERER_ACCELERATED);

	if(!sdl->renderer){
		SDL_Log("could not create renderer %s\n", SDL_GetError());
	}

	return true; // Initialization Done
}

// Initial Emulator Config from actual args
bool set_config(config_t *config, int argc, char **argv){
	// Defaults
	*config = (config_t){
		.window_width = 64,
		.window_height = 32, //OG CHIP8 Resolution
		.foreground_color = 0xFFFFFFFF, //White 
		.background_color = 0x000000FF,  //Yellow
		.scale_factor = 20 // Scale 20x
	};

	// Change Defaults
	for(int i = 0; i < argc; i++){
		(void)argv[i];
	}

	return true;
}

// Initialize CHIP8 machine
bool init_chip8(chip8_t *chip8, const char rom_name[]){
	const uint32_t entry_point = 0x200;
	const uint8_t font[] = {
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    	0x20, 0x60, 0x20, 0x20, 0x70, // 1
    	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

	// Load Font
	memcpy(&chip8 -> ram[0], font, sizeof(font));

	// Open ROM
	FILE *rom = fopen(rom_name, "rb");
	if(!rom){
		SDL_Log("ROM FILE is Invalid\n");
		return false;
	}


	// ROM Size
	fseek(rom, 0, SEEK_END);
	const size_t rom_size = ftell(rom);
	const size_t max_size = sizeof chip8->ram - entry_point;
	rewind(rom);

	if(rom_size > max_size){
		SDL_Log("ROM FILE is Too Large to Handle\n");
		return false;
	}

	// Read ROM
	if(fread(&chip8->ram[entry_point], rom_size, 1, rom) != 1){
		SDL_Log("Can't Read the ROM \n");
		return false;
	};

	// Close ROM
	fclose(rom);

	// Defaults
	chip8 -> state = RUNNING;
	chip8 -> PC = entry_point;
	chip8 -> rom_name = rom_name;

	return true; //Sucess
}

void final_cleanup(sdl_t sdl){
	SDL_DestroyRenderer(sdl.renderer);
	SDL_DestroyWindow(sdl.window);
	SDL_Quit(); // Quit SDL Subsystem
}

// Clear Screen
void clear_screen(const sdl_t sdl, const config_t config){
	const uint8_t r = (config.background_color >> 24) & 0xFF;
	const uint8_t g = (config.background_color >> 16) & 0xFF;
	const uint8_t b = (config.background_color >> 8) & 0xFF;
	const uint8_t a = (config.background_color >> 0) & 0xFF;
	SDL_SetRenderDrawColor(sdl.renderer, r,g,b,a);
	SDL_RenderClear(sdl.renderer);
}

// Update window changes
void update_screen(const sdl_t sdl){
	SDL_RenderPresent(sdl.renderer);
}

// User Input
void handle_input(chip8_t *chip8){
	SDL_Event event;

	while(SDL_PollEvent(&event)){
		switch (event.type) {
			case SDL_QUIT:
				chip8->state = QUIT;
				return ;

			case SDL_KEYDOWN:
				switch(event.key.keysym.sym){
					case SDLK_ESCAPE:
						chip8 -> state = QUIT;
						return;

					case SDLK_SPACE:
						if(chip8->state == RUNNING){
							chip8->state = PAUSED;
						}
						else{
							chip8->state = RUNNING;
							puts("paused");
						}
						return;

					default:
						break;

				}
				break;

			case SDL_KEYUP:
				break;

			default:
				break;
		}
	}
}

void emulate_instructions(chip8_t *chip8){
	// Get opcode from RAM
	chip8->inst.opcode = (chip8->ram[chip8->PC] << 8) | chip8 -> ram[chip8->PC+1];
	chip8->PC +=2;

	// SYMBOLS 
	chip8->inst.NNN = chip8->inst.opcode & 0x0FFF;
	chip8->inst.NN = chip8->inst.opcode & 0x0FF;
	chip8->inst.N = chip8->inst.opcode & 0x0F;
	chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
	chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;

	// OPCODES INSTRUCTIONS
	switch((chip8->inst.opcode >> 12) & 0x0F){

		case 0x00:
			if(chip8->inst.NN == 0xE0){
				// 0x00E0 Display Clear
				memset(chip8->display, false, sizeof(chip8->display));
			}
			else if(chip8->inst.NN == 0xEE){
				// Returns from a subroutine
				// 0x00EE

			}
			break;

		case 0x02:
			// Calls subroutine at NNN


		default:
			break;
	}

}


int main(int argc, char **argv){
	// NO ROM PASSED
	if(argc<2){
		printf("NO ROM PASSED\n");
		exit(EXIT_FAILURE);
	}


	// Emulator Config
	config_t config = {0};
	if(set_config(&config, argc, argv) == false){
		exit(EXIT_FAILURE);
	}

	// Initialization
	sdl_t sdl = {0};
	if(init_sdl(&sdl, config) == false){
		exit(EXIT_FAILURE);
	}

	// CHIP-8 Initialization
	chip8_t chip8 = {0};
	const char *rom_name = argv[1];
	if(!init_chip8(&chip8, rom_name)){
		exit(EXIT_FAILURE);
	}

	clear_screen(sdl, config);

	// Main Emulator Loop
	while(chip8.state != QUIT){
		// User Input
		handle_input(&chip8);

		if(chip8.state == PAUSED){continue;}

		emulate_instructions(&chip8);


		// CHIP 8 Instructions Emulation


		// Delay for 60fps
		SDL_Delay(16);
		// Update Window
		update_screen(sdl);
	}

	final_cleanup(sdl);

	exit(EXIT_SUCCESS);
}
