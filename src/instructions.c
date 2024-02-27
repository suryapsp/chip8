#include "instructions.h"
// #include "debug.h"

// CHIP8 INSTRUCTIONS
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

// #ifdef DEBUG
// 	print_debug_output(chip8);
// #endif
	// EMULATING OPCODES INSTRUCTIONS
	switch((chip8->inst.opcode >> 12) & 0x0F){

		case 0x00:
			if(chip8->inst.NN == 0xE0){
				// 0x00E0 Display Clear
				memset(chip8->display, false, sizeof(chip8->display));
				chip8->draw = true;
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
					else{
						chip8->V[0xF] = 1;
					}

					chip8->V[chip8->inst.X] -= chip8->V[chip8->inst.Y];

					break;					

				case 6:
					// 0X8XY6
					// Stores the least significant bit of VX in VF and then shifts VX to the right by 1

					chip8->V[0xF] = chip8->V[chip8->inst.X] & 1;

					chip8->V[chip8->inst.X] >>= 1;

					break;

				case 7:
					// 0x8XY7
					// Sets VX to VY minus VX. VF is set to 0 when there's an underflow, and 1 when there is not. (i.e. VF set to 1 if VY >= VX)

					if((int16_t) (chip8->V[chip8->inst.Y] -  chip8->V[chip8->inst.X]) < 0){
						chip8->V[0xF] = 0;
					}
					else{
						chip8->V[0xF] = 1;
					}

					chip8->V[chip8->inst.X] = chip8->V[chip8->inst.Y] - chip8->V[chip8->inst.X];

					break;

				case 0xE:
					// 0x8XYE
					// Stores the most significant bit of VX in VF and then shifts VX to the left by 1

					chip8->V[0xF] = (chip8->V[chip8->inst.X] & 0x80) >> 7;

					chip8->V[chip8->inst.X] <<= 1;

					break;


				default:
					break;
			}
			break;

		case 0x09:
			// 0x9XY0
			// Skips the next instruction if VX does not equal VY. (Usually the next instruction is a jump to skip a code block)

			if(chip8->V[chip8->inst.X] != chip8->V[chip8->inst.Y]){
				chip8->PC += 2;
			}

			break;

		case 0x0A:
			// 0xANNN
			// Sets I to the address NNN

			chip8->I = chip8->inst.NNN;
			break;

		case 0x0B:
			// 0xBNNN
			// Jumps to the address NNN plus V0

			chip8->PC = chip8->inst.NNN + chip8->V[0];

			break;

		case 0x0C:
			// 0xCXNN
			// Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN
			chip8->V[chip8->inst.X] = (rand() % 256) & chip8->inst.NN;

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
			chip8->draw = true;
			break;


		case 0xE:
			switch(chip8->inst.NN){
				case 0x9E:
					// 0xEX9E
					// Skips the next instruction if the key stored in VX is pressed (usually the next instruction is a jump to skip a code block)

					if(chip8->keypad[chip8->V[chip8->inst.X]] == true){
						chip8->PC += 2;
					}

					break;

				case 0xA1:
					// 0xEXA1
					// Skips the next instruction if the key stored in VX is not pressed (usually the next instruction is a jump to skip a code block)

					if(chip8->keypad[chip8->V[chip8->inst.X]] == false){
						chip8->PC += 2;
					}

					break;

				default:
					break;

			}
			break;

		case 0x0F:
			switch (chip8->inst.NN)
			{
			case 0x0A:
				// 0xFX0A
				// A key press is awaited, and then stored in VX (blocking operation, all instruction halted until next key event)

				bool key_pressed = false;

				for(uint8_t i = 0; i < sizeof(chip8->keypad); i++){
					if(chip8->keypad[i]){
						chip8->V[chip8->inst.X] = i;
						key_pressed = true;
						break;
					}
				}

				// keep getting the current opcode and running the this instruction until a key is pressed
				if(key_pressed == false){
					chip8->PC -= 2;
				}
				
				break;

			case 0x1E:
				// 0xFX1E
				// Adds VX to I. VF is not affected
				chip8->I += chip8->V[chip8->inst.X];
				break;
			
			case 0x07:
				// 0xFX07
				// Sets VX to the value of the delay timer

				chip8->V[chip8->inst.X] = chip8->delay_timer;

				break;

			case 0x15:
				// 0xFX15
				// Sets the delay timer to VX

				chip8->delay_timer = chip8->V[chip8->inst.X];

				break;

			case 0x18:
				// 0xFX18
				// Sets the sound timer to VX

				chip8->sound_timer = chip8->V[chip8->inst.X];

				break;

			case 0x29:
				// 0xFX29
				// Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font

				chip8->I = chip8->V[chip8->inst.X] * 5;

				break;

			case 0x33:
				// 0xFX33
				uint8_t bcd = chip8->V[chip8->inst.X];

				chip8->ram[chip8->I + 2] = bcd % 10;
				bcd /= 10;

				chip8->ram[chip8->I + 1] = bcd % 10;
				bcd /= 10;

				chip8->ram[chip8->I] = bcd % 10;

				break;

			case 0x55:
				// 0xFX55
				// Stores from V0 to VX (including VX) in memory, starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified

				for(uint8_t i = 0; i <= chip8->inst.X; i++){
					chip8->ram[chip8->I+i] = chip8->V[i];
				}

				break;

			case 0x65:
				// 0xFX65
				// Fills from V0 to VX (including VX) with values from memory, starting at address I. The offset from I is increased by 1 for each value read, but I itself is left unmodified

				for(uint8_t i = 0; i <= chip8->inst.X; i++){
					chip8->V[i] = chip8->ram[chip8->I+i];
				}

				break;

			default:
				break;
			}

			break;

		default:
			break;
	}

}