#include <stdio.h>
#include <stdint.h>
#include "AudioFile.h"

int main(void)
{
    AudioFile<float> wav;

    wav.load("test.wav");
    wav.printSummary();



    return 0;
}
