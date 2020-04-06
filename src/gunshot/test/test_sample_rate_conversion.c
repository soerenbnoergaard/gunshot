//
// Makefile:
//
//     .PHONY: all
//     
//     SOURCES = test_sample_rate_conversion.c $(wildcard ../../../libsamplerate/src/*.c)
//     INCLUDES = -I ../../../libsamplerate/src/
//     LIBS = -lm -DPACKAGE='"libsamplerate"' -DVERSION='"0.1.9"' -DCPU_CLIPS_POSITIVE=0 -DCPU_CLIPS_NEGATIVE=0
//     
//     TARGET = test_sample_rate_conversion
//     
//     all:
//         gcc $(INCLUDES) $(SOURCES) $(LIBS) -o $(TARGET)

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include <samplerate.h>

#define INPUT_SAMPLE_RATE_Hz 48000
#define RATIO 2.0

#define INPUT_LENGTH (300000)
#define OUTPUT_LENGTH ((uint32_t)(INPUT_LENGTH*RATIO))

float get_sample(uint32_t n)
{
    return 0.1*sin(1000.0 * n / INPUT_SAMPLE_RATE_Hz);
}

int buffer_to_file(const char *filename, const float *buffer, uint32_t length)
{
    uint32_t n;
    FILE *f = fopen(filename, "w");
    if (f == NULL) {
        return 1;
    }
    for (n = 0; n < length; n++) {
        fprintf(f, "%f\n", buffer[n]);
    }
    fclose(f);
}

int main(void)
{
    uint32_t n;
    uint32_t err;
    float input[INPUT_LENGTH];
    float output[OUTPUT_LENGTH];

	SRC_DATA src_data;

    // Generate input data
    for (n = 0; n < INPUT_LENGTH; n++) {
        input[n] = get_sample(n);
    }
    buffer_to_file("input.csv", input, INPUT_LENGTH);

    // Initialize sample rate converter
    src_data.data_in = input;
    src_data.data_out = output;
    src_data.input_frames = INPUT_LENGTH;
    src_data.output_frames = OUTPUT_LENGTH;
    src_data.src_ratio = RATIO;

    // Run sample-rate conversion
    err = src_simple(&src_data, SRC_SINC_BEST_QUALITY, 1);

    // Print result
    printf("src_simple returned %d\n", err);
    buffer_to_file("output.csv", output, src_data.output_frames_gen);
    return 0;
}

