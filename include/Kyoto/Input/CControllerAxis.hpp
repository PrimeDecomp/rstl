#ifndef _CCONTROLLERAXIS
#define _CCONTROLLERAXIS

#include "rstl/RstlVersions.h"
#include "rstl/types.h"

class CControllerAxis {
public:
  CControllerAxis();
  void SetRelativeValue(float val) { mRelative = val; }
  float GetRelativeValue() const { return mRelative; }
  void SetAbsoluteValue(float val) { mAbsolute = val; }
  float GetAbsoluteValue() const { return mAbsolute; }

public:
  float mRelative;
  float mAbsolute;
};

#if !RSTL_FINAL_INPUT_PAD_SNAPSHOT
inline CControllerAxis::CControllerAxis() : mRelative(0.f), mAbsolute(0.f) {}
#endif

#endif // _CCONTROLLERAXIS
