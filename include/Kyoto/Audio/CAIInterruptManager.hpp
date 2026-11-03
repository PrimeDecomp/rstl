#ifndef _CAIINTERRUPTMANAGER
#define _CAIINTERRUPTMANAGER

#include "Kyoto/Platform.h"

typedef void (*FAudioCallback)(IF_TARGET_PC(short* output, size_t frames, u32 rate, u32 channels));

// Guessed name: follows the asserted Corruption source-file identity.
class CAIInterruptManager {
public:
  static void RunDMACallback(FAudioCallback callback);
  static void CancelDMACallback(FAudioCallback callback);
  static void InstallAICallback();
  static void AICallback(IF_TARGET_PC(short* output, size_t frames, u32 rate, u32 channels));
};

#endif // _CAIINTERRUPTMANAGER
