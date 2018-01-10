#ifndef FILE_TRANSFER_H
#define FILE_TRANSFER_H

#include "Arduino.h"
#include <SD.h>

#define SERIAL_BUFF_SIZE 64

class FileTransfer
{

  public:
    File file;

    FileTransfer()
    {

    }

    boolean start()
    {
      String filename = Serial.readStringUntil('\n');
      long filesize = Serial.parseInt();

      Serial.println("getting file " + filename);
      Serial.print("file size ");
      Serial.print(filesize);
      Serial.println(" B");

      //create file      
      if(!createFile(filename)){
        return;
      }
      
      //get file content
      const int BUFFSIZE = 64;
      char buff [BUFFSIZE] ;
      long lastReport = 0;
      long len = 0;

      while (len < filesize) {
        int bytesLeft = filesize - len;
        int nextRead = bytesLeft < BUFFSIZE ? bytesLeft : BUFFSIZE;
        size_t currentLen = Serial.readBytes(buff, nextRead);
        len += currentLen;
        file.write(buff, currentLen );

        if (len > 100 + lastReport) {
          Serial.print("got: ");
          Serial.println(len);
          lastReport = len;
        }
      }
      file.flush();
      file.close();
      
      Serial.print("got: ");
      Serial.println(len);
      Serial.println("done.");
    }

    // ----------------------------------
    // ----------------------------------
  private:

    boolean createFile(String filename) {
      if (SD.exists(filename)) {
        Serial.println(filename + " exists - deleting");
        if (SD.remove(filename)) {

        } else {
          Serial.println("error deleting file");
          return false;
        }
      }

      File file = SD.open(filename, FILE_WRITE);
      if (file) {
        Serial.println("opening " + filename);
      } else {
        Serial.println("error opening " + filename);
        return false;
      }
      return true;
    }
};
#endif
