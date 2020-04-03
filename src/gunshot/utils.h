#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

typedef enum {
    STATUS_OK = 0,
    STATUS_ERROR = 1
} status_t;

status_t filesize(const char *filename, int32_t *filesize);
status_t load(const char *filename, float *left, float *right, uint32_t *length, uint32_t *sample_rate_Hz);

#endif
