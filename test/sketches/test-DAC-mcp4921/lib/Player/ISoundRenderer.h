#ifndef SoundRenderer_H_
#define SoundRenderer_H_

#include <Arduino.h>

class ISoundRenderer
{

  private:
  public:
    ISoundRenderer();
    virtual ~ISoundRenderer();

    virtual void play(uint32_t L, uint32_t R);
};

#endif //SoundRenderer_H_
