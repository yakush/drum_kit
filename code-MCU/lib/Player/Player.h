#ifndef PLAYER_H_
#define PLAYER_H_

#include <Arduino.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

namespace NS_player
{

}

//-----------------------------------------------------------------
// global instance:
using NS_player::PlayerClass;
using NS_player::PlayerCallback_play_ended_t;
extern PlayerClass Player;
//-----------------------------------------------------------------


#endif