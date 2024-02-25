#ifndef SOUND_H
#define SOUND_H

#include "chip8.h"

int audio_callback(void *userdata, uint8_t *stream, int len);

#endif