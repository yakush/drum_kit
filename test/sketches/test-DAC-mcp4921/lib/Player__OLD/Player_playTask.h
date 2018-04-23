#ifndef PLAYER_PLAY_TASK_H_
#define PLAYER_PLAY_TASK_H_

#include "player.h"

void player_playTask_setPlayer(PlayerClass *player);
void player_playTask_start(uint32_t freq=96000);
void player_playTask_stop();

#endif