#include "rstl/RstlVersions.h"
#include "rstl/macros.hpp"
#include "Kyoto/Audio/CAIInterruptManager.hpp"

#include "Kyoto/Alloc/Assert.hpp"
#if RSTL_AI_CALLBACK_USES_AUDIO_SYS
#include "Kyoto/Audio/CAudioSys.hpp"
#endif
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
#if RSTL_AI_CALLBACK_USES_AUDIO_SYS
  bool old = CAudioSys::IsAICallbackEnabled();
  CAudioSys::EnableAICallback(true);
#endif

  if (!sDMACallbackInstalled && sCallbacks.size() != 0) {
    sOldDMACallback = AIRegisterDMACallback(AICallback);
    sDMACallbackInstalled = true;
  } else if (sDMACallbackInstalled && sCallbacks.size() == 0) {
    AIRegisterDMACallback(sOldDMACallback);
    sOldDMACallback = 0;
    sDMACallbackInstalled = false;
  }

#if RSTL_AI_CALLBACK_USES_AUDIO_SYS
  CAudioSys::EnableAICallback(old);
#endif
#endif
}

void CAIInterruptManager::AICallback(IF_TARGET_PC(short* output, size_t frames, u32 rate, u32 channels)) {
  IF_NOT_TARGET_PC(sOldDMACallback());

  for (int i = 0; i < sCallbacks.size(); ++i) {
    sCallbacks[i](IF_TARGET_PC(output, frames, rate, channels));
  }
}

void CAIInterruptManager::RunDMACallback(const FAudioCallback callback) {
  volatile const bool old = OSDisableInterrupts();
  if (rstl::find(sCallbacks.begin(), sCallbacks.end(), callback) == sCallbacks.end()) {
    RS_VERIFY_THROW_DIRECT(124, sCallbacks.size() != sCallbacks.capacity(), false,
                           "Out of audio callback slots!");
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
