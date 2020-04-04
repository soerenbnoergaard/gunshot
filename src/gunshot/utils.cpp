#include "utils.h"
#include <math.h>

#include "audiofile/AudioFile.h"

#define FFT_BLOCK_SIZE 64

int init_plugin_state(plugin_state_t *state, const char *filename)
{
    bool ok;
    int n;
    AudioFile<float> ir;

    ok = ir.load(filename);
    if (!ok) {
        return 1;
    }

    if ((ir.getNumChannels() < 1) || (2 < ir.getNumChannels())) {
        return 1;
    }

    state->ir_left = NULL;
    state->ir_left = (float *)malloc(sizeof(float)*ir.getNumSamplesPerChannel());
    if (state->ir_left == NULL) {
        return 1;
    }

    state->ir_right = NULL;
    state->ir_right = (float *)malloc(sizeof(float)*ir.getNumSamplesPerChannel());
    if (state->ir_right == NULL) {
        return 1;
    }

    // Calculate scale factor
    float sum_sq_left = 0.0;
    float sum_sq_right = 0.0;
    for (n = 0; n < ir.getNumSamplesPerChannel(); n++) {
        float L = ir.samples[0][n];
        float R = ir.samples[1][n];
        sum_sq_left += L*L;
        sum_sq_right += R*R;
    }
    float sum_sq_max = sum_sq_left > sum_sq_right ? sum_sq_left : sum_sq_right;
    float scale = 1.0/sqrt(sum_sq_max);

    for (n = 0; n < ir.getNumSamplesPerChannel(); n++) {
        state->ir_left[n] = scale * ir.samples[0][n];
        if (ir.getNumChannels() > 1) {
            state->ir_right[n] =  scale * ir.samples[1][n];
        }
    }

    state->ir_num_samples_per_channel = ir.getNumSamplesPerChannel();
    state->ir_num_channels = ir.getNumChannels();
    state->ir_sample_rate_Hz = ir.getSampleRate();
    state->ir_bit_depth = ir.getBitDepth();
    state->fft_block_size = FFT_BLOCK_SIZE;

    return 0;
}

