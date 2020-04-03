#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include "utils.h"

#include "audiofile/AudioFile.h"
#include "fftconvolver/FFTConvolver.h"
#include "fftconvolver/Utilities.h"
#define FFT_BLOCK_SIZE 64
#define BUFFER_SIZE 128
#define NUM_TEST_SAMPLES (BUFFER_SIZE*2400)

float get_sample(uint32_t n, uint32_t sample_rate_Hz)
{
    return 0.01*sin(1000.0 * n / sample_rate_Hz);
}

int main(void)
{
    int err;
    fftconvolver::FFTConvolver convolver;
    float y[NUM_TEST_SAMPLES];
    float x[NUM_TEST_SAMPLES];

    // Initialize plugin state
    plugin_state_t state;
    err = init_plugin_state(&state, "test.wav");
    if (err) {
        printf("Got error\n");
        return 1;
    }

    // Initialize convolution kernel
    convolver.init(FFT_BLOCK_SIZE, (fftconvolver::Sample *)state.ir_left, state.ir_num_samples_per_channel);

    // Generate test data
    for (int n = 0; n < 24000; n++) {
        x[n] = get_sample(n, state.ir_sample_rate_Hz);
    }

    // Run convolution
    memset(y, 0.0, sizeof(float)*NUM_TEST_SAMPLES);

    // Block-wise convolution
    for (int n = 0; n < NUM_TEST_SAMPLES; n += BUFFER_SIZE) {
        convolver.process((fftconvolver::Sample *)(&x[n]), (fftconvolver::Sample *)(&y[n]), BUFFER_SIZE);
    }

    // Write output file
    AudioFile<float> out;
    AudioFile<float>::AudioBuffer out_buffer;

    out_buffer.resize(1);
    out_buffer[0].resize(NUM_TEST_SAMPLES);
    for (int n = 0; n < NUM_TEST_SAMPLES; n++) {
        out_buffer[0][n] = y[n];
    }

    out.setNumChannels(1);
    out.setBitDepth(state.ir_bit_depth);
    out.setSampleRate(state.ir_sample_rate_Hz);
    bool ok = out.setAudioBuffer(out_buffer);
    out.save("out.wav");
    printf("OK? %d\n", ok);

    return 0;
}
