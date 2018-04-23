#ifndef PLAYER_PLAY_READ_H_
#define PLAYER_PLAY_READ_H_

#include "player.h"

void player_readTask_setPlayer(PlayerClass *player);
void player_readTask_start(uint32_t freq=96000);
void player_readTask_stop();

#endif