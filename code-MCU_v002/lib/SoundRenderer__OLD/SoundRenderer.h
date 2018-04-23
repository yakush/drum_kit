/*
*
*
*/

#ifndef SOUND_RENDERER_H_
#define SOUND_RENDERER_H_

// #includes....
#include <Arduino.h>
#include "config.h"

namespace NS_SoundRenderer
{

  class SoundRendererClass {

  private:
    uint8_t pinHi;
    uint8_t pinLo;
    uint8_t channelHi;
    uint8_t channelLo;
    uint32_t carrierFreq;
    boolean highSpeedWrite;

  public:
    SoundRendererClass ();
    virtual ~SoundRendererClass ();

    void begin(uint8_t pinHi = 26,uint8_t pinLo = 25,uint8_t channelHi = 0,uint8_t channelLo = 1,uint32_t carrierFreq=312500,boolean highSpeedWrite = true);
    void end();

    void playValue( uint16_t value );
    void playValue( uint8_t valueHi, uint8_t valueLo );
  };

} //namespace

//-----------------------------------------------------------------
// global usings:
using NS_SoundRenderer::SoundRendererClass;

extern SoundRendererClass SoundRenderer;
//-----------------------------------------------------------------

#endif