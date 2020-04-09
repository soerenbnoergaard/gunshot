#include "biquad.h"
#include <math.h>

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

