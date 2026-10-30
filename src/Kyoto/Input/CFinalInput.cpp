#include "rstl/RstlVersions.h"
#include "Kyoto/Input/CFinalInput.hpp"
#if RSTL_PRIME1
#include "Kyoto/Input/CFinalInputInfo.hpp"
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_R3IJ

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
  case kPC_StickLeft: {
    const float value = mControllerData.GetAxis(0).GetAbsoluteValue();
    return value < 0.f ? -value : 0.f;
  }
  case kPC_StickRight: {
    const float value = mControllerData.GetAxis(0).GetAbsoluteValue();
    return value > 0.f ? value : 0.f;
  }
  case kPC_StickDown: {
    const float value = mControllerData.GetAxis(1).GetAbsoluteValue();
    return value < 0.f ? -value : 0.f;
  }
  case kPC_StickDiagonal3Negative: {
    const float value = mControllerData.GetAxis(3).GetAbsoluteValue();
    return value < 0.f ? -value : 0.f;
  }
  case kPC_StickDiagonal2Positive: {
    const float value = mControllerData.GetAxis(2).GetAbsoluteValue();
    return value > 0.f ? -value : 0.f;
  }
  case kPC_StickUp: {
    const float value = mControllerData.GetAxis(1).GetAbsoluteValue();
    return value > 0.f ? value : 0.f;
  }
  case kPC_StickDiagonal2Negative: {
    const float value = mControllerData.GetAxis(2).GetAbsoluteValue();
    return value < 0.f ? value : 0.f;
  }
  case kPC_StickDiagonal3Positive: {
    const float value = mControllerData.GetAxis(3).GetAbsoluteValue();
    return value > 0.f ? value : 0.f;
  }
  case kPC_NunchukRollNegative: {
    const float value = mControllerData.GetAxis(4).GetAbsoluteValue();
    return value < 0.f ? -value : 0.f;
  }
  case kPC_NunchukRollPositive: {
    const float value = mControllerData.GetAxis(4).GetAbsoluteValue();
    return value > 0.f ? value : 0.f;
  }
  case kPC_NunchukPitchNegative: {
    const float value = mControllerData.GetAxis(5).GetAbsoluteValue();
    return value < 0.f ? -value : 0.f;
  }
  case kPC_NunchukPitchPositive: {
    const float value = mControllerData.GetAxis(5).GetAbsoluteValue();
    return value > 0.f ? value : 0.f;
  }
  case kPC_PointerLeft: {
    const float value = mControllerData.GetAxis(6).GetAbsoluteValue();
    return value < 0.f ? -value : 0.f;
  }
  case kPC_PointerRight: {
    const float value = mControllerData.GetAxis(6).GetAbsoluteValue();
    return value > 0.f ? value : 0.f;
  }
  case kPC_PointerDown: {
    const float value = mControllerData.GetAxis(7).GetAbsoluteValue();
    return value > 0.f ? value : 0.f;
  }
  case kPC_PointerUp: {
    const float value = mControllerData.GetAxis(7).GetAbsoluteValue();
    return value < 0.f ? -value : 0.f;
  }
  case kPC_WiimoteRollNegative: {
    const float value = mControllerData.GetAxis(8).GetAbsoluteValue();
    return value < 0.f ? -value : 0.f;
  }
  case kPC_WiimoteRollPositive: {
    const float value = mControllerData.GetAxis(8).GetAbsoluteValue();
    return value > 0.f ? value : 0.f;
  }
  case kPC_WiimotePitchNegative: {
    const float value = mControllerData.GetAxis(9).GetAbsoluteValue();
    return value < 0.f ? -value : 0.f;
  }
  case kPC_WiimotePitchPositive: {
    const float value = mControllerData.GetAxis(9).GetAbsoluteValue();
    return value > 0.f ? value : 0.f;
  }
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

#else

#include "Kyoto/Basics/COsContext.hpp"
#include "Kyoto/Input/CControllerGamepadData.hpp"
#include "Kyoto/Math/CMath.hpp"

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
, mB24_A(false)
, mB25_B(false)
, mB26_X(false)
, mB27_Y(false)
, mB28_Z(false)
, mB29_L(false)
, mB30_R(false)
, mB31_DPUp(false)
, mB24_DPRight(false)
, mB25_DPDown(false)
, mB26_DPLeft(false)
, mB27_Start(false)
, mB28_PA(false)
, mB29_PB(false)
, mB30_PX(false)
, mB31_PY(false)
, mB24_PZ(false)
, mB25_PL(false)
, mB26_PR(false)
, mB27_PDPUp(false)
, mB28_PDPRight(false)
, mB29_PDPDown(false)
, mB30_PDPLeft(false)
, mB31_PStart(false) {}

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
, mB24_A(data.GetButton(kBU_A).GetIsPressed())
, mB25_B(data.GetButton(kBU_B).GetIsPressed())
, mB26_X(data.GetButton(kBU_X).GetIsPressed())
, mB27_Y(data.GetButton(kBU_Y).GetIsPressed())
, mB28_Z(data.GetButton(kBU_Z).GetIsPressed())
, mB29_L(data.GetButton(kBU_L).GetIsPressed())
, mB30_R(data.GetButton(kBU_R).GetIsPressed())
, mB31_DPUp(data.GetButton(kBU_Up).GetIsPressed())
, mB24_DPRight(data.GetButton(kBU_Right).GetIsPressed())
, mB25_DPDown(data.GetButton(kBU_Down).GetIsPressed())
, mB26_DPLeft(data.GetButton(kBU_Left).GetIsPressed())
, mB27_Start(data.GetButton(kBU_Start).GetIsPressed())
, mB28_PA(data.GetButton(kBU_A).GetPressEvent())
, mB29_PB(data.GetButton(kBU_B).GetPressEvent())
, mB30_PX(data.GetButton(kBU_X).GetPressEvent())
, mB31_PY(data.GetButton(kBU_Y).GetPressEvent())
, mB24_PZ(data.GetButton(kBU_Z).GetPressEvent())
, mB25_PL(data.GetButton(kBU_L).GetPressEvent())
, mB26_PR(data.GetButton(kBU_R).GetPressEvent())
, mB27_PDPUp(data.GetButton(kBU_Up).GetPressEvent())
, mB28_PDPRight(data.GetButton(kBU_Right).GetPressEvent())
, mB29_PDPDown(data.GetButton(kBU_Down).GetPressEvent())
, mB30_PDPLeft(data.GetButton(kBU_Left).GetPressEvent())
, mB31_PStart(data.GetButton(kBU_Start).GetPressEvent()) {
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

CFinalInput::CFinalInput(int channel, float dt, const COsContext& ctx)
: mDt(dt)
, mControllerIdx(channel)
, mAnaLeftX(0.f)
, mAnaLeftY(0.f)
, mAnaRightX(0.f)
, mAnaRightY(0.f)
, mAnaLeftTrigger(0.f)
, mAnaRightTrigger(0.f)
#if NONMATCHING
, mEnableAnaLeftXP(false)
, mEnableAnaLeftYP(false)
#endif
, mEnableAnaRightXP(false)
, mEnableAnaRightYP(false)
, mAnaLeftTriggerP(0.f)
, mAnaRightTriggerP(0.f)
, mB24_A(ctx.GetOsKeyState(0x6d).IsPressed())
, mB25_B(ctx.GetOsKeyState(0x6e).IsPressed())
, mB26_X(ctx.GetOsKeyState(0x84).IsPressed())
, mB27_Y(ctx.GetOsKeyState(0x85).IsPressed())
, mB28_Z(ctx.GetOsKeyState(0x16).IsPressed())
, mB29_L(ctx.GetOsKeyState(0x78).IsPressed())
, mB30_R(ctx.GetOsKeyState(0x7e).IsPressed())
, mB31_DPUp(ctx.GetOsKeyState(0x1c).IsPressed())
, mB24_DPRight(ctx.GetOsKeyState(0x1d).IsPressed())
, mB25_DPDown(ctx.GetOsKeyState(0x1e).IsPressed())
, mB26_DPLeft(ctx.GetOsKeyState(0x1b).IsPressed())
, mB27_Start(ctx.GetOsKeyState(5).IsPressed())
, mB28_PA(ctx.GetOsKeyState(0x6d).JustPressed())
, mB29_PB(ctx.GetOsKeyState(0x6e).JustPressed())
, mB30_PX(ctx.GetOsKeyState(0x84).JustPressed())
, mB31_PY(ctx.GetOsKeyState(0x85).JustPressed())
, mB24_PZ(ctx.GetOsKeyState(0x16).JustPressed())
, mB25_PL(ctx.GetOsKeyState(0x78).JustPressed())
, mB26_PR(ctx.GetOsKeyState(0x7e).JustPressed())
, mB27_PDPUp(ctx.GetOsKeyState(0x1c).JustPressed())
, mB28_PDPRight(ctx.GetOsKeyState(0x1d).JustPressed())
, mB29_PDPDown(ctx.GetOsKeyState(0x1e).JustPressed())
, mB30_PDPLeft(ctx.GetOsKeyState(0x1b).JustPressed())
, mB31_PStart(ctx.GetOsKeyState(5).JustPressed()) {
}

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
  mButtons[4] = input.mB24_A;
  mButtons[5] = input.mB25_B;
  mButtons[6] = input.mB26_X;
  mButtons[7] = input.mB27_Y;
  mButtons[8] = input.mB28_Z;
  mButtons[9] = input.mB29_L;
  mButtons[10] = input.mB30_R;
  mButtons[11] = input.mB31_DPUp;
  mButtons[12] = input.mB24_DPRight;
  mButtons[13] = input.mB25_DPDown;
  mButtons[14] = input.mB26_DPLeft;
  mButtons[15] = input.mB27_Start;
  mButtons[16] = input.mB28_PA;
  mButtons[17] = input.mB29_PB;
  mButtons[18] = input.mB30_PX;
  mButtons[19] = input.mB31_PY;
  mButtons[20] = input.mB24_PZ;
  mButtons[21] = input.mB25_PL;
  mButtons[22] = input.mB26_PR;
  mButtons[23] = input.mB27_PDPUp;
  mButtons[24] = input.mB28_PDPRight;
  mButtons[25] = input.mB29_PDPDown;
  mButtons[26] = input.mB30_PDPLeft;
  mButtons[27] = input.mB31_PStart;
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
, mB24_A(info.mButtons[4])
, mB25_B(info.mButtons[5])
, mB26_X(info.mButtons[6])
, mB27_Y(info.mButtons[7])
, mB28_Z(info.mButtons[8])
, mB29_L(info.mButtons[9])
, mB30_R(info.mButtons[10])
, mB31_DPUp(info.mButtons[11])
, mB24_DPRight(info.mButtons[12])
, mB25_DPDown(info.mButtons[13])
, mB26_DPLeft(info.mButtons[14])
, mB27_Start(info.mButtons[15])
, mB28_PA(info.mButtons[16])
, mB29_PB(info.mButtons[17])
, mB30_PX(info.mButtons[18])
, mB31_PY(info.mButtons[19])
, mB24_PZ(info.mButtons[20])
, mB25_PL(info.mButtons[21])
, mB26_PR(info.mButtons[22])
, mB27_PDPUp(info.mButtons[23])
, mB28_PDPRight(info.mButtons[24])
, mB29_PDPDown(info.mButtons[25])
, mB30_PDPLeft(info.mButtons[26])
, mB31_PStart(info.mButtons[27])
{
  // The recorded values already include the controller dead zones. Preserve the
  // recorded press edges, independently of the live controller's analog history.
  *this = ScaleAnalogueSticks(leftDiv, rightDiv);
}

#endif
#else
#include "Kyoto/Input/CControllerGamepadData.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Basics/COsContext.hpp"
#endif

#if !RSTL_PRIME1
const float CFinalInput::kInput_AnalogOnThreshhold = 0.7f;
const float CFinalInput::kInput_AnalogTriggerOnThreshhold = 0.05f;

#if RSTL_VERSION < RSTL_R32J
CFinalInput::CFinalInput()
: mDt(0.f)
, mControllerIdx(0)
, mAnaLeftX(0.f)
, mAnaLeftY(0.f)
, mAnaRightX(0.f)
, mAnaRightY(0.f)
, mAnaLeftTrigger(0.f)
, mAnaRightTrigger(0.f)
, mB24_enableAnaLeftXP(false)
, mB25_enableAnaLeftYP(false)
, mB26_enableAnaRightXP(false)
, mB27_enableAnaRightYP(false)
, mAnaLeftTriggerP(0.f)
, mAnaRightTriggerP(0.f)
, mB28_A(false)
, mB29_B(false)
, mB30_X(false)
, mB31_Y(false)
, mB24_Z(false)
, mB25_L(false)
, mB26_R(false)
, mB27_DPUp(false)
, mB28_DPRight(false)
, mB29_DPDown(false)
, mB30_DPLeft(false)
, mB31_Start(false)
, mB24_PA(false)
, mB25_PB(false)
, mB26_PX(false)
, mB27_PY(false)
, mB28_PZ(false)
, mB29_PL(false)
, mB30_PR(false)
, mB31_PDPUp(false)
, mB24_PDPRight(false)
, mB25_PDPDown(false)
, mB26_PDPLeft(false)
, mB27_PStart(false) {}

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
, mB24_enableAnaLeftXP(false)
, mB25_enableAnaLeftYP(false)
, mB26_enableAnaRightXP(false)
, mB27_enableAnaRightYP(false)
, mB28_A(data.GetButton(kBU_A).GetIsPressed())
, mB29_B(data.GetButton(kBU_B).GetIsPressed())
, mB30_X(data.GetButton(kBU_X).GetIsPressed())
, mB31_Y(data.GetButton(kBU_Y).GetIsPressed())
, mB24_Z(data.GetButton(kBU_Z).GetIsPressed())
, mB25_L(data.GetButton(kBU_L).GetIsPressed())
, mB26_R(data.GetButton(kBU_R).GetIsPressed())
, mB27_DPUp(data.GetButton(kBU_Up).GetIsPressed())
, mB28_DPRight(data.GetButton(kBU_Right).GetIsPressed())
, mB29_DPDown(data.GetButton(kBU_Down).GetIsPressed())
, mB30_DPLeft(data.GetButton(kBU_Left).GetIsPressed())
, mB31_Start(data.GetButton(kBU_Start).GetIsPressed())
, mB24_PA(data.GetButton(kBU_A).GetPressEvent())
, mB25_PB(data.GetButton(kBU_B).GetPressEvent())
, mB26_PX(data.GetButton(kBU_X).GetPressEvent())
, mB27_PY(data.GetButton(kBU_Y).GetPressEvent())
, mB28_PZ(data.GetButton(kBU_Z).GetPressEvent())
, mB29_PL(data.GetButton(kBU_L).GetPressEvent())
, mB30_PR(data.GetButton(kBU_R).GetPressEvent())
, mB31_PDPUp(data.GetButton(kBU_Up).GetPressEvent())
, mB24_PDPRight(data.GetButton(kBU_Right).GetPressEvent())
, mB25_PDPDown(data.GetButton(kBU_Down).GetPressEvent())
, mB26_PDPLeft(data.GetButton(kBU_Left).GetPressEvent())
, mB27_PStart(data.GetButton(kBU_Start).GetPressEvent()) {
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
    mB25_enableAnaLeftYP = true;
  } else if (mAnaLeftY > 0.7f && sIsAnalogPressed[mControllerIdx][0]) {
    mB25_enableAnaLeftYP = false;
  } else if (mAnaLeftY < -0.7f && !sIsAnalogPressed[mControllerIdx][0]) {
    mB25_enableAnaLeftYP = true;
    sIsAnalogPressed[mControllerIdx][0] = true;
  } else if (mAnaLeftY < -0.7f && sIsAnalogPressed[mControllerIdx][0]) {
    mB25_enableAnaLeftYP = false;
  } else if (fabsf(mAnaLeftY) < 0.7f) {
    mB25_enableAnaLeftYP = false;
    sIsAnalogPressed[mControllerIdx][0] = false;
  }

  if (mAnaLeftX > 0.7f && !sIsAnalogPressed[mControllerIdx][1]) {
    mB24_enableAnaLeftXP = true;
    sIsAnalogPressed[mControllerIdx][1] = true;
  } else if (mAnaLeftX > 0.7f && sIsAnalogPressed[mControllerIdx][1]) {
    mB24_enableAnaLeftXP = false;
  } else if (mAnaLeftX < -0.7f && !sIsAnalogPressed[mControllerIdx][1]) {
    sIsAnalogPressed[mControllerIdx][1] = true;
    mB24_enableAnaLeftXP = true;
  } else if (mAnaLeftX < -0.7f && sIsAnalogPressed[mControllerIdx][1]) {
    mB24_enableAnaLeftXP = false;
  } else if (fabsf(mAnaLeftX) < 0.7f) {
    mB24_enableAnaLeftXP = false;
    sIsAnalogPressed[mControllerIdx][1] = false;
  }

  if (mAnaRightY > 0.7f && !sIsAnalogPressed[mControllerIdx][2]) {
    sIsAnalogPressed[mControllerIdx][2] = true;
    mB27_enableAnaRightYP = true;
  } else if (mAnaRightY > 0.7f && sIsAnalogPressed[mControllerIdx][2]) {
    mB27_enableAnaRightYP = false;
  } else if (mAnaRightY < -0.7f && !sIsAnalogPressed[mControllerIdx][2]) {
    mB27_enableAnaRightYP = true;
    sIsAnalogPressed[mControllerIdx][2] = true;
  } else if (mAnaRightY < -0.7f && sIsAnalogPressed[mControllerIdx][2]) {
    mB27_enableAnaRightYP = false;
  } else if (fabsf(mAnaRightY) < 0.7f) {
    mB27_enableAnaRightYP = false;
    sIsAnalogPressed[mControllerIdx][2] = false;
  }

  if (mAnaRightX > 0.7f && !sIsAnalogPressed[mControllerIdx][3]) {
    mB26_enableAnaRightXP = true;
    sIsAnalogPressed[mControllerIdx][3] = true;
  } else if (mAnaRightX > 0.7f && sIsAnalogPressed[mControllerIdx][3]) {
    mB26_enableAnaRightXP = false;
  } else if (mAnaRightX < -0.7f && !sIsAnalogPressed[mControllerIdx][3]) {
    sIsAnalogPressed[mControllerIdx][3] = true;
    mB26_enableAnaRightXP = true;
  } else if (mAnaRightX < -0.7f && sIsAnalogPressed[mControllerIdx][3]) {
    mB26_enableAnaRightXP = false;
  } else if (fabsf(mAnaRightX) < 0.7f) {
    mB26_enableAnaRightXP = false;
    sIsAnalogPressed[mControllerIdx][3] = false;
  }
}

CFinalInput CFinalInput::ScaleAnalogueSticks(float leftDiv, float rightDiv) const {
  CFinalInput ret = *this;
  ret.mAnaLeftX = CMath::Clamp(-1.f, mAnaLeftX / leftDiv, 1.f);
  ret.mAnaLeftY = CMath::Clamp(-1.f, mAnaLeftY / leftDiv, 1.f);
  ret.mAnaRightX = CMath::Clamp(-1.f, mAnaRightX / rightDiv, 1.f);
  ret.mAnaRightY = CMath::Clamp(-1.f, mAnaRightY / rightDiv, 1.f);
  return ret;
}

CFinalInput::CFinalInput(int channel, float dt, const COsContext& ctx)
: mDt(dt)
, mControllerIdx(channel)
, mAnaLeftX(0.f)
, mAnaLeftY(0.f)
, mAnaRightX(0.f)
, mAnaRightY(0.f)
, mAnaLeftTrigger(0.f)
, mAnaRightTrigger(0.f)
, mB24_enableAnaLeftXP(false)
, mB25_enableAnaLeftYP(false)
, mB26_enableAnaRightXP(false)
, mB27_enableAnaRightYP(false)
, mAnaLeftTriggerP(0.f)
, mAnaRightTriggerP(0.f)
, mB28_A(ctx.GetOsKeyState(0x6d).IsPressed())
, mB29_B(ctx.GetOsKeyState(0x6e).IsPressed())
, mB30_X(ctx.GetOsKeyState(0x84).IsPressed())
, mB31_Y(ctx.GetOsKeyState(0x85).IsPressed())
, mB24_Z(ctx.GetOsKeyState(0x16).IsPressed())
, mB25_L(ctx.GetOsKeyState(0x78).IsPressed())
, mB26_R(ctx.GetOsKeyState(0x7e).IsPressed())
, mB27_DPUp(ctx.GetOsKeyState(0x1c).IsPressed())
, mB28_DPRight(ctx.GetOsKeyState(0x1d).IsPressed())
, mB29_DPDown(ctx.GetOsKeyState(0x1e).IsPressed())
, mB30_DPLeft(ctx.GetOsKeyState(0x1b).IsPressed())
, mB31_Start(ctx.GetOsKeyState(5).IsPressed())
, mB24_PA(ctx.GetOsKeyState(0x6d).JustPressed())
, mB25_PB(ctx.GetOsKeyState(0x6e).JustPressed())
, mB26_PX(ctx.GetOsKeyState(0x84).JustPressed())
, mB27_PY(ctx.GetOsKeyState(0x85).JustPressed())
, mB28_PZ(ctx.GetOsKeyState(0x16).JustPressed())
, mB29_PL(ctx.GetOsKeyState(0x78).JustPressed())
, mB30_PR(ctx.GetOsKeyState(0x7e).JustPressed())
, mB31_PDPUp(ctx.GetOsKeyState(0x1c).JustPressed())
, mB24_PDPRight(ctx.GetOsKeyState(0x1d).JustPressed())
, mB25_PDPDown(ctx.GetOsKeyState(0x1e).JustPressed())
, mB26_PDPLeft(ctx.GetOsKeyState(0x1b).JustPressed())
, mB27_PStart(ctx.GetOsKeyState(5).JustPressed()) {}

#else
CFinalInput::CFinalInput(int channel, float dt, const CControllerData& data, uint inputType)
: mDt(dt), mControllerIdx(channel), mInputType(inputType), mControllerData(data) {}
#endif
#endif
