#ifndef _CINTERRUPTGUARD
#define _CINTERRUPTGUARD

#include "dolphin/os.h"

class CInterruptGuard {
  bool mEnabled;

public:
  CInterruptGuard() : mEnabled(OSDisableInterrupts()) {}
  ~CInterruptGuard() { OSRestoreInterrupts(mEnabled); }
};

class CRestorableInterruptGuard {
public:
  CRestorableInterruptGuard() : mEnabled(OSDisableInterrupts()), mRestored(false) {}
  ~CRestorableInterruptGuard() { Restore(); }

  void Restore() {
    if (!mRestored) {
      OSRestoreInterrupts(mEnabled);
      mRestored = true;
    }
  }

private:
  bool mEnabled;
  bool mRestored;
};

#endif
