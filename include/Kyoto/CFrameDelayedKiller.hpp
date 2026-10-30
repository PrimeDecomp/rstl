#ifndef _CFRAMEDELAYEDKILLER
#define _CFRAMEDELAYEDKILLER
#include "rstl/RstlVersions.h"

#include "types.h"

class CFrameDelayedKiller {
public:
#if RSTL_PRIME1
  struct Stats {
    uint unk1;
    uint unk2;
    uint unk3;
    uint unk4;
    uint unk5;
    uint unk6;
    uint unk7;
    uint unk8;
  };
#endif

  enum EWhichFrame {
    kWhichFrame_ThisFrame,
    kWhichFrame_NextFrame,
  };
  static void Initialize();
  static void ShutDown();

  static void FlushAllocationsForFrame();
  static void ScheduleDeletion(RSTL_PRIME1_CONST EWhichFrame whichFrame, void* victim);

  static void FlushAllAllocations();
  static void StallAndFlushAllAllocations();
#if RSTL_PRIME1
  const CFrameDelayedKiller::Stats& GetStats() {
    return mUnusedStats;
  }
  static Stats mUnusedStats;
#endif

};

#endif // _CFRAMEDELAYEDKILLER
