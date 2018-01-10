//MY LIBS
#include "com.h"
#include "soundRenderer.h"

#define START_TONE 220
#define TONE_DELTA_HALF 1.059463
#define TONE_DELTA_FULL (TONE_DELTA_HALF * TONE_DELTA_HALF )

int freq = 440;
uint8_t *soundWaveHi = 0;
uint8_t *soundWaveLo = 0;
int soundWaveLength = 0;
boolean playing;
boolean hiBitRange = true;
boolean playFromBuff = true;

Com com;

// -------------------------------------
void setup() {
  Serial.begin(9600);
  delay(2000);

  com.begin(comCallback, NULL);
  SoundRenderer.begin();
  SoundRenderer.play();

  createSoundBuffer(freq);
  printSoundBuffer(false, false);
  playing = true;
}

void loop() {

  com.read();

  static int pos = 0;
  static uint16_t val;
  static uint8_t hi;
  static uint8_t lo;

  hi= 0x7F;
  lo= 0x7F;
  val = 0x7F7F;
  
  if (playing && soundWaveLength > 0) {
    if (pos >= soundWaveLength)
      pos = 0;

    if (playFromBuff ) {
      hi = soundWaveHi[pos];
      lo = soundWaveLo[pos];
      val = (hi<<8) | lo;
    } else {
      val = 0x7FFF + 0x7FFF * sin(2 * PI * (float)pos / soundWaveLength);          
    }

    pos++;
  }
  
  
  //SoundRenderer.setSample(hiBitRange ? val : (val & 0x7700 | 0x7F));
  static int16_t x=0;
  static int16_t x2=0;
  x+=1;
  if (x==10){
    x=0;
    x2++;
    
  }
  SoundRenderer.setSample(x2);
}


// -------------------------------------------------------
// communacation callbacks
// -------------------------------------------------------
void comCallback(comMessage message) {
  Serial.println();
  Serial.print("MESSAGE");
  Serial.println(message.msg);

  switch (message.msg) {
    
    case COM_MSG_NOTE_ON :
      playing = true;

      if (message.freq > 0) {
        freq = message.freq;
        createSoundBuffer(freq);
      }
      Serial.print("playing ON (");
      Serial.print(freq);
      Serial.println(")");
      break;
    case COM_MSG_NOTE_OFF :
      playing = false;
      Serial.println("playing OFF");
      break;

    case COM_MSG_8_BIT:
      hiBitRange = false;
      Serial.println("8 bit");
      break;

    case COM_MSG_16_BIT:
      hiBitRange = true;
      Serial.println("16 bit");
      break;

    case COM_MSG_PLAY_BUFF:
      playFromBuff = true;
      Serial.println("play from buff");
      break;

    case COM_MSG_PLAY_CALC:
      playFromBuff = false;
      Serial.println("play from calc");
      break;

    case COM_MSG_WRITE_SPEED_HIGH :
      SoundRenderer.setHighSpeedWrite(true);
      createSoundBuffer(freq);      
      Serial.println("write speed: high");
      break;

    case COM_MSG_WRITE_SPEED_LOW:
      SoundRenderer.setHighSpeedWrite(false);
      createSoundBuffer(freq);
      Serial.println("write speed: low");
      break;

    default:
      break;
  }
}

// -------------------------------------------------------
// sound buffer
// -------------------------------------------------------

void clearSoundBuffer() {

  soundWaveLength = 0;

  if (soundWaveHi) {
    delete [] soundWaveHi;
    soundWaveHi = 0;
  }

  if (soundWaveLo) {
    delete [] soundWaveHi;
    soundWaveHi = 0;
  }
}

void createSoundBuffer(int freq) {

  clearSoundBuffer();

  int playRate = 100000;
  if (SoundRenderer.isHighSpeedWrite())
    playRate = 800000;
  else
    playRate = 100000;

  soundWaveLength = playRate / freq;
  soundWaveHi = new uint8_t [soundWaveLength]; // to delete : delete [] soundWave;
  soundWaveLo = new uint8_t [soundWaveLength]; // to delete : delete [] soundWave;

  uint16_t val = 0;

  for (int i = 0; i < soundWaveLength; i++) {
    val = 0x7FFF + 0x7FFF * sin(2 * PI * (float)i / soundWaveLength);
    //val = 0xFFFF * ((float)i/ soundWaveLength) ;

    soundWaveHi[i] = (uint8_t )((val & 0xFF00) >> 8);
    soundWaveLo[i] = (uint8_t )((val & 0x00FF) >> 0);
  }
}

void printHex16(long x) {
  if (x <= 0x0FFF)  Serial.print("0");
  if (x <= 0x00FF)  Serial.print("0");
  if (x <= 0x000F)  Serial.print("0");
  Serial.print(x, HEX);
}

void printHex8(int x) {
  if (x <= 0x0F)  Serial.print("0");
  Serial.print(x, HEX);
}

void printSoundBuffer(boolean full , boolean graph ) {
  //-- print hex array:
  if (full) {
    for (int i = 0; i < soundWaveLength; i++) {
      uint16_t val = 256 * soundWaveLo[i] + soundWaveHi[i];
      //printHex16(val);
      //Serial.print(" : ");

      printHex8(soundWaveHi[i]);
      Serial.print(" | ");
      printHex8(soundWaveLo[i]);
      Serial.println("");

      //graph
      if (graph) {
        for (int j = 0; j < 10; j++) {
          Serial.print(val > j * 0xffff / 10 ? "#" : " ");
        }
        Serial.println("");
      }
    }
  }

  //summery
  Serial.print("\n\n----------------\ntotal: \n");
  Serial.println(soundWaveLength);
}

