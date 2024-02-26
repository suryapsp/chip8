#include "sound.h"

void audio_callback(void *userdata, uint8_t *stream, int len){
    config_t *config = (config_t *) userdata;

    int16_t *audio_data = (int16_t *) stream;

    static uint32_t running_sample_index = 0;

    const int32_t sqaure_wave_period = config->audio_sample_rate / config->square_wave_freq;
    const int32_t half_sqaure_wave_period = sqaure_wave_period/2;

    for(int i = 0; i < len/2; i++){
        audio_data[i] = ((running_sample_index++ / half_sqaure_wave_period) % 2) ? 
        config->volume : 
        -config->volume;
    }
}