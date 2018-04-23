#include <Arduino.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "driver/i2s.h"
#include "I2S_Player.h"
#include "test.h"

#define SAMPLE_RATE 44100
#define I2S_PORT 0
#define PIN_I2S_BCK 26
#define PIN_I2S_LRCK 25
#define PIN_I2S_DATA 22

//////////////////////////////////////////////

void play(String path);

//////////////////////////////////////////////

i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // high interrupt priority
    .dma_buf_count = 8,
    .dma_buf_len = 64 //Interrupt level 1
};

void setup()
{
  test1();
  return;

  Serial.begin(115200);
  Serial.print("Initializing SD card...");
  if (!SD.begin(SS, SPI, 20000000))
  {
    Serial.println("SD Card Mount Failed");
    return;
  };

  delay(1000);
  Serial.print("Initializing I2S...");
  if (!I2S_Player.begin(I2S_PORT,
                        i2s_config,
                        PIN_I2S_BCK,
                        PIN_I2S_LRCK,
                        PIN_I2S_DATA,
                        SAMPLE_RATE))
  {
    Serial.println("I2S begin Failed");
    return;
  }

  Serial.println("initialization done.");
  delay(1000);

  int16_t i = 0;
  while (true)
  {
    uint32_t now = millis();
    I2S_Player.writeSample((now % 100) * ((uint32_t)0xffffffff / 100), 100);
  }

  String path = "/drumkit/kit1/3.H16";
  play(path);

  SD.end();
  I2S_Player.stop();
}
//-----------------------------------------------------------------
void loop()
{
}
//-----------------------------------------------------------------

void play(String path)
{
  File file = SD.open(path);
  if (!file)
  {
    Serial.println("Failed to open file");
    return;
  }

  size_t fileSize = file.size();
  Serial.printf("%u bytes file size\n", fileSize);

  size_t reads = 0;
  size_t lastLog = 0;
  size_t n;
  uint32_t data;

  uint32_t startTime = millis();
  while (file.available())
  {
    n = file.read((uint8_t *)&data, sizeof(uint32_t));
    reads += n;
    if (reads > lastLog + 1024)
    {
      lastLog = reads;
      Serial.printf("%u KB \n", reads / 1024);
    }
    I2S_Player.writeSample(data, 100);
  }
  uint32_t timeDelta = (millis() - startTime);
  Serial.printf("done! %u KB in %u millis (%u BPS)\n", reads / 1024, timeDelta, 1000 * reads / timeDelta);
}