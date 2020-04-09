#include "utils.h"
#include <math.h>
#include <assert.h>
#include <string.h>

#include "audiofile/AudioFile.h"
extern "C" {
#include "base64/base64.h"
}

#define FFT_BLOCK_SIZE 1024

#ifdef GUNSHOT_LOG_FILE
static char line[1024];
#endif

////////////////////////////////////////////////////////////////////////////////

int plugin_state_init(plugin_state_t *state, const char *filename)
{
    bool ok;
    int n;
    AudioFile<float> ir;

    ok = ir.load(filename);
    if (!ok) {
        log_write("Error loading impulse response from file");
        return 1;
    }

#ifdef GUNSHOT_LOG_FILE
    sprintf(line, "Filename: %s", filename);
    log_write(line);
    sprintf(line, "Num channels: %d", ir.getNumChannels());
    log_write(line);
    sprintf(line, "Num samples per channel: %d", ir.getNumSamplesPerChannel());
    log_write(line);
    sprintf(line, "Sample rate: %d", ir.getSampleRate());
    log_write(line);
    sprintf(line, "Bit depth: %d", ir.getBitDepth());
    log_write(line);
    sprintf(line, "Length in seconds: %f", ir.getLengthInSeconds());
    log_write(line);
#endif

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
        float R = (ir.getNumChannels() > 1) ? ir.samples[1][n] : L;

        sum_sq_left += L*L;
        sum_sq_right += R*R;
    }
    float sum_sq_max = sum_sq_left > sum_sq_right ? sum_sq_left : sum_sq_right;
    float scale = 1.0/sqrt(sum_sq_max);

    for (n = 0; n < ir.getNumSamplesPerChannel(); n++) {
        state->ir_left[n] = scale * ir.samples[0][n];
        state->ir_right[n] =  (ir.getNumChannels() > 1) ? (scale * ir.samples[1][n]) : (scale * ir.samples[0][n]);
    }

    state->ir_num_samples_per_channel = ir.getNumSamplesPerChannel();
    state->ir_num_channels = ir.getNumChannels();
    state->ir_sample_rate_Hz = ir.getSampleRate();
    state->ir_bit_depth = ir.getBitDepth();
    state->fft_block_size = FFT_BLOCK_SIZE;

    return 0;
}

int plugin_state_init_dirac(plugin_state_t *state, uint32_t sample_rate_Hz)
{
    state->ir_left = NULL;
    state->ir_right = NULL;

    state->ir_left = (float *)malloc(1 * sizeof(float));
    if (state->ir_left == NULL) {
        return 1;
    }
    state->ir_right = (float *)malloc(1 * sizeof(float));
    if (state->ir_right == NULL) {
        return 1;
    }
    state->ir_left[0] = 1.0;
    state->ir_right[0] = 1.0;

    state->ir_num_samples_per_channel = 1;
    state->ir_num_channels = 2;
    state->ir_sample_rate_Hz = sample_rate_Hz;
    state->ir_bit_depth = 24;
    state->fft_block_size = FFT_BLOCK_SIZE;

    return 0;
}

int plugin_state_free(plugin_state_t *state)
{
    free(state->ir_left);
    free(state->ir_right);
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
    uint32_t s_length; // Size of the state object [bytes]

    s_length = 0;
    s_length += sizeof(uint32_t); // ir_sample_rate_Hz
    s_length += sizeof(uint32_t); // ir_num_channels
    s_length += sizeof(uint32_t); // ir_num_samples_per_channel
    s_length += sizeof(uint32_t); // ir_bit_depth
    s_length += sizeof(uint32_t); // fft_block_size
    s_length += sizeof(float)*state->ir_num_samples_per_channel; // ir_left
    s_length += sizeof(float)*state->ir_num_samples_per_channel; // ir_right

    // Serialize the struct.
    // Multibyte objects are encoded as Little Endian.
    uint8_t *s = NULL;
    s = (uint8_t *)malloc(s_length);
    if (s == NULL) {
        return 1;
    }

    uint32_t i;
    uint32_t n = 0;

    // Serialize fixed-length members
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

    // Serialize dynamic-length members
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

    // Base64-encode the byte-string
    uint32_t out_length = 0;
    uint8_t *out = NULL;
    out = (uint8_t *)malloc(b64e_size(s_length)+1);
    if (out == NULL) {
        return 1;
    }
    out_length = b64_encode(s, s_length, out);
    free(s);

    // Return pointers to the result
    *output = (char *)out;
    *length = out_length;

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
    uint32_t i;
    uint32_t n;

    // Decode byte-string from base64 string
    uint32_t x_length = 0;
    uint8_t *x = NULL;
    x = (uint8_t *)malloc(b64d_size(length));
    if (x == NULL) {
        return 1;
    }
    x_length = b64_decode((uint8_t *)input, length, x);

    n = 0;

    // Decode fixes-length members
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

    // Allocate space for the dynamic-length members
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

    // Decode dynamic-length members
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

    // Clean up
    free(x);
    return 0;
}

int log_init(void)
{
#ifdef GUNSHOT_LOG_FILE
    FILE *f = fopen(GUNSHOT_LOG_FILE, "w");
    if (f == NULL) {
        return 1;
    }
    fclose(f);
#endif
    return 0;
}

int log_write(const char *s)
{
#ifdef GUNSHOT_LOG_FILE
    FILE *f = fopen(GUNSHOT_LOG_FILE, "a");
    if (f == NULL) {
        return 1;
    }
    fprintf(f, "%s\n", s);
    fclose(f);
#endif
    return 0;
}

static biquad_t _biquad_calculate_generic(float cutoff_Hz, float sample_rate_Hz, bool is_low_pass)
{
    // Apogee Filter Design Equations
    float Q = 0.707;
    float wc = 2.0*M_PI * cutoff_Hz/sample_rate_Hz;
    float wS = sin(wc);
    float wC = cos(wc);
    float alpha = wS/(2.0*Q);
    biquad_t s;

    s.a[0] = 1.0+alpha;
    s.a[1] = -2.0*wC;
    s.a[2] = 1.0-alpha;
    
    if (is_low_pass) {
        s.b[0] = (1.0-wC)/2.0;
        s.b[1] = 1.0-wC;
        s.b[2] = s.b[0];
    }
    else {
        s.b[0] = (1.0+wC)/2.0;
        s.b[1] = -(1.0+wC);
        s.b[2] = s.b[0];
    }

    s.x[0] = 0.0;
    s.x[1] = 0.0;
    s.x[2] = 0.0;

    s.y[0] = 0.0;
    s.y[1] = 0.0;
    s.y[2] = 0.0;

    return s;
}

biquad_t biquad_calculate_highpass(float cutoff_Hz, float sample_rate_Hz)
{
    return _biquad_calculate_generic(cutoff_Hz, sample_rate_Hz, false);
}

biquad_t biquad_calculate_lowpass(float cutoff_Hz, float sample_rate_Hz)
{
    return _biquad_calculate_generic(cutoff_Hz, sample_rate_Hz, true);
}

float convert_dB_to_linear(float x_dB)
{
    if (x_dB < -59.0) {
        return 0.0;
    } else {
        return pow(10.0, x_dB/20.0);
    }
}
