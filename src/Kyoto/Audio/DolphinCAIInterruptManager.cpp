#include "rstl/RstlVersions.h"
#include "Kyoto/Audio/CAIInterruptManager.hpp"

#include "Kyoto/Audio/CAudioSys.hpp"
#include "dolphin/os.h"
#include "rstl/algorithm.hpp"
#include "rstl/reserved_vector.hpp"

#include "dolphin/ai.h"

#if defined(TARGET_PC)
#include "Metaforce/Audio.hpp"
#include <algorithm>
#endif

static rstl::reserved_vector< FAudioCallback, 4 > sCallbacks;
#if !defined(TARGET_PC)
ATTRIBUTE_ALIGN_DECL(8, static bool sDMACallbackInstalled) = false;
static FAudioCallback sOldDMACallback = nullptr;
#endif

void CAIInterruptManager::InstallAICallback() {
#if IS_TARGET_PC
  sndPCSetOutputCallback(sCallbacks.empty() ? nullptr : AICallback);
#else
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
#endif
}

void CAIInterruptManager::AICallback(IF_TARGET_PC(short* output, size_t frames, u32 rate, u32 channels)) {
  if (!IS_TARGET_PC) {
    sOldDMACallback();
  }

  for (int i = 0; i < sCallbacks.size(); ++i) {
    sCallbacks[i](IF_TARGET_PC(output, frames, rate, channels));
  }
}

void CAIInterruptManager::RunDMACallback(const FAudioCallback callback) {
  volatile const bool old = OSDisableInterrupts();
  if (rstl::find(sCallbacks.begin(), sCallbacks.end(), callback) == sCallbacks.end()) {
    sCallbacks.push_back(callback);
  }

  InstallAICallback();
  OSRestoreInterrupts(old);
}

void CAIInterruptManager::CancelDMACallback(FAudioCallback callback) {
  volatile const bool old = OSDisableInterrupts();

  const rstl::reserved_vector< FAudioCallback, 4 >::iterator it =
      rstl::find(sCallbacks.begin(), sCallbacks.end(), callback);
  if (it != sCallbacks.end()) {
    sCallbacks.erase(it);
  }

  InstallAICallback();
  OSRestoreInterrupts(old);
}
