#ifndef SOUND_RENDERER_H_
#define SOUND_RENDERER_H_

//ESP LIBS
#include "esp32-hal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "rom/ets_sys.h"
#include "esp32-hal-matrix.h"
#include "soc/gpio_sd_reg.h"
#include "soc/gpio_sd_struct.h"

typedef struct {
  int timerNumber = 0;
  int pinHi = 26;
  int pinLo = 25;
  int channelHi = 0;
  int channelLo = 1;
  boolean highSpeedWrite = false;
} SoundRendererConfig;



//timer stuff
portMUX_TYPE _SoundRenderer_timerMux = portMUX_INITIALIZER_UNLOCKED;
void IRAM_ATTR _SoundRenderer_onTimer ();

//class
class SoundRendererClass {
  public:

    void begin() {
      SoundRendererConfig config;
      begin(config);
    }

    void begin(SoundRendererConfig config) {
      //-- init sigma delta DAC
      _config = config;

      sigmaDeltaSetup(config.channelHi, 312500); //==>20 MHz
      sigmaDeltaSetup(config.channelLo, 312500); //==>20 MHz
      sigmaDeltaAttachPin(config.pinHi, config.channelHi);
      sigmaDeltaAttachPin(config.pinLo, config.channelLo);

      sigmaDeltaWrite(config.channelHi, 0);
      sigmaDeltaWrite(config.channelLo, 0);
    }

    void play() {
      // play at 96 KHz = 80,000,000 / 833
      // so divider =1 and count 833
      _timer = timerBegin(_config.timerNumber, 100, true);
      timerAttachInterrupt(_timer, &_SoundRenderer_onTimer, true);

      timerAlarmWrite(_timer, 4, true);
      timerAlarmEnable(_timer);
    }

    void stop() {
      if (_timer) {
        timerEnd(_timer);
        _timer = NULL;
      }
    }

    void setSample(uint16_t sample) {
      portENTER_CRITICAL_ISR(&_SoundRenderer_timerMux);
      _sample = sample;
      portEXIT_CRITICAL_ISR(&_SoundRenderer_timerMux);
    }

    uint16_t getLastSample(){       
      return _sample;      
    }

    void setHighSpeedWrite(boolean val) {
      _config.highSpeedWrite = val;
    }
    boolean isHighSpeedWrite() {
      return _config.highSpeedWrite;
    }

    void playSample_2x8(uint8_t hi, uint8_t lo) {

      if (_config.highSpeedWrite) {
        //HIGH SPEED
        SIGMADELTA.channel[_config.channelHi].duty = 128 + hi; //128-380 (255 range)
        SIGMADELTA.channel[_config.channelLo].duty = 128 + lo; //128-380 (255 range)
      } else {
        //LOW SPEED
        sigmaDeltaWrite(_config.channelHi, hi);
        sigmaDeltaWrite(_config.channelLo, lo);
      }
    }

    void playSample_16(uint16_t value) {
      playSample_2x8(
        (uint8_t )(value/256),
        (uint8_t )((value))
      );
    }

  private:
    hw_timer_t * _timer ;
    SoundRendererConfig _config;
    uint16_t _sample;
};

// -------------------------------------------------

//define a member
SoundRendererClass SoundRenderer;

// -------------------------------------------------
// timer stuff


void IRAM_ATTR _SoundRenderer_onTimer () {
  static bool flip;
  static uint8_t lo;
   static uint8_t hi;
   lo++;
   if (lo==0) hi++;
   flip=!flip;
  portENTER_CRITICAL_ISR(&_SoundRenderer_timerMux);
//  SoundRenderer.playSample_16(SoundRenderer.getLastSample());
  SoundRenderer.playSample_16(
      flip?0xffff:0
    );
  portEXIT_CRITICAL_ISR(&_SoundRenderer_timerMux);
}
#endif
