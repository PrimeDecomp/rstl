#ifndef _CGAMESPLINEDESC
#define _CGAMESPLINEDESC
#include "rstl/RstlVersions.h"

#include "rstl/static_assert.hpp"
#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Math/CMotionSpline.hpp"

// Class name corroborated by the Echoes Wii CScriptEffect constructor export.
class CGameSplineDesc {
public:
#if RSTL_CORRUPTION
  CGameSplineDesc(const SLdrSpline& spline, CMotionSpline::ESplineType type, float duration,
                  bool closedLoop);
#else
  CGameSplineDesc(const SLdrSpline& spline, CMotionSpline::ESplineType type, const float duration,
                  const bool closedLoop)
  : mSpline(spline), mType(type), mDuration(duration), mClosedLoop(closedLoop) {}
#endif
  ~CGameSplineDesc() {}

  const SLdrSpline& GetSpline() const { return mSpline; }
  CMotionSpline::ESplineType GetType() const { return mType; }
  float GetDuration() const { return mDuration; }
  bool IsClosedLoop() const { return mClosedLoop; }

private:
  SLdrSpline mSpline;
  CMotionSpline::ESplineType mType;
  float mDuration;
  bool mClosedLoop : 1;
};
CHECK_SIZEOF(CGameSplineDesc, 0x50)

#endif // _CGAMESPLINEDESC
