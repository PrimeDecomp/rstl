#include "rstl/RstlVersions.h"
#include "Kyoto/Input/CRevolutionController.hpp"

#if RSTL_CORRUPTION
#include "Kyoto/Alloc/Assert.hpp"
#else
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Basics/CInterruptGuard.hpp"
#endif
#include "Kyoto/Input/CFinalInput.hpp"
#if !RSTL_CORRUPTION
#include "Kyoto/Input/CInputFilter.hpp"
#include "Kyoto/Input/CRevolutionMotionProcessor.hpp"
#endif
#include "Kyoto/Math/CMath.hpp"

#include "dolphin/vi.h"
#if RSTL_CORRUPTION
#include <revolution/kpad.h>
#include <revolution/wpad.h>
#else
#include "revolution/os.h"
#endif

#if RSTL_CORRUPTION


// KPAD hold masks per EButton. Core buttons use the Core/FS formats' bits and GameCube
// buttons the Dolphin format's; stick directions and triggers are filled from axes.
static uint sButtonMasks[kBU_MAX] = {
    0,      0x1,    0x2,    0x4,    0x8,    0x10,   0x20,   0x100,  0x200,  0x400, 0x800,
    0x1000, 0,      0,      0,      0,      0x2000, 0x4000, 0,      0,      0,     0,
    0x100,  0x200,  0x400,  0x800,  0x1000, 0x10,   0x40,   0x20,   0x8,    0x2,   0x4,
    0x1,    0x40,   0x20,   0,      0,      0,      0,      0,      0,      0,     0,
    0,      0,      0,      0,      0,      0,      0,
#else
static CRevolutionController* sRevolutionController;
static uint sButtonMasks[64] = {
    0, 0x8000, 0x1000, 0x800, 0x400,  0x100,  0x200, 0x10, 0x8,  0x4, 0x2, 0x1, 0xf, 0, 0, 0, 0,
    0, 0,      0,      0,     0x2000, 0x4000, 0,     0,    0,    0,   0,   0,   0,   0, 0, 0, 0,
    0, 0x100,  0x200,  0x400, 0x800,  0x1000, 0x10,  0x40, 0x20, 0x8, 0x2, 0x4, 0x1, 0, 0, 0, 0,
    0, 0,      0,      0,     0,      0,      0,     0,    0,    0,   0,   0,   0,
};
static uint sMotionButtonMasks[16] = {
    0x1,     0x2,     0x4,     0x8,     0x10,     0x20,     0x40,     0x80,
    0x10000, 0x20000, 0x40000, 0x80000, 0x100000, 0x200000, 0x400000, 0x800000,
};
static uint sSwingButtonMasks[12] = {
    0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x10000, 0x20000, 0x40000, 0x80000, 0x100000, 0x200000,
#endif
};

#if !RSTL_CORRUPTION
void* CRevolutionController::AllocateWpadMemory(u32 size) {
  return CMemory::Alloc(size, static_cast< IAllocator::EHint >(6));
}

uchar CRevolutionController::FreeWpadMemory(void* memory) {
  CMemory::Free(memory);
  return 1;
}

#endif
CRevolutionController::CRevolutionController()
#if RSTL_CORRUPTION
: mStatus(4, KPADStatus())
, mGamepadStates(4, CControllerGamepadData())
, mMotorStates(4, kMS_StopHard)
, mDeviceTypes(4, 0)
, mTrackingStates(4, 0)
, mTrackedPositions(4, CVector2f::Zero())
, mTrackedDistances(4, 0.f)
, mConsecutiveValidFrames(4, 0)
, mConsecutiveInvalidFrames(4, 0) {
#else
: mStatus(KPADStatus())
, mControllerTypes(kDT_Disconnected)
, mInput(CControllerData())
, mPointerRecenterMode(kPRM_Hold)
, mPointerReacquireFrames(4, 0)
, mMotionProcessors(4)
, mUnknownInputData(SUnknownInputData())
, mWpadInfo(4)
, mWpadInfoBuf(4)
, mInfoPollTimers(4)
, mMotionIdleTimes(4)
, mButtonIdleTimes(4)
, mPendingConnectionEvents(kCE_None)
, mPendingExtensionEvents(kEE_None)
, mPointerFilterX(4)
, mPointerFilterY(4)
, mMotorEnabledFlags(0xf)
, mPointerMinDistance(0.75f)
, mPointerMaxDistance(4.f)
, mPointerMinScale(1.3f)
, mPointerMaxScale(1.f)
#if RSTL_CONTROLLER_CHANNEL_LIMIT
, mMaxControllers(1)
#endif
#if RSTL_CONTROLLER_EXTENDED_VIRTUALS
, mAcceptAdditionalConnections(false)
#endif
{
  sRevolutionController = this;
#endif
  Initialize();
}

#if RSTL_CORRUPTION
CRevolutionController::~CRevolutionController() {}
#else
CRevolutionController::~CRevolutionController() {
  sRevolutionController = nullptr;
  for (int i = 0; i < 4; ++i) {
    WPADSetConnectCallback(i, nullptr);
    WPADSetExtensionCallback(i, nullptr);
  }
}

void CRevolutionController::Poll() {}
#endif

uint CRevolutionController::GetDeviceCount() const { return 4; }

#if RSTL_CORRUPTION
CControllerGamepadData& CRevolutionController::GetGamepadData(int controller) {
  return mGamepadStates[controller];
#else
CControllerData& CRevolutionController::GetInput(uint channel) { return mInput[channel]; }

IController::EDeviceType CRevolutionController::GetControllerType(int channel) const {
  return mControllerTypes[channel];
#endif
}

#if RSTL_CORRUPTION
uint CRevolutionController::GetControllerType(int controller) const {
  switch (mStatus[controller].dev_type) {
  case WPAD_DEV_CORE:
    return kControllerTypeRevn;
  case WPAD_DEV_FREESTYLE:
    return kControllerTypeChak;
  case WPAD_DEV_DOLPHIN:
    return kControllerTypeCube;
  default:
    return kControllerTypeUnknown;
#else
void CRevolutionController::QueueConnectionEvent(int channel, EConnectionEvent event) {
  mPendingConnectionEvents[channel] = event;
}

void CRevolutionController::QueueExtensionEvent(int channel, EExtensionEvent event) {
  mPendingExtensionEvents[channel] = event;
}

#if RSTL_CONTROLLER_CHANNEL_LIMIT
void CRevolutionController::SetMaxControllers(int count) {
  mMaxControllers = CMath::Max(1, count);
  if (mMaxControllers > 1 && !mAcceptAdditionalConnections) {
    WPADSetAcceptConnection(true);
  }
}
#endif

void CRevolutionController::ProcessConnectionEvents() {
  rstl::reserved_vector< EConnectionEvent, 4 > connections(4);
  rstl::reserved_vector< EExtensionEvent, 4 > extensions(4);
  CRestorableInterruptGuard interrupts;
  for (int i = 0; i < 4; ++i) {
    connections[i] = mPendingConnectionEvents[i];
    extensions[i] = mPendingExtensionEvents[i];
    mPendingConnectionEvents[i] = kCE_None;
    mPendingExtensionEvents[i] = kEE_None;
  }
  interrupts.Restore();

  for (int i = 0; i < 4; ++i) {
    switch (connections[i]) {
    case kCE_Connected:
      InitializeController(i);
      if (GetMaxControllers() == 1 && i == 0 && !AcceptsAdditionalConnections()) {
        WPADSetAcceptConnection(false);
      }
      break;
    case kCE_Disconnected:
      if (((GetMaxControllers() == 1 && i == 0) || GetMaxControllers() == 4) &&
          !AcceptsAdditionalConnections()) {
        WPADSetAcceptConnection(true);
      }
      break;
    case kCE_Rejected:
      WPADDisconnect(i);
      break;
    }
  }
  for (int i = 0; i < 4; ++i) {
    switch (extensions[i]) {
    case kEE_Wiimote:
      InitializeController(i);
      break;
    case kEE_Nunchuk:
      InitializeController(i);
      break;
    }
#endif
  }
}

#if RSTL_CORRUPTION
void CRevolutionController::Poll() {
  for (int i = 0; i < 4; ++i) {
    u32 type = WPAD_DEV_UNKNOWN;
    const s32 result = WPADProbe(i, &type);
    switch (result) {
    case WPAD_ERR_NONE:
      if (type != mDeviceTypes[i]) {
        switch (type) {
        case WPAD_DEV_CORE:
          WPADSetDataFormat(i, WPAD_FMT_CORE);
          rs_debugger_printf("Controller %d changed to CORE\n", i);
#else
void CRevolutionController::Update(float dt) {
  KPADStatus samples[16];
  KPADUnifiedWpadStatus rawSamples[16];
  ProcessConnectionEvents();
  for (int channel = 0; channel < kActiveChannelCount; ++channel) {
    if (mInput[channel].DeviceIsPresent()) {
      if (mControllerTypes[channel] != kDT_Unsupported) {
        CRestorableInterruptGuard interrupts;
        const int count = KPADRead(channel, samples, 16);
        if (count != 0 && samples[0].wpad_err == WPAD_ERR_OK) {
          CBasics::CopyMemory(&mStatus[channel], samples, sizeof(KPADStatus));
          mInfoPollTimers[channel] -= dt;
          if (mInfoPollTimers[channel] < 0.f) {
            mInfoPollTimers[channel] = 5.f;
            WPADGetInfoAsync(channel, &mWpadInfoBuf[channel], WpadInfoCallback);
          }
          UpdateIdleTimes(channel, dt);
          if (!mMotionProcessors[channel].null()) {
            KPADGetUnifiedWpadStatus(channel, rawSamples, 16);
            interrupts.Restore();
            if (rawSamples[0].u.core.err == WPAD_ERR_OK &&
                rawSamples[0].u.core.dev == WPAD_DEV_FS && uint(rawSamples[0].fmt - 3) <= 2) {
              for (int i = count - 1; i >= 0; --i) {
                mMotionProcessors[channel]->Update(rawSamples[i].u.fs, samples[i], 0.005f);
              }
              mInput[channel].mMotionMask = mMotionProcessors[channel]->GetMotionMask();
              mInput[channel].mSwingMask = mMotionProcessors[channel]->GetSwingMask();
            }
          }
          ProcessControllerInput(channel);
        }
      }
      if (uint(mControllerTypes[channel] - kDT_Unsupported) <= 1) {
        CBasics::ZeroMemory(&mStatus[channel], sizeof(KPADStatus));
        mStatus[channel].dev_type = 0xff;
        mStatus[channel].wpad_err = 0;
      }
    } else {
      CBasics::ZeroMemory(&mStatus[channel], sizeof(KPADStatus));
      mStatus[channel].dev_type = 0xff;
      mStatus[channel].wpad_err = 0;
    }
  }
}

bool CRevolutionController::IsPointerValid(int channel) const {
  if (IsPointerDevicePresent(channel) && mStatus[channel].dpd_valid_fg > 0) {
    return true;
  }
  return false;
}

bool CRevolutionController::IsPointerDevicePresent(int channel) const {
#if NONMATCHING
  if (channel < 0) {
    return false;
  }
#endif
  if (channel >= 4 || !mInput[channel].DeviceIsPresent()) {
    return false;
  }
  switch (mControllerTypes[channel]) {
  case kDT_Wiimote:
  case kDT_Nunchuk:
  case kDT_Classic:
    return true;
  default:
    return false;
  }
}

CControllerData::EPointerState CRevolutionController::GetPointerState(int channel) const {
  if (IsPointerDevicePresent(channel)) {
    return mInput[channel].GetPointerState();
  }
  return CControllerData::kPS_RecentlyLost;
}

uint CRevolutionController::GetPointerValidFrameCount(int channel) const {
  return mInput[channel].GetPointerValidFrameCount();
}

uint CRevolutionController::GetPointerInvalidFrameCount(int channel) const {
  return mInput[channel].GetPointerInvalidFrameCount();
}

CVector2f CRevolutionController::GetPointerPosition(int channel) const {
  return mInput[channel].GetPointerPosition();
}

void CRevolutionController::ApplyPointerDistanceScale(int channel) {
  const float distance = (mInput[channel].mPointerDistance - mPointerMinDistance) /
                         (mPointerMaxDistance - mPointerMinDistance);
  const float scale = mPointerMinScale +
                      (mPointerMaxScale - mPointerMinScale) * CMath::FastClamp(0.f, distance, 1.f);
  mStatus[channel].pos.x = CMath::FastLimit(mStatus[channel].pos.x * scale, 1.f);
}

void CRevolutionController::UpdatePointerState(int channel) {
  const bool valid = IsPointerValid(channel);
  if (valid) {
    ++mInput[channel].mPointerValidFrameCount;
    mInput[channel].mPointerInvalidFrameCount = 0;
  } else {
    ++mInput[channel].mPointerInvalidFrameCount;
    mInput[channel].mPointerValidFrameCount = 0;
  }

  CControllerData& input = mInput[channel];
  switch (input.mPointerState) {
  case CControllerData::kPS_Tracking:
    if (!valid) {
      input.mPointerState = CControllerData::kPS_RecentlyLost;
      mStatus[channel].pos.x = input.mPointerPosition.GetX();
      mStatus[channel].pos.y = input.mPointerPosition.GetY();
    } else {
      const int filterChannel = channel;
      KPADStatus& status = mStatus[channel];
      input.mPointerPosition[0] = mPointerFilterX[filterChannel]->Filter(status.pos.x);
      input.mPointerPosition[1] = mPointerFilterY[filterChannel]->Filter(status.pos.y);
      status.pos.x = input.mPointerPosition.GetX();
      status.pos.y = input.mPointerPosition.GetY();
    }
#endif
    break;
#if RSTL_CORRUPTION
        case WPAD_DEV_FREESTYLE:
          WPADSetDataFormat(i, WPAD_FMT_FREESTYLE);
          rs_debugger_printf("Controller %d changed to CORE + NUNCHAKU\n", i);
#else
  case CControllerData::kPS_RecentlyLost:
    if (!valid) {
      if (input.mPointerInvalidFrameCount >= 120) {
        input.mPointerState = CControllerData::kPS_Lost;
      }
    } else if (input.mPointerValidFrameCount >= 3) {
      input.mPointerState = CControllerData::kPS_Reacquiring;
      input.mPointerValidFrameCount = 0;
      mPointerReacquireFrames[channel] = 0;
    }
    mStatus[channel].pos.x = input.mPointerPosition.GetX();
    mStatus[channel].pos.y = input.mPointerPosition.GetY();
#endif
    break;
#if RSTL_CORRUPTION
        case WPAD_DEV_DOLPHIN:
          WPADSetDataFormat(i, WPAD_FMT_DOLPHIN);
          rs_debugger_printf("Controller %d changed to GAMECUBE\n", i);
#else
  case CControllerData::kPS_Lost: {
    bool recenter = true;
    if (valid && input.mPointerValidFrameCount > 3) {
      input.mPointerState = CControllerData::kPS_Reacquiring;
      recenter = false;
      input.mPointerValidFrameCount = 0;
    }
    if (recenter) {
      switch (uint(mPointerRecenterMode[channel])) {
      case kPRM_Both:
        if (input.mPointerPosition.IsNormalizable()) {
          float magnitude = input.mPointerPosition.Magnitude();
          magnitude *= 0.95f;
          input.mPointerPosition = input.mPointerPosition.AsNormalized() * magnitude;
        }
        break;
      case kPRM_X:
        input.mPointerPosition[0] *= 0.95f;
        break;
      case kPRM_Y:
        input.mPointerPosition[1] *= 0.95f;
#endif
        break;
      }
#if RSTL_CORRUPTION
        if (type != WPAD_DEV_UNKNOWN) {
          rs_debugger_printf("Recalibrating controller %d\n", i);
          WPADRecalibrate(i);
#endif
    }
#if RSTL_CORRUPTION
      }
      mDeviceTypes[i] = type;
      break;
    case WPAD_ERR_INVALID:
      rs_debugger_printf("Controller %d is INVALID\n", i);
      mDeviceTypes[i] = 0;
      break;
    case WPAD_ERR_TRANSFER:
    case WPAD_ERR_BUSY:
      rs_debugger_printf("Controller %d is BUSY\n", i);
      break;
    case WPAD_ERR_NO_CONTROLLER:
      if (mDeviceTypes[i] != 0) {
        rs_debugger_printf("Controller %d was DISCONNECTED\n", i);
        mGamepadStates[i].SetDeviceJustDisconnected(true);
      } else {
        mGamepadStates[i].SetDeviceJustDisconnected(false);
      }
      mDeviceTypes[i] = 0;
#else
    mStatus[channel].pos.x = input.mPointerPosition.GetX();
    mStatus[channel].pos.y = input.mPointerPosition.GetY();
#endif
    break;
  }
#if RSTL_CORRUPTION
    mGamepadStates[i].SetDeviceIsPresent(result == WPAD_ERR_NONE);
#else
  case CControllerData::kPS_Reacquiring:
    if (!valid) {
      input.mPointerState = CControllerData::kPS_Lost;
    } else {
      if (++mPointerReacquireFrames[channel] >= 10u) {
        input.mPointerState = CControllerData::kPS_Tracking;
#endif
      }
#if RSTL_CORRUPTION

  for (int i = 0; i < 4; ++i) {
    KPADRead(i, &mStatus[i], 1);
  }

  ProcessInputData();
}

int CRevolutionController::GetTrackingState(int controller) const {
  if (!mGamepadStates[controller].DeviceIsPresent()) {
    return 1;
  }
  return mTrackingStates[controller];
}

uint CRevolutionController::GetConsecutiveValidTrackingFrames(int controller) const {
  return mConsecutiveValidFrames[controller];
}

uint CRevolutionController::GetConsecutiveInvalidTrackingFrames(int controller) const {
  return mConsecutiveInvalidFrames[controller];
}

CVector2f CRevolutionController::GetTrackedPosition(int controller) const {
  return mTrackedPositions[controller];
}

void CRevolutionController::AdjustTrackedPositionScale(int controller) {
  KPADStatus& status = mStatus[controller];
  status.pos.x *= 1.33f;
  if (status.dpd_valid_fg > 0) {
    mTrackedDistances[controller] = status.dist;
  }

  const float t = CMath::Clamp(0.f, (mTrackedDistances[controller] - 0.5f) / 1.5f, 1.f);
  const float scale = -0.5f * t + 1.5f;
  status.pos.x = CMath::Limit(scale * status.pos.x, 1.f);
  status.pos.y = CMath::Limit(scale * status.pos.y, 1.f);
}

void CRevolutionController::UpdateTrackedPosition(int controller) {
  KPADStatus& status = mStatus[controller];
  bool valid = status.dpd_valid_fg > 0;
  if (status.dev_type == WPAD_DEV_DOLPHIN) {
    valid = true;
  }

  if (valid) {
    ++mConsecutiveValidFrames[controller];
    mConsecutiveInvalidFrames[controller] = 0;
  } else {
    ++mConsecutiveInvalidFrames[controller];
    mConsecutiveValidFrames[controller] = 0;
  }

  switch (mTrackingStates[controller]) {
  case 0:
    if (!valid) {
      mTrackingStates[controller] = 1;
      status.pos.x = mTrackedPositions[controller].GetX();
      status.pos.y = mTrackedPositions[controller].GetY();
    } else {
      mTrackedPositions[controller].SetX(status.pos.x);
      mTrackedPositions[controller].SetY(status.pos.y);
      if (status.dev_type == WPAD_DEV_DOLPHIN) {
        mTrackedPositions[controller].SetX(status.ex_status.gc.substick.x);
        mTrackedPositions[controller].SetY(status.ex_status.gc.substick.y);
      }
    }
    break;

  case 1:
    if (!valid) {
      if (mConsecutiveInvalidFrames[controller] >= 90) {
        mTrackingStates[controller] = 2;
      }
    } else if (mConsecutiveValidFrames[controller] >= 10) {
      mTrackingStates[controller] = 3;
      mConsecutiveValidFrames[controller] = 0;
    }
    status.pos.x = mTrackedPositions[controller].GetX();
    status.pos.y = mTrackedPositions[controller].GetY();
    break;

  case 2: {
    bool decay = true;
    if (valid && mConsecutiveValidFrames[controller] > 10) {
      mTrackingStates[controller] = 3;
      decay = false;
      mConsecutiveValidFrames[controller] = 0;
    }
    if (decay) {
      CVector2f& pos = mTrackedPositions[controller];
      float mag = pos.Magnitude();
      mag *= 0.95f;
      if (pos.IsNormalizable()) {
        pos = pos.AsNormalized() * mag;
      }
    }
    status.pos.x = mTrackedPositions[controller].GetX();
    status.pos.y = mTrackedPositions[controller].GetY();
    break;
  }

  case 3:
    if (!valid) {
      mTrackingStates[controller] = 2;
    } else {
      CVector2f& tracked = mTrackedPositions[controller];
      CVector2f delta(tracked.GetX() - status.pos.x, tracked.GetY() - status.pos.y);
      if (delta.MagSquared() < 0.01f) {
        mTrackingStates[controller] = 0;
      } else {
        delta *= 0.9f;
        status.pos.x += delta.GetX();
        status.pos.y += delta.GetY();
        tracked.SetX(status.pos.x);
        tracked.SetY(status.pos.y);
      }
#else
      const int filterChannel = channel;
      KPADStatus& status = mStatus[channel];
      status.pos.x = mPointerFilterX[filterChannel]->Filter(status.pos.x);
      status.pos.y = mPointerFilterY[filterChannel]->Filter(status.pos.y);
      const float& low = 0.f;
      const float& high = 1.f;
      const float blend = CMath::FastClamp(low, mPointerReacquireFrames[channel] / 10.f, high);
      const float currentX = status.pos.x;
      const float previousX = input.mPointerPosition.GetX();
      input.mPointerPosition[0] = (1.f - blend) * previousX + blend * currentX;
      const float currentY = status.pos.y;
      const float previousY = input.mPointerPosition.GetY();
      input.mPointerPosition[1] = (1.f - blend) * previousY + blend * currentY;
#endif
    }
    break;
  }
}

#if RSTL_CORRUPTION
void CRevolutionController::ProcessInputData() {
  for (int i = 0; i < 4; ++i) {
    if (mGamepadStates[i].DeviceIsPresent()) {
      const u32 devType = mStatus[i].dev_type;
      if (devType == WPAD_DEV_CORE || devType == WPAD_DEV_FREESTYLE) {
        AdjustTrackedPositionScale(i);
#else
void CRevolutionController::ProcessControllerInput(int channel) {
  if (IsPointerDevicePresent(channel) && mStatus[channel].wpad_err == WPAD_ERR_OK) {
    if (IsPointerValid(channel)) {
      mInput[channel].mPointerDistance = mStatus[channel].dist;
      ApplyPointerDistanceScale(channel);
#endif
    }
#if RSTL_CORRUPTION
      UpdateTrackedPosition(i);
#else
    UpdatePointerState(channel);
  }
  if (!mInput[channel].DeviceIsPresent()) {
    return;
  }
  ClearButtonEvents(channel);
  CControllerData& input = mInput[channel];
  switch (mControllerTypes[channel]) {
  case kDT_Nunchuk:
    if (mStatus[channel].wpad_err == WPAD_ERR_OK) {
      UpdateAnalogInput(CFinalInput::kAnalogAxisDigitalThreshold, channel, 0, input.mButtons[20],
                        input.mButtons[19]);
      UpdateAnalogInput(CFinalInput::kAnalogAxisDigitalThreshold, channel, 1, input.mButtons[16],
                        input.mButtons[13]);
      UpdateAnalogInput(CFinalInput::kAnalogAxisDigitalThreshold, channel, 2, input.mButtons[14],
                        input.mButtons[18]);
      UpdateAnalogInput(CFinalInput::kAnalogAxisDigitalThreshold, channel, 3, input.mButtons[17],
                        input.mButtons[15]);
      UpdateAnalogInput(CFinalInput::kRollDigitalThreshold, channel, 4, input.mButtons[23],
                        input.mButtons[24]);
      UpdateAnalogInput(CFinalInput::kPitchDigitalThreshold, channel, 5, input.mButtons[25],
                        input.mButtons[26]);
      UpdateContinuousAngleAxis(channel, 0);
      UpdateContinuousAngleAxis(channel, 1);
    }
  case kDT_Classic:
  case kDT_Wiimote:
    if (mStatus[channel].wpad_err == WPAD_ERR_OK) {
      UpdateAnalogInput(CFinalInput::kPointerDigitalThreshold, channel, 6, input.mButtons[30],
                        input.mButtons[29]);
      UpdateAnalogInput(CFinalInput::kPointerDigitalThreshold, channel, 7, input.mButtons[27],
                        input.mButtons[28]);
      UpdateAnalogInput(CFinalInput::kRollDigitalThreshold, channel, 8, input.mButtons[31],
                        input.mButtons[32]);
      UpdateAnalogInput(CFinalInput::kPitchDigitalThreshold, channel, 9, input.mButtons[33],
                        input.mButtons[34]);
      UpdateContinuousAngleAxis(channel, 2);
      UpdateContinuousAngleAxis(channel, 3);
      if (!mMotionProcessors[channel].null()) {
        input.SetSwingMask(0, mMotionProcessors[channel]->GetSwingMask(0));
        input.SetSwingMask(1, mMotionProcessors[channel]->GetSwingMask(1));
#endif
      }
    }
#if RSTL_CORRUPTION

  for (int i = 0; i < 4; ++i) {
    CControllerGamepadData& data = mGamepadStates[i];
    if (data.DeviceIsPresent()) {
      ProcessAxis(i, kJA_GcLeftX, data.GetButton(kBU_GcLeftStickLeft),
                  data.GetButton(kBU_GcLeftStickRight));
      ProcessAxis(i, kJA_GcLeftY, data.GetButton(kBU_GcLeftStickUp),
                  data.GetButton(kBU_GcLeftStickDown));
      ProcessAxis(i, kJA_GcRightX, data.GetButton(kBU_GcRightStickLeft),
                  data.GetButton(kBU_GcRightStickRight));
      ProcessAxis(i, kJA_GcRightY, data.GetButton(kBU_GcRightStickUp),
                  data.GetButton(kBU_GcRightStickDown));
      // These two pairs are the reverse of the directions CFinalInput reads from the axes.
      ProcessAxis(i, kJA_LeftX, data.GetButton(kBU_LeftStickRight),
                  data.GetButton(kBU_LeftStickLeft));
      ProcessAxis(i, kJA_LeftY, data.GetButton(kBU_LeftStickDown),
                  data.GetButton(kBU_LeftStickUp));
      ProcessAxis(i, kJA_RightX, data.GetButton(kBU_RightStickLeft),
                  data.GetButton(kBU_RightStickRight));
      ProcessAxis(i, kJA_RightY, data.GetButton(kBU_RightStickUp),
                  data.GetButton(kBU_RightStickDown));
      ProcessButtons(i);
#else
    break;
  }
  if (mControllerTypes[channel] != kDT_Unsupported) {
    UpdateDigitalInput(channel);
  } else {
    CBasics::ZeroMemory(&mStatus[channel], sizeof(KPADStatus));
    mStatus[channel].dev_type = 0xff;
    const CControllerButton button;
    for (int i = 0; i < 64; ++i) {
      input.SetButton(i, button);
#endif
    }
  }
}

#if RSTL_CORRUPTION
void CRevolutionController::ProcessAxis(int controller, EJoyAxis axis, CControllerButton& negative,
                                        CControllerButton& positive) {
  CControllerGamepadData& data = mGamepadStates[controller];
  if (!data.DeviceIsPresent()) {
#else
void CRevolutionController::UpdateAnalogInput(float threshold, int channel, int axis,
                                              CControllerButton& negativeButton,
                                              CControllerButton& positiveButton) {
  CControllerData& input = mInput[channel];
  if (!input.DeviceIsPresent()) {
#endif
    return;
  }
#if RSTL_CORRUPTION

  const float scale = 1.f / GetAnalogStickMaxValue(axis);
  CControllerAxis& ctrlAxis = data.GetAxis(axis);
  const KPADStatus& status = mStatus[controller];
#endif
  float value = 0.f;
#if RSTL_CORRUPTION
  switch (status.dev_type) {
  case WPAD_DEV_DOLPHIN:
    switch (axis) {
    case kJA_GcLeftX:
      value = status.ex_status.gc.stick.x;
      break;
    case kJA_GcLeftY:
      value = -status.ex_status.gc.stick.y;
      break;
    case kJA_RightX:
    case kJA_GcRightX:
      value = status.ex_status.gc.substick.x;
      break;
    case kJA_GcRightY:
      value = -status.ex_status.gc.substick.y;
      break;
    case kJA_RightY:
      value = status.ex_status.gc.substick.y;
#else
  const float previous = input.mAxes[axis].GetAbsoluteValue();
  switch (mControllerTypes[channel]) {
  case kDT_Nunchuk:
    if (axis == 0) {
      CVector2f stick(mStatus[channel].ex_status.fs.stick.x, mStatus[channel].ex_status.fs.stick.y);
      if (stick.IsNormalizable()) {
        const float magnitude = CMath::FastClamp(0.f, stick.Magnitude() / 0.707f, 1.f);
        stick = stick.AsNormalized() * magnitude;
      }
      value = stick.GetX();
    } else if (axis == 1) {
      CVector2f stick(mStatus[channel].ex_status.fs.stick.x, mStatus[channel].ex_status.fs.stick.y);
      if (stick.IsNormalizable()) {
        const float magnitude = CMath::FastClamp(0.f, stick.Magnitude() / 0.707f, 1.f);
        stick = stick.AsNormalized() * magnitude;
      }
      value = stick.GetY();
    } else if (axis == 2) {
      const CVector2f stick(mStatus[channel].ex_status.fs.stick.x,
                            mStatus[channel].ex_status.fs.stick.y);
      if (stick.IsNormalizable()) {
        value = CVector2f::Dot(stick, CVector2f(1.f, -1.f).AsNormalized()) / 1.4142135f / 0.707f;
        value = CMath::FastClamp(0.f, value, 1.f);
      }
    } else if (axis == 3) {
      const CVector2f stick(mStatus[channel].ex_status.fs.stick.x,
                            mStatus[channel].ex_status.fs.stick.y);
      if (stick.IsNormalizable()) {
        value = CVector2f::Dot(stick, CVector2f(1.f, 1.f).AsNormalized()) / 1.4142135f / 0.707f;
        value = CMath::FastClamp(0.f, value, 1.f);
      }
    } else if (axis == 4) {
      if (!mMotionProcessors[channel].null()) {
        value = mMotionProcessors[channel]->GetNunchukTracker().GetWrappedRoll() / (M_PIF / 2.f);
        value = CMath::FastLimit(value, 2.f);
      }
    } else if (axis == 5 && !mMotionProcessors[channel].null()) {
      value = mMotionProcessors[channel]->GetNunchukTracker().GetWrappedPitch() / (M_PIF / 2.f);
      value = CMath::FastLimit(value, 2.f);
    }
  case kDT_Classic:
  case kDT_Wiimote:
    if (axis == 6) {
      value = mStatus[channel].pos.x;
    } else if (axis == 7) {
      value = mStatus[channel].pos.y;
    } else if (axis == 8) {
      if (!mMotionProcessors[channel].null()) {
        value = mMotionProcessors[channel]->GetWiimoteTracker().GetWrappedRoll() / (M_PIF / 2.f);
        value = CMath::FastLimit(value, 2.f);
      }
    } else if (axis == 9 && !mMotionProcessors[channel].null()) {
      value = mMotionProcessors[channel]->GetWiimoteTracker().GetWrappedPitch() / (M_PIF / 2.f);
      value = CMath::FastLimit(value, 2.f);
    }
#endif
    break;
  }
#if !RSTL_CORRUPTION

  input.mAxes[axis].SetRelativeValue(value - input.mAxes[axis].GetAbsoluteValue());
  input.mAxes[axis].SetAbsoluteValue(value);
  const bool wasNegative = negativeButton.GetIsPressed();
  bool negative = wasNegative;
  if (!negative) {
    if (previous > -threshold && value <= -threshold) {
      negative = true;
    }
  } else {
    const float releaseThreshold = -threshold + 0.2f;
    if (previous <= releaseThreshold && value > releaseThreshold) {
      negative = false;
    }
  }
  negativeButton.SetIsPressed(negative);
  negativeButton.SetPressEvent(negative & (negative ^ wasNegative));
  negativeButton.SetReleaseEvent(wasNegative & (negative ^ wasNegative));

  const bool wasPositive = positiveButton.GetIsPressed();
  bool positive = wasPositive;
  if (!positive) {
    if (previous < threshold && value >= threshold) {
      positive = true;
    }
  } else {
    const float releaseThreshold = threshold - 0.2f;
    if (previous >= releaseThreshold && value < releaseThreshold) {
      positive = false;
    }
  }
  positiveButton.SetIsPressed(positive);
  positiveButton.SetPressEvent(positive & (positive ^ wasPositive));
  positiveButton.SetReleaseEvent(wasPositive & (positive ^ wasPositive));
}

void CRevolutionController::UpdateContinuousAngleAxis(int channel, int axis) {
  CControllerData& input = mInput[channel];
  if (!input.DeviceIsPresent()) {
    return;
  }
  CControllerAxis& data = input.mContinuousAngleAxes[axis];
  float value = 0.f;
  switch (mControllerTypes[channel]) {
  case kDT_Nunchuk:
    switch (axis) {
    case 0:
      if (!mMotionProcessors[channel].null()) {
        value = mMotionProcessors[channel]->GetNunchukTracker().GetContinuousRoll();
      }
#endif
      break;
#if RSTL_CORRUPTION
  case WPAD_DEV_FREESTYLE:
    switch (axis) {
    case kJA_LeftX:
      value = status.ex_status.fs.stick.x;
      break;
    case kJA_LeftY:
      value = status.ex_status.fs.stick.y;
#else
    case 1:
      if (!mMotionProcessors[channel].null()) {
        value = mMotionProcessors[channel]->GetNunchukTracker().GetContinuousPitch();
      }
#endif
      break;
    }
#if RSTL_CORRUPTION
  case WPAD_DEV_CORE:
#else
  case kDT_Classic:
  case kDT_Wiimote:
#endif
    switch (axis) {
#if RSTL_CORRUPTION
    case kJA_RightX:
      value = status.pos.x;
#else
    case 2:
      if (!mMotionProcessors[channel].null()) {
        value = mMotionProcessors[channel]->GetWiimoteTracker().GetContinuousRoll();
      }
#endif
      break;
#if RSTL_CORRUPTION
    case kJA_RightY:
      value = status.pos.y;
#else
    case 3:
      if (!mMotionProcessors[channel].null()) {
        value = mMotionProcessors[channel]->GetWiimoteTracker().GetContinuousPitch();
      }
#endif
      break;
    }
    break;
  }
#if RSTL_CORRUPTION

  float absolute = value * scale;
  if (absolute < kAbsoluteMinimum) {
    absolute = kAbsoluteMinimum;
  } else if (absolute > kAbsoluteMaximum) {
    absolute = kAbsoluteMaximum;
  }
  float relative = absolute - ctrlAxis.GetAbsoluteValue();
  if (relative < kRelativeMinimum) {
    relative = kRelativeMinimum;
  } else if (relative > kRelativeMaximum) {
    relative = kRelativeMaximum;
  }
  ctrlAxis.SetRelativeValue(relative);
  ctrlAxis.SetAbsoluteValue(absolute);
  const bool negativePressed = absolute <= -CFinalInput::kInput_AnalogOnThreshhold;
  negative.Update(negativePressed);
  const bool positivePressed = absolute >= CFinalInput::kInput_AnalogOnThreshhold;
  positive.Update(positivePressed);
#else
  data.SetRelativeValue(value - data.GetAbsoluteValue());
  data.SetAbsoluteValue(value);
#endif
}

#if RSTL_CORRUPTION
void CRevolutionController::ProcessButtons(int controller) {
  if (!mGamepadStates[controller].DeviceIsPresent()) {
#else
void CRevolutionController::ClearButtonEvents(int channel) {
  if (!mInput[channel].DeviceIsPresent()) {
#endif
    return;
  }
#if RSTL_CORRUPTION
  CControllerGamepadData& data = mGamepadStates[controller];
#endif

#if RSTL_CORRUPTION
  for (int i = 1; i < kBU_MAX; ++i) {
#else
  CControllerData& input = mInput[channel];
  for (int i = 0; i < 64; ++i) {
#endif
    if (sButtonMasks[i] != 0) {
#if RSTL_CORRUPTION
      ProcessDigitalButton(controller, data.GetButton(static_cast< EButton >(i)), sButtonMasks[i]);
#else
      input.Button(i).SetPressEvent(false);
      input.Button(i).SetReleaseEvent(false);
    }
  }
  for (int i = 0; i < 16; ++i) {
    if (sMotionButtonMasks[i] != 0) {
      input.MotionButton(i).SetPressEvent(false);
      input.MotionButton(i).SetReleaseEvent(false);
    }
  }
  for (int i = 0; i < 12; ++i) {
    if (sSwingButtonMasks[i] != 0) {
      input.SwingButton(i).SetPressEvent(false);
      input.SwingButton(i).SetReleaseEvent(false);
    }
#endif
  }
}

#if RSTL_CORRUPTION
  // Both triggers update the same button.
  CControllerButton& triggerButton = data.GetButton(kBU_GcLeftStickLeft);
  const KPADStatus& status = mStatus[controller];
  ProcessAnalogButton(status.ex_status.gc.ltrigger, data.GetAnalogButton(kBA_Left), triggerButton);
  ProcessAnalogButton(status.ex_status.gc.rtrigger, data.GetAnalogButton(kBA_Right), triggerButton);
#else
void CRevolutionController::UpdateDigitalInput(int channel) {
  if (mInput[channel].DeviceIsPresent()) {
    if (int(mControllerTypes[channel]) >= 0 && int(mControllerTypes[channel]) < 3) {
      const uint held = mStatus[channel].hold;
      for (int i = 1; i < 35; ++i) {
        if (sButtonMasks[i] != 0) {
          UpdateButton(held, mInput[channel].mButtons[i], sButtonMasks[i], i);
        }
      }
    }
    UpdateMotionButtons(channel);
    UpdateSwingButtons(channel);
  }
#endif
}

#if RSTL_CORRUPTION
void CRevolutionController::ProcessDigitalButton(int controller, CControllerButton& button,
                                                 uint mask) {
  if (!mGamepadStates[controller].DeviceIsPresent()) {
#else
void CRevolutionController::UpdateButton(uint heldMask, CControllerButton& button, uint mask,
                                         int buttonId) {
  const int wasPressed = button.GetIsPressed();
  heldMask &= mask;
  const bool pressed = heldMask != 0;
  button.SetIsPressed(pressed);
  button.SetPressEvent(pressed & (pressed ^ wasPressed));
  button.SetReleaseEvent(wasPressed & (pressed ^ wasPressed));
}

void CRevolutionController::UpdateMotionButton(int channel, CControllerButton& button, uint mask) {
  if (!mInput[channel].DeviceIsPresent()) {
#endif
    return;
  }
#if RSTL_CORRUPTION
  button.Update((mStatus[controller].hold & mask) != 0);
#else
  const bool wasPressed = button.GetIsPressed();
  mask &= mInput[channel].mMotionMask;
  const bool pressed = mask != 0;
  button.SetIsPressed(pressed);
  button.SetPressEvent(pressed & (pressed ^ wasPressed));
  button.SetReleaseEvent(wasPressed & (pressed ^ wasPressed));
#endif
}

#if RSTL_CORRUPTION
void CRevolutionController::ProcessAnalogButton(float value, CControllerAxis& axis,
                                                CControllerButton& button) {
  value *= (1.f / 150.f);
  if (value > kAbsoluteMaximum) {
    value = kAbsoluteMaximum;
#else
void CRevolutionController::UpdateMotionButtons(int channel) {
  CControllerData& input = mInput[channel];
  if (input.DeviceIsPresent()) {
    for (int i = 0; i < 16; ++i) {
      if (sMotionButtonMasks[i] != 0) {
        UpdateMotionButton(channel, input.mMotionButtons[i], sMotionButtonMasks[i]);
#endif
      }
#if RSTL_CORRUPTION
  float relative = value - axis.GetAbsoluteValue();
  if (relative > kRelativeMaximum) {
    relative = kRelativeMaximum;
#endif
    }
#if RSTL_CORRUPTION
  axis.SetRelativeValue(relative);
  axis.SetAbsoluteValue(value);
  const bool pressed = CMath::AbsF(value) > CFinalInput::kInput_AnalogTriggerOnThreshhold;
  button.Update(pressed);
#else
  }
#endif
}

#if RSTL_CORRUPTION
void CRevolutionController::SetMotorState(EIOPort port, EMotorState state) {
  if (!mGamepadStates[port].DeviceIsPresent()) {
#else
void CRevolutionController::UpdateSwingButton(int channel, CControllerButton& button, uint mask) {
  if (!mInput[channel].DeviceIsPresent()) {
#endif
    return;
  }
#if !RSTL_CORRUPTION
  const bool wasPressed = button.GetIsPressed();
  mask &= mInput[channel].mSwingMask;
  const bool pressed = mask != 0;
  button.SetIsPressed(pressed);
  button.SetPressEvent(pressed & (pressed ^ wasPressed));
  button.SetReleaseEvent(wasPressed & (pressed ^ wasPressed));
}
#endif

#if RSTL_CORRUPTION
  mMotorStates[port] = state;
#else
void CRevolutionController::UpdateSwingButtons(int channel) {
  CControllerData& input = mInput[channel];
  if (input.DeviceIsPresent()) {
    for (int i = 0; i < 12; ++i) {
      if (sSwingButtonMasks[i] != 0) {
        UpdateSwingButton(channel, input.mSwingButtons[i], sSwingButtonMasks[i]);
      }
    }
  }
}

void CRevolutionController::SetMotorState(int channel, EMotorState state) {
#if NONMATCHING
  if (uint(channel) >= 4) {
    return;
  }
#endif
  if (mInput[channel].DeviceIsPresent()) {
    mInput[channel].mMotorState = state;
    if (!IsControllerIdle(channel) && channel < 4 && (mMotorEnabledFlags & (1 << channel))) {
#endif
      switch (state) {
      case kMS_Rumble:
#if RSTL_CORRUPTION
    WPADControlMotor(port, WPAD_MOTOR_RUMBLE);
#else
        WPADControlMotor(channel, WPAD_MOTOR_RUMBLE);
#endif
        break;
      case kMS_Stop:
#if RSTL_CORRUPTION
  case kMS_StopHard:
    WPADControlMotor(port, WPAD_MOTOR_STOP);
#else
        WPADControlMotor(channel, WPAD_MOTOR_STOP);
#endif
        break;
      }
    }
#if !RSTL_CORRUPTION
  }
}
#endif

#if RSTL_CORRUPTION
float CRevolutionController::GetAnalogStickMaxValue(EJoyAxis axis) const {
  switch (axis) {
  case kJA_GcLeftX:
  case kJA_GcLeftY:
    return 1.f;
  case kJA_GcRightX:
  case kJA_GcRightY:
    return 1.f;
  case kJA_LeftX:
  case kJA_LeftY:
  case kJA_RightX:
  case kJA_RightY:
    return 1.f;
  default:
#else
void CRevolutionController::SetMotorEnabled(int channel, bool enabled) {
#if NONMATCHING
  if (channel < 0) {
    return;
  }
#endif
  if (channel >= 4) {
    return;
  }
  const uint bit = 1 << channel;
  const uint value = uint(enabled) << channel;
  if (value == (mMotorEnabledFlags & bit)) {
    return;
  }
  mMotorEnabledFlags = (mMotorEnabledFlags & ~bit) | value;
  if (!enabled) {
    WPADControlMotor(channel, 0);
  }
}

bool CRevolutionController::HasMotionActivity(uint channel) const {
#if NONMATCHING
  if (channel >= 4) {
#else
  if (channel > 4) {
#endif
    return false;
  }
  const KPADStatus& status = mStatus[channel];
  if (status.dev_type > WPAD_DEV_FS) {
    return false;
  }
  bool active = status.acc_speed > 0.03f || (status.speed > 0.f && status.dpd_valid_fg > 0);
  if (status.dev_type == WPAD_DEV_FS && status.ex_status.fs.acc_speed > 0.03f) {
    active = true;
  }
  return active;
}

float CRevolutionController::GetMotionIdleTime(uint channel) const {
#if NONMATCHING
  if (channel < 4) {
#else
  if (channel <= 4) {
#endif
    return mMotionIdleTimes[channel];
  }
#endif
  return 0.f;
#if !RSTL_CORRUPTION
}

bool CRevolutionController::HasButtonActivity(uint channel) const {
#if NONMATCHING
  if (channel >= 4) {
#else
  if (channel > 4) {
#endif
    return false;
  }
  const KPADStatus& status = mStatus[channel];
  if (status.dev_type > WPAD_DEV_FS) {
    return false;
  }
  bool active = false;
  if (status.hold != 0) {
    active = true;
  }
  if (status.dev_type == WPAD_DEV_FS &&
      (!CMath::IsEpsilon(status.ex_status.fs.stick.x, 0.f, 1.e-5f) ||
       !CMath::IsEpsilon(status.ex_status.fs.stick.y, 0.f, 1.e-5f))) {
    active = true;
  }
  return active;
}

float CRevolutionController::GetButtonIdleTime(uint channel) const {
#if NONMATCHING
  if (channel < 4) {
#else
  if (channel <= 4) {
#endif
    return mButtonIdleTimes[channel];
  }
  return 0.f;
}

bool CRevolutionController::IsControllerIdle(uint channel) const {
  return GetButtonIdleTime(channel) >= 60.f;
}

void CRevolutionController::UpdateIdleTimes(uint channel, float dt) {
#if NONMATCHING
  if (channel >= 4) {
#else
  if (channel > 4) {
#endif
    return;
  }
  if (mStatus[channel].dev_type <= WPAD_DEV_FS) {
    if (HasMotionActivity(channel)) {
      mMotionIdleTimes[channel] = 0.f;
    } else {
      mMotionIdleTimes[channel] += dt;
    }
    if (HasButtonActivity(channel)) {
      mButtonIdleTimes[channel] = 0.f;
    } else {
      const rstl::reserved_vector< float, 4 >& buttonIdleTimes = mButtonIdleTimes;
      const float previous = buttonIdleTimes[channel];
      mButtonIdleTimes[channel] += dt;
      if (previous < 60.f && mButtonIdleTimes[channel] >= 60.f) {
        WPADControlMotor(channel, 0);
      }
    }
#endif
  }
}

bool CRevolutionController::Initialize() {
  for (int i = 0; i < 4; ++i) {
#if RSTL_CORRUPTION
    mGamepadStates[i].SetDeviceIsPresent(false);
    mMotorStates[i] = kMS_StopHard;
    mTrackingStates[i] = 0;
#else
    mInput[i].mConnected = false;
    mInput[i].mPointerPosition = CVector2f::Zero();
    mInput[i].mMotorState = kMS_StopHard;
    mInput[i].mPointerState = CControllerData::kPS_Tracking;
  }
  for (int i = 0; i < 4; ++i) {
    mPointerFilterX[i] = rs_new CAdaptiveInputFilter(2, 3, 0, 0.1f);
    mPointerFilterY[i] = rs_new CAdaptiveInputFilter(2, 3, 0, 0.1f);
#endif
  }

#if RSTL_CORRUPTION
  KPADInit();
  KPADSetPosParam(0, 0.05f, 1.f);
  KPADSetObjInterval(0.15f);
#endif
  VIWaitForRetrace();
  VIWaitForRetrace();
  VIWaitForRetrace();
  VIWaitForRetrace();
#if !RSTL_CORRUPTION
  WPADRegisterAllocator(AllocateWpadMemory, FreeWpadMemory);
  KPADInit();
  while (WPADGetStatus() != 3) {
  }
#endif

  for (int i = 0; i < 4; ++i) {
#if RSTL_CORRUPTION
    u32 type = WPAD_DEV_UNKNOWN;
    switch (WPADProbe(i, &type)) {
    case WPAD_ERR_NONE:
      switch (type) {
      case WPAD_DEV_CORE:
        WPADSetDataFormat(i, WPAD_FMT_CORE);
        rs_debugger_printf("Controller %d is a CORE controller\n", i);
        break;
      case WPAD_DEV_FREESTYLE:
        WPADSetDataFormat(i, WPAD_FMT_FREESTYLE);
        rs_debugger_printf("Controller %d is a CORE + NUNCHAKU controller\n", i);
        break;
      case WPAD_DEV_DOLPHIN:
        WPADSetDataFormat(i, WPAD_FMT_DOLPHIN);
        rs_debugger_printf("Controller %d is a GAMECUBE controller\n", i);
        break;
      case WPAD_DEV_UNKNOWN:
        rs_debugger_printf("Controller %d is an UNKNOWN controller\n", i);
        break;
      default:
        rs_debugger_printf("Controller %d is an '%d' controller\n", i, type);
        break;
#else
    KPADSetPosParam(i, 0.05f, 1.f);
    KPADSetDistParam(i, 0.05f, 1.f);
    KPADDisableAimingMode(i);
    WPADSetConnectCallback(i, WpadConnectCallback);
    mInfoPollTimers[i] = 1.f;
    mMotionIdleTimes[i] = 0.f;
    mButtonIdleTimes[i] = 0.f;
#endif
  }
#if RSTL_CORRUPTION
      if (type != WPAD_DEV_UNKNOWN) {
        VIWaitForRetrace();
        VIWaitForRetrace();
        rs_debugger_printf("Recalibrating controller %d\n", i);
        WPADRecalibrate(i);
        mDeviceTypes[i] = type;
      }
      break;
    case WPAD_ERR_INVALID:
      rs_debugger_printf("Controller %d is INVALID\n", i);
      break;
    case WPAD_ERR_TRANSFER:
    case WPAD_ERR_BUSY:
      rs_debugger_printf("Controller %d is BUSY\n", i);
      break;
    case WPAD_ERR_NO_CONTROLLER:
      rs_debugger_printf("Controller %d is NOT ATTACHED\n", i);
      break;
    }
    mDeviceTypes[i] = type;
  }

#endif
  return true;
}

#if RSTL_CORRUPTION
const uint kControllerTypeUnknown = 'UNKN';
const uint kControllerTypeCube = 'CUBE';
const uint kControllerTypeRevn = 'REVN';
const uint kControllerTypeChak = 'CHAK';
#else
void CRevolutionController::InitializeController(uint channel) {
#if NONMATCHING
  if (channel >= 4) {
#else
  if (channel > 4) {
#endif
    return;
  }
  if (int(mControllerTypes[channel]) >= 0 && int(mControllerTypes[channel]) < 3) {
    if (mMotionProcessors[channel].null()) {
      rstl::single_ptr< CRevolutionMotionProcessor > processor(rs_new CRevolutionMotionProcessor(channel));
      mMotionProcessors[channel] = processor;
    }
    if (!mMotionProcessors[channel].null()) {
      mMotionProcessors[channel]->ConfigureFiltersAndCalibration(channel);
    }
    mInput[channel] = CControllerData();
    mInput[channel].mConnected = true;
  }
}

void CRevolutionController::SetPointerRecenterMode(uint channel, EPointerRecenterMode mode) {
#if NONMATCHING
  if (channel >= 4) {
#else
  if (channel > 4) {
#endif
    return;
  }
  mPointerRecenterMode[channel] = mode;
}

void CRevolutionController::SetControllerType(int channel, EDeviceType type) {
  mControllerTypes[channel] = type;
  mInput[channel].mConnected = type != kDT_Disconnected;
}

WPADInfo CRevolutionController::GetWpadInfo(int channel) const { return mWpadInfo[channel]; }

const KPADStatus& CRevolutionController::GetKpadStatus(int channel) const {
  return mStatus[channel];
}

void CRevolutionController::CopyWpadInfo(int channel) {
  mWpadInfo[channel] = mWpadInfoBuf[channel];
}

void CRevolutionController::WpadInfoCallback(s32 channel, s32 result) {
  if (sRevolutionController != nullptr && result == 0) {
    sRevolutionController->CopyWpadInfo(channel);
  }
}

void CRevolutionController::WpadConnectCallback(s32 channel, s32 result) {
  if (sRevolutionController != nullptr) {
    if (channel >= sRevolutionController->GetMaxControllers()) {
      sRevolutionController->QueueConnectionEvent(channel, kCE_Rejected);
    } else {
      switch (result) {
      case WPAD_ERR_OK:
        sRevolutionController->QueueConnectionEvent(channel, kCE_Connected);
        sRevolutionController->SetControllerType(channel, kDT_Wiimote);
        WPADSetExtensionCallback(channel, WpadExtensionCallback);
        break;
      case WPAD_ERR_NO_CONTROLLER:
        sRevolutionController->QueueConnectionEvent(channel, kCE_Disconnected);
        sRevolutionController->SetControllerType(channel, kDT_Disconnected);
        break;
      case WPAD_ERR_BUSY:
        sRevolutionController->SetControllerType(channel, kDT_Disconnected);
        break;
      case WPAD_ERR_TRANSFER:
        sRevolutionController->SetControllerType(channel, kDT_Disconnected);
        break;
      }
    }
  }
}

void CRevolutionController::WpadExtensionCallback(s32 channel, s32 extension) {
  if (sRevolutionController != nullptr) {
    if (channel >= kActiveChannelCount) {
      sRevolutionController->SetControllerType(channel, kDT_Wiimote);
    } else {
      switch (extension) {
      case WPAD_DEV_CORE:
        sRevolutionController->QueueExtensionEvent(channel, kEE_Wiimote);
        sRevolutionController->SetControllerType(channel, kDT_Wiimote);
        break;
      case WPAD_DEV_FS:
        sRevolutionController->QueueExtensionEvent(channel, kEE_Nunchuk);
        sRevolutionController->SetControllerType(channel, kDT_Nunchuk);
        break;
      case WPAD_DEV_CLASSIC:
        sRevolutionController->QueueExtensionEvent(channel, kEE_Classic);
        sRevolutionController->SetControllerType(channel, kDT_Classic);
        break;
      case 0xfd:
        sRevolutionController->SetControllerType(channel, kDT_Wiimote);
        break;
      case 0xfb:
        sRevolutionController->QueueExtensionEvent(channel, kEE_Unsupported);
        sRevolutionController->SetControllerType(channel, kDT_Wiimote);
        break;
      default:
        sRevolutionController->QueueExtensionEvent(channel, kEE_Unsupported);
        sRevolutionController->SetControllerType(channel, kDT_Unsupported);
        break;
      }
    }
  }
}

#if RSTL_CONTROLLER_EXTENDED_VIRTUALS
void CRevolutionController::SetAcceptAdditionalConnections(bool accept) {
  if (accept == mAcceptAdditionalConnections) {
    return;
  }
  mAcceptAdditionalConnections = accept;
  if (!mAcceptAdditionalConnections && mInput[0].DeviceIsPresent()) {
    WPADSetAcceptConnection(false);
  }
}

#endif

void CRevolutionController::SetInput(const CControllerData& input, int channel) {
  mInput[channel] = input;
}
#endif
