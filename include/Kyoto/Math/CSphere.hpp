#ifndef _CSPHERE
#define _CSPHERE
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

class CAABox;
class CTransform4f;

class CSphere {
public:
  CSphere(const CVector3f& pos, RSTL_KYOTO_SCALAR_PARAM_CONST float radius) : mCenter(pos), mRadius(radius) {}
#if !RSTL_PRIME1
  explicit CSphere(CInputStream& in) : mCenter(in), mRadius(in.ReadFloat()) {}
#endif

  const CVector3f& GetCenter() const { return mCenter; }
#if !RSTL_PRIME1
  void SetCenter(const CVector3f& center) { mCenter = center; }
#endif
  RSTL_PRIME1_CONST float GetRadius() const { return mRadius; }

  CUnitVector3f GetSurfaceNormal(const CVector3f& v) const;
  // Guessed name (target-derived: nearest surface point, or the center when degenerate).
  CVector3f GetSurfacePoint(const CVector3f& v) const;
  // Guessed names
  CAABox GetBoundingBox() const;
  CAABox GetTransformedBoundingBox(const CTransform4f& xf) const;

private:
  CVector3f mCenter;
  float mRadius;
};
CHECK_SIZEOF(CSphere, 0x10)

#endif // _CSPHERE
