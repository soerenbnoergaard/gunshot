#ifndef BIQUAD_H
#define BIQUAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float b[3]; // Input coefficients
    float a[3]; // Output coefficients
    float x[3]; // Input delay line
    float y[3]; // Output delay line
} biquad_t;

biquad_t biquad_calculate_highpass(float cutoff_Hz, float sample_rate_Hz);
biquad_t biquad_calculate_lowpass(float cutoff_Hz, float sample_rate_Hz);

#ifdef __cplusplus
}
#endif
#endif

