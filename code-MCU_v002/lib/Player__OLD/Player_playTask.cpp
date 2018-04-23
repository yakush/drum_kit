#include "Player_playTask.h"

#include <Arduino.h>
#include "player.h"

//-----------------------------------------------------------------

PlayerClass *_player_play_player;
hw_timer_t * _player_play_playTimer = NULL;
bool _player_play_notifiedErr_noPlayer=false;

//-----------------------------------------------------------------

void IRAM_ATTR _player_read_onPlayTimer();

//-----------------------------------------------------------------

void player_playTask_setPlayer(PlayerClass *player){  
  _player_play_player=player;
  _player_play_notifiedErr_noPlayer=false;
}

void player_playTask_start(uint32_t freq){
  player_playTask_stop();

  _player_play_notifiedErr_noPlayer=false;

  // 1 tick take 1/(80MHZ/divider) . i.e 80 -> 1us
  _player_play_playTimer = timerBegin(0, 2, true);
  timerAttachInterrupt(_player_play_playTimer, &_player_read_onPlayTimer, true);
  timerAlarmWrite(_player_play_playTimer, 40000000 / freq, true); // 20 uSecs = 96khz
  timerAlarmEnable(_player_play_playTimer);
}

void player_playTask_stop(){
  if (!_player_play_playTimer){
    return;
  }
  timerDetachInterrupt(_player_play_playTimer);
  timerEnd(_player_play_playTimer);
  _player_play_playTimer=NULL;
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------

void IRAM_ATTR _player_read_onPlayTimer() {
  if (_player_play_player){
    _player_play_player->_handlePlayTimerTick();
  }else{
    if ( !_player_play_notifiedErr_noPlayer){
      Serial.println("ERROR: playerTimer::onPlayTimer() no player defined!");
      _player_play_notifiedErr_noPlayer=true;
    }
  }
}
