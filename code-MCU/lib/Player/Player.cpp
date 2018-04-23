#include "Player.h"
#include <Arduino.h>
#include "SampleInfo.h"
#include "WavFile.h"
#include "Utils.h"

using namespace NS_Player;

//-----------------------------------------------------------------
//-- class instance
Player_CLASS Player;
//-----------------------------------------------------------------

Player_CLASS::Player_CLASS()
{
}

Player_CLASS::~Player_CLASS()
{
}

bool Player_CLASS::begin(ISoundRenderer *in_soundRenderer)
{
    soundRenderer = in_soundRenderer;
    return true;
}
void Player_CLASS::end()
{
}

int Player_CLASS::LoadSample(String filepath, SampleInfo_t *sampleInfo)
{
    int wavRes = getWaveProps(filepath, &sampleInfo->wavProps);
    if (wavRes == WAV_OK)
    {
        sampleInfo->filepath = filepath;
    }
    return wavRes;
}

//------------------------------------------------------------
//HELPERS
//------------------------------------------------------------

Player_CLASS *player;
hw_timer_t *playTimer = NULL;
bool isRunning_readTask = false;
bool isRunning_playISR = false;
unsigned long lastReadTaskYieldTime = 0;

void start_readTask();
void start_playISR(uint32_t freq);
void stop_readTask();
void stop_playISR();
void loop_readTask(void *parameter);
void IRAM_ATTR handlePlay_ISR();
void handleRead_TASK();

void start_readTask()
{
    stop_readTask();

    // start task (core 0):
    isRunning_readTask = true;
    xTaskCreatePinnedToCore(
        loop_readTask,            // task func
        "Player::_loop_readTask", // name
        10000,                    // stack size?!
        NULL,                     // params
        8,                        // priority
        NULL,                     // task handle
        0                         // core (0 is sencondary core)
    );
}

void start_playISR(uint32_t freq)
{
    stop_playISR();

    isRunning_playISR = true;
    // 1 tick take 1/(80MHZ/divider) . i.e 80 -> 1us
    playTimer = timerBegin(0, 2, true);
    timerAttachInterrupt(playTimer, &handlePlay_ISR, true);
    timerAlarmWrite(playTimer, 40000000 / freq, true); // 20 uSecs = 96khz
    timerAlarmEnable(playTimer);
}

void stop_readTask()
{
    isRunning_readTask = false;
}

void stop_playISR()
{
    isRunning_playISR = false;
    if (!playTimer)
    {
        return;
    }
    timerDetachInterrupt(playTimer);
    timerEnd(playTimer);
    playTimer = NULL;
}

//-----------------------------------------------------------------
void loop_readTask(void *parameter)
{
    while (isRunning_readTask)
    {
        handleRead_TASK();

        //play nice pause every second...
        unsigned long now = millis();
        if ((now > lastReadTaskYieldTime + 1000) || (now < lastReadTaskYieldTime))
        {
            lastReadTaskYieldTime = now;
            vTaskDelay(1);
        }
    }
    vTaskDelete(NULL);
}

//------------------------------------------------------------
// HANDLERS
//------------------------------------------------------------

void IRAM_ATTR handlePlay_ISR()
{
    long now = micros();

    int32_t sumL_32;
    int32_t sumR_32;
    for (int i = 0; i < PLAYER_MAX_PLAYING_SAMPLES; i++)
    {
        SampleWrapper_t wrapper = player->samples[i];
        Sample *s = wrapper.sample;
        s->readAndInterpolate(now);
        sumL_32 += s->currentValue_L; //these are 24bit numbers, so no overflow can happen
        sumR_32 += s->currentValue_R; //these are 24bit numbers, so no overflow can happen
    }

    int64_t sumL_64 = player->volume * sumL_32; // now it can overflow the 32bit ints
    int64_t sumR_64 = player->volume * sumR_32; // now it can overflow the 32bit ints

    sumL_32 = fastSigmoid_64to32(sumL_64); // soft clip back to 32bit
    sumR_32 = fastSigmoid_64to32(sumR_64); // soft clip back to 32bit

    player->soundRenderer->play(sumL_32, sumR_32);

    player->lockSamples_ISR();
    for (int i = 0; i < PLAYER_MAX_PLAYING_SAMPLES; i++)
    {
        SampleWrapper_t wrapper = player->samples[i];
        Sample *s = wrapper.sample;
        s->updateState();
    }
    player->unlockSamples_ISR();
}

void handleRead_TASK()
{
    static SampleWrapper_t readList[PLAYER_MAX_PLAYING_SAMPLES];
    static int numSamplesToRead;

    player->lockSamples_TASK();
    numSamplesToRead = 0;
    for (int i = 0; i < PLAYER_MAX_PLAYING_SAMPLES; i++)
    {
        SampleWrapper_t wrapper = player->samples[i];
        Sample *s = wrapper.sample;
        if (s->isPlaying && s->needFillBuffer)
        {
            s->isReadingFile = true;
            readList[numSamplesToRead] = wrapper;
            numSamplesToRead++;
        }
    }
    player->unlockSamples_TASK();

    //read samples
    for (size_t i = 0; i < numSamplesToRead; i++)
    {
        SampleWrapper_t wrapper = readList[i];
        Sample *s = wrapper.sample;

        size_t reads = s->file.read(s->buffer[!s->activeBuffer], s->bufferSize);

        //first time only - fill both buffers (active and non active)
        if (!s->firstBufferReady)
        {
            s->file.read(s->buffer[s->activeBuffer], s->bufferSize);
        }
    }

    //update state
    player->lockSamples_TASK();
    for (size_t i = 0; i < numSamplesToRead; i++)
    {
        SampleWrapper_t wrapper = readList[i];
        Sample *s = wrapper.sample;

        s->isReadingFile = false;

        // update states (only for samples that didn't stop/change while the read was going on)
        if (s->isPlaying)
        {
            s->firstBufferReady = true;
            s->needFillBuffer = false;
        }
    }

    player->unlockSamples_TASK();
}