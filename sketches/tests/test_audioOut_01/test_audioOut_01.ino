#include <SD.h>

#include "sound.h"

#define PIN_SD 10
#define PIN_PWM 5

#define PORT_RW_1 DDRD
#define PORT_PIN_MASK_1 B11111100
#define PORT_DATA_1 PORTD

#define PORT_RW_2 DDRB
#define PORT_PIN_MASK_2 B00000011
#define PORT_DATA_2 PORTB

#define PLAY_FREQ 44100

boolean playing = false;
int freq;
int samplesPerCycle;
long x = 0;
long xx = 0;

File dataFile;
char sdVal;

void setup() {
  Serial.begin(9600);



  //-- init tone
  freq = 440; // "C" note
  samplesPerCycle = PLAY_FREQ / freq;
  //samplesPerCycle = 256;

  // -- setup I/O pins
  PORT_RW_1 |= PORT_PIN_MASK_1;
  PORT_RW_2 |= PORT_PIN_MASK_2;

  // -- setup SD
  Serial.print("Initializing SD card...");
  if (SD.begin(PIN_SD)) {
    Serial.println("card initialized.");
  } else {
    Serial.println("Card failed, or not present");
    return;
  }

  delay(999);

  dataFile = SD.open("zemer_8.raw");
  if (dataFile) {
    Serial.println("openning file");
  } else {
    Serial.println("File failed, or not present");
    return;
  }

  //dataFile.close();

  // -- setup interrupts and timers (timer2)
  initTimerInterrupts();

  playing = true;

  /**********************
     //-- PWM SETUP
    // https://playground.arduino.cc/Main/TimerPWMCheatsheet
    // set clock prescaler:
    // 0x01 = 1 = 62,500 KHz
    // will affect millis() and delay()
    pinMode(PIN_PWM,OUTPUT);
    TCCR0B = (TCCR0B & 0b11111000) | 0x01;
    analogWrite(PIN_PWM,128);
  *******************************/
}

void loop() {
  //check btns for stop|play request

  //a. play sin wave

  if (playing) {
    // -- TONE
    //char val=(127 + 127 * sin(2.0 * PI * (float)x / (float)samplesPerCycle));
    //char val=x;
    //playByte(val);
    //playByte((char)(x));

    // -- h. file
    playByte(sound[xx]);
    //playBytePWM(sound[xx]);
    //playBytePWM(0);

    // -- SD
    //playByte(sdVal);
  }

}

void initTimerInterrupts() {
  //set timer2 interrupt at ~44kHz

  noInterrupts();
  //reset timer 2
  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0

  // turn on CTC mode
  TCCR2A |= (1 << WGM21);

  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);

  // Set CS21 bit for 8 prescaler
  TCCR2B |= (1 << CS21);

  // set compare match register for 44.1khz increments
  OCR2A = 45;// = (16*10^6) / (44000*8) - 1 (must be <256)
  //OCR2A = 249;// = (16*10^6) / (44000*8) - 1 (must be <256)

  interrupts();
}

//Interrupt at freq of 1kHz to measure reed switch
ISR(TIMER2_COMPA_vect) {

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
#define SET_PORT_VAL(port, mask, val) port = (port&~mask) | (val&mask)


void playByte(byte val) {
  SET_PORT_VAL(PORT_DATA_1, PORT_PIN_MASK_1, val >> 2);
  SET_PORT_VAL(PORT_DATA_2, PORT_PIN_MASK_2, val << 6);
}

/****************
void playBytePWM(byte val) {
  analogWrite(PIN_PWM, (int)val);
}
*****************/
//////////////////////////////////////

