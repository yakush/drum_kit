#include "MAX_i2s_renderer.h"
#include <Arduino.h>
#include "driver/i2s.h"

using namespace NS_MAX_i2s_renderer;

//-----------------------------------------------------------------
//-- class instance
MAX_i2s_renderer_CLASS MAX_i2s_renderer;
//-----------------------------------------------------------------

MAX_i2s_renderer_CLASS::MAX_i2s_renderer_CLASS()
{
}

MAX_i2s_renderer_CLASS::~MAX_i2s_renderer_CLASS()
{
}

bool MAX_i2s_renderer_CLASS::begin(int port, int pin_bck, int pin_lrck, int pin_data, int freq)
{
    setFreq(freq);

    _port = (i2s_port_t)port;

    _i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = freq,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // high interrupt priority
        .dma_buf_count = 8,
        .dma_buf_len = 64 //Interrupt level 1
    };

    _pin_config = {
        .bck_io_num = pin_bck,    //this is BCK pin
        .ws_io_num = pin_lrck,    // this is LRCK pin
        .data_out_num = pin_data, // this is DATA output pin
        .data_in_num = -1         //Not used
    };

    bool ok = true;
    ok = ok && i2s_driver_install((i2s_port_t)_port, &_i2s_config, 0, NULL) == ESP_OK;
    ok = ok && i2s_set_pin(_port, &_pin_config) == ESP_OK;
    ok = ok && i2s_set_sample_rates(_port, freq) == ESP_OK;
    return ok;
}

void MAX_i2s_renderer_CLASS::stop()
{
    i2s_driver_uninstall(_port); //stop & destroy i2s driver
}

void MAX_i2s_renderer_CLASS::play(uint32_t L, uint32_t R)
{
    uint64_t sample = ((uint64_t)R << 32) | ((uint64_t)L & 0x0000FFFF);
    i2s_write_bytes(_port, (const char *)&sample, sizeof(uint64_t), portMAX_DELAY);
}
