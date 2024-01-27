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

// CHIP-8 Obj
typedef struct{
	emulator_state_t state;
	
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
		.background_color = 0xFFFF00FF,  //Yellow
		.scale_factor = 20 // Scale 20x
	};

	// Change Defaults
	for(int i = 0; i < argc; i++){
		(void)argv[i];
	}

	return true;
}

// Initialize CHIP8 machine
bool init_chip8(chip8_t *chip8){
	chip8 -> state = RUNNING; //Default Machine State
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

int main(int argc, char **argv){
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
	if(!init_chip8(&chip8)){
		exit(EXIT_FAILURE);
	}

	clear_screen(sdl, config);

	// Main Emulator Loop
	while(chip8.state != QUIT){
		// User Input
		handle_input(&chip8);


		// CHIP 8 Instructions Emulation


		// Delay for 60fps
		SDL_Delay(16);
		// Update Window
		update_screen(sdl);
	}

	final_cleanup(sdl);

	exit(EXIT_SUCCESS);
}
