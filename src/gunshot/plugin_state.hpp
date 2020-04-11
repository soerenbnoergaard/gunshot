#ifndef PLUGIN_STATE_H
#define PLUGIN_STATE_H

#include <stdint.h>

// Plugin state ////////////////////////////////////////////////////////////////

#define PLUGIN_STATE_VERSION 2
#define PLUGIN_STATE_FILENAME_LENGTH 1024

// The plugin state version is stored from version 2 and onwards.
// This makes it possible to have backwards compatible plugin states.
// Unfortunately, this was not included in the first version, so this is
// incompatible with any newer version (which will break DAW presets/projects).

typedef struct {
    uint32_t version;
    uint32_t ir_sample_rate_Hz;
    uint32_t ir_num_channels;
    uint32_t ir_num_samples_per_channel;
    uint32_t ir_bit_depth;
    uint32_t fft_block_size;
    char filename[PLUGIN_STATE_FILENAME_LENGTH];
    float *ir_left;
    float *ir_right;
} plugin_state_t;

int plugin_state_init(plugin_state_t *state, const char *filename);
int plugin_state_init_dirac(plugin_state_t *state, uint32_t sample_rate_Hz);
int plugin_state_reset(plugin_state_t *state, bool free_buffers, bool dirac_impulse_response);
int plugin_state_free(plugin_state_t *state);
int plugin_state_serialize(plugin_state_t *state, char **output, uint32_t *length);
int plugin_state_deserialize(plugin_state_t *state, char *input, uint32_t length);

#endif
