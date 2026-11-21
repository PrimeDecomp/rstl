#ifndef _CSPLINE
#define _CSPLINE
#include "rstl/RstlVersions.h"

#include "rstl/static_assert.hpp"
#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Math/CMotionSpline.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "rstl/vector.hpp"

// Guessed name. Runtime spline, distinct from the serialized CGameSplineDesc.
class CSpline {
public:
  // Guessed flag names, based on native construction and sampling controls.
  enum EFlags {
    kF_LoopPosition = 0x1,
    kF_LoopLookAt = 0x2,
    kF_UsePositionTimeForLookAt = 0x4,
    kF_UsePositionForLookAt = 0x8,
  };

  CSpline(float duration, uint flags, const CMayaSpline& positionTimeSpline,
          const CMayaSpline& lookAtTimeSpline, CMotionSpline::ESplineType positionType,
          CMotionSpline::ESplineType lookAtType);
  virtual ~CSpline();

  // Guessed name; replaces both motion paths and their position-key orientations.
  void Initialise(const rstl::vector< CVector3f >& positions,
                  const rstl::vector< CQuaternion >& orientations,
                  const rstl::vector< CVector3f >& lookAtPoints);

  // Guessed accessor names, recovered from the script-camera callers.
  uint GetPositionKnotCount() const;
#if RSTL_CORRUPTION
  uint GetLookAtKnotCount() const { return mLookAtSpline.GetKnotCount(); }
#else
  uint GetLookAtKnotCount() const;
#endif
  CMotionSpline& PositionSpline();
#if RSTL_CORRUPTION
  CMotionSpline& LookAtSpline() { return mLookAtSpline; }
#else
  CMotionSpline& LookAtSpline();
#endif
  const CMotionSpline& GetPositionSpline() const;
#if RSTL_CORRUPTION
  const CMotionSpline& GetLookAtSpline() const { return mLookAtSpline; }
#else
  const CMotionSpline& GetLookAtSpline() const;
#endif
  CMayaSpline& PositionTimeSpline();
#if RSTL_CORRUPTION
  CMayaSpline& LookAtTimeSpline() { return mLookAtTimeSpline; }
#else
  CMayaSpline& LookAtTimeSpline();
  float GetLength() const;
#endif
  float GetDuration() const;
  uint GetFlags() const { return mFlags; }
#if !RSTL_CORRUPTION
  float FindClosestLengthOnSpline(float start, const CVector3f& position) const;
  float ValidateLength(float distance) const;
#endif
  CVector3f GetPositionByTime(float time) const;
#if !RSTL_CORRUPTION
  CVector3f GetPositionByLength(float distance) const;
#endif
  CVector3f GetLookAtByTime(float time) const;
#if !RSTL_CORRUPTION
  CVector3f GetLookAtByLength(float distance) const;
  CQuaternion GetOrientationByTime(float time) const;
  CQuaternion GetOrientationByLength(float distance) const;
#endif

protected:
  // Guessed semantic names, supported by construction and native samplers.
  CMotionSpline mPositionSpline;
  CMayaSpline mPositionTimeSpline;
  CMotionSpline mLookAtSpline;
  CMayaSpline mLookAtTimeSpline;
  rstl::vector< CQuaternion > mOrientations;
  float mDuration;
  uint mFlags;
};
DECLARE_FULL_SIZE_FOR(CSpline, 0x12c)
CHECK_SIZEOF(CSpline, CSpline_FULL_SIZE)

#endif // _CSPLINE
