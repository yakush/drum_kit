#include "WavFile.h"

#include <Arduino.h>
#include <SD.h>

#define CCCC(c1, c2, c3, c4) ((c4 << 24) | (c3 << 16) | (c2 << 8) | c1)

#define __RIFF__ CCCC('R', 'I', 'F', 'F')
#define __WAVE__ CCCC('W', 'A', 'V', 'E')
#define __DATA__ CCCC('d', 'a', 't', 'a')

typedef struct wavHeader_s
{
    uint32_t riffChunkID;
    uint32_t riffChunkSize;
    uint32_t riffFormat;

    uint32_t wavChunkID;
    uint32_t wavChunkSize;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;

    uint32_t dataChunkID;
    uint32_t dataSize;
} wavHeader_t;

int getWaveProps(String path, wavProps_t *wavProps)
{
    File file = SD.open(path, FILE_READ);
    if (!file)
    {
        return WAV_ERR_OPEN_FILE;
    }

    size_t fileSize = file.size();
    size_t reads = 0;
    wavHeader_t headers;

    reads += file.read((uint8_t *)&headers, sizeof(wavHeader_t));
    file.close();

    if (reads != sizeof(wavHeader_t))
    {
        return WAV_ERR_HEADER;
    }

    if (headers.riffChunkID != __RIFF__ ||
        headers.riffFormat != __WAVE__ ||
        headers.dataChunkID != __DATA__)
    {
        return WAV_ERR_HEADER;
    }

    if (fileSize - sizeof(wavHeader_t) != headers.dataSize)
    {
        return WAV_ERR_SIZE_MISMATCH;
    }

    wavProps->sampleRate = headers.sampleRate;
    wavProps->sampleTimeUSec = round((float)1000000 / headers.sampleRate);
    wavProps->numChannels = headers.numChannels;
    wavProps->bitsPerSample = headers.bitsPerSample;
    wavProps->dataStart = sizeof(wavHeader_t);
    wavProps->dataLength = headers.dataSize;

    return WAV_OK;
}

String getWaveError(int errCode)
{
    switch (errCode)
    {
    case WAV_OK:
        return "all good";
    case WAV_ERR_OPEN_FILE:
        return "error reading file";
    case WAV_ERR_HEADER:
        return "error while parsing header";
    case WAV_ERR_SIZE_MISMATCH:
        return "error - file size doesn't match wave header";
    default:
        return "unknown error";
    }
}