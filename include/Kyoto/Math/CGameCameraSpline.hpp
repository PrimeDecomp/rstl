#ifndef _CGAMECAMERASPLINE
#define _CGAMECAMERASPLINE

#include "Kyoto/Math/CSpline.hpp"

// Guessed name.
class CGameCameraSpline : public CSpline {
public:
  CGameCameraSpline(float duration, uint flags, const CMayaSpline& positionTimeSpline,
                    const CMayaSpline& lookAtTimeSpline, const CMayaSpline& fovSpline,
                    const CMayaSpline& rollSpline, CMotionSpline::ESplineType positionType,
                    CMotionSpline::ESplineType lookAtType);

  // CSpline
  ~CGameCameraSpline() override;

  // Guessed names, established by the runtime path-camera caller.
  float GetFovByLength(float distance) const;
  float GetFovByTime(float time) const;
  float GetRollByTime(float time) const; // Guessed name; cinematic roll in degrees.

private:
  // Guessed semantic names, corroborated by camera FOV and cinematic-roll consumers.
  CMayaSpline mFovSpline;
  CMayaSpline mRollSpline;
};
CHECK_CHILD_SIZEOF(CGameCameraSpline, CSpline, 0x88)

#endif // _CGAMECAMERASPLINE
