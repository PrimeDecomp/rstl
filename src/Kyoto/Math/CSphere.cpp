#include "rstl/RstlVersions.h"
#include "Kyoto/Math/CSphere.hpp"

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"

CUnitVector3f CSphere::GetSurfaceNormal(const CVector3f& vec) const {
  return CUnitVector3f(vec - mCenter);
}

#if !RSTL_PRIME1
CVector3f CSphere::GetSurfacePoint(const CVector3f& vec) const {
  const CVector3f delta = vec - mCenter;
  if (delta.IsNormalizable()) {
    return mCenter + delta.AsNormalized() * mRadius;
  }
  return mCenter;
}
#endif

#if RSTL_KYOTO_SPHERE_BOUNDING_BOX
CAABox CSphere::GetBoundingBox() const {
  const CVector3f extent(mRadius, mRadius, mRadius);
  return CAABox(mCenter - extent, mCenter + extent);
}

CAABox CSphere::GetTransformedBoundingBox(const CTransform4f& xf) const {
  const float radius = mRadius;
  const CVector3f center = xf * mCenter;
  const CVector3f extent(radius, radius, radius);
  return CAABox(center - extent, center + extent);
}
#endif
