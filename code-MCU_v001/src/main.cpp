#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include "config.h"
#include "SoundRenderer.h"
#include "Player.h"
#include "Logger.h"
#include "Button.h"

#include "ff.h"

//-----------------------------------------------------------------

void callback_ended(uint32_t sampleHandle,uint8_t reason);

void loadSamples();
void test1();
void test2();
void btnsListener(uint8_t id, bool newState);
//-----------------------------------------------------------------
#define BTN_01    1
#define BTN_02    2
#define BTN_03    3
#define BTN_04    4
#define BTN_05    5
#define BTN_06    6
#define BTN_07    7
#define BTN_08    8
#define BTN_L     9
#define BTN_R     10

Button btn1(BTN_01,   4);
Button btn2(BTN_02,  34);
Button btn3(BTN_03,  35);
Button btn4(BTN_04,  32);
Button btn5(BTN_05,  25);
Button btn6(BTN_06,  33);
Button btn7(BTN_07,  27);
Button btn8(BTN_08,  26);
Button btnL(BTN_L,    2);
Button btnR(BTN_R,   14);

//-----------------------------------------------------------------

void setup() {

  btn1.setListeners(btnsListener);
  btn2.setListeners(btnsListener);
  btn3.setListeners(btnsListener);
  btn4.setListeners(btnsListener);
  btn5.setListeners(btnsListener);
  btn6.setListeners(btnsListener);
  btn7.setListeners(btnsListener);
  btn8.setListeners(btnsListener);
  btnL.setListeners(btnsListener);
  btnR.setListeners(btnsListener);


  Logger.setLogLevel(LOG_LEVEL_DEBUG);

  Serial.begin(115200);
  delay(1000);

  Serial.printf("FF SIZE : %u\n",sizeof(FIL));

  Serial.println("staring SD...");
  if(!SD.begin(SS, SPI, 20000000)){
    Serial.println("SD Card Mount Failed");
    return;
  };
  Serial.println("staring SoundRenderer...");
  SoundRenderer.begin(12,13);

  Serial.println("staring Player...");
  Player.begin();

  loadSamples();
  //test1();
  //test2();
  // test2();
  // delay(2000);
  // test2();
  // delay(2000);

  //Player.end();
  //Player.stopAll();

}

//-----------------------------------------------------------------
void loop() {
  btn1.update();
  btn2.update();
  btn3.update();
  btn4.update();
  btn5.update();
  btn6.update();
  btn7.update();
  btn8.update();
  btnL.update();
  btnR.update();

  vTaskDelay(1);
}

//-----------------------------------------------------------------
void callback_ended(uint32_t sampleHandle,uint8_t reason){
  Serial.printf("stopped sample %u\n", sampleHandle);
  Serial.println("");

}


//-----------------------------------------------------------------
void btnsListener(uint8_t id, bool newState){
  if (newState){

    switch (id) {

      case BTN_01:
      case BTN_02:
      case BTN_03:
      case BTN_04:
      case BTN_05:
      case BTN_06:
      case BTN_07:
      case BTN_08:
      Serial.printf("PRESSED %u\n",id);
      Player.play(id,128, &callback_ended);
      break;

      case BTN_L :
      Serial.println("PRESSED L (song)");
      Player.play(0,128, &callback_ended);
      break;

      case BTN_R :
      Serial.println("PRESSED R (stop all)");
      Player.stopAll();
      break;
    }
  }
}

//-----------------------------------------------------------------
bool loadSingleSample(uint8_t slot, char *path){
  if (!Player.load(slot, path)){
    Serial.printf("unable to load sample %s\n", path);
    return false;
  }
  return true;
}

void loadSamples(){
  char kit1_1[] = "/drumkit/kit1/1";
  char kit1_2[] = "/drumkit/kit1/2";
  char kit1_3[] = "/drumkit/kit1/3";
  char kit1_4[] = "/drumkit/kit1/4";
  char kit1_5[] = "/drumkit/kit1/5";
  char kit1_6[] = "/drumkit/kit1/6";
  char kit1_7[] = "/drumkit/kit1/7";
  char kit1_8[] = "/drumkit/kit1/8";

  char sampleSong[] = "/stromae";

  Serial.println("loading samples...");

  loadSingleSample(0,sampleSong);
  loadSingleSample(1,kit1_1);
  loadSingleSample(2,kit1_2);
  loadSingleSample(3,kit1_3);
  loadSingleSample(4,kit1_4);
  loadSingleSample(5,kit1_5);
  loadSingleSample(6,kit1_6);
  loadSingleSample(7,kit1_7);
  loadSingleSample(8,kit1_8);

  Serial.println("done");
}

//-----------------------------------------------------------------

void test2(){
  //char samplePath[] = "/stromae";
  //char samplePath[] = "/short";
  char samplePath0[] = "/stromae";
  char samplePath1[] = "/stromae";

  //0
  Serial.println("loading sample 0 ...");
  if (!Player.load(0, samplePath0)){
    Serial.printf("unable to load sample %s\n", samplePath0);
    return;
  }
  Serial.println("loaded 0");

  //1
  Serial.println("loading sample 1...");
  if (!Player.load(1, samplePath1)){
    Serial.printf("unable to load sample %s\n", samplePath1);
    return;
  }
  Serial.println("loaded 1");

  long handle0 = Player.play(0,128, &callback_ended);
  delay(1000);
  long handle1 = Player.play(1, 128, &callback_ended);
  /*
  for (size_t i = 0; i < 1; i++) {

  Serial.println("playing sample...");
  long handle1 = Player.play(0, &callback_ended);
  if (!handle1){
  Serial.printf("unable to play sample %u\n", 0);
  return;
}
Serial.printf("playing sample , handle=%u\n",handle1);
delay(10000);
}

*/

}
//-----------------------------------------------------------------
void test1(){
  char samplePath[] = "/drumkit/samples/0/test";
  Serial.println("loading sample...");
  if (!Player.load(0, samplePath)){
    Serial.printf("unable to load sample %s\n", samplePath);
    return;
  }
  Serial.println("loaded");

  Serial.println("playing sample...");
  long handle1 = Player.play(0,128, &callback_ended);
  if (!handle1){
    Serial.printf("unable to play sample %u\n", 0);
    return;
  }
  Serial.printf("playing sample , handle=%u\n",handle1);

  delay(3000);

  Serial.println("playing sample again...");
  long handle2 = Player.play(0, 128, &callback_ended);
  if (!handle2){
    Serial.printf("unable to play sample %u\n", 0);
    return;
  }
  Serial.printf("playing sample , handle=%u\n",handle2);

  delay(3000);
}