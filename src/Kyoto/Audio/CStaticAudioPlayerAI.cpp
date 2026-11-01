#include "Kyoto/Audio/CAudioSys.hpp"
#include "rstl/algorithm.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Audio/CStaticAudioPlayer.hpp"

#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/CDvdRequest.hpp"

#include "rstl/list.hpp"
#include "rstl/math.hpp"

#include "dolphin/ai.h"
#include "dolphin/os.h"

static rstl::reserved_vector< FAudioCallback, 4 > sCallbacks;
ATTRIBUTE_ALIGN_DECL(8, static bool sDMACallbackInstalled) = false;
static FAudioCallback sOldDMACallback = nullptr;

void CStaticAudioPlayer::InstallAICallback() {
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

void CStaticAudioPlayer::AICallback() {
  sOldDMACallback();

  for (int i = 0; i < sCallbacks.size(); ++i) {
    sCallbacks[i]();
  }
}

void CStaticAudioPlayer::RunDMACallback(const FAudioCallback callback) {
  volatile const bool old = OSDisableInterrupts();
  if (rstl::find(sCallbacks.begin(), sCallbacks.end(), callback) == sCallbacks.end()) {
    sCallbacks.push_back(callback);
  }

  InstallAICallback();
  OSRestoreInterrupts(old);
}

void CStaticAudioPlayer::CancelDMACallback(FAudioCallback callback) {
  volatile const bool old = OSDisableInterrupts();

  const rstl::reserved_vector< FAudioCallback, 4 >::iterator it =
      rstl::find(sCallbacks.begin(), sCallbacks.end(), callback);
  if (it != sCallbacks.end()) {
    sCallbacks.erase(it);
  }

  InstallAICallback();
  OSRestoreInterrupts(old);
}

