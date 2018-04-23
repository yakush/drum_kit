#ifndef I2S_PLAYER_H_
#define I2S_PLAYER_H_

#include "driver/i2s.h"

namespace NS_player
{

class PlayerClass
{

private:
  i2s_port_t _port;
  i2s_config_t _config;
  int _rate;

public:
  PlayerClass();
  virtual ~PlayerClass();

  bool begin(int port, i2s_config_t config, int pin_bck, int pin_lrck, int pin_data, int rate);
  void stop();

  int writeSample(uint32_t sample, int ticksToWait);
  int write(uint32_t *buffer, int size, int ticksToWait);
};
}

//-----------------------------------------------------------------
// expose class instance:
using NS_player::PlayerClass;
extern PlayerClass I2S_Player;
//-----------------------------------------------------------------

#endif
