#ifndef PLAYER_H_
#define PLAYER_H_

#include <Arduino.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

namespace NS_player
{

  //-----------------------------------------------------------------
  //-----------------------------------------------------------------
  #define PLAYER_SAMPLE_RATE 44100
  #define PLAYER_SAMPLE_T_USEC 23
  #define PLAYER_PLAY_RATE 96000
  #define PLAYER_PLAY_T_USEC 10

  #define PLAYER_FILE_16_BIT_44100  ".H16"
  #define PLAYER_FILE_8_BIT_44100   ".H08"
  #define PLAYER_FILE_8_BIT_22050   ".L08"

  #define PLAYER_SAMPLE_16_BIT_44100  0
  #define PLAYER_SAMPLE_8_BIT_44100   1
  #define PLAYER_SAMPLE_8_BIT_22050   2

  #define PLAYER_BUFFER_SIZE 512
  #define PLAYER_MAX_LOADED_SAMPLES 10
  #define PLAYER_MAX_PLAYING_SAMPLES 10
  //-----------------------------------------------------------------

  //play callback
  #define PLAYER_SAMPLE_ENDED_FINISHED      1
  #define PLAYER_SAMPLE_ENDED_STOPPED       2
  #define PLAYER_SAMPLE_ENDED_PRUNED        3

  typedef void (*PlayerCallback_play_ended_t)(uint32_t sampleHandle,uint8_t reason);

  //-----------------------------------------------------------------
  typedef struct {
    uint8_t volume=128;
    unsigned long sampleHandle;
    PlayerCallback_play_ended_t cbEnd=NULL;

    File file;
    size_t fileSize = 0;
    size_t pos = 0;

    byte buffer [2][PLAYER_BUFFER_SIZE];
    short activeBuffer = 0;
    short bufferPos = 0;
    bool needFillBuffer = false;
    bool firstBufferReady = false;

    bool isPlaying = false;
    bool isReadingFile=false;
    bool isStopping=false;

    int16_t val_0 = 0;
    int16_t val_1 = 0;
    uint8_t quality = PLAYER_SAMPLE_16_BIT_44100;
  } Sample;


  typedef struct {
    bool loaded=false;
    char filename[256];
    // optional - initial buffer ?
    // carefull if implemented varying resolution options ,
    // may need to load multiple buffers (for each resolution) ... :(
  } LoadedSample;

  //-----------------------------------------------------------------
  //-----------------------------------------------------------------
  class PlayerClass {

  private:

    bool started=false;
    unsigned long lastSampleHandle=0;
    unsigned long lastReadTaskYeildTime=0;
    unsigned long lastTimeRead=0;

    portMUX_TYPE samplesMutex = portMUX_INITIALIZER_UNLOCKED;

    LoadedSample loadedSamples[PLAYER_MAX_LOADED_SAMPLES];
    Sample playingSamples[PLAYER_MAX_PLAYING_SAMPLES];

    //helpers
    void inline lockSamples(){ portENTER_CRITICAL(&samplesMutex);};
    void inline unlockSamples(){ portEXIT_CRITICAL(&samplesMutex);};

    void inline lockSamples_ISR(){ portENTER_CRITICAL_ISR(&samplesMutex);};
    void inline unlockSamples_ISR(){ portEXIT_CRITICAL_ISR(&samplesMutex);};

    void stopSampleByIdx(uint8_t sampleIdx,uint8_t reason);
    static char* getFilePath(char* dest ,const char* path, uint8_t quality);
    static void resetSample(Sample* s);

  public:
    PlayerClass ();
    virtual ~PlayerClass ();

    void begin();
    void end();

    bool load(uint8_t sampleId,const char* path);
    void unload(uint8_t sampleId);
    void unloadAll();

    /**
    * @param volume 0-255, 128 is 100%
    */
    long play(uint8_t sampleId,uint8_t volume=128, PlayerCallback_play_ended_t=NULL);
    void stop(long sampleHandle);
    void stopAll();

    long* getPlayingSamplesHandles();

    void _handlePlayTimerTick();
    void _handleReadTask();
  };


} //namespace

//-----------------------------------------------------------------
// global usings:
using NS_player::PlayerClass;
using NS_player::PlayerCallback_play_ended_t;

extern PlayerClass Player;
//-----------------------------------------------------------------

#endif