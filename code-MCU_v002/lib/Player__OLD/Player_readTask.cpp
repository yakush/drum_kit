#include "Player_readTask.h"

#include "player.h"
#include <Arduino.h>

//-----------------------------------------------------------------

PlayerClass *_player_read_player;
bool _player_readTask_isRunning=false;
bool _player_read_notifiedErr_noPlayer=false;
//-----------------------------------------------------------------

void _player_readTask_taskLoop(void * parameter);

//-----------------------------------------------------------------

void player_readTask_setPlayer(PlayerClass *player){
  _player_read_player=player;
  _player_read_notifiedErr_noPlayer=false;
}

void player_readTask_start(uint32_t freq){
  player_readTask_stop();
  _player_read_notifiedErr_noPlayer=false;

  // start task (core 0):
  _player_readTask_isRunning = true;
  xTaskCreatePinnedToCore(
    _player_readTask_taskLoop,    // task func
    "_player_readTask_taskLoop",  // name
    10000,                        // stack size?!
    NULL,                         // params
    8,                            // priority
    NULL,                         // task handle
    0                             // core (0 is sencondary core)
  );
}

void player_readTask_stop(){
  _player_readTask_isRunning=false;
}

//-----------------------------------------------------------------
void _player_readTask_taskLoop(void * parameter) {

  while(_player_readTask_isRunning){

    if (_player_read_player){
      _player_read_player->_handleReadTask();
    }else{
      if ( !_player_read_notifiedErr_noPlayer){
        Serial.println("ERROR: readerTask::taskLoop() no player defined!");
        _player_read_notifiedErr_noPlayer=true;
      }
    }
  }

  vTaskDelete( NULL );
}
