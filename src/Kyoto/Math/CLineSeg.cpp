#include "Kyoto/Math/CLineSeg.hpp"

// Guessed name
CVector3f CLineSeg::GetClosestPoint(const CVector3f& point) const {
  const CVector3f fromStart = point - GetRefPoint();
  const float startDot = CVector3f::Dot(fromStart, GetNormal());
  if (startDot < 0.f) {
    return GetRefPoint();
  }

  const CVector3f fromEnd = point - mEnd;
  const float endDot = CVector3f::Dot(fromEnd, GetNormal());
  if (endDot > 0.f) {
    return mEnd;
  }

  return CLine::GetClosestPoint(point);
}

