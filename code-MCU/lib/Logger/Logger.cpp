#include "Logger.h"
#include <Arduino.h>

using namespace NS_logger;

//-----------------------------------------------------------------
//class instance
LoggerClass Logger;
//-----------------------------------------------------------------

LoggerClass::LoggerClass (){

}
//-----------------------------------------------------------------
LoggerClass::~LoggerClass (){

}
//-----------------------------------------------------------------
void LoggerClass::setLogLevel(uint8_t level){
  logLevel = level;
}
//-----------------------------------------------------------------
void LoggerClass::debug(const char* log, ...) {

  Serial.printf("DEBUG: %20u :",millis());

  va_list argptr;
  va_start(argptr, log);
  Serial.printf(log,argptr);
  va_end(argptr);

  Serial.println("");
}
//-----------------------------------------------------------------
