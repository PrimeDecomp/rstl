#ifndef _CRENDERPROFILESCOPE
#define _CRENDERPROFILESCOPE

#include "types.h"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CStopwatch.hpp"

// Guessed name. A named scope of the render passes: while a profile callback is installed (and no
// other scope is active) it times the scope, flushing the delayed frees around it, and reports
// the name and both stopwatches to the callback when it ends.
class CRenderProfileScope {
public:
  explicit CRenderProfileScope(const char* name);
  ~CRenderProfileScope();

  static void operator delete(void* ptr) { CMemory::Free(ptr); }

private:
  void Finish();

  CStopwatch mStopwatch;
  const char* mName;
  bool mActive;

  static bool sScopeActive;
};

#endif // _CRENDERPROFILESCOPE
