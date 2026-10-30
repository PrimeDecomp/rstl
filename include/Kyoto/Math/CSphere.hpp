#ifndef _CSPHERE
#define _CSPHERE
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

class CSphere {
public:
  CSphere(const CVector3f& pos, const float radius) : mCenter(pos), mRadius(radius) {}
#if !RSTL_PRIME1
  explicit CSphere(CInputStream& in) : mCenter(in), mRadius(in.ReadFloat()) {}
#endif

  const CVector3f& GetCenter() const { return mCenter; }
#if !RSTL_PRIME1
  void SetCenter(const CVector3f& center) { mCenter = center; }
#endif
  RSTL_PRIME1_CONST float GetRadius() const { return mRadius; }

  CUnitVector3f GetSurfaceNormal(const CVector3f& v) const;
#if !RSTL_PRIME1
  // Guessed name (target-derived: nearest surface point, or the center when degenerate).
  CVector3f GetSurfacePoint(const CVector3f& v) const;
#endif

private:
  CVector3f mCenter;
  float mRadius;
};
CHECK_SIZEOF(CSphere, 0x10)

#endif // _CSPHERE
