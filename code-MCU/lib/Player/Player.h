#ifndef Player_H_
#define Player_H_

#include "ClipInfo.h"
#include "Clip.h"
#include "ISoundRenderer.h"

#define PLAYER_MAX_PLAYING_CLIPS 20

namespace NS_Player
{

typedef void (*PlayerCallback_play_ended_t)(uint32_t sampleHandle, uint8_t reason);

typedef struct ClipWrapper_s
{
  long handle;
  Clip *clip;
  PlayerCallback_play_ended_t callback;

} ClipWrapper_t;

class Player_CLASS
{

private:
  portMUX_TYPE clipsMutex = portMUX_INITIALIZER_UNLOCKED;

public:
  Player_CLASS();
  virtual ~Player_CLASS();

  ClipWrapper_t *clips; //array
  ISoundRenderer *soundRenderer;
  uint16_t volume;

  bool begin(ISoundRenderer *soundRenderer);
  void end();

  int LoadClip(String filepath, ClipInfo_t *clipInfo);

  long play(ClipInfo_t clipInfo, uint8_t volume = 128, uint8_t blendLR = 128, PlayerCallback_play_ended_t = NULL);
  void stop(long clipHandle);
  void stopAll();

  //helpers
  void inline lockClips_TASK() { portENTER_CRITICAL(&clipsMutex); };
  void inline unlockClips_TASK() { portEXIT_CRITICAL(&clipsMutex); };
  void inline lockClips_ISR() { portENTER_CRITICAL_ISR(&clipsMutex); };
  void inline unlockClips_ISR() { portEXIT_CRITICAL_ISR(&clipsMutex); };

}; //Player_CLASS

} //namespace NS_Player

//-----------------------------------------------------------------
// global :
using NS_Player::Player_CLASS;
extern Player_CLASS Player;
//-----------------------------------------------------------------

#endif //Player_H_