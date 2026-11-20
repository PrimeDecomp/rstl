#ifndef _CCONTROLLERBUTTON
#define _CCONTROLLERBUTTON

#include "rstl/RstlVersions.h"
#include "rstl/types.h"

class CControllerButton {
public:
  CControllerButton();
  void SetIsPressed(const bool pressed) { mPressed = pressed; }
  const bool GetIsPressed() const { return mPressed; }
  void SetPressEvent(const bool press) { mPressEvent = press; }
  const bool GetPressEvent() const { return mPressEvent; }
  void SetReleaseEvent(const bool release) { mReleaseEvent = release; };
  const bool GetReleaseEvent() const { return mReleaseEvent; }

private:
  bool mPressed;
  bool mPressEvent;
  bool mReleaseEvent;
};

#if !RSTL_FINAL_INPUT_PAD_SNAPSHOT
inline CControllerButton::CControllerButton() : mPressed(false), mPressEvent(false), mReleaseEvent(false) {}
#endif

#endif // _CCONTROLLERBUTTON
