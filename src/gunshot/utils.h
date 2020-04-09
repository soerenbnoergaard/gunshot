#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

// Plugin state ////////////////////////////////////////////////////////////////
typedef struct {
    uint32_t ir_sample_rate_Hz;
    uint32_t ir_num_channels;
    uint32_t ir_num_samples_per_channel;
    uint32_t ir_bit_depth;
    uint32_t fft_block_size;
    float *ir_left;
    float *ir_right;
} plugin_state_t;

int plugin_state_init(plugin_state_t *state, const char *filename);
int plugin_state_init_dirac(plugin_state_t *state, uint32_t sample_rate_Hz);
int plugin_state_reset(plugin_state_t *state, bool free_buffers, bool dirac_impulse_response);
int plugin_state_free(plugin_state_t *state);
int plugin_state_serialize(plugin_state_t *state, char **output, uint32_t *length);
int plugin_state_deserialize(plugin_state_t *state, char *input, uint32_t length);

// Log /////////////////////////////////////////////////////////////////////////
/* #define GUNSHOT_LOG_FILE "gunshot.log" */
/* #define GUNSHOT_LOG_FILE "/home/soren/Desktop/gunshot.log" */
/* #define GUNSHOT_LOG_FILE "C:/Users/Christine/Desktop/gunshot.log" */

int log_init(void);
int log_write(const char *s);

// Biquad filter ///////////////////////////////////////////////////////////////

typedef struct {
    float b[3]; // Input coefficients
    float a[3]; // Output coefficients
    float x[3]; // Input delay line
    float y[3]; // Output delay line
} biquad_t;

biquad_t biquad_calculate_highpass(float cutoff_Hz, float sample_rate_Hz);
biquad_t biquad_calculate_lowpass(float cutoff_Hz, float sample_rate_Hz);

// General /////////////////////////////////////////////////////////////////////
float convert_dB_to_linear(float x_dB);

#endif
