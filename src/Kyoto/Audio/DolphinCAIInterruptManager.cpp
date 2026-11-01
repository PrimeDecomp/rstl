#include "rstl/RstlVersions.h"
// Prime keeps these callbacks on CStaticAudioPlayer; Echoes moved them to CAIInterruptManager.
#if RSTL_PRIME1
#include "Kyoto/Audio/CStaticAudioPlayer.hpp"
typedef CStaticAudioPlayer CAICallbackOwner;
#else
#include "Kyoto/Audio/CAIInterruptManager.hpp"
typedef CAIInterruptManager CAICallbackOwner;
#endif

#include "Kyoto/Audio/CAudioSys.hpp"
#include "dolphin/os.h"
#include "rstl/algorithm.hpp"
#include "rstl/reserved_vector.hpp"

#include "dolphin/ai.h"

static rstl::reserved_vector< FAudioCallback, 4 > sCallbacks;
ATTRIBUTE_ALIGN_DECL(8, static bool sDMACallbackInstalled) = false;
static FAudioCallback sOldDMACallback = nullptr;

void CAICallbackOwner::InstallAICallback() {
  bool old = CAudioSys::IsAICallbackEnabled();
  CAudioSys::EnableAICallback(true);

  if (!sDMACallbackInstalled && sCallbacks.size() != 0) {
    sOldDMACallback = AIRegisterDMACallback(AICallback);
    sDMACallbackInstalled = true;
  } else if (sDMACallbackInstalled && sCallbacks.size() == 0) {
    AIRegisterDMACallback(sOldDMACallback);
    sOldDMACallback = 0;
    sDMACallbackInstalled = false;
  }

  CAudioSys::EnableAICallback(old);
}

void CAICallbackOwner::AICallback() {
  sOldDMACallback();

  for (int i = 0; i < sCallbacks.size(); ++i) {
    sCallbacks[i]();
  }
}

void CAICallbackOwner::RunDMACallback(const FAudioCallback callback) {
  volatile const bool old = OSDisableInterrupts();
  if (rstl::find(sCallbacks.begin(), sCallbacks.end(), callback) == sCallbacks.end()) {
    sCallbacks.push_back(callback);
  }

  InstallAICallback();
  OSRestoreInterrupts(old);
}

void CAICallbackOwner::CancelDMACallback(FAudioCallback callback) {
  volatile const bool old = OSDisableInterrupts();

  const rstl::reserved_vector< FAudioCallback, 4 >::iterator it =
      rstl::find(sCallbacks.begin(), sCallbacks.end(), callback);
  if (it != sCallbacks.end()) {
    sCallbacks.erase(it);
  }

  InstallAICallback();
  OSRestoreInterrupts(old);
}
