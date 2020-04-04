#include "utils.h"
#include <math.h>
#include <assert.h>

#include "audiofile/AudioFile.h"

#define FFT_BLOCK_SIZE 64

int plugin_state_init(plugin_state_t *state, const char *filename)
{
    bool ok;
    int n;
    AudioFile<float> ir;

    plugin_state_reset(state, false);

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

int plugin_state_reset(plugin_state_t *state, bool dirac_impulse_response)
{
    if (state->ir_left != NULL) {
        free(state->ir_left);
        state->ir_left = NULL;

    }
    if (state->ir_right != NULL) {
        free(state->ir_right);
        state->ir_right = NULL;
    }
    if (dirac_impulse_response) {
        state->ir_left = (float *)malloc(1);
        if (state->ir_left == NULL) {
            return 1;
        }
        state->ir_right = (float *)malloc(1);
        if (state->ir_right == NULL) {
            return 1;
        }
        state->ir_left[0] = 1.0;
        state->ir_right[0] = 1.0;
    }

    state->ir_num_samples_per_channel = 1;
    state->ir_num_channels = 2;
    state->ir_sample_rate_Hz = 1;
    state->ir_bit_depth = 24;
    state->fft_block_size = FFT_BLOCK_SIZE;

    return 0;
}

#define MASK0(x) ((( *(uint32_t*) (&x) ) >>  0) & 0xff)
#define MASK1(x) ((( *(uint32_t*) (&x) ) >>  8) & 0xff)
#define MASK2(x) ((( *(uint32_t*) (&x) ) >> 16) & 0xff)
#define MASK3(x) ((( *(uint32_t*) (&x) ) >> 24) & 0xff)

int plugin_state_serialize(plugin_state_t *state, char **output, uint32_t *length)
{
    assert(sizeof(uint32_t) == 4);
    assert(sizeof(float) == 4);

    // The output must be and ASCII string.
    // - Serialize the struct as a byte array (done manually to avoid architectural differences).
    // - Fixed-length members are encoded first as they can be used to predict the dynamic length members).
    // - Base64 encode the whole bit-string afterwards.

    plugin_state_t *S = state; // Short-hand for `state`
    uint32_t raw_size; // Size of the state object [bytes]

    raw_size = 0;
    raw_size += sizeof(uint32_t); // ir_sample_rate_Hz
    raw_size += sizeof(uint32_t); // ir_num_channels
    raw_size += sizeof(uint32_t); // ir_num_samples_per_channel
    raw_size += sizeof(uint32_t); // ir_bit_depth
    raw_size += sizeof(uint32_t); // fft_block_size
    raw_size += sizeof(float)*state->ir_num_samples_per_channel; // ir_left
    raw_size += sizeof(float)*state->ir_num_samples_per_channel; // ir_right

    // Serialize the struct.
    // Multibyte objects are encoded as Little Endian.
    uint8_t *s = NULL;
    s = (uint8_t *)malloc(raw_size);
    if (s == NULL) {
        return 1;
    }

    uint32_t i;
    uint32_t n = 0;

    s[n++] = MASK0(S->ir_sample_rate_Hz);
    s[n++] = MASK1(S->ir_sample_rate_Hz);
    s[n++] = MASK2(S->ir_sample_rate_Hz);
    s[n++] = MASK3(S->ir_sample_rate_Hz);

    s[n++] = MASK0(S->ir_num_channels);
    s[n++] = MASK1(S->ir_num_channels);
    s[n++] = MASK2(S->ir_num_channels);
    s[n++] = MASK3(S->ir_num_channels);

    s[n++] = MASK0(S->ir_num_samples_per_channel);
    s[n++] = MASK1(S->ir_num_samples_per_channel);
    s[n++] = MASK2(S->ir_num_samples_per_channel);
    s[n++] = MASK3(S->ir_num_samples_per_channel);

    s[n++] = MASK0(S->ir_bit_depth);
    s[n++] = MASK1(S->ir_bit_depth);
    s[n++] = MASK2(S->ir_bit_depth);
    s[n++] = MASK3(S->ir_bit_depth);

    s[n++] = MASK0(S->fft_block_size);
    s[n++] = MASK1(S->fft_block_size);
    s[n++] = MASK2(S->fft_block_size);
    s[n++] = MASK3(S->fft_block_size);

    for (i = 0; i < S->ir_num_samples_per_channel; i++) {
        s[n++] = MASK0(S->ir_left[i]);
        s[n++] = MASK1(S->ir_left[i]);
        s[n++] = MASK2(S->ir_left[i]);
        s[n++] = MASK3(S->ir_left[i]);
    }

    for (i = 0; i < S->ir_num_samples_per_channel; i++) {
        s[n++] = MASK0(S->ir_right[i]);
        s[n++] = MASK1(S->ir_right[i]);
        s[n++] = MASK2(S->ir_right[i]);
        s[n++] = MASK3(S->ir_right[i]);
    }

    *output = (char *)s;
    *length = raw_size;

    printf("\nSERIALIZED:\n");
    printf("%d\n", S->ir_sample_rate_Hz);
    printf("%d\n", S->ir_num_channels);
    printf("%d\n", S->ir_num_samples_per_channel);
    printf("%d\n", S->ir_bit_depth);
    printf("%d\n", S->fft_block_size);
    printf("%f\n", S->ir_left[0]);
    printf("%f\n", S->ir_left[1]);
    printf("%f\n", S->ir_right[0]);
    printf("%f\n", S->ir_right[1]);


    return 0;
}

#define UNMASK_UINT32(x0,x1,x2,x3) (((x3)<<24) | ((x2)<<16) | ((x1)<<8) | (x0))
#define FLOAT_FROM_UINT32(x) (*(float *)(&(x)))

int plugin_state_deserialize(plugin_state_t *state, char *input, uint32_t length)
{
    assert(sizeof(float) == 4);
    assert(sizeof(uint32_t) == 4);

    uint32_t b;
    plugin_state_t *S = state; // Short-hand for `state`
    uint8_t *x = (uint8_t *)input; // Short-hand for `input`
    uint32_t i;
    uint32_t n;

    n = 0;

    S->ir_sample_rate_Hz = UNMASK_UINT32(x[n], x[n+1], x[n+2], x[n+3]);
    n += 4;

    S->ir_num_channels = UNMASK_UINT32(x[n], x[n+1], x[n+2], x[n+3]);
    n += 4;

    S->ir_num_samples_per_channel = UNMASK_UINT32(x[n], x[n+1], x[n+2], x[n+3]);
    n += 4;

    S->ir_bit_depth = UNMASK_UINT32(x[n], x[n+1], x[n+2], x[n+3]);
    n += 4;

    S->fft_block_size = UNMASK_UINT32(x[n], x[n+1], x[n+2], x[n+3]);
    n += 4;

    S->ir_left = NULL;
    S->ir_left = (float *)malloc(sizeof(float) * S->ir_num_samples_per_channel);
    if (S->ir_left == NULL) {
        return 1;
    }
    S->ir_right = NULL;
    S->ir_right = (float *)malloc(sizeof(float) * S->ir_num_samples_per_channel);
    if (S->ir_right == NULL) {
        return 1;
    }

    for (i = 0; i < S->ir_num_samples_per_channel; i++) {
        b = UNMASK_UINT32(x[n], x[n+1], x[n+2], x[n+3]);
        n += 4;
        S->ir_left[i] = FLOAT_FROM_UINT32(b);
    }

    for (i = 0; i < S->ir_num_samples_per_channel; i++) {
        b = UNMASK_UINT32(x[n], x[n+1], x[n+2], x[n+3]);
        n += 4;
        S->ir_right[i] = FLOAT_FROM_UINT32(b);
    }

    printf("\nDESERIALIZED:\n");
    printf("%d\n", S->ir_sample_rate_Hz);
    printf("%d\n", S->ir_num_channels);
    printf("%d\n", S->ir_num_samples_per_channel);
    printf("%d\n", S->ir_bit_depth);
    printf("%d\n", S->fft_block_size);
    printf("%f\n", S->ir_left[0]);
    printf("%f\n", S->ir_left[1]);
    printf("%f\n", S->ir_right[0]);
    printf("%f\n", S->ir_right[1]);

    return 0;
}
