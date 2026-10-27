#include "Kyoto/Basics/CRenderProfileScope.hpp"

#include "Corruption/Kyoto/CFrameDelayedKiller.hpp"
#include "Corruption/MetroidPrime/CScopedProfiler.hpp"

TFunctor3< const char*, const CStopwatch&, const CStopwatch& > gRenderProfileCallback;

bool CRenderProfileScope::sScopeActive = false;

CRenderProfileScope::CRenderProfileScope(const char* name) : mStopwatch(), mName(name) {
  mActive = gRenderProfileCallback && !sScopeActive;
  if (mActive) {
    CFrameDelayedKiller::StallAndFlushAllAllocations();
    mStopwatch = CStopwatch();
    sScopeActive = true;
  }
}

CRenderProfileScope::~CRenderProfileScope() { Finish(); }

void CRenderProfileScope::Finish() {
  if (mActive) {
    if (gRenderProfileCallback) {
      CStopwatch end;
      CFrameDelayedKiller::StallAndFlushAllAllocations();
      gRenderProfileCallback(mName, mStopwatch, end);
    }
    sScopeActive = false;
    mActive = false;
  }
}
