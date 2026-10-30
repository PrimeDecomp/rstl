#ifndef _CRUMBLEGENERATOR
#define _CRUMBLEGENERATOR
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/Input/CRumbleVoice.hpp"
#include "Kyoto/Input/InputTypes.hpp"

#if !RSTL_PRIME1
class IController;
#endif

class CRumbleGenerator {
private:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static const EMotorState kStopAll[4];
  CRumbleVoice mVoices[4];
  float mPeriodTime[4];
  float mOnTime[4];
  EMotorState mCommandArray[4];
#else
#if RSTL_VERSION >= RSTL_R32J
  IController* mController;
#endif
  CRumbleVoice mVoice;
  float mPeriodTime;
  float mOnTime;
  EMotorState mCommand;
  EIOPort mInputIndex;
#endif

  bool mDisabled : 1;

public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CRumbleGenerator();
#else
#if RSTL_VERSION >= RSTL_R32J
  CRumbleGenerator(IController* controller, EIOPort inputIndex);
#else
  CRumbleGenerator(EIOPort inputIndex);
#endif
#endif

  ~CRumbleGenerator();

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  short Rumble(const SAdsrData& adsr, float, ERumblePriority prio, EIOPort port);
  void Stop(short id, EIOPort port) {
    if (id == -1)
      return;
    CRumbleVoice& vox = mVoices[size_t(port)];
    vox.Deactivate(id, false);
  }
  void Deactivate(short, bool);
#else
  short Rumble(const SAdsrData& adsr, float, ERumblePriority prio);
  void Stop(short id);
#endif

  void Update(float);
  void HardStopAll();
#if !RSTL_PRIME1
  void SetDisabled(const bool disabled);
#endif

  bool GetDisabled() const { return mDisabled; }
#if RSTL_PRIME1
  void SetDisabled(const bool disabled);
#endif
};

#if !RSTL_PRIME1
CHECK_SIZEOF(CRumbleGenerator, RSTL_VERSION >= RSTL_R32J ? 0x48 : 0x44)
#endif

#endif // _CRUMBLEGENERATOR
