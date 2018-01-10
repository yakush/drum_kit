#ifndef TONE_OUT_H_
#define TONE_OUT_H_

#include "audioOut.h"

int toneOut_freq;
long toneOut_x;
int toneOut_playing = false;
int playMult = 10;

hw_timer_t* toneOut_timer = NULL;
void IRAM_ATTR toneOut_onTimer();

void toneOut_init() {
  //Use timer 1 (of 4)
  // tick = (80MHZ/x)
  // count to = 181 (~44000 hz)
  
//  int countTo = 181;
//  toneOut_timer = timerBegin(0, 10, true);
//  timerAttachInterrupt(toneOut_timer, &toneOut_onTimer, true);
//  timerAlarmWrite(toneOut_timer, countTo, true);
//  timerAlarmEnable(toneOut_timer);

  toneOut_playing = true;
}

void IRAM_ATTR toneOut_onTimer() {

  if (!toneOut_playing) {
    return;
  }

  toneOut_x++;
  //  if (toneOut_x >= 44199) {
  //    toneOut_x = 0;
  //  }

  //audioOut_playSample(0x7FFF);
}

void toneOut_play() {
  
  static long lastX = 0;
  static uint32_t lastVal = 0;
  static uint32_t currentVal = 0;
  static int count = 1;
  static boolean flip1=1;
  static boolean flip2=1;
  
  //sawtooth
  lastX++;
  audioOut_playSample(  toneOut_freq * lastX*30);
  return;
  if (lastX != toneOut_x) {
    lastX = toneOut_x;

    flip1= !flip1;
    //count = 0;    
    lastVal = currentVal;
    //currentVal = toneOut_freq * toneOut_x    ;
    currentVal = 7000* toneOut_x    ;
  }
  flip2= !flip2;
  //audioOut_playSample(flip1 ? 0xFFFF *flip2: 0x7FFF *flip2);
  //audioOut_playSample(flip2 ? 0xFFFF : 0);
  
  audioOut_playSample(  20000 * toneOut_x);
  count++;
  
  /*

    if (lastX != toneOut_x) {
      lastX = toneOut_x;

      count = 0;
      lastVal = currentVal;
      currentVal = (0x7FFF + 0x7FFF * sin(
                      3.14 * 2.0 * toneOut_freq * toneOut_x
                      /  44199));
    }

    //interpolate
    audioOut_playSample( lastVal + (currentVal - lastVal) * count / playMult );
    count++;
  */
}
#endif
