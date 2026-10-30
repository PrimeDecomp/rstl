#include "rstl/RstlVersions.h"
#include "Kyoto/Math/CPlane.hpp"
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "Kyoto/Math/CMath.hpp"
#else
#include "Kyoto/Streams/CInputStream.hpp"
#endif

CPlane::CPlane(const CVector3f& a, const CVector3f& b, const CVector3f& c)
: mNormal(CVector3f::Cross(b - a, c - a)), mConstant(CVector3f::Dot(mNormal, a)) {}
#if !RSTL_PRIME1
CPlane::CPlane(CInputStream& in)
: mNormal(in), mConstant(in.ReadFloat()) {}
#endif

float CPlane::ClipLineSegment(const CVector3f& start, const CVector3f& end) const {
  float dist = -(CVector3f::Dot(start, GetNormal()) - GetConstant()) /
               CVector3f::Dot(end - start, GetNormal());
  return dist <= 0.f ? 0.f : (dist >= 1.f ? 1.f : dist);
}

#if !RSTL_PRIME1
CVector3f CPlane::GetClosestPoint(const CVector3f& point) const {
  return point - GetHeight(point) * GetNormal();
}
#endif

