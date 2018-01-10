#include <string.h>
#include <stdlib.h>
#include <SPI.h>
#include <SD.h>
#include "fileTransfer.h"

#define PIN_SD 10

//this following macro is good for debugging, e.g.  print2("myVar= ", myVar);
#define print2(x,y) (Serial.print(x), Serial.println(y))


#define CR '\r'
#define LF '\n'
#define BS '\b'
#define NULLCHAR '\0'
#define SPACE ' '

#define COMMAND_BUFFER_LENGTH        25                        //length of serial buffer for incoming commands
char   CommandLine[COMMAND_BUFFER_LENGTH + 1];                 //Read commands into this buffer from Serial.  +1 in length for a termination char

const char *delimiters            = ", \n";                    //commands can be separated by return, space or comma

/*************************************************************************************************************
     your Command Names Here
*/
const char *addCommandToken       = "download";                     //Modify here
const char *subtractCommandToken  = "list";                         //Modify here

File myFile;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  Serial.print("Initializing SD card...");
  if (!SD.begin(PIN_SD)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

}

void loop() {
  //command parser:
  if (getCommandLineFromSerialPort(CommandLine)) {
    if (strcmp(CommandLine, "list") == 0) {
      listFiles();
    } else if (strcmp(CommandLine, "download") == 0) {
      getFile();
    }
  }
}

void getFile() {
  Serial.println("downloading file");

  String filename = Serial.readStringUntil('\n');
  long filesize = Serial.parseInt();

  Serial.println("getting file " + filename);
  Serial.print("file size ");
  Serial.print(filesize);
  Serial.println(" B");

  //create file
  if (SD.exists(filename)) {
    Serial.println(filename + " exists - deleting");
    if (SD.remove(filename)) {

    } else {
      Serial.println("error deleting file");
      return;
    }
  }

  myFile = SD.open(filename, FILE_WRITE);
  if (myFile) {
    Serial.println("opening " + filename);
  } else {
    Serial.println("error opening " + filename);
    return;
  }

  //get file content
  const int BUFFSIZE = 32;
  char buff [BUFFSIZE] ;
  long lastReport = 0;
  long len = 0;

  while (len < filesize) {
    int bytesLeft = filesize - len;
    int nextRead = bytesLeft < BUFFSIZE ? bytesLeft : BUFFSIZE;
    size_t currentLen = Serial.readBytes(buff, nextRead);
    len += currentLen;
    //myFile.write(buff, currentLen );

    if (len > 100 + lastReport) {
      //Serial.print("got: ");
      //Serial.println(len);
      lastReport = len;
    }
  }
  myFile.flush();
  myFile.close();
  Serial.print("got: ");
  Serial.println(len);
  Serial.println("done.");
}

void listFiles() {
  Serial.println("list Files");
  File root = SD.open("/");
  printDirectory(root, 0);
  root.close();
  Serial.println("done!");
}

void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}

//-------------------------------
bool getCommandLineFromSerialPort(char * commandLine)
{
  static uint8_t charsRead = 0;                      //note: COMAND_BUFFER_LENGTH must be less than 255 chars long
  //read asynchronously until full command input
  while (Serial.available()) {
    char c = Serial.read();
    switch (c) {
      case CR:
      case LF:
        commandLine[charsRead] = NULLCHAR;       //null terminate our command char array
        if (charsRead > 0)  {
          charsRead = 0;
          return true;
        }
        break;
      //      case BS:                                    // handle backspace in input: put a space in last char
      //        if (charsRead > 0) {                        //and adjust commandLine and charsRead
      //          commandLine[--charsRead] = NULLCHAR;
      //          Serial << byte(BS) << byte(SPACE) << byte(BS);  //no idea how this works, found it on the Internet
      //        }
      //        break;
      default:
        // c = tolower(c);
        if (charsRead < COMMAND_BUFFER_LENGTH) {
          commandLine[charsRead++] = c;
        }
        commandLine[charsRead] = NULLCHAR;     //just in case
        break;
    }
  }
  return false;
}

