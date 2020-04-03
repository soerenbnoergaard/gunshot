#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
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
    AudioFile<float> ir;
    fftconvolver::FFTConvolver convolver;
    uint32_t N;
    float y[NUM_TEST_SAMPLES];
    float x[NUM_TEST_SAMPLES];

    // Load impulse response from file
    ir.load("test.wav");
    ir.printSummary();
    convolver.init(FFT_BLOCK_SIZE, (fftconvolver::Sample *)(&ir.samples[0][0]), ir.getNumSamplesPerChannel());

    // Generate test data
    for (int n = 0; n < 24000; n++) {
        x[n] = get_sample(n, ir.getSampleRate());
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
    out.setBitDepth(ir.getBitDepth());
    out.setSampleRate(ir.getSampleRate());
    bool ok = out.setAudioBuffer(out_buffer);
    out.save("out.wav");
    printf("OK? %d\n", ok);

    return 0;
}
