#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include "config.h"
#include "SoundRenderer.h"
#include "Player.h"
#include "Logger.h"
#include "Button.h"
#include "WavFile.h"
#include "ff.h"

void setup()
{

  Serial.begin(115200);
  delay(1000);

  Serial.println("staring SD...");
  if (!SD.begin(SS, SPI, 20000000))
  {
    Serial.println("SD Card Mount Failed");
    return;
  };

  wavProps_t wavProps;
  int res = getWaveProps("/T.wav", &wavProps);
  if (res != WAV_OK)
  {
    Serial.println("WAV FILE READ Failed : " + getWaveError(res));
    return;
  }

  Serial.printf("sampleRate : %u\n", wavProps.sampleRate);
  Serial.printf("sampleTimeUSec : %u\n", wavProps.sampleTimeUSec);
  Serial.printf("numChannels : %u\n", wavProps.numChannels);
  Serial.printf("bitsPerSample : %u\n", wavProps.bitsPerSample);
  Serial.printf("dataStart : %u\n", wavProps.dataStart);
  Serial.printf("dataLength : %u\n", wavProps.dataLength);

  /*
    uint32_t sampleRate;
    uint16_t channels;
    uint16_t bps;
    uint16_t dataStart;
    uint32_t dataLength;
*/
}

//-----------------------------------------------------------------
void loop()
{
}
