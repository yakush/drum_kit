#include "Player.h"
#include <Arduino.h>
#include "ClipInfo.h"
#include "Clip.h"
#include "WavFile.h"
#include "Utils.h"

using namespace NS_Player;

//-----------------------------------------------------------------
//-- class instance
Player_CLASS Player;
//-----------------------------------------------------------------

//-----------------------------------------------------------------
//-----------------------------------------------------------------
Player_CLASS *player;
void loop_readTask(void *parameter);
void IRAM_ATTR loopPlay_ISR();

//-----------------------------------------------------------------
//-----------------------------------------------------------------

Player_CLASS::Player_CLASS()
{
    player = this;
}

Player_CLASS::~Player_CLASS()
{
}

bool Player_CLASS::begin(ISoundRenderer *in_soundRenderer)
{
    if (started)
    {
        return true;
    }

    clips = new ClipWrapper_t[PLAYER_MAX_PLAYING_CLIPS];
    for (int i = 0; i < PLAYER_MAX_PLAYING_CLIPS; i++)
    {
        clips[i].handle = 0;
        clips[i].clip = new Clip(PLAYER_CLIP_BUFFER_SIZE);
        clips[i].callback = NULL;
    }

    soundRenderer = in_soundRenderer;

    //start tasks and timers
    start_readTask();
    start_playISR(soundRenderer->getFreq());

    started = true;
    return true;
}

void Player_CLASS::end()
{
    if (!started)
    {
        return;
    }

    started = false;
    stopAllClips();
    stop_readTask();
    stop_playISR();
    delete[] clips;
}

wave_err_t Player_CLASS::LoadClip(String filepath, ClipInfo_t *clipInfo)
{
    int wavRes = getWaveProps(filepath, &clipInfo->wavProps);
    if (wavRes == WAV_OK)
    {
        clipInfo->filepath = filepath;
    }
    return wavRes;
}

String Player_CLASS::getClipEndReason(clipEndedReason_t reason)
{
    switch (reason)
    {
    case PLAYER_CLIP_ENDED_FINISHED:
        return "FINISHED";
    case PLAYER_CLIP_ENDED_STOPPED:
        return "STOPPED";
    case PLAYER_CLIP_ENDED_PRUNED:
        return "PRUNED";
    default:
        return "unknon";
    }
}

long Player_CLASS::playClip(ClipInfo_t clipInfo, uint8_t volume, uint8_t blendLR, PlayerCallback_play_ended_t callback)
{
    lockClips_TASK();

    //find a free slot
    uint8_t idx = 0;
    uint8_t oldestClipIdx = 0;
    size_t oldestClipPlayPos = 0;
    bool found = false;
    bool foundOldest = false;

    for (size_t i = 0; i < PLAYER_MAX_PLAYING_CLIPS; i++)
    {
        Clip *clip = clips[i].clip;
        if (!clip->isPlaying)
        {
            idx = i;
            found = true;
            break;
        }
        //collect oldest sample (playing pos is highest)
        if (clip->playPosition > oldestClipPlayPos)
        {
            oldestClipPlayPos = clip->playPosition;
            oldestClipIdx = idx;
            foundOldest = true;
        }
    }

    //if not found - stop and use the oldest sample slot
    if (!found && foundOldest)
    {
        found = true;
        idx = oldestClipIdx;
    }

    if (!found)
    {
        Serial.printf(" -- didn't find available clip slot\n");
        return 0;
    }

    stopClipByIdx(idx, PLAYER_CLIP_ENDED_PRUNED);

    //set up the sample
    ClipWrapper_t *wrapper = &clips[idx];
    Clip *clip = wrapper->clip;

    String path = clipInfo.filepath;
    Serial.print(" -- filename ");
    Serial.println(path);

    clip->prepare(clipInfo);
    if (!clip->file)
    {
        return 0;
    }
    
    //prepare sample
    lastSampleHandle++;
    wrapper->callback = callback;
    wrapper->handle = lastSampleHandle;

    //info:
    wavProps_t wavProps = clip->info.wavProps;
    int millis = (wavProps.dataLength / (wavProps.numChannels * wavProps.bitsPerSample / 8)) * wavProps.sampleTimeUSec / 1000;
    int mins = millis / 1000 / 60;
    int secs = (millis / 1000) % 60;
    millis = millis % 1000;
    Serial.printf(" -- opened, fileSize %u\n", clip->file.size());
    Serial.printf(" -- play time : %02d:%02d.%03d\n", mins, secs, millis);
    Serial.printf(" -- playing clip with handle %u on slot [%u]\n", wrapper->handle, idx);
    Serial.println();


    unlockClips_TASK();

    return lastSampleHandle;
}

void Player_CLASS::stopClip(long clipHandle)
{
    lockClips_TASK();

    //try to find the clip
    for (size_t i = 0; i < PLAYER_MAX_PLAYING_CLIPS; i++)
    {
        ClipWrapper_t wrapper = clips[i];
        Clip *clip = wrapper.clip;
        if (wrapper.handle == clipHandle)
        {
            stopClipByIdx(i, PLAYER_CLIP_ENDED_STOPPED);
            break;
        }
    }

    unlockClips_TASK();
}

void Player_CLASS::stopAllClips()
{
    lockClips_TASK();
    for (size_t i = 0; i < PLAYER_MAX_PLAYING_CLIPS; i++)
    {
        stopClipByIdx(i, PLAYER_CLIP_ENDED_STOPPED);
    }
    unlockClips_TASK();
}

void Player_CLASS::stopClipByIdx(uint8_t clipIdx, clipEndedReason_t reason)
{
    lockClips_TASK();

    ClipWrapper_t wrapper = clips[clipIdx];
    Clip *clip = wrapper.clip;

    if (clip->isPlaying && !clip->isRequestStop)
    {
        clip->isRequestStop = true;
        clip->requestStopReason = reason;
    }

    unlockClips_TASK();
}

//------------------------------------------------------------
//TASKS AND TIMERS
//------------------------------------------------------------

void Player_CLASS::start_readTask()
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
        0                         // core (0 is secondary core)
    );
}

void Player_CLASS::start_playISR(uint32_t freq)
{
    stop_playISR();

    isRunning_playISR = true;
    // 1 tick take 1/(80MHZ/divider) . i.e 80 -> 1us
    playTimer = timerBegin(0, 2, true);
    timerAttachInterrupt(playTimer, &loopPlay_ISR, true);
    timerAlarmWrite(playTimer, 40000000 / freq, true); // 20 uSecs = 96khz
    timerAlarmEnable(playTimer);
}

void Player_CLASS::stop_readTask()
{
    isRunning_readTask = false;
}

void Player_CLASS::stop_playISR()
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

// -------------------------------------------------------
// handlers

void Player_CLASS::_handleRead_TASK()
{
    static ClipWrapper_t readList[PLAYER_MAX_PLAYING_CLIPS];
    static int numClipsToRead;

    static PlayerCallback_play_ended_t callbacks_Funcs[PLAYER_MAX_PLAYING_CLIPS];
    static uint32_t callbacks_Handles[PLAYER_MAX_PLAYING_CLIPS];
    static clipEndedReason_t callbacks_Reasons[PLAYER_MAX_PLAYING_CLIPS];
    static int numCallbacks;

    // LOCK START <<============
    lockClips_TASK();

    numClipsToRead = 0;
    numCallbacks = 0;
    for (int i = 0; i < PLAYER_MAX_PLAYING_CLIPS; i++)
    {
        ClipWrapper_t wrapper = clips[i];
        Clip *s = wrapper.clip;

        // if requested to stop - prepare callbacks and reset clips
        if (s->isRequestStop)
        {
            if (wrapper.callback != NULL)
            {
                callbacks_Funcs[numCallbacks] = wrapper.callback;
                callbacks_Handles[numCallbacks] = wrapper.handle;
                callbacks_Reasons[numCallbacks] = s->requestStopReason;
                numCallbacks++;
            }

            s->reset();
        }

        // prepare list of buffers that need filling
        if (s->isPlaying && s->needFillBuffer)
        {
            s->isReadingFile = true;
            readList[numClipsToRead] = wrapper;
            numClipsToRead++;
        }
    }
    unlockClips_TASK();
    // LOCK END   ============>>

    //perform callbacks
    for (int i = 0; i < numCallbacks; i++)
    {
        callbacks_Funcs[i](callbacks_Handles[i], callbacks_Reasons[i]);
    }

    //read samples
    for (size_t i = 0; i < numClipsToRead; i++)
    {
        ClipWrapper_t wrapper = readList[i];
        Clip *s = wrapper.clip;

        size_t reads = s->file.read(s->buffer[!s->activeBuffer], s->bufferSize);

        //first time only - fill both buffers (active and non active)
        if (!s->firstBufferReady)
        {
            s->file.read(s->buffer[s->activeBuffer], s->bufferSize);
        }
    }

    // LOCK START <<============
    lockClips_TASK();

    //update state
    for (size_t i = 0; i < numClipsToRead; i++)
    {
        ClipWrapper_t wrapper = readList[i];
        Clip *s = wrapper.clip;

        s->isReadingFile = false;

        // update states (only for samples that didn't stop/change while the read was going on)
        if (s->isPlaying)
        {
            s->firstBufferReady = true;
            s->needFillBuffer = false;
        }
    }

    unlockClips_TASK();
    // LOCK END   ============>>

    //play nice - pause every second...
    unsigned long now = millis();
    if ((now > lastReadTaskYieldTime + 1000) || (now < lastReadTaskYieldTime))
    {
        lastReadTaskYieldTime = now;
        vTaskDelay(1);
    }

    //exit if needed
    if (!isRunning_readTask)
        vTaskDelete(NULL);
}

// -------------------------------------------------------

void Player_CLASS::_handlePlay_ISR()
{
    long now = micros();

    int32_t sumL_32=0;
    int32_t sumR_32=0;
    for (int i = 0; i < PLAYER_MAX_PLAYING_CLIPS; i++)
    {
        ClipWrapper_t wrapper = clips[i];
        Clip *s = wrapper.clip;
        s->readAndInterpolate(now);
        sumL_32 += s->currentValue_L; //these are 24bit numbers, so no overflow can happen
        sumR_32 += s->currentValue_R; //these are 24bit numbers, so no overflow can happen
    }

    int64_t sumL_64 = player->volume * sumL_32; // now it can overflow the 32bit ints
    int64_t sumR_64 = player->volume * sumR_32; // now it can overflow the 32bit ints

    sumL_32 = fastSigmoid_64to32(sumL_64); // soft clip back to 32bit
    sumR_32 = fastSigmoid_64to32(sumR_64); // soft clip back to 32bit

    soundRenderer->play(sumL_32, sumR_32);

    // LOCK START <<============
    lockClips_ISR();
    for (int i = 0; i < PLAYER_MAX_PLAYING_CLIPS; i++)
    {
        ClipWrapper_t wrapper = clips[i];
        Clip *s = wrapper.clip;
        s->updateState();
    }
    unlockClips_ISR();
    // LOCK END   ============>>
}

//------------------------------------------------------------
// LOOPERS (must be static functions)
//------------------------------------------------------------

void loop_readTask(void *parameter)
{
    while (true)
    {
        player->_handleRead_TASK();
    }
}

void IRAM_ATTR loopPlay_ISR()
{
    player->_handlePlay_ISR();
}
