#include "rstl/RstlVersions.h"
#include "Kyoto/Input/CDolphinController.hpp"
#if !RSTL_PRIME1
#include "Kyoto/Math/CMath.hpp"
#endif

#include "Kyoto/Alloc/CMemory.hpp"

#if RSTL_PRIME1
#include "dolphin/gba.h"
#endif

#include "dolphin/si.h"

#include "string.h"

CDolphinController::CDolphinController()
: mValidControllers(PAD_CHAN0_BIT | PAD_CHAN1_BIT | PAD_CHAN2_BIT | PAD_CHAN3_BIT)
, mInvalidControllers(0)
, x1cc_(0) {
  static bool sIsInitialized = false;
  if (!sIsInitialized) {
    PADSetSpec(PAD_SPEC_5);
    PADInit();
    sIsInitialized = true;
  }
}

CDolphinController::~CDolphinController() {}

bool CDolphinController::Initialize() {
#if RSTL_PRIME1
  GBAInit();
#endif

  memset(mStatus, 0, sizeof(PADStatus) * 4);
  for (uint i = 0; i < 4; ++i) {
    mGamepadStates[i].SetDeviceIsPresent(false);
    mMotorStates[i] = kMS_StopHard;
    mControllerTypePollTime[i] = 0;
    mControllerTypes[i] = skTypeUnknown;
  }

  PADControlAllMotors((const u32*)mMotorStates);
  Poll();
  return true;
}

void CDolphinController::Poll() {
  ReadDevices();
  ProcessInputData();
}

void CDolphinController::ReadDevices() {
  PADStatus status[4];
  PADRead(status);
#if RSTL_PRIME1
  if (status[0].err == PAD_ERR_NONE) {
#endif

    PADClamp(status);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    memcpy(mStatus, status, sizeof(PADStatus) * 4);
#else
  for (int i = 0; i < 4; ++i) {
    if (status[i].err == 0) {
      memcpy(&mStatus[i], &status[i], sizeof(PADStatus));
#endif

  } else {
#if RSTL_PRIME1
    for (int i = 0; i < 4; ++i) {
#endif

      mStatus[i].err = status[i].err;
    }
  }

  for (int i = 0; i < 4; ++i) {
    uint controller = (PAD_CHAN0_BIT >> i);
#if !RSTL_PRIME1
    mGamepadStates[i].SetDeviceJustDisconnected(false);
#endif

    if (mStatus[i].err != PAD_ERR_NOT_READY) {
      if (mStatus[i].err == PAD_ERR_NONE) {
        mGamepadStates[i].SetDeviceIsPresent(true);
      } else if (mStatus[i].err == PAD_ERR_NO_CONTROLLER) {
        mInvalidControllers |= controller;
#if !RSTL_PRIME1
        if (mGamepadStates[i].DeviceIsPresent()) {
          mGamepadStates[i].SetDeviceJustDisconnected(true);
        }
#endif

        mGamepadStates[i].SetDeviceIsPresent(false);
      }
    }

    if (mControllerTypePollTime[i] != 0) {
      --mControllerTypePollTime[i];
    } else {
      const uint type = SIProbe(i);
      if ((type & (SI_ERROR_NO_RESPONSE | SI_ERROR_UNKNOWN | SI_ERROR_BUSY)) != 0) {
        if (mControllerTypePollTime[i] == 0) {
          mControllerTypes[i] = skTypeUnknown;
        }
      } else {
        mControllerTypePollTime[i] = 60;
        if (type == SI_GC_WAVEBIRD) {
          mControllerTypes[i] = skTypeWavebird;
        } else if (type == SI_GBA) {
          mControllerTypes[i] = skTypeGBA;
        } else if (type == SI_GC_CONTROLLER) {
          mControllerTypes[i] = skTypeStandard;
        }
      }
    }
  }

  if (mInvalidControllers != 0 && PADReset(mInvalidControllers)) {
    mInvalidControllers = 0;
  }
}

void CDolphinController::ProcessInputData() {
  for (int i = 0; i < 4; ++i) {
    if (mGamepadStates[i].DeviceIsPresent()) {
      ProcessAxis(i, kJA_LeftX);
      ProcessAxis(i, kJA_LeftY);
      ProcessAxis(i, kJA_RightX);
      ProcessAxis(i, kJA_RightY);
      ProcessButtons(i);
#if !RSTL_PRIME1
    } else if (mGamepadStates[i].DeviceJustDisconnected()) {
      ClearAxis(i, kJA_LeftX);
      ClearAxis(i, kJA_LeftY);
      ClearAxis(i, kJA_RightX);
      ClearAxis(i, kJA_RightY);
      ClearButtons(i);
#endif

    }
  }
}

#if !RSTL_PRIME1
void CDolphinController::ClearAxis(int controller, EJoyAxis axis) {
  mGamepadStates[controller].GetAxis(axis).SetRelativeValue(0.f);
  mGamepadStates[controller].GetAxis(axis).SetAbsoluteValue(0.f);
}

void CDolphinController::ClearButtons(int controller) {
  for (int i = 0; i < int(kBU_MAX); ++i) {
    CControllerButton& button = mGamepadStates[controller].GetButton(EButton(i));
    button.SetPressEvent(false);
    button.SetReleaseEvent(button.GetIsPressed());
    button.SetIsPressed(false);
  }

  ProcessAnalogButton(0.f, mGamepadStates[controller].GetAnalogButton(kBA_Left));
  ProcessAnalogButton(0.f, mGamepadStates[controller].GetAnalogButton(kBA_Right));
}
#endif

void CDolphinController::ProcessAxis(int controller, EJoyAxis axis) {
  const float maxAxisValue = 1.f / GetAnalogStickMaxValue(axis);
  CControllerAxis& data = mGamepadStates[controller].GetAxis(axis);

  float axisValue = 0.f;
  switch (axis) {
  case kJA_LeftX:
    axisValue = mStatus[controller].stickX;
    break;
  case kJA_LeftY:
    axisValue = mStatus[controller].stickY;
    break;
  case kJA_RightX:
    axisValue = mStatus[controller].substickX;
    break;
  case kJA_RightY:
    axisValue = mStatus[controller].substickY;
#if RSTL_PRIME1
    break;
  default:
#endif

    break;
  }

  float absolute = axisValue * maxAxisValue;
  if (absolute < kAbsoluteMinimum) {
    absolute = kAbsoluteMinimum;
  } else if (absolute > kAbsoluteMaximum) {
    absolute = kAbsoluteMaximum;
  }

  float relativeValue = absolute - data.GetAbsoluteValue();
  if (relativeValue < kRelativeMinimum) {
    relativeValue = kRelativeMinimum;
  } else if (relativeValue > kRelativeMaximum) {
    relativeValue = kRelativeMaximum;
  }
  data.SetRelativeValue(relativeValue);
  data.SetAbsoluteValue(absolute);
}

static ushort mButtonMapping[size_t(kBU_MAX)] = {
    PAD_BUTTON_A,     PAD_BUTTON_B,    PAD_BUTTON_X,  PAD_BUTTON_Y,
    PAD_BUTTON_START, PAD_TRIGGER_Z,   PAD_BUTTON_UP, PAD_BUTTON_RIGHT,
    PAD_BUTTON_DOWN,  PAD_BUTTON_LEFT, PAD_TRIGGER_L, PAD_TRIGGER_R,
};

void CDolphinController::ProcessButtons(int controller) {
  for (int i = 0; i < int(kBU_MAX); ++i) {
    ProcessDigitalButton(controller, mGamepadStates[controller].GetButton(EButton(i)),
                         mButtonMapping[i]);
  }

  ProcessAnalogButton(mStatus[controller].triggerLeft,
                      mGamepadStates[controller].GetAnalogButton(kBA_Left));
  ProcessAnalogButton(mStatus[controller].triggerRight,
                      mGamepadStates[controller].GetAnalogButton(kBA_Right));
}

void CDolphinController::ProcessDigitalButton(int controller, CControllerButton& button,
                                              ushort mapping) {
  bool btnPressed = (mStatus[controller].button & mapping);
  button.SetPressEvent(PADButtonDown(button.GetIsPressed(), btnPressed));
  button.SetReleaseEvent(PADButtonUp(button.GetIsPressed(), btnPressed));
  button.SetIsPressed(btnPressed);
}

void CDolphinController::ProcessAnalogButton(float value, CControllerAxis& axis) {
  value *= 1.f / 150.f;
  if (value > kAbsoluteMaximum) {
    value = kAbsoluteMaximum;
  }

  float relative = value - axis.GetAbsoluteValue();
  if (relative > kRelativeMaximum) {
    relative = kRelativeMaximum;
  }

  axis.SetRelativeValue(relative);
  axis.SetAbsoluteValue(value);
}

uint CDolphinController::GetDeviceCount() const { return 4; }

#if !RSTL_PRIME1
CControllerGamepadData& CDolphinController::GetGamepadData(int controller) {
  return mGamepadStates[controller];
}
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION < RSTL_R3IJ
CControllerGamepadData& CDolphinController::GetGamepadData(int controller) {
  return mGamepadStates[controller];
}

uint CDolphinController::GetControllerType(int controller) const {
  return mControllerTypes[controller];
}

void CDolphinController::SetMotorState(EIOPort port, EMotorState state) {
  mMotorStates[port] = state;
  PADControlAllMotors((const u32*)mMotorStates);
}
#endif
#else
uint CDolphinController::GetControllerType(int controller) const {
  return mControllerTypes[controller];
}

void CDolphinController::SetMotorState(EIOPort port, EMotorState state) {
  mMotorStates[port] = state;
  PADControlAllMotors((const u32*)mMotorStates);
}
#endif

float CDolphinController::GetAnalogStickMaxValue(EJoyAxis axis) const {
  switch (axis) {
  case kJA_LeftX:
  case kJA_LeftY:
    return 72.0f;

  case kJA_RightX:
  case kJA_RightY:
    return 59.0f;

  default:
    return 0.0f;
  }
}

const uint CDolphinController::skTypeUnknown = 'UNKN';
const uint CDolphinController::skTypeStandard = 'STND';
const uint CDolphinController::skTypeGBA = 'GBA_';
const uint CDolphinController::skTypeWavebird = 'WAVE';

