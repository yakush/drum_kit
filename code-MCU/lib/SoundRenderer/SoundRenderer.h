#ifndef SOUND_RENDERER_H_
#define SOUND_RENDERER_H_

// #includes....
#include <Arduino.h>
#include "config.h"

namespace NS_SoundRenderer
{

class SoundRendererClass
{
  private:

  public:
    SoundRendererClass();
    virtual ~SoundRendererClass();

    void begin();
    void end();

    void playValue(uint16_t value);
    void playValue(uint8_t valueHi, uint8_t valueLo);
};
}

} //namespace

//-----------------------------------------------------------------
// globals:
// using NS_SoundRenderer::SoundRendererClass;
// extern SoundRendererClass SoundRenderer;
//-----------------------------------------------------------------

#endif