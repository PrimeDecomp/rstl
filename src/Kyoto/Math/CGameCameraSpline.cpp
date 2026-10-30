#include "Kyoto/Math/CGameCameraSpline.hpp"

CGameCameraSpline::CGameCameraSpline(float duration, uint flags,
                                     const CMayaSpline& positionTimeSpline,
                                     const CMayaSpline& lookAtTimeSpline,
                                     const CMayaSpline& fovSpline, const CMayaSpline& rollSpline,
                                     CMotionSpline::ESplineType positionType,
                                     CMotionSpline::ESplineType lookAtType)
: CSpline(duration, flags, positionTimeSpline, lookAtTimeSpline, positionType, lookAtType)
, mFovSpline(fovSpline)
, mRollSpline(rollSpline) {}

CGameCameraSpline::~CGameCameraSpline() {}

float CGameCameraSpline::GetFovByTime(float time) const { return mFovSpline.EvaluateAt(time); }

float CGameCameraSpline::GetFovByLength(float distance) const {
  float time = 0.f;
  if (GetLookAtSpline().GetControlPointCount() != 0) {
    time = GetDuration() * (distance / GetLookAtSpline().GetLength());
  } else if (GetPositionSpline().GetControlPointCount() != 0) {
    time = GetDuration() * (distance / GetPositionSpline().GetLength());
  }
  return mFovSpline.EvaluateAt(time);
}

float CGameCameraSpline::GetRollByTime(float time) const { return mRollSpline.EvaluateAt(time); }

