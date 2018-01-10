
#include "FS.h"
#include "SD.h"
#include "SPI.h"

void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println("loading SD card");
  if (!SD.begin(SS, SPI, 20000000)) {
    Serial.println("SD Card Mount Failed");
    return;
  };
  delay(1000);

  //write
  writeFile("/test.txt");
  Serial.println("");

  readFileTwice("/test.txt", 512);
  Serial.println(""); 


}

void loop() {
  // put your main code here, to run repeatedly:

}

//////////////////////////////////////////
void readFileTwice(const char *path, uint16_t buffSize) {
  byte buff1[buffSize];
  byte buff2[buffSize];

  File file1 = SD.open(path);
  if (!file1) {
    Serial.println("Failed to open file 1");
    return;
  }

  File file2 = SD.open(path);
  if (!file2) {
    Serial.println("Failed to open file 2");
    return;
  }

  Serial.println("read from 1");
  file1.read(buff1, buffSize);
  Serial.println(file1.name());
  Serial.println((char)buff1[0]);

  Serial.println("read from 2");
  file2.read(buff2, buffSize);
  Serial.println(file2.name());
  Serial.println((char)buff2[0]);  

  file1.close();
  file2.close();

  Serial.println("DONE");  
}
//////////////////////////////////////////
void writeFile (const char * path) {

  Serial.printf("Writing file: %s\n", path);
  
  static uint8_t buf[512];
  int writes = 0;

  File file = SD.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  if(file.print("hello there!")){
        Serial.println("Write OK");
    } else {
        Serial.println("Write failed");
    }
    
  file.close();
  Serial.println("\ndone");

}
//////////////////////////////////////////////////////
