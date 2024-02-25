#include "debug.h"

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

		case 0x08:
			switch(chip8->inst.N){
				case 0:
					// 0x8XY0
					// Sets VX to the value of VY

					printf("Set V%X to V%X (0x%02X)\n", chip8->inst.X, chip8->inst.Y, chip8->V[chip8->inst.Y]);

					break;

				case 1:
					// 0x8XY1
					// Sets VX to VX or VY (bitwise OR operation)
					printf("Set V%X |= V%X =>(0x%02X)\n", chip8->inst.X, chip8->inst.Y, chip8->V[chip8->inst.X] | chip8->V[chip8->inst.Y]);


					break;

				case 2:
					// 0x8XY2
					// Sets VX to VX and VY (bitwise AND operation)

					printf("Set V%X &= V%X =>(0x%02X)\n", chip8->inst.X, chip8->inst.Y, chip8->V[chip8->inst.X] & chip8->V[chip8->inst.Y]);

					break;

				case 3:
					// 0x8XY3
					// Sets VX to VX xor VY

					printf("Set V%X ^= V%X =>(0x%02X)\n", chip8->inst.X, chip8->inst.Y, chip8->V[chip8->inst.X] ^ chip8->V[chip8->inst.Y]);

					break;

				case 4:
					// 0x8XY4
					// Adds VY to VX
					// VF is set to 1 when there's an overflow, and to 0 when there is not

					printf("Set V%X += V%X =>(0x%02X) VF = %X\n", chip8->inst.X, chip8->inst.Y, chip8->V[chip8->inst.X] + chip8->V[chip8->inst.Y], ((uint16_t)(chip8->V[chip8->inst.X] + chip8->V[chip8->inst.Y]) > 255));

					break;

				case 5:
					// 0x8XY5
					// VY is subtracted from VX
					// VF is set to 0 when there's an underflow, and 1 when there is not. (i.e. VF set to 1 if VX >= VY and 0 if not). 

					printf("Set V%X -= V%X =>(0x%02X) VF = %X\n", chip8->inst.X,
					 chip8->inst.Y,
					  chip8->V[chip8->inst.X] - chip8->V[chip8->inst.Y],
					  (int16_t)(chip8->V[chip8->inst.X] - chip8->V[chip8->inst.Y]) < 0);

					break;					

				case 6:
					// 0X8XY6
					// Stores the least significant bit of VX in VF and then shifts VX to the right by 1

					printf("Set V%X >>= 1 =>(0x%02X) VF = %X\n", chip8->inst.X, chip8->V[chip8->inst.X] >> 1, chip8->V[chip8->inst.X] & 1);

					break;

				case 7:
					// 0x8XY7
					// Sets VX to VY minus VX. VF is set to 0 when there's an underflow, and 1 when there is not. (i.e. VF set to 1 if VY >= VX)

					printf("Set V%X = V%X - V%X=>(0x%02X) VF = %X\n",
					 chip8->inst.X, 
					 chip8->inst.Y, 
					 chip8->inst.X, 
					 chip8->V[chip8->inst.X] - chip8->V[chip8->inst.Y], 
					 (int16_t) (chip8->V[chip8->inst.Y] -  chip8->V[chip8->inst.X]) < 0);

					break;

				case 0xE:
					// 0x8XYE
					// Stores the most significant bit of VX in VF and then shifts VX to the left by 1

					printf("Set V%X <<= 1 =>(0x%02X) VF = %X\n", chip8->inst.X, chip8->V[chip8->inst.X] << 1, (chip8->V[chip8->inst.X] & 0x80) >> 7);

					break;
			}
			break;

		case 0x09:
			// 0x9XY0
			// Skips the next instruction if VX does not equal VY. (Usually the next instruction is a jump to skip a code block)

			printf("V%X != V%X so skipping next intruction\n", chip8->inst.X, chip8->inst.Y);

			break;


		case 0x0B:
			// 0xBNNN
			// Jumps to the address NNN plus V0

			printf("set PC to V0 + NNN => (0x%04X) \n", chip8->inst.NNN + chip8->V[0]);

			break;

		case 0x0C:
			// 0xCXNN
			// Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN

			printf("Set V%X to rand()&NN (0x%02X)\n", chip8->inst.X, (rand() % 256) & chip8->inst.NN);

			break;

		case 0x0D:
			/*
			Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. Each row of 8 pixels is read as bit-coded starting from memory location I; I value does not change after the execution of this instruction. As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that does not happen
			*/
			// 0xDXYN
			printf("Drawing %u height sprite at coordinates V%X [0x%02X] and V%X [0x%02X]\n", chip8->inst.N, chip8->inst.X, chip8->V[chip8->inst.X], chip8->inst.Y, chip8->V[chip8->inst.Y]);
			break;

		case 0xE:
			switch(chip8->inst.NN){
				case 0x9E:
					// 0xEX9E
					// Skips the next instruction if the key stored in VX is pressed (usually the next instruction is a jump to skip a code block)

					printf("skipping next instruction because key in V%X is pressed\n", chip8->inst.X);

					break;

				case 0xA1:
					// 0xEXA1
					// Skips the next instruction if the key stored in VX is not pressed (usually the next instruction is a jump to skip a code block)

					printf("skipping next instruction because key in V%X is not pressed\n", chip8->inst.X);

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

				printf("wait until key is pressed and then store the key in V%X\n", chip8->inst.X);
				
				break;

			case 0x1E:
				// 0xFX1E
				// Adds VX to I. VF is not affected
				printf("I += V%X => (0%2X)\n", chip8->inst.X, chip8->I+chip8->V[chip8->inst.X]);
				break;

			case 0x07:
				// 0xFX07
				// Sets VX to the value of the delay timer
				printf("V%X = delay_timer() (0x%2X)\n", chip8->inst.X, chip8->delay_timer);

				break;

			case 0x15:
				// 0xFX15
				// Sets the delay timer to VX
				printf("delay_timer() (0x%2X) = V%X\n", chip8->delay_timer, chip8->inst.X);

				break;

			case 0x18:
				// 0xFX18
				// Sets the sound timer to VX

				printf("sound_timer() (0x%2X) = V%X\n", chip8->sound_timer, chip8->inst.X);

				break;

			case 0x29:
				// 0xFX29
				// Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font

				printf("set I to sprite location in memory for character in V%X  \n", chip8->inst.X);

				break;
 
			case 0x33:
				// 0xFX33

				printf("store BCD representation of V%X\n", chip8->inst.X);
				break;

			case 0x55:
				// 0xFX55
				// Stores from V0 to VX (including VX) in memory, starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified

				printf("register dump from V0 to V%X\n", chip8->inst.X);
				break;

			case 0x65:
				// 0xFX65
				// Fills from V0 to VX (including VX) with values from memory, starting at address I. The offset from I is increased by 1 for each value read, but I itself is left unmodified

				printf("register load from V0 to V%X\n", chip8->inst.X);
				break;
			
			default:
				break;
			}

			break;

		default:
			printf("unimplemented \n");
			break;
	}
	}
