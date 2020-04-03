#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

// Plugin state
typedef struct {
    float *ir_left;
    float *ir_right;
    uint32_t ir_sample_rate_Hz;
    uint32_t ir_num_channels;
    uint32_t ir_num_samples_per_channel;
    uint32_t ir_bit_depth;
    uint32_t fft_block_size;
} plugin_state_t;

int init_plugin_state(plugin_state_t *state, const char *filename);

#endif
