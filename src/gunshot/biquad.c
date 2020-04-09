#include "biquad.h"
#include <math.h>
#include "log.h"

static void _clear_delay_line(biquad_t *s)
{
    s->x[0] = 0.0;
    s->x[1] = 0.0;
    s->x[2] = 0.0;

    s->y[0] = 0.0;
    s->y[1] = 0.0;
    s->y[2] = 0.0;
}

static biquad_t _biquad_calculate_generic(float cutoff_Hz, float sample_rate_Hz, bool clear_delay_line, bool is_low_pass)
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

    if (clear_delay_line) {
        _clear_delay_line(&s);
    }

    return s;
}

biquad_t biquad_calculate_highpass(float cutoff_Hz, float sample_rate_Hz, bool clear_delay_line)
{
    return _biquad_calculate_generic(cutoff_Hz, sample_rate_Hz, clear_delay_line, false);
}

biquad_t biquad_calculate_lowpass(float cutoff_Hz, float sample_rate_Hz, bool clear_delay_line)
{
    return _biquad_calculate_generic(cutoff_Hz, sample_rate_Hz, clear_delay_line, true);
}

biquad_t biquad_calculate_nofilter(bool clear_delay_line)
{
    log_write("Call: biquad_calculate_nofilter()");
    biquad_t s;
    s.a[0] = 1.0;
    s.a[1] = 0.0;
    s.a[2] = 0.0;
    s.b[0] = 1.0;
    s.b[1] = 0.0;
    s.b[2] = 0.0;

    if (clear_delay_line) {
        _clear_delay_line(&s);
    }
    return s;
}

float biquad_process_sample(biquad_t *s,  float input)
{
    // Apply difference equation
    s->x[0] = input;
    s->y[0] = s->b[0]*s->x[0] + s->b[1]*s->x[1] + s->b[2]*s->x[2] - s->a[1]*s->y[1] - s->a[2]*s->y[2];
    s->y[0] /= s->a[0];
    
    // Update delay lines
    s->x[2] = s->x[1];
    s->x[1] = s->x[0];
    
    s->y[2] = s->y[1];
    s->y[1] = s->y[0];

    return s->y[0];
}
