#ifndef ClipQue_H_
#define ClipQue_H_

#include "Clip.h"
#include "ClipInfo.h"

namespace NS_Player
{

class ClipQue
{

private:
  Clip *clipSlots;

public:
  ClipQue();
  virtual ~ClipQue();

  Clip *insert(ClipInfo_t clipInfo);

}; //ClipQue

} //namespace

#endif //ClipQue_H_