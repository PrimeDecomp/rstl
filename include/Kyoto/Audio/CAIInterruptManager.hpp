#ifndef _CAIINTERRUPTMANAGER
#define _CAIINTERRUPTMANAGER

typedef void (*FAudioCallback)();

// Guessed name: follows the asserted Corruption source-file identity.
class CAIInterruptManager {
public:
  static void RunDMACallback(FAudioCallback callback);
  static void CancelDMACallback(FAudioCallback callback);
  static void InstallAICallback();
  static void AICallback();
};

#endif // _CAIINTERRUPTMANAGER
