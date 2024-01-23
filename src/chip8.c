#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "SDL.h"

typedef struct 
{
	SDL_Window *window;
}sdl_t;


bool init_sdl(void){
	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER) != 0){
		SDL_Log("Can't Initialize SDL Subsystem %s \n", SDL_GetError());
		return false; // Initialization Failed
	}

	return true; // Initialization Done
}

void final_cleanup(void){
	// SDL_DestroyWindow(window);
	SDL_Quit(); // Quit SDL Subsystem
}

int main(int argc, char **argv){
	(void) argc;
	(void) argv;

	sdl_t sdl = {0};
	if(init_sdl() == false){
		exit(EXIT_FAILURE);
	}

	final_cleanup();

	exit(EXIT_SUCCESS);
}
