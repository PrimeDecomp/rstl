#include "rstl/RstlVersions.h"
#include "Kyoto/Input/CFinalInput.hpp"
#if RSTL_PRIME1
#include "Kyoto/Input/CFinalInputInfo.hpp"
#endif

#if RSTL_FINAL_INPUT_CONTROLLER_DATA || RSTL_FINAL_INPUT_PAD_SNAPSHOT
// The half of a stick axis that a direction reads: its positive part (right/up) or its negative
// part (left/down) as a magnitude, and whether the signed axis passes a threshold.
static inline float PositivePart(const CControllerAxis& axis) {
  const float value = axis.GetAbsoluteValue();
  return value > 0.f ? value : 0.f;
}

static inline float NegativePart(const CControllerAxis& axis) {
  const float value = axis.GetAbsoluteValue();
  return value < 0.f ? -value : 0.f;
}

static inline bool PositivePartAbove(const CControllerAxis& axis, float threshold) {
  return axis.GetAbsoluteValue() > threshold;
}

static inline bool NegativePartAbove(const CControllerAxis& axis, float threshold) {
  return -axis.GetAbsoluteValue() > threshold;
}
#endif

#if RSTL_FINAL_INPUT_CONTROLLER_DATA

const float CFinalInput::kAnalogAxisDigitalThreshold = 0.7f;
const float CFinalInput::kRollDigitalThreshold = 0.5f;
const float CFinalInput::kPitchDigitalThreshold = 0.5f;
const float CFinalInput::kPointerDigitalThreshold = 0.7f;

CFinalInput::CFinalInput(int channel, float dt, const CControllerData& data, uint inputType)
: mDt(dt), mControllerIdx(channel), mInputType(inputType), mControllerData(data) {}

CFinalInput::CFinalInput() : mDt(0.0166667f), mControllerIdx(0), mInputType(4), mControllerData() {}

CFinalInput CFinalInput::NoInput(int channel, float dt) {
  return CFinalInput(channel, dt, CControllerData(), 4);
}

bool CFinalInput::GetKnownDeviceIsPresent() const { return mInputType <= 2; }

bool CFinalInput::CheckValidControl(int control) const {
#if NONMATCHING
  if (control < 0) {
    return false;
  }
#endif
  switch (mInputType) {
  case 0:
    if ((control >= kPC_Home && control <= kPC_AnyDPad) ||
        (control >= kPC_PointerUp && control <= kPC_WiimotePitchPositive)) {
      return true;
    }
    break;
  case 1:
    if (control <= kPC_WiimotePitchPositive) {
      return true;
    }
    break;
  case 2:
    if ((control >= kPC_Home && control <= kPC_AnyDPad) ||
        (control >= kPC_PointerUp && control <= kPC_WiimotePitchPositive)) {
      return true;
    }
    break;
  }
  return false;
}

float CFinalInput::GetAnalog(int control) const {
  if (!CheckValidControl(control)) {
    return 0.f;
  }
  if (!GetKnownDeviceIsPresent()) {
    return 0.f;
  }
  switch (control) {
  case kPC_None:
    return 0.f;
  case kPC_Home:
  case kPC_Minus:
  case kPC_A:
  case kPC_B:
  case kPC_Two:
  case kPC_One:
  case kPC_Plus:
  case kPC_DPadUp:
  case kPC_DPadDown:
  case kPC_DPadRight:
  case kPC_DPadLeft:
  case kPC_NunchukZ:
  case kPC_NunchukC:
    return mControllerData.GetButton(control).GetIsPressed() ? 1.f : 0.f;
  case kPC_StickLeft:
    return NegativePart(mControllerData.GetAxis(0));
  case kPC_StickRight:
    return PositivePart(mControllerData.GetAxis(0));
  case kPC_StickDown:
    return NegativePart(mControllerData.GetAxis(1));
  case kPC_StickDiagonal3Negative:
    return NegativePart(mControllerData.GetAxis(3));
  case kPC_StickDiagonal2Positive: {
    const float value = mControllerData.GetAxis(2).GetAbsoluteValue();
    return value > 0.f ? -value : 0.f;
  }
  case kPC_StickUp:
    return PositivePart(mControllerData.GetAxis(1));
  case kPC_StickDiagonal2Negative: {
    const float value = mControllerData.GetAxis(2).GetAbsoluteValue();
    return value < 0.f ? value : 0.f;
  }
  case kPC_StickDiagonal3Positive:
    return PositivePart(mControllerData.GetAxis(3));
  case kPC_NunchukRollNegative:
    return NegativePart(mControllerData.GetAxis(4));
  case kPC_NunchukRollPositive:
    return PositivePart(mControllerData.GetAxis(4));
  case kPC_NunchukPitchNegative:
    return NegativePart(mControllerData.GetAxis(5));
  case kPC_NunchukPitchPositive:
    return PositivePart(mControllerData.GetAxis(5));
  case kPC_PointerLeft:
    return NegativePart(mControllerData.GetAxis(6));
  case kPC_PointerRight:
    return PositivePart(mControllerData.GetAxis(6));
  case kPC_PointerDown:
    return PositivePart(mControllerData.GetAxis(7));
  case kPC_PointerUp:
    return NegativePart(mControllerData.GetAxis(7));
  case kPC_WiimoteRollNegative:
    return NegativePart(mControllerData.GetAxis(8));
  case kPC_WiimoteRollPositive:
    return PositivePart(mControllerData.GetAxis(8));
  case kPC_WiimotePitchNegative:
    return NegativePart(mControllerData.GetAxis(9));
  case kPC_WiimotePitchPositive:
    return PositivePart(mControllerData.GetAxis(9));
  default:
    return 0.f;
  }
}

float CFinalInput::GetMotionAnalog(int control) const {
#if NONMATCHING
  if (uint(control) >= 16) {
    return 0.f;
  }
#endif
  if (!GetKnownDeviceIsPresent()) {
    return 0.f;
  }
  return mControllerData.GetMotionButton(control).GetIsPressed() ? 1.f : 0.f;
}

float CFinalInput::GetSwingAnalog(int control) const {
#if NONMATCHING
  if (uint(control) >= 12) {
    return 0.f;
  }
#endif
  if (!GetKnownDeviceIsPresent()) {
    return 0.f;
  }
  return mControllerData.GetSwingButton(control).GetIsPressed() ? 1.f : 0.f;
}

bool CFinalInput::GetDigital(int control) const {
  if (!CheckValidControl(control)) {
    return false;
  }
  if (!GetKnownDeviceIsPresent()) {
    return false;
  }
  return mControllerData.GetButton(control).GetIsPressed();
}

bool CFinalInput::GetMotionDigital(int control) const {
#if NONMATCHING
  if (uint(control) >= 16) {
    return false;
  }
#endif
  return mControllerData.GetMotionButton(control).GetIsPressed();
}

bool CFinalInput::GetSwingDigital(int control) const {
#if NONMATCHING
  if (uint(control) >= 12) {
    return false;
  }
#endif
  if (!GetKnownDeviceIsPresent()) {
    return false;
  }
  return mControllerData.GetSwingButton(control).GetIsPressed();
}

bool CFinalInput::GetPressed(int control) const {
  if (!CheckValidControl(control)) {
    return false;
  }
  if (!GetKnownDeviceIsPresent()) {
    return false;
  }
  return mControllerData.GetButton(control).GetPressEvent();
}

bool CFinalInput::GetMotionPressed(int control) const {
#if NONMATCHING
  if (uint(control) >= 16) {
    return false;
  }
#endif
  if (!GetKnownDeviceIsPresent()) {
    return false;
  }
  return mControllerData.GetMotionButton(control).GetPressEvent();
}

bool CFinalInput::GetSwingPressed(int control) const {
#if NONMATCHING
  if (uint(control) >= 12) {
    return false;
  }
#endif
  if (!GetKnownDeviceIsPresent()) {
    return false;
  }
  return mControllerData.GetSwingButton(control).GetPressEvent();
}

bool CFinalInput::GetReleased(int control) const {
  if (!CheckValidControl(control)) {
    return false;
  }
  if (!GetKnownDeviceIsPresent()) {
    return false;
  }
  return mControllerData.GetButton(control).GetReleaseEvent();
}

bool CFinalInput::GetMotionReleased(int control) const {
#if NONMATCHING
  if (uint(control) >= 16) {
    return false;
  }
#endif
  if (!GetKnownDeviceIsPresent()) {
    return false;
  }
  return mControllerData.GetMotionButton(control).GetReleaseEvent();
}

bool CFinalInput::GetSwingReleased(int control) const {
#if NONMATCHING
  if (uint(control) >= 12) {
    return false;
  }
#endif
  if (!GetKnownDeviceIsPresent()) {
    return false;
  }
  return mControllerData.GetSwingButton(control).GetReleaseEvent();
}

CVector2f CFinalInput::GetPointerPosition() const {
  if (!GetKnownDeviceIsPresent()) {
    return CVector2f::Zero();
  }
  return CVector2f(mControllerData.GetAxis(6).GetAbsoluteValue(),
                   mControllerData.GetAxis(7).GetAbsoluteValue());
}

#elif RSTL_FINAL_INPUT_PAD_SNAPSHOT

#include "Kyoto/Input/CControllerGamepadData.hpp"

// The plain buttons: those that are neither a trigger nor a stick direction.
#define FINAL_INPUT_PAD_BUTTON_CASES \
  case 1: \
  case 2: \
  case 3: \
  case 4: \
  case 5: \
  case 6: \
  case 7: \
  case 8: \
  case 9: \
  case 10: \
  case 11: \
  case 16: \
  case 17: \
  case 22: \
  case 23: \
  case 24: \
  case 25: \
  case 26: \
  case 27: \
  case 28: \
  case 29: \
  case 30: \
  case 31: \
  case 32: \
  case 33: \
  case 48: \
  case 49: \
  case 50:

const float CFinalInput::kInput_AnalogOnThreshhold = 0.7f;
const float CFinalInput::kInput_AnalogTriggerOnThreshhold = 0.05f;

CFinalInput::CFinalInput() : mDt(0.f), mControllerIdx(0) {}

CFinalInput::CFinalInput(int controllerIdx, float dt, const CControllerGamepadData& data)
: mDt(dt), mControllerIdx(controllerIdx), mPad(data) {}

CControllerAxis::CControllerAxis() : mRelative(0.f), mAbsolute(0.f) {}

CControllerButton::CControllerButton() : mPressed(false), mPressEvent(false), mReleaseEvent(false) {}

float CFinalInput::GetAnalog(int input) const {
  switch (input) {
  case kBU_None:
    return 0.f;
  FINAL_INPUT_PAD_BUTTON_CASES
    return mPad.GetButton(static_cast< EButton >(input)).GetIsPressed() ? 1.f : 0.f;
  case kBU_LTrigger:
    return mPad.GetAnalogButton(kBA_Left).GetAbsoluteValue();
  case kBU_RTrigger:
    return mPad.GetAnalogButton(kBA_Right).GetAbsoluteValue();
  case kBU_LeftStickLeft:
    return NegativePart(mPad.GetAxis(kJA_LeftX));
  case kBU_LeftStickRight:
    return PositivePart(mPad.GetAxis(kJA_LeftX));
  case kBU_LeftStickDown:
    return NegativePart(mPad.GetAxis(kJA_LeftY));
  case kBU_LeftStickUp:
    return PositivePart(mPad.GetAxis(kJA_LeftY));
  case kBU_RightStickLeft:
    return NegativePart(mPad.GetAxis(kJA_RightX));
  case kBU_RightStickRight:
    return PositivePart(mPad.GetAxis(kJA_RightX));
  case kBU_RightStickDown:
    return PositivePart(mPad.GetAxis(kJA_RightY));
  case kBU_RightStickUp:
    return NegativePart(mPad.GetAxis(kJA_RightY));
  case kBU_GcLeftStickLeft:
    return NegativePart(mPad.GetAxis(kJA_GcLeftX));
  case kBU_GcLeftStickRight:
    return PositivePart(mPad.GetAxis(kJA_GcLeftX));
  case kBU_GcLeftStickUp:
    return NegativePart(mPad.GetAxis(kJA_GcLeftY));
  case kBU_GcLeftStickDown:
    return PositivePart(mPad.GetAxis(kJA_GcLeftY));
  case kBU_GcRightStickLeft:
    return NegativePart(mPad.GetAxis(kJA_GcRightX));
  case kBU_GcRightStickRight:
    return PositivePart(mPad.GetAxis(kJA_GcRightX));
  case kBU_GcRightStickDown:
    return PositivePart(mPad.GetAxis(kJA_GcRightY));
  case kBU_GcRightStickUp:
    return NegativePart(mPad.GetAxis(kJA_GcRightY));
  case kBU_Axis4Left:
    return NegativePart(mPad.GetAxis(kJA_Axis4));
  case kBU_Axis4Right:
    return PositivePart(mPad.GetAxis(kJA_Axis4));
  case kBU_Axis5Down:
    return PositivePart(mPad.GetAxis(kJA_Axis5));
  case kBU_Axis5Up:
    return NegativePart(mPad.GetAxis(kJA_Axis5));
  }
  return 0.f;
}

bool CFinalInput::GetDigital(int input) const {
  switch (input) {
  case kBU_None:
    return false;
  FINAL_INPUT_PAD_BUTTON_CASES
    return mPad.GetButton(static_cast< EButton >(input)).GetIsPressed();
  case kBU_LTrigger:
    if (mPad.GetAnalogButton(kBA_Left).GetAbsoluteValue() > kInput_AnalogTriggerOnThreshhold) {
      return true;
    }
    break;
  case kBU_RTrigger:
    if (mPad.GetAnalogButton(kBA_Right).GetAbsoluteValue() > kInput_AnalogTriggerOnThreshhold) {
      return true;
    }
    break;
  case kBU_LeftStickLeft:
    if (NegativePartAbove(mPad.GetAxis(kJA_LeftX), kInput_AnalogOnThreshhold)) {
      return true;
    }
    break;
  case kBU_LeftStickRight:
    if (PositivePartAbove(mPad.GetAxis(kJA_LeftX), kInput_AnalogOnThreshhold)) {
      return true;
    }
    break;
  case kBU_LeftStickDown:
    if (NegativePartAbove(mPad.GetAxis(kJA_LeftY), kInput_AnalogOnThreshhold)) {
      return true;
    }
    break;
  case kBU_LeftStickUp:
    if (PositivePartAbove(mPad.GetAxis(kJA_LeftY), kInput_AnalogOnThreshhold)) {
      return true;
    }
    break;
  case kBU_RightStickLeft:
    if (NegativePartAbove(mPad.GetAxis(kJA_RightX), kInput_AnalogOnThreshhold)) {
      return true;
    }
    break;
  case kBU_RightStickRight:
    if (PositivePartAbove(mPad.GetAxis(kJA_RightX), kInput_AnalogOnThreshhold)) {
      return true;
    }
    break;
  case kBU_RightStickDown:
    if (NegativePartAbove(mPad.GetAxis(kJA_RightY), kInput_AnalogOnThreshhold)) {
      return true;
    }
    break;
  case kBU_RightStickUp:
    if (PositivePartAbove(mPad.GetAxis(kJA_RightY), kInput_AnalogOnThreshhold)) {
      return true;
    }
    break;
  case kBU_GcLeftStickLeft:
    if (GetAnalog(input) > kInput_AnalogOnThreshhold) {
      return true;
    }
    break;
  case kBU_GcLeftStickRight:
    if (GetAnalog(input) > kInput_AnalogOnThreshhold) {
      return true;
    }
    break;
  case kBU_GcLeftStickDown:
    if (GetAnalog(input) > kInput_AnalogOnThreshhold) {
      return true;
    }
    break;
  case kBU_GcLeftStickUp:
    if (GetAnalog(input) > kInput_AnalogOnThreshhold) {
      return true;
    }
    break;
  case kBU_GcRightStickLeft:
    if (GetAnalog(input) > kInput_AnalogOnThreshhold) {
      return true;
    }
    break;
  case kBU_GcRightStickRight:
    if (GetAnalog(input) > kInput_AnalogOnThreshhold) {
      return true;
    }
    break;
  case kBU_GcRightStickDown:
    if (GetAnalog(input) > kInput_AnalogOnThreshhold) {
      return true;
    }
    break;
  case kBU_GcRightStickUp:
    if (GetAnalog(input) > kInput_AnalogOnThreshhold) {
      return true;
    }
    break;
  case kBU_Axis4Left:
    if (NegativePartAbove(mPad.GetAxis(kJA_Axis4), kInput_AnalogOnThreshhold)) {
      return true;
    }
    break;
  case kBU_Axis4Right:
    if (PositivePartAbove(mPad.GetAxis(kJA_Axis4), kInput_AnalogOnThreshhold)) {
      return true;
    }
    break;
  case kBU_Axis5Down:
    if (NegativePartAbove(mPad.GetAxis(kJA_Axis5), kInput_AnalogOnThreshhold)) {
      return true;
    }
    break;
  case kBU_Axis5Up:
    if (PositivePartAbove(mPad.GetAxis(kJA_Axis5), kInput_AnalogOnThreshhold)) {
      return true;
    }
    break;
  }
  return false;
}

bool CFinalInput::GetPressed(int input) const {
  return mPad.GetButton(static_cast< EButton >(input)).GetPressEvent();
}

#else

#include "Kyoto/Input/CControllerGamepadData.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Basics/COsContext.hpp"

const float CFinalInput::kInput_AnalogOnThreshhold = 0.7f;
const float CFinalInput::kInput_AnalogTriggerOnThreshhold = 0.05f;

CFinalInput::CFinalInput()
: mDt(0.f)
, mControllerIdx(0)
, mAnaLeftX(0.f)
, mAnaLeftY(0.f)
, mAnaRightX(0.f)
, mAnaRightY(0.f)
, mAnaLeftTrigger(0.f)
, mAnaRightTrigger(0.f)
, mEnableAnaLeftXP(false)
, mEnableAnaLeftYP(false)
, mEnableAnaRightXP(false)
, mEnableAnaRightYP(false)
, mAnaLeftTriggerP(0.f)
, mAnaRightTriggerP(0.f)
, mA(false)
, mB(false)
, mX(false)
, mY(false)
, mZ(false)
, mL(false)
, mR(false)
, mDPUp(false)
, mDPRight(false)
, mDPDown(false)
, mDPLeft(false)
, mStart(false)
, mPA(false)
, mPB(false)
, mPX(false)
, mPY(false)
, mPZ(false)
, mPL(false)
, mPR(false)
, mPDPUp(false)
, mPDPRight(false)
, mPDPDown(false)
, mPDPLeft(false)
, mPStart(false) {}

CFinalInput::CFinalInput(int channel, float dt, const CControllerGamepadData& data, float leftDiv,
                         float rightDiv)
: mDt(dt)
, mControllerIdx(channel)
, mAnaLeftX(data.GetAxis(kJA_LeftX).GetAbsoluteValue())
, mAnaLeftY(data.GetAxis(kJA_LeftY).GetAbsoluteValue())
, mAnaRightX(data.GetAxis(kJA_RightX).GetAbsoluteValue())
, mAnaRightY(data.GetAxis(kJA_RightY).GetAbsoluteValue())
, mAnaLeftTrigger(data.GetAnalogButton(kBA_Left).GetAbsoluteValue())
, mAnaRightTrigger(data.GetAnalogButton(kBA_Right).GetAbsoluteValue())
, mAnaLeftTriggerP(data.GetAnalogButton(kBA_Left).GetRelativeValue())
, mAnaRightTriggerP(data.GetAnalogButton(kBA_Right).GetRelativeValue())
, mEnableAnaLeftXP(false)
, mEnableAnaLeftYP(false)
, mEnableAnaRightXP(false)
, mEnableAnaRightYP(false)
, mA(data.GetButton(kBU_A).GetIsPressed())
, mB(data.GetButton(kBU_B).GetIsPressed())
, mX(data.GetButton(kBU_X).GetIsPressed())
, mY(data.GetButton(kBU_Y).GetIsPressed())
, mZ(data.GetButton(kBU_Z).GetIsPressed())
, mL(data.GetButton(kBU_L).GetIsPressed())
, mR(data.GetButton(kBU_R).GetIsPressed())
, mDPUp(data.GetButton(kBU_Up).GetIsPressed())
, mDPRight(data.GetButton(kBU_Right).GetIsPressed())
, mDPDown(data.GetButton(kBU_Down).GetIsPressed())
, mDPLeft(data.GetButton(kBU_Left).GetIsPressed())
, mStart(data.GetButton(kBU_Start).GetIsPressed())
, mPA(data.GetButton(kBU_A).GetPressEvent())
, mPB(data.GetButton(kBU_B).GetPressEvent())
, mPX(data.GetButton(kBU_X).GetPressEvent())
, mPY(data.GetButton(kBU_Y).GetPressEvent())
, mPZ(data.GetButton(kBU_Z).GetPressEvent())
, mPL(data.GetButton(kBU_L).GetPressEvent())
, mPR(data.GetButton(kBU_R).GetPressEvent())
, mPDPUp(data.GetButton(kBU_Up).GetPressEvent())
, mPDPRight(data.GetButton(kBU_Right).GetPressEvent())
, mPDPDown(data.GetButton(kBU_Down).GetPressEvent())
, mPDPLeft(data.GetButton(kBU_Left).GetPressEvent())
, mPStart(data.GetButton(kBU_Start).GetPressEvent()) {
  InitializeAnalog(leftDiv, rightDiv);
}

static bool sIsAnalogPressed[4][4] = {};

void CFinalInput::InitializeAnalog(float leftDiv, float rightDiv) {
  mAnaLeftX = CMath::Clamp(-1.f, mAnaLeftX / leftDiv, 1.f);
  mAnaLeftY = CMath::Clamp(-1.f, mAnaLeftY / leftDiv, 1.f);
  mAnaRightX = CMath::Clamp(-1.f, mAnaRightX / rightDiv, 1.f);
  mAnaRightY = CMath::Clamp(-1.f, mAnaRightY / rightDiv, 1.f);

  if (mAnaLeftY > 0.7f && !sIsAnalogPressed[mControllerIdx][0]) {
    sIsAnalogPressed[mControllerIdx][0] = true;
    mEnableAnaLeftYP = true;
  } else if (mAnaLeftY > 0.7f && sIsAnalogPressed[mControllerIdx][0]) {
    mEnableAnaLeftYP = false;
  } else if (mAnaLeftY < -0.7f && !sIsAnalogPressed[mControllerIdx][0]) {
    mEnableAnaLeftYP = true;
    sIsAnalogPressed[mControllerIdx][0] = true;
  } else if (mAnaLeftY < -0.7f && sIsAnalogPressed[mControllerIdx][0]) {
    mEnableAnaLeftYP = false;
  } else if (fabsf(mAnaLeftY) < 0.7f) {
    mEnableAnaLeftYP = false;
    sIsAnalogPressed[mControllerIdx][0] = false;
  }

  if (mAnaLeftX > 0.7f && !sIsAnalogPressed[mControllerIdx][1]) {
    mEnableAnaLeftXP = true;
    sIsAnalogPressed[mControllerIdx][1] = true;
  } else if (mAnaLeftX > 0.7f && sIsAnalogPressed[mControllerIdx][1]) {
    mEnableAnaLeftXP = false;
  } else if (mAnaLeftX < -0.7f && !sIsAnalogPressed[mControllerIdx][1]) {
    sIsAnalogPressed[mControllerIdx][1] = true;
    mEnableAnaLeftXP = true;
  } else if (mAnaLeftX < -0.7f && sIsAnalogPressed[mControllerIdx][1]) {
    mEnableAnaLeftXP = false;
  } else if (fabsf(mAnaLeftX) < 0.7f) {
    mEnableAnaLeftXP = false;
    sIsAnalogPressed[mControllerIdx][1] = false;
  }

  if (mAnaRightY > 0.7f && !sIsAnalogPressed[mControllerIdx][2]) {
    sIsAnalogPressed[mControllerIdx][2] = true;
    mEnableAnaRightYP = true;
  } else if (mAnaRightY > 0.7f && sIsAnalogPressed[mControllerIdx][2]) {
    mEnableAnaRightYP = false;
  } else if (mAnaRightY < -0.7f && !sIsAnalogPressed[mControllerIdx][2]) {
    mEnableAnaRightYP = true;
    sIsAnalogPressed[mControllerIdx][2] = true;
  } else if (mAnaRightY < -0.7f && sIsAnalogPressed[mControllerIdx][2]) {
    mEnableAnaRightYP = false;
  } else if (fabsf(mAnaRightY) < 0.7f) {
    mEnableAnaRightYP = false;
    sIsAnalogPressed[mControllerIdx][2] = false;
  }

  if (mAnaRightX > 0.7f && !sIsAnalogPressed[mControllerIdx][3]) {
    mEnableAnaRightXP = true;
    sIsAnalogPressed[mControllerIdx][3] = true;
  } else if (mAnaRightX > 0.7f && sIsAnalogPressed[mControllerIdx][3]) {
    mEnableAnaRightXP = false;
  } else if (mAnaRightX < -0.7f && !sIsAnalogPressed[mControllerIdx][3]) {
    sIsAnalogPressed[mControllerIdx][3] = true;
    mEnableAnaRightXP = true;
  } else if (mAnaRightX < -0.7f && sIsAnalogPressed[mControllerIdx][3]) {
    mEnableAnaRightXP = false;
  } else if (fabsf(mAnaRightX) < 0.7f) {
    mEnableAnaRightXP = false;
    sIsAnalogPressed[mControllerIdx][3] = false;
  }
}

// Echoes defines the COsContext constructor after ScaleAnalogueSticks, Prime before it.
#if !RSTL_PRIME1
CFinalInput CFinalInput::ScaleAnalogueSticks(float leftDiv, float rightDiv) const {
  CFinalInput ret = *this;
  ret.mAnaLeftX = CMath::Clamp(-1.f, mAnaLeftX / leftDiv, 1.f);
  ret.mAnaLeftY = CMath::Clamp(-1.f, mAnaLeftY / leftDiv, 1.f);
  ret.mAnaRightX = CMath::Clamp(-1.f, mAnaRightX / rightDiv, 1.f);
  ret.mAnaRightY = CMath::Clamp(-1.f, mAnaRightY / rightDiv, 1.f);
  return ret;
}

#endif
CFinalInput::CFinalInput(int channel, float dt, const COsContext& ctx)
: mDt(dt)
, mControllerIdx(channel)
, mAnaLeftX(0.f)
, mAnaLeftY(0.f)
, mAnaRightX(0.f)
, mAnaRightY(0.f)
, mAnaLeftTrigger(0.f)
, mAnaRightTrigger(0.f)
#if RSTL_FINAL_INPUT_PACKED_FLAGS || NONMATCHING
, mEnableAnaLeftXP(false)
, mEnableAnaLeftYP(false)
#endif
, mEnableAnaRightXP(false)
, mEnableAnaRightYP(false)
, mAnaLeftTriggerP(0.f)
, mAnaRightTriggerP(0.f)
, mA(ctx.GetOsKeyState(0x6d).IsPressed())
, mB(ctx.GetOsKeyState(0x6e).IsPressed())
, mX(ctx.GetOsKeyState(0x84).IsPressed())
, mY(ctx.GetOsKeyState(0x85).IsPressed())
, mZ(ctx.GetOsKeyState(0x16).IsPressed())
, mL(ctx.GetOsKeyState(0x78).IsPressed())
, mR(ctx.GetOsKeyState(0x7e).IsPressed())
, mDPUp(ctx.GetOsKeyState(0x1c).IsPressed())
, mDPRight(ctx.GetOsKeyState(0x1d).IsPressed())
, mDPDown(ctx.GetOsKeyState(0x1e).IsPressed())
, mDPLeft(ctx.GetOsKeyState(0x1b).IsPressed())
, mStart(ctx.GetOsKeyState(5).IsPressed())
, mPA(ctx.GetOsKeyState(0x6d).JustPressed())
, mPB(ctx.GetOsKeyState(0x6e).JustPressed())
, mPX(ctx.GetOsKeyState(0x84).JustPressed())
, mPY(ctx.GetOsKeyState(0x85).JustPressed())
, mPZ(ctx.GetOsKeyState(0x16).JustPressed())
, mPL(ctx.GetOsKeyState(0x78).JustPressed())
, mPR(ctx.GetOsKeyState(0x7e).JustPressed())
, mPDPUp(ctx.GetOsKeyState(0x1c).JustPressed())
, mPDPRight(ctx.GetOsKeyState(0x1d).JustPressed())
, mPDPDown(ctx.GetOsKeyState(0x1e).JustPressed())
, mPDPLeft(ctx.GetOsKeyState(0x1b).JustPressed())
, mPStart(ctx.GetOsKeyState(5).JustPressed()) {
}

#if RSTL_PRIME1
CFinalInput CFinalInput::ScaleAnalogueSticks(float leftDiv, float rightDiv) const {
  CFinalInput ret = *this;
  ret.mAnaLeftX = CMath::Clamp(-1.f, mAnaLeftX / leftDiv, 1.f);
  ret.mAnaLeftY = CMath::Clamp(-1.f, mAnaLeftY / leftDiv, 1.f);
  ret.mAnaRightX = CMath::Clamp(-1.f, mAnaRightX / rightDiv, 1.f);
  ret.mAnaRightY = CMath::Clamp(-1.f, mAnaRightY / rightDiv, 1.f);
  return ret;
}

CFinalInputInfo::CFinalInputInfo(const CFinalInput& input)
: mDt(input.mDt), mControllerIdx(input.mControllerIdx) {
  mAnalog[0] = input.mAnaLeftX;
  mAnalog[1] = input.mAnaLeftY;
  mAnalog[2] = input.mAnaRightX;
  mAnalog[3] = input.mAnaRightY;
  mAnalog[4] = input.mAnaLeftTrigger;
  mAnalog[5] = input.mAnaRightTrigger;
  mAnalog[6] = input.mAnaLeftTriggerP;
  mAnalog[7] = input.mAnaRightTriggerP;
  mButtons[0] = input.mEnableAnaLeftXP;
  mButtons[1] = input.mEnableAnaLeftYP;
  mButtons[2] = input.mEnableAnaRightXP;
  mButtons[3] = input.mEnableAnaRightYP;
  mButtons[4] = input.mA;
  mButtons[5] = input.mB;
  mButtons[6] = input.mX;
  mButtons[7] = input.mY;
  mButtons[8] = input.mZ;
  mButtons[9] = input.mL;
  mButtons[10] = input.mR;
  mButtons[11] = input.mDPUp;
  mButtons[12] = input.mDPRight;
  mButtons[13] = input.mDPDown;
  mButtons[14] = input.mDPLeft;
  mButtons[15] = input.mStart;
  mButtons[16] = input.mPA;
  mButtons[17] = input.mPB;
  mButtons[18] = input.mPX;
  mButtons[19] = input.mPY;
  mButtons[20] = input.mPZ;
  mButtons[21] = input.mPL;
  mButtons[22] = input.mPR;
  mButtons[23] = input.mPDPUp;
  mButtons[24] = input.mPDPRight;
  mButtons[25] = input.mPDPDown;
  mButtons[26] = input.mPDPLeft;
  mButtons[27] = input.mPStart;
}

CFinalInput::CFinalInput(const CFinalInputInfo& info, float leftDiv, float rightDiv)
: mDt(info.mDt)
, mControllerIdx(info.mControllerIdx)
, mAnaLeftX(info.mAnalog[0])
, mAnaLeftY(info.mAnalog[1])
, mAnaRightX(info.mAnalog[2])
, mAnaRightY(info.mAnalog[3])
, mAnaLeftTrigger(info.mAnalog[4])
, mAnaRightTrigger(info.mAnalog[5])
, mAnaLeftTriggerP(info.mAnalog[6])
, mAnaRightTriggerP(info.mAnalog[7])
, mEnableAnaLeftXP(info.mButtons[0])
, mEnableAnaLeftYP(info.mButtons[1])
, mEnableAnaRightXP(info.mButtons[2])
, mEnableAnaRightYP(info.mButtons[3])
, mA(info.mButtons[4])
, mB(info.mButtons[5])
, mX(info.mButtons[6])
, mY(info.mButtons[7])
, mZ(info.mButtons[8])
, mL(info.mButtons[9])
, mR(info.mButtons[10])
, mDPUp(info.mButtons[11])
, mDPRight(info.mButtons[12])
, mDPDown(info.mButtons[13])
, mDPLeft(info.mButtons[14])
, mStart(info.mButtons[15])
, mPA(info.mButtons[16])
, mPB(info.mButtons[17])
, mPX(info.mButtons[18])
, mPY(info.mButtons[19])
, mPZ(info.mButtons[20])
, mPL(info.mButtons[21])
, mPR(info.mButtons[22])
, mPDPUp(info.mButtons[23])
, mPDPRight(info.mButtons[24])
, mPDPDown(info.mButtons[25])
, mPDPLeft(info.mButtons[26])
, mPStart(info.mButtons[27])
{
  // The recorded values already include the controller dead zones. Preserve the
  // recorded press edges, independently of the live controller's analog history.
  *this = ScaleAnalogueSticks(leftDiv, rightDiv);
}
#endif

#endif
