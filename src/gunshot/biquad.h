#ifndef BIQUAD_H
#define BIQUAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define BIQUAD_MAX_Hz 20000.0
#define BIQUAD_MIN_Hz 20.0

typedef struct {
    float b[3]; // Input coefficients
    float a[3]; // Output coefficients
    float x[3]; // Input delay line
    float y[3]; // Output delay line
} biquad_t;

biquad_t biquad_calculate_highpass(float cutoff_Hz, float sample_rate_Hz, bool clear_delay_line);
biquad_t biquad_calculate_lowpass(float cutoff_Hz, float sample_rate_Hz, bool clear_delay_line);
biquad_t biquad_calculate_nofilter(bool clear_delay_line);
float biquad_process_sample(biquad_t *s,  float input);

#ifdef __cplusplus
}
#endif
#endif

