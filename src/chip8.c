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

	bool pixel_outlines; // does the user want pixel outlines or not
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
		.scale_factor = 20, // Scale 20x
		.pixel_outlines = true // Draw pixel outlines
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
	chip8 -> SP = chip8->stack;

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
void update_screen(const sdl_t sdl, config_t config, chip8_t chip8){
	SDL_Rect rect = {.x = 0, .y = 0, .w = config.scale_factor, .h = config.scale_factor};

// Color Values
	const uint8_t bg_r = (config.background_color >> 24) & 0xFF;
	const uint8_t bg_g = (config.background_color >> 16) & 0xFF;
	const uint8_t bg_b = (config.background_color >> 8) & 0xFF;
	const uint8_t bg_a = (config.background_color >> 0) & 0xFF;

	const uint8_t fg_r = (config.foreground_color >> 24) & 0xFF;
	const uint8_t fg_g = (config.foreground_color >> 16) & 0xFF;
	const uint8_t fg_b = (config.foreground_color >> 8) & 0xFF;
	const uint8_t fg_a = (config.foreground_color >> 0) & 0xFF;



// Draw Rectange per pixel
	for(u_int32_t i = 0; i < sizeof(chip8.display); i++){
		// Translate i value to x y coordinates
		rect.x = (i % config.window_width)*config.scale_factor;
		rect.y = (i / config.window_width) *config.scale_factor;


		if(chip8.display[i]){
			// draw foreground
			SDL_SetRenderDrawColor(sdl.renderer, fg_r, fg_g, fg_b, fg_a);
			SDL_RenderFillRect(sdl.renderer, &rect);

			// pixel outlines not necessary can be used as user option
			if(config.pixel_outlines){
				SDL_SetRenderDrawColor(sdl.renderer, bg_r, bg_g, bg_b, bg_a);
				SDL_RenderDrawRect(sdl.renderer, &rect);
			}


		}
		else{
			// draw background
			SDL_SetRenderDrawColor(sdl.renderer, bg_r, bg_g, bg_b, bg_a);
			SDL_RenderFillRect(sdl.renderer, &rect);
		}
	}



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

#ifdef DEBUG
	void print_debug_output(chip8_t *chip8){
		printf("Address: 0x%04X, Opcode: 0x%04X Description: \n", chip8->PC-2, chip8->inst.opcode);
		switch((chip8->inst.opcode >> 12) & 0x0F){

		case 0x00:
			if(chip8->inst.NN == 0xE0){
				// 0x00E0 Display Clear
				printf("clear the screen\n");
			}
			else if(chip8->inst.NN == 0xEE){
				// Returns from a subroutine
				// 0x00EE
				/*
				Set Program Counter to last address of function(subroutine) call (pop it off the stack)
				*/
				printf("return from a subroutine\n");
			}
			else{
				printf("unimplemented opcode\n");
			}
			break;

		case 0x01:
			// 1NNN
			// Jumps to address NNN
			printf("jums to address NNN 0x0%4X\n", chip8->inst.NNN);
			break;

		case 0x02:
			// Calls subroutine at NNN
			// 0x2NNN
			/*
			Store Current Address from the program counter to the stack (PUSH IT TO THE STACK)
			Set the program counter to NNN 
			*/
			*chip8->SP++ = chip8->PC; 
			chip8->PC = chip8->inst.NNN;
			break;

		case 0x03:
			// 0x3XNN
			// Skips the next instruction if VX equals NN (usually the next instruction is a jump to skip a code block)

			printf("V%X == NN (0x%02X) so skipping next intruction 0x%04X\n", chip8->inst.X, chip8->inst.NN, chip8->PC);

			break;

		case 0x04:
			// 0x4XNN
			// Skips the next instruction if VX does not equal NN (usually the next instruction is a jump to skip a code block).
			// Opposite of 0x3XNN

			printf("V%X != NN (0x%02X) so skipping next instruction 0x%04X\n", chip8->inst.X, chip8->inst.NN, chip8->PC);

			break;

		case 0x05:
			// 0x5XY0
			// Skips the next instruction if VX equals VY (usually the next instruction is a jump to skip a code block)

			printf("VX(V%X) == VY(V%X) skkiping the next instruction \n", chip8->inst.X, chip8->inst.Y);

			break;

		case 0x0A:
			// 0xANNN
			// Sets I to the address NNN

			printf("set I to NNN: 0x%04X\n", chip8->inst.NNN);
			break;

		case 0x06:
			// Sets VX to NN
			// 0x6XNN

			printf("Set V%X to NN(0x%02X)\n", chip8->inst.X, chip8->inst.NN);

			break;

		case 0x07:
		// Adds NN to VX (carry flag is not changed)
		// 0x7XNN
			printf("Added NN (0x%02X) to V%X\n", chip8->inst.NN, chip8->inst.X);

			break;


		case 0x0D:
			/*
			Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. Each row of 8 pixels is read as bit-coded starting from memory location I; I value does not change after the execution of this instruction. As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that does not happen
			*/
			// 0xDXYN
			printf("Drawing %u height sprite at coordinates V%X [0x%02X] and V%X [0x%02X]\n", chip8->inst.N, chip8->inst.X, chip8->V[chip8->inst.X], chip8->inst.Y, chip8->V[chip8->inst.Y]);
			break;

		default:
			printf("unimplemented \n");
			break;
	}
	}
#endif

// FIRST CHIP8 INSTRUCTIONS
void emulate_instructions(chip8_t *chip8, const config_t config){
	// Get opcode from RAM
	chip8->inst.opcode = (chip8->ram[chip8->PC] << 8) | chip8 -> ram[chip8->PC+1];
	chip8->PC +=2;

	// SYMBOLS 
	chip8->inst.NNN = chip8->inst.opcode & 0x0FFF;
	chip8->inst.NN = chip8->inst.opcode & 0x0FF;
	chip8->inst.N = chip8->inst.opcode & 0x0F;
	chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
	chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;

#ifdef DEBUG
	print_debug_output(chip8);
#endif
	// EMULATING OPCODES INSTRUCTIONS
	switch((chip8->inst.opcode >> 12) & 0x0F){

		case 0x00:
			if(chip8->inst.NN == 0xE0){
				// 0x00E0 Display Clear
				memset(chip8->display, false, sizeof(chip8->display));
			}
			else if(chip8->inst.NN == 0xEE){
				// Returns from a subroutine
				// 0x00EE
				/*
				Set Program Counter to last address of function(subroutine) call (pop it off the stack)
				*/
				chip8->PC = *--chip8->SP;

			}
			break;

		case 0x01:
			// 1NNN

			// Jumps to address NNN

			chip8->PC = chip8->inst.NNN;
			break;

		case 0x02:
			// Calls subroutine at NNN
			// 0x2NNN
			/*
			Store Current Address from the program counter to the stack (PUSH IT TO THE STACK)
			Set the program counter to NNN 
			*/
			*chip8->SP++ = chip8->PC; 
			chip8->PC = chip8->inst.NNN;
			break;

		case 0x03:
			// 0x3XNN
			// Skips the next instruction if VX equals NN (usually the next instruction is a jump to skip a code block)

			if(chip8->V[chip8->inst.X] == chip8->inst.NN){
				chip8->PC += 2;
			}

			break;

		case 0x04:
			// 0x4XNN
			// Skips the next instruction if VX does not equal NN (usually the next instruction is a jump to skip a code block).
			// Opposite of 0x3XNN

			if(chip8->V[chip8->inst.X] != chip8->inst.NN){
				chip8->PC += 2;
			}

			break;

		case 0x05:
			// 0x5XY0
			// Skips the next instruction if VX equals VY (usually the next instruction is a jump to skip a code block)

			if(chip8->inst.N != 0){
				break; // wrong opcode
			}

			if(chip8->V[chip8->inst.X] == chip8->V[chip8->inst.Y]){
				chip8->PC += 2;
			}

			break;

		case 0x06:
			// Sets VX to NN
			// 0x6XNN

			chip8->V[chip8->inst.X] = chip8->inst.NN;

			break;

		case 0x07:
			// Adds NN to VX (carry flag is not changed)
			// 0x7XNN
			chip8->V[chip8->inst.X] += chip8->inst.NN;
			break;

		case 0x08:
			switch(chip8->inst.N){
				case 0:
					// 0x8XY0
					// Sets VX to the value of VY

					chip8->V[chip8->inst.X] = chip8->V[chip8->inst.Y];

					break;

				case 1:
					// 0x8XY1
					// Sets VX to VX or VY (bitwise OR operation)

					chip8->V[chip8->inst.X] |= chip8->V[chip8->inst.Y];

					break;

				case 2:
					// 0x8XY2
					// Sets VX to VX and VY (bitwise AND operation)

					chip8->V[chip8->inst.X] &= chip8->V[chip8->inst.Y];

					break;

				case 3:
					// 0x8XY3
					// Sets VX to VX xor VY

					chip8->V[chip8->inst.X] ^= chip8->V[chip8->inst.Y];

					break;

				case 4:
					// 0x8XY4
					// Adds VY to VX
					// VF is set to 1 when there's an overflow, and to 0 when there is not

					if((uint16_t)(chip8->V[chip8->inst.X] + chip8->V[chip8->inst.Y]) > 255){
						chip8->V[0xF] = 1;
					}

					chip8->V[chip8->inst.X] += chip8->V[chip8->inst.Y];

					break;

				case 5:
					// 0x8XY5
					// VY is subtracted from VX
					// VF is set to 0 when there's an underflow, and 1 when there is not. (i.e. VF set to 1 if VX >= VY and 0 if not). 

					if((int16_t)(chip8->V[chip8->inst.X] - chip8->V[chip8->inst.Y]) < 0){
						chip8->V[0xF] = 0;
					}

					chip8->V[chip8->inst.X] -= chip8->V[chip8->inst.Y];

					break;					

				default:
					break;
			}
			break;

		case 0x0A:
			// 0xANNN
			// Sets I to the address NNN

			chip8->I = chip8->inst.NNN;
			break;


		case 0x0D:
			/*
			Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. Each row of 8 pixels is read as bit-coded starting from memory location I; I value does not change after the execution of this instruction. As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that does not happen
			*/
			// 0xDXYN

			uint8_t x = chip8->V[chip8->inst.X];
			uint8_t y = chip8->V[chip8->inst.Y];

			uint8_t og_x = x; // original x value

			uint8_t height = chip8->inst.N;

			// wrap the coordinates if they are bigger than the screen size
			x %= config.window_width;
			y %= config.window_height;

			// Set carry/collision flag to 0
			chip8->V[0xF] = 0;

			// Loop to iterate over N rows in the sprite
			for(uint8_t i = 0; i < height; i++){
				uint8_t sprite_data = chip8->ram[chip8->I + i];

				x = og_x; //reset x for next row
				// Loop to iterate over each bit(pixel) in the sprite
				for(int8_t j = 7; j >= 0; j--){
					bool *pixel = &chip8 -> display[y*config.window_width + x];

					bool sprite_bit = (sprite_data&(1<<j));

					if(sprite_bit && *pixel){
						chip8->V[0xF] = 1;
					}

					// XOR display pixel with sprite pixel to set it on or off
					*pixel ^= sprite_bit;


					// 
					if(++x >= config.window_width) break;
				}
				// 
				if(++y >= config.window_height) break;
			}
			break;

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

		emulate_instructions(&chip8, config);


		// CHIP 8 Instructions Emulation


		// Delay for 60fps
		SDL_Delay(16);
		// Update Window
		update_screen(sdl, config, chip8);
	}

	final_cleanup(sdl);

	exit(EXIT_SUCCESS);
}
