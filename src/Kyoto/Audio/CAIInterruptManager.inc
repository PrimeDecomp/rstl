// AI DMA callback list, shared by Prime's CStaticAudioPlayerAI.cpp and Echoes'
// DolphinCAIInterruptManager.cpp. The including file defines CAICallbackOwner as the
// class that owns these functions in that game.

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

