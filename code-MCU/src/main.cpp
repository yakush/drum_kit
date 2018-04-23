#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include "config.h"
#include "Player.h"
#include "Logger.h"
#include "Button.h"
#include "WavFile.h"
#include "ff.h"

#include "MAX_i2s_renderer.h"

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.printf("\n\n\ninitializing\n");

  Serial.println("staring SD...");
  if (!SD.begin(SS, SPI, 20000000))
  {
    Serial.println("SD Card Mount Failed");
    return;
  };

  Serial.println("staring MAX i2s driver...");
  if (!MAX_i2s_renderer.begin(
          0,    // i2s port (0 or 1)
          26,   // BCK pin
          25,   // LRCK pin
          22,   // DATA pin
          96000 // play freq
          ))
  {
    Serial.println("MAX i2s driver initialization Failed");
    return;
  };

  Serial.println("staring Player...");
  if (!Player.begin(&MAX_i2s_renderer))
  {
    Serial.println("Player initialization Failed");
    return;
  };

  Serial.printf("initialization complete\n\n\n");

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
