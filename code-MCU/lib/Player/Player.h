#ifndef Player_H_
#define Player_H_

#include "SampleInfo.h"
#include "Sample.h"
#include "ISoundRenderer.h"

#define PLAYER_MAX_PLAYING_SAMPLES 20

namespace NS_Player
{

typedef void (*PlayerCallback_play_ended_t)(uint32_t sampleHandle, uint8_t reason);

typedef struct SampleWrapper_s
{
  long handle;
  Sample *sample;
  PlayerCallback_play_ended_t callback;

} SampleWrapper_t;

class Player_CLASS
{

private:
  portMUX_TYPE samplesMutex = portMUX_INITIALIZER_UNLOCKED;

public:
  Player_CLASS();
  virtual ~Player_CLASS();

  SampleWrapper_t *samples; //array
  ISoundRenderer *soundRenderer;
  uint16_t volume;

  bool begin(ISoundRenderer *soundRenderer);
  void end();

  int LoadSample(String filepath, SampleInfo_t *sampleInfo);

  long play(SampleInfo_t sampleInfo, uint8_t volume = 128, uint8_t blendLR = 128, PlayerCallback_play_ended_t = NULL);
  void stop(long sampleHandle);
  void stopAll();

  //helpers
  void inline lockSamples_TASK() { portENTER_CRITICAL(&samplesMutex); };
  void inline unlockSamples_TASK() { portEXIT_CRITICAL(&samplesMutex); };
  void inline lockSamples_ISR() { portENTER_CRITICAL_ISR(&samplesMutex); };
  void inline unlockSamples_ISR() { portEXIT_CRITICAL_ISR(&samplesMutex); };

}; //Player_CLASS

} //namespace NS_Player

//-----------------------------------------------------------------
// global :
using NS_Player::Player_CLASS;
extern Player_CLASS Player;
//-----------------------------------------------------------------

#endif //Player_H_