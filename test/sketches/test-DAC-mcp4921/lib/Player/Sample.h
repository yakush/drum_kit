#ifndef Sample_H_
#define Sample_H_

#include "SampleInfo.h"

namespace NS_Player
{

class Sample
{

private:
  int16_t consumeSingleSample();

public:
  Sample(uint buffSize);
  virtual ~Sample();

  //-- methods:
  void prepare(SampleInfo_t info);
  void reset();
  void readAndInterpolate(long timeUS);
  void updateState();

  //-- fields:

  SampleInfo_t info;

  uint8_t volume = 0x10;
  uint8_t blendLR = 0x10;

  // FILE stuff
  File file;
  size_t filePos;

  // buffers stuff
  byte *buffer[2]; //arr[][]
  uint bufferSize = 0;
  short activeBuffer = 0;
  short buffPos_play = 0;
  bool needFillBuffer = false;
  bool firstBufferReady = false;

  bool isPlaying = false;
  bool isRequestStop = false;
  bool isReadingFile = false;

  long lastSampledTime = 0;
  long startTime = 0;
  int16_t val_0 = 0;
  int16_t val_1 = 0;
  int32_t currentValue_L = 0;
  int32_t currentValue_R = 0;
}; //Sample

} //namespace
#endif //Sample_H_