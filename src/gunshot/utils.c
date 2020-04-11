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

uint32_t find_basename(const char *abspath)
{
    int32_t n;

#ifdef DISTRHO_OS_WINDOWS
    char sep = '\\';
#else
    char sep = '/';
#endif

    // Find last directory separator
    for (n = strlen(abspath)-1; n >= 0; n--) {
        if (abspath[n] == sep) {
            return n+1;
        }
    }

    return 0;
}

