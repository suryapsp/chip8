#include <stdbool.h>
#include <stdint.h>
#include "chip8.h"
#include "sound.h"


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
	chip8 -> SP = chip8->stack;

	return true; //Sucess
}

bool init_sdl(sdl_t *sdl, config_t *config){
	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER) != 0){
		SDL_Log("Can't Initialize SDL Subsystem %s \n", SDL_GetError());
		return false; // Initialization Failed
	}


	sdl -> window = SDL_CreateWindow("CHIP-8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, config->window_width * config->scale_factor, config->window_height * config->scale_factor, 0);

	if(!sdl -> window){
		SDL_Log("could not create window %s\n", SDL_GetError());
		return false;
	}

	sdl -> renderer = SDL_CreateRenderer(sdl -> window, -1, SDL_RENDERER_ACCELERATED);

	if(!sdl->renderer){
		SDL_Log("could not create renderer %s\n", SDL_GetError());
		return false;
	}

	sdl->want = (SDL_AudioSpec){
		.freq = 44100,
		.format = AUDIO_S16LSB, //signed 16 bit little indian
		.channels = 1, //mono 1 channel
		.samples = 512,
		.callback = audio_callback,
		.userdata = config
	};

	sdl->dev = SDL_OpenAudioDevice(NULL,0, &sdl->want, &sdl->have, 0);

	if(sdl->dev == 0){
		SDL_Log("could not get any audio device %s\n", SDL_GetError());
		return false;
	}

	if((sdl->want.format != sdl->have.format)||(sdl->want.channels != sdl->have.channels)){
		SDL_Log("could not get desired audio spec\n");
		return false;
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
		.scale_factor = 20, // Scale 20x
		.pixel_outlines = true, // Draw pixel outlines
		.inst_per_sec = 700, // Default Clock Rate
		.square_wave_freq = 440, 
		.audio_sample_rate = 44100,
		.volume = 3000,
	};

	// Change Defaults
	for(int i = 0; i < argc; i++){
		(void)argv[i];
	}

	return true;
}

// Initialize CHIP8 machine

void final_cleanup(sdl_t sdl){
	SDL_DestroyRenderer(sdl.renderer);
	SDL_DestroyWindow(sdl.window);
	SDL_CloseAudioDevice(sdl.dev);
	SDL_Quit(); // Quit SDL Subsystem
}



void update_timers(const sdl_t sdl, chip8_t *chip8){
	if(chip8->delay_timer > 0){
		chip8->delay_timer--;
	}

	if(chip8->sound_timer > 0){
		chip8->sound_timer--;
		SDL_PauseAudioDevice(sdl.dev, 0); // Play Audio
	}
	else{
		SDL_PauseAudioDevice(sdl.dev, 1); //Pause Audio
	}
}

