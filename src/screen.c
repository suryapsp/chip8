#include "screen.h"

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
	for(uint32_t i = 0; i < sizeof(chip8.display); i++){
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