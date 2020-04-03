#include "utils.h"

#include "audiofile/AudioFile.h"

int init_plugin_state(plugin_state_t *state, const char *filename)
{
    int n;
    AudioFile<float> ir;

    ir.load(filename);

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

    for (n = 0; n < ir.getNumSamplesPerChannel(); n++) {
        state->ir_left[n] = ir.samples[0][n];
        if (ir.getNumChannels() > 1) {
            state->ir_right[n] = ir.samples[1][n];
        }
    }

    state->ir_num_samples_per_channel = ir.getNumSamplesPerChannel();
    state->ir_num_channels = ir.getNumChannels();
    state->ir_sample_rate_Hz = ir.getSampleRate();
    state->ir_bit_depth = ir.getBitDepth();

    return 0;
}

