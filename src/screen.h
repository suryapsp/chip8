#ifndef SCREEN_H
#define SCREEN_H

#include "chip8.h"

void clear_screen(const sdl_t sdl, const config_t config);

// Update window changes
void update_screen(const sdl_t sdl, config_t config, chip8_t chip8);

#endif