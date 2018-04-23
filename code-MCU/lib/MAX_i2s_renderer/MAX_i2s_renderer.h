#ifndef MAX_i2s_renderer_H_
#define MAX_i2s_renderer_H_

#include <Arduino.h>
#include "driver/i2s.h"
#include "ISoundRenderer.h"

namespace NS_MAX_i2s_renderer
{

class MAX_i2s_renderer_CLASS : public ISoundRenderer
{

  private:
    i2s_port_t _port;
    i2s_config_t _i2s_config;
    i2s_pin_config_t _pin_config;

  public:
    MAX_i2s_renderer_CLASS();
    virtual ~MAX_i2s_renderer_CLASS();

    bool begin(int port, int pin_bck, int pin_lrck, int pin_data, int freq);
    void stop();

    void play(uint32_t L, uint32_t R);

}; //MAX_i2s_renderer_CLASS

} //namespace NS_MAX_i2s_renderer


//-----------------------------------------------------------------
// global :
using NS_MAX_i2s_renderer::MAX_i2s_renderer_CLASS;
extern MAX_i2s_renderer_CLASS MAX_i2s_renderer;
//-----------------------------------------------------------------

#endif //MAX_i2s_renderer_H_