#include "rstl/RstlVersions.h"
#include "Kyoto/Input/CRumbleGenerator.hpp"

#include "Kyoto/Input/IController.hpp"

#if RSTL_PRIME1
const EMotorState CRumbleGenerator::kStopAll[4] = {
    kMS_StopHard,
    kMS_StopHard,
    kMS_StopHard,
    kMS_StopHard,
};
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CRumbleGenerator::CRumbleGenerator() : mDisabled(false) { HardStopAll(); }
#else
#if RSTL_VERSION >= RSTL_R32J
CRumbleGenerator::CRumbleGenerator(IController* controller, EIOPort inputIndex)
: mController(controller), mInputIndex(inputIndex), mDisabled(false) {
#else
CRumbleGenerator::CRumbleGenerator(EIOPort inputIndex)
: mInputIndex(inputIndex), mDisabled(false) {
#endif
  HardStopAll();
}
#endif

CRumbleGenerator::~CRumbleGenerator() { HardStopAll(); }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
short CRumbleGenerator::Rumble(const SAdsrData& adsr, float gain, ERumblePriority prio,
                               EIOPort port) {
  ushort freeChan = mVoices[port].GetFreeChannel();
  if (prio >= mVoices[port].GetPriority(freeChan)) {
    mPeriodTime[port] = 0.f;
    mOnTime[port] = 0.f;
    return mVoices[port].Activate(adsr, freeChan, gain, prio);
  }
  return -1;
}
#else
short CRumbleGenerator::Rumble(const SAdsrData& adsr, float gain, ERumblePriority prio) {
  ushort freeChan = mVoice.GetFreeChannel();
  if (prio >= mVoice.GetPriority(freeChan)) {
    mPeriodTime = 0.f;
    mOnTime = 0.f;
    return mVoice.Activate(adsr, freeChan, gain, prio);
  }
  return -1;
}
#endif

void CRumbleGenerator::Update(float dt) {
  if (!mDisabled) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    bool updated = false;
    for (int i = 0; i < 4; ++i) {
      const float intensity = mVoices[i].GetIntensity();
      if (!mVoices[i].Update(dt) || intensity <= 0.f) {
        mPeriodTime[i] = 0.f;
        mOnTime[i] = 0.f;
        if (mCommandArray[i] != kMS_Stop) {
          mCommandArray[i] = kMS_Stop;
          updated = true;
        }
      } else {
        mPeriodTime[i] += dt;
        if (mPeriodTime[i] >= 1.f / (30.f * intensity)) {
          mPeriodTime[i] = 0.f;
          if (mCommandArray[i] != kMS_Rumble) {
            mCommandArray[i] = kMS_Rumble;
            updated = true;
          }
        } else {
          mOnTime[i] += dt;
          if (mOnTime[i] >= (1.f / 30.f)) {
            mOnTime[i] = 0.f;
            if (mCommandArray[i] != kMS_Stop) {
              mCommandArray[i] = kMS_Stop;
              updated = true;
            }
          }
        }
      }
    }
    if (updated) {
      PADControlAllMotors(reinterpret_cast< const u32* >(mCommandArray));
    }
#else
#if RSTL_VERSION >= RSTL_R32J
    const EMotorState prevCommand = mCommand;
    const float intensity = mVoice.GetIntensity();
    if (!mVoice.Update(dt) || intensity <= 0.f) {
      mPeriodTime = 0.f;
      mOnTime = 0.f;
      mCommand = kMS_Stop;
    } else {
      mPeriodTime += dt;
      if (mPeriodTime >= 1.f / (30.f * intensity)) {
        mPeriodTime = 0.f;
        mCommand = kMS_Rumble;
      } else {
        mOnTime += dt;
        if (mOnTime >= (1.f / 30.f)) {
          mOnTime = 0.f;
          mCommand = kMS_Stop;
        }
      }
    }
    if (mCommand != prevCommand) {
      mController->SetMotorState(mInputIndex, mCommand);
    }
#else
    bool updated = false;
    const float intensity = mVoice.GetIntensity();
    if (!mVoice.Update(dt) || intensity <= 0.f) {
      mPeriodTime = 0.f;
      mOnTime = 0.f;
      if (mCommand != kMS_Stop) {
        mCommand = kMS_Stop;
        updated = true;
      }
    } else {
      mPeriodTime += dt;
      if (mPeriodTime >= 1.f / (30.f * intensity)) {
        mPeriodTime = 0.f;
        if (mCommand != kMS_Rumble) {
          mCommand = kMS_Rumble;
          updated = true;
        }
      } else {
        mOnTime += dt;
        if (mOnTime >= (1.f / 30.f)) {
          mOnTime = 0.f;
          if (mCommand != kMS_Stop) {
            mCommand = kMS_Stop;
            updated = true;
          }
        }
      }
    }
    if (updated) {
      PADControlMotor(mInputIndex, mCommand);
    }
#endif
#endif
  }
}

void CRumbleGenerator::HardStopAll() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  for (int i = 0; i < 4; ++i) {
    mPeriodTime[i] = 0.f;
    mOnTime[i] = 0.f;
    mCommandArray[i] = kMS_Stop;
    mVoices[i].HardReset();
  }

  PADControlAllMotors(reinterpret_cast< const u32* >(kStopAll));
#else
  mPeriodTime = 0.f;
  mOnTime = 0.f;
  mCommand = kMS_Stop;
  mVoice.HardReset();
#if RSTL_VERSION >= RSTL_R32J
  mController->SetMotorState(mInputIndex, kMS_Stop);
#else
  PADControlMotor(mInputIndex, kMS_StopHard);
#endif
#endif
}

void CRumbleGenerator::SetDisabled(const bool disabled) {
  if (disabled) {
    HardStopAll();
  }

  mDisabled = disabled;
}

#if !RSTL_PRIME1
void CRumbleGenerator::Stop(short id) {
  if (id != -1)
    mVoice.Deactivate(id, false);
}
#endif

