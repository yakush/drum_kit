#ifndef SampleQue_H_
#define SampleQue_H_

#include "Sample.h"
#include "SampleInfo.h"

namespace NS_Player
{

class SampleQue_CLASS
{

  private:
    Sample *sampleSlots;

  public:
    SampleQue_CLASS();
    virtual ~SampleQue_CLASS();

    Sample *insert(SampleInfo_t sampleInfo);

}; //SampleQue_CLASS

} //namespace
#endif //SampleQue_H_