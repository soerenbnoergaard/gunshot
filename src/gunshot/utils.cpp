#include "utils.h"
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>

#define error(s) puts(s)
#define WAV_HEADER_SIZE 44
#define MAX_FILENAME_LENGTH 1024

status_t filesize(const char *filename, int32_t *filesize)
{
    struct stat st;

    if (stat(filename, &st) == 0) {
        *filesize = (int32_t)(st.st_size);
        return STATUS_OK;
    }

    return STATUS_ERROR;
}

status_t load(const char *filename, float *left, float *right, uint32_t *length, uint32_t *sample_rate_Hz)
{
    uint8_t *all_data = NULL;
    int32_t size;
    FILE *f;

    // Determine file size and allocate memory
    if (filesize(filename, &size) != STATUS_OK) {
        return STATUS_ERROR;
    }

    size -= WAV_HEADER_SIZE;
    all_data = (uint8_t *)malloc(size);

    if (all_data == NULL) {
        error("Failed to allocate memory");
        return STATUS_ERROR;
    }

    // TODO: Assert the header: 16LE, interleaved.
    // TODO: Make sure to sample-rate convert in the sampleRateChanged() method.

    // Read data
    f = fopen(filename, "rb");
    fseek(f, WAV_HEADER_SIZE, SEEK_SET);
    *length = fread(left, sizeof(int16_t), size/2, f);
    fclose(f);

    if (*length != size/2) {
        error("Failed to read the whole file");
        return STATUS_ERROR;
    }

    return STATUS_OK;
}

