#include "FS.h"
#include "SD.h"
#include "SPI.h"

size_t tests[] = {1, 5, 10, 20, 40, 80};

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("loading SD card");
  if (!SD.begin(SS, SPI, 20000000)) {
    Serial.println("SD Card Mount Failed");
    return;
  };
  delay(1000);

  uint8_t numSizes = sizeof(tests) / sizeof(tests[0]);

  //find/write big files
  for (int i = 0; i < numSizes ; i++) {
    createOrFindBigFile(tests[i]);
  }

  Serial.println("");
  Serial.println("");

  for (int i = 0; i < numSizes ; i++) {
    testSeeks(tests[i]);
  }

  for (int i = 0; i < numSizes ; i++) {
    //showSummery(tests[i]);
  }

  Serial.println("");
  Serial.println("ALL DONE");
}
////////////////////////////////////////////////////////////////////
void loop() {
}
////////////////////////////////////////////////////////////////////
void getFilepath(char*path, size_t sizeMB) {
  sprintf(path, "/BIG_%u.dat", sizeMB);
}

////////////////////////////////////////////////////////////////////
void testSeeks(size_t sizeMB) {
  static size_t buffSize = 512;

  char path[256];
  getFilepath(path, sizeMB);

  Serial.printf("--------------------\ntesting %s\n", path);

  byte buff[buffSize];

  File file = SD.open(path);
  if (!file) {
    Serial.println("Failed to open file");
    return;
  }
  size_t fileSize = file.size();

  long testSizeBytes = 1024 * 1024;
  long reads = 0;
  long t = millis();
  bool flag=true;
  size_t seekTo=0;
  while (reads < testSizeBytes) {
    //file.seek(random(0, fileSize - 512));
    //file.seek(100);
    seekTo = flag ? 100 : fileSize-buffSize-100;
    file.seek(seekTo );
    reads += file.read(buff, buffSize);
    flag=!flag;
  }
  long time = millis() - t;

  Serial.printf("read %F MB in %d millis\nspeed=%F KB/SEC\n",
                ((float)reads) / (1024 * 1024),
                time,
                ((float)reads)/ time / 1024 * 1000
               );
  file.close();
}

////////////////////////////////////////////////////////////////////

void createOrFindBigFile(size_t sizeMB) {
  char path[256];
  getFilepath(path, sizeMB);

  if (SD.exists(path)) {
    Serial.printf("found file: %s\n", path);
    return;
  }

  Serial.printf("creating file: %s ...\n", path);

  static uint8_t buf[512];
  int writes = 0;

  File file = SD.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  size_t size = sizeMB * 1024 * 1024;

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

