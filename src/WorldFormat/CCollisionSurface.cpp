#include "WorldFormat/CCollisionSurface.hpp"

#if RSTL_PRIME1
CCollisionSurface::CCollisionSurface(const CVector3f& a, const CVector3f& b, const CVector3f& c,
                                     TFlags flags)
: mA(a), mB(b), mC(c), mFlags(flags) {}
#endif

CCollisionSurface::TNormal CCollisionSurface::GetNormal() const {
  const CVector3f a = mB - mA;
  const CVector3f b = mC - mA;
#if RSTL_COLLISION_SURFACE_UNIT_NORMAL
  return CVector3f::Cross(a, b);
#else
  return CVector3f::Cross(a, b).AsNormalized();
#endif
}

CPlane CCollisionSurface::GetPlane() const {
  const CUnitVector3f normal(GetNormal());
  return CPlane(CVector3f::Dot(normal, GetVert(0)), normal);
}

#if !RSTL_PRIME1
// Guessed name
CPlane CCollisionSurface::GetEdgePlane(int edge) const {
  const CUnitVector3f normal(GetNormal());
  const int nextVertex[] = {1, 2, 0};
  const CVector3f& vertex = GetVert(edge);
  const CVector3f edgeDirection = GetVert(nextVertex[edge]) - vertex;
  const CUnitVector3f edgeNormal(CVector3f::Cross(normal, edgeDirection));
  return CPlane(CVector3f::Dot(edgeNormal, vertex), edgeNormal);
}

// Guessed name
bool CCollisionSurface::IsDegenerate() const { return mA == mB || mB == mC || mA == mC; }
#endif
