#ifndef _CFINALINPUTINFO
#define _CFINALINPUTINFO

#include "types.h"

class CFinalInput;

// The demo map retains this intermediate input type and the CFinalInput overload.
// Field layout is reconstructed; it is not a recovered binary ABI.
class CFinalInputInfo {
public:
  CFinalInputInfo() : mDt(0.f), mControllerIdx(0) {
    for (int i = 0; i < 8; ++i)
      mAnalog[i] = 0.f;
    for (int i = 0; i < 28; ++i)
      mButtons[i] = false;
  }
  explicit CFinalInputInfo(const CFinalInput& input);

  float mDt;
  int mControllerIdx;
  float mAnalog[8];
  bool mButtons[28];
};
#endif
