#ifndef LOGGER_H_
#define LOGGER_H_

#include <Arduino.h>

#define LOG_LEVEL_NONE        0
#define LOG_LEVEL_VERBOSE     1
#define LOG_LEVEL_DEBUG       2
#define LOG_LEVEL_INFO        3
#define LOG_LEVEL_WARNING     4
#define LOG_LEVEL_ERROR       5


namespace NS_logger {

  class LoggerClass {

  private:
    uint8_t logLevel= LOG_LEVEL_NONE;

    void log(uint8_t level,const char* levelTag, const char* msg, ...);

  public:
    LoggerClass ();
    virtual ~LoggerClass ();

    void setLogLevel(uint8_t level);
    void debug(const char* msg, ...);
  };


}//namespace

//-----------------------------------------------------------------
using NS_logger::LoggerClass;
extern LoggerClass Logger;


#endif