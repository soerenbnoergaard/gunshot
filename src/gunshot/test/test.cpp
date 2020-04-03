#include <stdio.h>
#include <stdint.h>
#include "../utils.h"

int main(void)
{
    status_t err;
    float *left = NULL;
    float *right = NULL;
    uint32_t length = 0;
    uint32_t sample_rate_Hz = 0;

    err = load("test.wav", left, right, &length, &sample_rate_Hz);
    if (err != STATUS_OK) {
        printf("Failed to load test.wav\n");
    }
}
