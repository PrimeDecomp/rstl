#ifndef _CAIINTERRUPTMANAGER
#define _CAIINTERRUPTMANAGER

#if defined(TARGET_PC)
typedef void (*FAudioCallback)(short* output, size_t frames, u32 rate, u32 channels);
#else
typedef void (*FAudioCallback)();
#endif

// Guessed name: follows the asserted Corruption source-file identity.
class CAIInterruptManager {
public:
  static void RunDMACallback(FAudioCallback callback);
  static void CancelDMACallback(FAudioCallback callback);
  static void InstallAICallback();
#if defined(TARGET_PC)
  static void AICallback(short* output, size_t frames, u32 rate, u32 channels);
#else
  static void AICallback();
#endif
};

#endif // _CAIINTERRUPTMANAGER
