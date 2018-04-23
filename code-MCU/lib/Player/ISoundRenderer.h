#ifndef SoundRenderer_H_
#define SoundRenderer_H_

#include <Arduino.h>

class ISoundRenderer
{

private:
  int _freq;

protected:
  void setFreq(int freq) { _freq = freq; };

public:

  int getFreq() { return _freq; };

  virtual void play(uint32_t L, uint32_t R);
};

#endif //SoundRenderer_H_
