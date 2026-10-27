#include "Kyoto/Basics/CStopwatch.hpp"

#include "rstl/RstlVersions.h"

#include <math.h>

CStopwatch::CSWData CStopwatch::mData;
CStopwatch CStopwatch::mGlobalTimer;

bool CStopwatch::InitGlobalTimer() {
  mGlobalTimer.Reset();
  return true;
}

void CStopwatch::Wait(float wait) {
#if RSTL_KYOTO_STOPWATCH_EPSILON_WAIT
  if (wait > 1.1920929E-7f) {
    mData.Wait(wait);
  }
#else
#if RSTL_KYOTO_REVOLUTION_SDK
  if (fabsf(wait) < 0.001f) {
#else
  if (fabs(wait) < 0.001f) {
#endif
    mData.Wait(0.f);
  } else {
    mData.Wait(wait);
  }
#endif
}

CStopwatch& CStopwatch::GetGlobalTimerObj() { return mGlobalTimer; }
