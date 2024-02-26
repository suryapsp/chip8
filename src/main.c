#include "chip8.h"
#include "screen.h"
#include "keyboard.h"
#include "instructions.h"

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
	if(init_sdl(&sdl, &config) == false){
		exit(EXIT_FAILURE);
	}

	// CHIP-8 Initialization
	chip8_t chip8 = {0};
	const char *rom_name = argv[1];
	if(!init_chip8(&chip8, rom_name)){
		exit(EXIT_FAILURE);
	}

	clear_screen(sdl, config);

	srand(time(NULL));

	// Main Emulator Loop
	while(chip8.state != QUIT){
		// User Input
		handle_input(&chip8);

		if(chip8.state == PAUSED){continue;}

		// Get time before running instructions
		uint64_t start = SDL_GetPerformanceCounter();

		for(uint32_t i = 0; i < config.inst_per_sec/60; i++)
		emulate_instructions(&chip8, config);

		// Get time after running instructions
		uint64_t end = SDL_GetPerformanceCounter();


		double time_elapsed = (double)((end-start)*1000)/SDL_GetPerformanceFrequency();

		// Delay for 60fps
		SDL_Delay(16.67f > time_elapsed ? 16.67f - time_elapsed : 0);
		// Update Window
		update_screen(sdl, config, chip8);
		update_timers(sdl, &chip8);
	}

	final_cleanup(sdl);

	exit(EXIT_SUCCESS);
}
