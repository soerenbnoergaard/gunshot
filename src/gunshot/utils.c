#include "utils.h"
#include <math.h>

float convert_dB_to_linear(float x_dB)
{
    if (x_dB < -59.0) {
        return 0.0;
    } else {
        return pow(10.0, x_dB/20.0);
    }
}
