
#include "sound.h"

#define PWM_PIN_HI 26
#define PWM_PIN_LO 25

#define PWM_CHANNEL_HI 0
#define PWM_CHANNEL_LO 2

#define PWM_FREQ = 5000;
#define PWM_RESOLUTION = 16;

#define PLAY_FREQ 44100

boolean playing = false;

int freq = 10;
int samplesPerCycle;
long x = 0;
long xx = 0;
uint32_t xxx = 0;

volatile int interruptCounter;
int totalInterruptCounter;

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer();

void setup() {
  Serial.begin(9600);

  //-- init tone
  freq = 440; // "C" note
  samplesPerCycle = PLAY_FREQ / freq;

  // -- setup interrupts and timers (timer2)
  initTimerInterrupts();

  //ledcSetup(uint8_t channel, double freq, uint8_t resolution_bits);
  ledcSetup(PWM_CHANNEL_HI, 262143, 8);
  ledcSetup(PWM_CHANNEL_LO, 262143, 8);
  ledcAttachPin(PWM_PIN_HI, PWM_CHANNEL_HI);
  ledcAttachPin(PWM_PIN_LO, PWM_CHANNEL_LO);
  ledcWrite(PWM_CHANNEL_HI, 0 );
  ledcWrite(PWM_CHANNEL_LO, 0 );

  playing = true;
}

void loop() {
static int flip=0;
flip= (flip +1 )%2;
if (flip==0) playByte(0xffff);
if (flip==1) playByte(0);


return;
  if (playing) {
    // -- TONE
    //char val=(127 + 127 * sin(2.0 * PI * (float)x / (float)samplesPerCycle));
    //char val=x;
    //playByte(val);
    //playByte((char)(x));


    // -- TONE
    //char val=(127 + 127 * sin(2.0 * PI * (float)x / (float)samplesPerCycle));
    //char val=x;
    //playByte(val);
    //playByte((char)(x));

    static uint32_t lastXXX;
    static uint64_t prevVal;
    static uint64_t nextVal;
    static short counting = 1 ;
    static short lastCount = 1 ;
    
    if (xxx != lastXXX) {
      lastXXX = xxx;

      lastCount = counting;
      counting = 1;

      prevVal = nextVal;
      //nextVal = (0x7fff / 2 * (1 + sin(3.14 * 2.0 * freq * lastXXX /  44199)));
      nextVal =0xffff*(( micros()%4)/4.0);
      //nextVal = 
      playByte(nextVal);
    } else {
      playByte(nextVal);
      counting ++;
    }
    
    //interpolate:
    //if (lastCount ==0)lastCount =1;
    //playByte(prevVal+ (nextVal-prevVal)*counting/2);

    //playByte(512 + 512 * sin(3.14 * 2.0 * freq * xxx /  44199));

    // -- h. file
    //playByte(sound[xx]);
    //playBytePWM(sound[xx]);
    //playBytePWM(0);

    // -- SD
    //playByte(sdVal);
  }

}


void initTimerInterrupts() {
  /* Use 1st timer of 4 */
  /* 1 tick take 1/(80MHZ/80) = 1us so we set divider 80 and count up */
  timer = timerBegin(0, 10, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 181, true);
  timerAlarmEnable(timer);
}

void IRAM_ATTR onTimer() {

  if (!playing)
    return;

  //from tone generator
  x++;
  if (x >= samplesPerCycle)
    x = 0;

  //from .h file
  xx++;
  if (xx >= sizeof(sound))
    xx = 0;

  xxx++;
  if (xxx >= 44199) {
    xxx = 0;
    freq = (freq * 1.059463);
    if (freq > 20000)
      freq = 20;
  }



  //from SD
  /*
    if (dataFile) {
    if (dataFile.available()) {
      sdVal = dataFile.read();
    } else {
      dataFile.seek(0);
    }
    }
  */

}

//////////////////////////////////////

void playByte(uint32_t val) {
  //analogWrite(PIN_PWM, (val >> 0));
  ledcWrite(PWM_CHANNEL_HI, val >> 8);
  ledcWrite(PWM_CHANNEL_LO, val);

}

/****************
  void playBytePWM(byte val) {
  analogWrite(PIN_PWM, (int)val);
  }
*****************/
//////////////////////////////////////

