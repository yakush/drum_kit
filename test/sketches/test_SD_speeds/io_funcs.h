#ifndef IO_FUNCS_H_
#define IO_FUNCS_H_

#include "SD.h"

void writeFile (const char * path, int size) {
  Serial.printf("Writing file: %s\n", path);
  static uint8_t buf[512];
  int writes = 0;

  File file = SD.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  long t = millis();

  while (writes < size) {
    int toWrite = size - writes;
    if (toWrite > 512)
      toWrite = 512;
    file.write(buf, toWrite);
    writes += toWrite;

    if (millis() > t + 1000) {
      t = millis();
      Serial.print(".");
    }
  }
  file.close();
  Serial.println("\ndone");

}

float readFile(const char *path, uint16_t buffSize, bool keepFileOpen) {
  byte buff[buffSize];

  File file;

  file = SD.open(path);
  if (!file) {
    Serial.println("Failed to open file");
    return 0;
  }

  size_t fileSize = file.size();
  size_t reads = 0;

  Serial.printf("%u bytes buffer @ %u bytes file size\n", buffSize, fileSize);

  uint32_t start = millis();

  if (!keepFileOpen) {
    file.close();
  }

  while (reads < fileSize) {
    int toRead = fileSize - reads;
    if (toRead > buffSize)
      toRead = buffSize;

    if (!keepFileOpen) {
      file = SD.open(path);      
      file.seek(reads);
      
      if (!file) {
        Serial.println("Failed to open file");
        return 0;
      }
    }

    file.read(buff, toRead);
    reads += toRead;

    if (!keepFileOpen) {
      file.close();
    }
    //taskYIELD ();
    //vTaskDelay(1);
  }
  uint32_t dTime = millis() - start;
  float speedKBS = ((float)fileSize / 1024) / ((float)dTime / 1000);

  if (keepFileOpen) {
    file.close();
  }

  Serial.printf("read in %u ms\n", dTime);
  Serial.printf("speed   %F KB/SEC\n", speedKBS );
  Serial.println("");

  return speedKBS ;
}

#endif
