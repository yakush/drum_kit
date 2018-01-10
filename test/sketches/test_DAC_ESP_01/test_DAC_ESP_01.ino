#include "audioOut.h"
#include "toneOut.h"


#include "esp32-hal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "rom/ets_sys.h"
#include "esp32-hal-matrix.h"
#include "soc/gpio_sd_reg.h"
#include "soc/gpio_sd_struct.h"

#define START_TONE 220
#define TONE_DELTA_HALF 1.059463
#define TONE_DELTA_FULL (TONE_DELTA_HALF * TONE_DELTA_HALF )
#define DELAY 1000

// -------------------------------------
void setup() {
  /*
    toneOut_init();
    toneOut_freq = START_TONE ;
    audioOut_init();
    audioOut_playSample(0x7FFF);
  */

  sigmaDeltaSetup(0, 312500); //==>8000khz
  sigmaDeltaSetup(1, 312500); //==>8000khz
  sigmaDeltaAttachPin(26, 0);
  sigmaDeltaAttachPin(25, 1);

  sigmaDeltaWrite(0, 0);
  sigmaDeltaWrite(1, 0);

}

uint32_t count;
uint16_t time;
uint16_t last_time;

int flip;
int flipMAX = 2;
void loop() {
  flip = !flip;
  //flip = (flip + 1) % flipMAX;

  //count=0;
  SIGMADELTA.channel[0].duty = flip ? 128 : 380;
  SIGMADELTA.channel[1].duty = flip ? 128 : 380;
  //sigmaDeltaWrite(0, flip  ? 0 : 255);
  //sigmaDeltaWrite(1, flip  ? 0 : 255);
  return;



  //  time = millis();
  //  if (time-last_time>1000){
  //    last_time=time;
  //    toneOut_freq *= TONE_DELTA_HALF;
  //    /*
  //    toneOut_freq *= TONE_DELTA_FULL;
  //    toneOut_freq *= TONE_DELTA_FULL;
  //    toneOut_freq *= TONE_DELTA_HALF;
  //    toneOut_freq *= TONE_DELTA_FULL;
  //    toneOut_freq *= TONE_DELTA_FULL;
  //    toneOut_freq *= TONE_DELTA_FULL;
  //    toneOut_freq *= TONE_DELTA_HALF;
  //    */
  //  }
  //
  //  if (toneOut_freq > 20000)
  //    toneOut_freq = START_TONE ;
  //

  //  while (true) {
  //    toneOut_play();
  //  }
}


