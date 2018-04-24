#ifndef Player_H_
#define Player_H_

#include "ClipInfo.h"
#include "Clip.h"
#include "ISoundRenderer.h"
#include "ClipEndCallback.h"

#define PLAYER_MAX_PLAYING_CLIPS 20
#define PLAYER_CLIP_BUFFER_SIZE 512

namespace NS_Player
{

typedef struct ClipWrapper_s
{
  long handle;
  Clip *clip;
  PlayerCallback_play_ended_t callback;

} ClipWrapper_t;

class Player_CLASS
{

private:
  bool started = false;
  long lastSampleHandle = 0;

  ClipWrapper_t *clips; //array
  ISoundRenderer *soundRenderer;

  void stopClipByIdx(uint8_t clipIdx, clipEndedReason_t reason);

  //tasks
  hw_timer_t *playTimer = NULL;
  bool isRunning_readTask = false;
  bool isRunning_playISR = false;
  unsigned long lastReadTaskYieldTime = 0;
  void start_readTask();
  void start_playISR(uint32_t freq);
  void stop_readTask();
  void stop_playISR();

  //locks
  portMUX_TYPE clipsMutex = portMUX_INITIALIZER_UNLOCKED;
  void inline lockClips_TASK() { portENTER_CRITICAL(&clipsMutex); };
  void inline unlockClips_TASK() { portEXIT_CRITICAL(&clipsMutex); };
  void inline lockClips_ISR() { portENTER_CRITICAL_ISR(&clipsMutex); };
  void inline unlockClips_ISR() { portEXIT_CRITICAL_ISR(&clipsMutex); };

public:
  Player_CLASS();
  virtual ~Player_CLASS();

  //task runners
  void _handleRead_TASK();
  void _handlePlay_ISR();

  uint16_t volume = 0x7FFF;

  bool begin(ISoundRenderer *soundRenderer);
  void end();

  wave_err_t LoadClip(String filepath, ClipInfo_t *clipInfo);

  long playClip(ClipInfo_t clipInfo, uint8_t volume = 128, uint8_t blendLR = 128, PlayerCallback_play_ended_t = NULL);
  void stopClip(long clipHandle);
  void stopAllClips();

  String getClipEndReason(clipEndedReason_t reason);

}; //Player_CLASS

} //namespace NS_Player

//-----------------------------------------------------------------
// global :
using NS_Player::Player_CLASS;
extern Player_CLASS Player;
//-----------------------------------------------------------------

#endif //Player_H_