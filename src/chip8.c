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
}config_t;

typedef struct 
{
	SDL_Window *window;
	SDL_Renderer *renderer;
}sdl_t;


bool init_sdl(sdl_t *sdl, config_t config){
	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER) != 0){
		SDL_Log("Can't Initialize SDL Subsystem %s \n", SDL_GetError());
		return false; // Initialization Failed
	}


	sdl -> window = SDL_CreateWindow("CHIP-8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, config.window_width, config.window_height, 0);

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
		.foreground_color = 0xFFFF00FF, //Yellow 
		.background_color = 0x00000000  //Black
	};

	// Change Defaults
	for(int i = 0; i < argc; i++){
		(void)argv[i];
	}

	return true;
}

void final_cleanup(sdl_t sdl){
	SDL_DestroyRenderer(sdl.renderer);
	SDL_DestroyWindow(sdl.window);
	SDL_Quit(); // Quit SDL Subsystem
}

// Clear Screen
void clear_screen(const config_t config){
	const uint8_t r = (config.background_color >> 24) & 0xFF;
	const uint8_t g = (config.background_color >> 16) & 0xFF;
	const uint8_t b = (config.background_color >> 8) & 0xFF;
	const uint8_t a = (config.background_color >> 0) & 0xFF;
	SDL_SetRenderDrawColor(sdl.renderer, r,g,b,a);
	SDL_RenderClear(sdl.renderer);
}

void update_screen(const config_t config){
	
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



	// Main Emulator Loop
	while(true){
		// Delay for 60fps
		SDL_Delay();
		// Update Window
		update_screen(config);
	}

	final_cleanup(sdl);

	exit(EXIT_SUCCESS);
}
