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

void callback_ended(uint32_t handle, uint8_t reason)
{
  Serial.println("CALLBACK");
  Serial.printf("stopped sample %u : ", handle);
  Serial.println(Player.getClipEndReason(reason));
}

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
          44100 // play freq
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

  String path = "/T.wav";
  Serial.println("Loading clip : " + path);
  ClipInfo_t clipInfo1;
  int clipRes = Player.LoadClip(path, &clipInfo1);
  if (clipRes != WAV_OK)
  {
    Serial.println("Clip Load Failed : " + getWaveError(clipRes));
    return;
  }

  Serial.printf("sampleRate : %u\n", clipInfo1.wavProps.sampleRate);
  Serial.printf("sampleTimeUSec : %u\n", clipInfo1.wavProps.sampleTimeUSec);
  Serial.printf("numChannels : %u\n", clipInfo1.wavProps.numChannels);
  Serial.printf("bitsPerSample : %u\n", clipInfo1.wavProps.bitsPerSample);
  Serial.printf("dataStart : %u\n", clipInfo1.wavProps.dataStart);
  Serial.printf("dataLength : %u\n", clipInfo1.wavProps.dataLength);

  //PLAY:
  long handle1 = Player.playClip(clipInfo1, 128, 128, callback_ended);
  delay(5000);
  long handle2 = Player.playClip(clipInfo1, 128, 128, callback_ended);

  delay(10000);
  Player.stopClip(handle1);
  delay(10000);
  Player.stopClip(handle2);

}

//-----------------------------------------------------------------
void loop()
{
}
