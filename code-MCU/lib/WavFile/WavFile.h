#ifndef WavFile_H_
#define WavFile_H_

#include <Arduino.h>
#include "FS.h"

#define WAV_OK 1
#define WAV_ERR_OPEN_FILE 2
#define WAV_ERR_HEADER 3
#define WAV_ERR_SIZE_MISMATCH 4

typedef int wave_err_t;

typedef struct
{
    uint32_t sampleRate;     // freq (Hz)
    uint32_t sampleTimeUSec; // =1,000,000/freq (microSeconds)
    uint16_t numChannels;
    uint16_t bitsPerSample;
    uint16_t dataStart;
    uint32_t dataLength;
} wavProps_t;

wave_err_t getWaveProps(String path, wavProps_t *wavProps);

String getWaveError(wave_err_t errCode);
#endif //WavFile_H_