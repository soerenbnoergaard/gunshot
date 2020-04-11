#ifndef UTILS_H
#define UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

float convert_dB_to_linear(float x_dB);
uint32_t find_basename(const char *abspath);

#ifdef __cplusplus
}
#endif

#endif
