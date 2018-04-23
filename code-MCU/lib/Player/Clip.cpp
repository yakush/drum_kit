#include "Clip.h"
#include <Arduino.h>
#include "SD.h"
#include "FS.h"
#include "ClipInfo.h"
#include "Utils.h"

using namespace NS_Player;

Clip::Clip(uint in_bufferSize)
{
    bufferSize = in_bufferSize;
    buffer[0] = new byte[bufferSize];
    buffer[1] = new byte[bufferSize];
    reset();
}

Clip::~Clip()
{
    delete[] buffer[0];
    delete[] buffer[1];

    if (file)
    {
        file.close();
    }
}

//-- methods:
void Clip::prepare(ClipInfo_t in_info)
{
    reset();

    info = in_info;
    file = SD.open(in_info.filepath, FILE_READ);
    file.seek(in_info.wavProps.dataStart);

    isPlaying = true;
    needFillBuffer = true;
    firstBufferReady = false;
}

void Clip::reset()
{
    if (file)
    {
        file.close();
    }

    filePos = 0;
    activeBuffer = 0;
    buffPos_play = 0;
    needFillBuffer = true;
    firstBufferReady = false;
    isPlaying = false;
    isReadingFile = false;
    isRequestStop = false;
    lastSampledTime = 0;
    startTime = 0;
    val_0 = 0;
    val_1 = 0;
    currentValue_L = 0;
    currentValue_R = 0;
}

void Clip::readAndInterpolate(long timeUS)
{
    if (!isPlaying || !firstBufferReady)
    {
        return;
    }

    // time since last read + overflow fix
    uint16_t dT_lastRead =
        (timeUS >= lastSampledTime)
            ? timeUS - lastSampledTime
            : timeUS + (UINT32_MAX - lastSampledTime);

    if (dT_lastRead >= info.wavProps.sampleTimeUSec)
    {
        lastSampledTime =
            (timeUS >= lastSampledTime)
                ? lastSampledTime + info.wavProps.sampleTimeUSec
                : info.wavProps.sampleTimeUSec - (UINT32_MAX - lastSampledTime);

        val_0 = val_1;
        val_1 = consumeSingleSample();
    }

    uint32_t sampleVal = val_0 + ((((int32_t)val_1 - val_0) * dT_lastRead) / info.wavProps.sampleTimeUSec);
    currentValue_L = sampleVal * volume * (0x00 + blendLR);
    currentValue_R = sampleVal * volume * (0xFF - blendLR);
}

void Clip::updateState()
{
    // MUST BE LOCKED !

    if (!isRequestStop && isPlaying && firstBufferReady)
    {
        if (filePos >= info.wavProps.dataLength)
        {
            isRequestStop = true;
        }
        else if (buffPos_play >= bufferSize)
        {
            //finished reading buffer - flip buffers
            activeBuffer = !activeBuffer;
            buffPos_play = 0;
            needFillBuffer = true;
        }
    }

    // ok to dispose?
    if (isRequestStop && !isReadingFile)
    {
        reset();
    }
}

int16_t Clip::consumeSingleSample()
{
    int numBytes;
    int16_t res;

    if (info.wavProps.bitsPerSample == 32)
    {
        // 32bit singed little endian , ignore 2 LSB (we're doing 16 bit output here...)
        numBytes = 4;
        res = buffer[activeBuffer][buffPos_play + 3];
        res = res << 8;
        res |= buffer[activeBuffer][buffPos_play + 2];
    }
    else if (info.wavProps.bitsPerSample == 24)
    {
        // 24bit singed little endian , ignore 1 LSB (we're doing 16 bit output here...)
        numBytes = 3;
        res = buffer[activeBuffer][buffPos_play + 2];
        res = res << 8;
        res |= buffer[activeBuffer][buffPos_play + 1];
    }
    else if (info.wavProps.bitsPerSample == 16)
    {
        // 16bit singed little endian
        numBytes = 2;
        res = buffer[activeBuffer][buffPos_play + 1];
        res = res << 8;
        res |= buffer[activeBuffer][buffPos_play + 0];
    }
    else if (info.wavProps.bitsPerSample == 8)
    {
        // 8bit unsinged
        numBytes = 1;
        res = buffer[activeBuffer][buffPos_play + 0];
        res = res << 8;
        res ^= 0x8000; //convert to singed (flip most significant bit)
    }
    else
    {
        // ...?!?!
    }

    filePos += numBytes;
    buffPos_play += numBytes;
    return res;
}