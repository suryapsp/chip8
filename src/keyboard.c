#include "keyboard.h"



// User Input

/*
CHIP-8 KEYPAD		QWERTY

1 2 3 C 			1 2 3 4
4 5 6 D				Q W E R
7 8 9 E				A S D F
A 0 B F				Z X C V

*/


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
							chip8->state = PAUSED; // pause
						}
						else{
							chip8->state = RUNNING; // resume
							puts("paused"); 
						}
						return;

					case SDLK_1:
						chip8->keypad[0x1] =  true;
						break;

					case SDLK_2:
						chip8->keypad[0x2] =  true;
						break;

					case SDLK_3:
						chip8->keypad[0x3] =  true;
						break;

					case SDLK_4:
						chip8->keypad[0xC] =  true;
						break;

					case SDLK_q:
						chip8->keypad[0x4] =  true;
						break;

					case SDLK_w:
						chip8->keypad[0x5] =  true;
						break;

					case SDLK_e:
						chip8->keypad[0x6] =  true;
						break;

					case SDLK_r:
						chip8->keypad[0xD] =  true;
						break;

					case SDLK_a:
						chip8->keypad[0x7] =  true;
						break;

					case SDLK_s:
						chip8->keypad[0x8] =  true;
						break;

					case SDLK_d:
						chip8->keypad[0x9] =  true;
						break;

					case SDLK_f:
						chip8->keypad[0xE] =  true;
						break;

					case SDLK_z:
						chip8->keypad[0xA] =  true;
						break;

					case SDLK_x:
						chip8->keypad[0x0] =  true;
						break;

					case SDLK_c:
						chip8->keypad[0xB] =  true;
						break;

					case SDLK_v:
						chip8->keypad[0xF] =  true;
						break;


					default:
						break;

				}
				break;

			case SDL_KEYUP:
				switch(event.key.keysym.sym){

					case SDLK_1:
						chip8->keypad[0x1] =  false;
						break;

					case SDLK_2:
						chip8->keypad[0x2] =  false;
						break;

					case SDLK_3:
						chip8->keypad[0x3] =  false;
						break;

					case SDLK_4:
						chip8->keypad[0xC] =  false;
						break;

					case SDLK_q:
						chip8->keypad[0x4] =  false;
						break;

					case SDLK_w:
						chip8->keypad[0x5] =  false;
						break;

					case SDLK_e:
						chip8->keypad[0x6] =  false;
						break;

					case SDLK_r:
						chip8->keypad[0xD] =  false;
						break;

					case SDLK_a:
						chip8->keypad[0x7] =  false;
						break;

					case SDLK_s:
						chip8->keypad[0x8] =  false;
						break;

					case SDLK_d:
						chip8->keypad[0x9] =  false;
						break;

					case SDLK_f:
						chip8->keypad[0xE] =  false;
						break;

					case SDLK_z:
						chip8->keypad[0xA] =  false;
						break;

					case SDLK_x:
						chip8->keypad[0x0] =  false;
						break;

					case SDLK_c:
						chip8->keypad[0xB] =  false;
						break;

					case SDLK_v:
						chip8->keypad[0xF] =  false;
						break;


					default:
						break;

				}
				break;

			default:
				break;
		}
	}
}