#include <Arduino.h>
#include "I2S_Player.h"
#include "driver/i2s.h"

using namespace NS_player;

//-----------------------------------------------------------------
//-- class instance
PlayerClass I2S_Player;
//-----------------------------------------------------------------

PlayerClass::PlayerClass()
{
}

PlayerClass::~PlayerClass()
{
}

bool PlayerClass::begin(int port, i2s_config_t config, int pin_bck, int pin_lrck, int pin_data, int rate)
{
    _port = (i2s_port_t)port;
    _config = config;
    _rate = rate;

    i2s_pin_config_t pin_config = {
        .bck_io_num = pin_bck,    //this is BCK pin
        .ws_io_num = pin_lrck,    // this is LRCK pin
        .data_out_num = pin_data, // this is DATA output pin
        .data_in_num = -1         //Not used
    };
    bool ok = true;
    ok = ok && i2s_driver_install((i2s_port_t)_port, &_config, 0, NULL) == ESP_OK;
    ok = ok && i2s_set_pin(_port, &pin_config) == ESP_OK;
    ok = ok && i2s_set_sample_rates(_port, rate) == ESP_OK;
    return ok;
}

void PlayerClass::stop()
{
    i2s_driver_uninstall(_port); //stop & destroy i2s driver
}

int PlayerClass::writeSample(uint32_t sample, int ticksToWait)
{
    return i2s_write_bytes(_port, (const char *)&sample, sizeof(uint32_t), ticksToWait);
}

int PlayerClass::write(uint32_t *buffer, int size, int ticksToWait)
{
    return i2s_write_bytes(_port, (const char *)&buffer, size * sizeof(uint32_t), ticksToWait);
}
