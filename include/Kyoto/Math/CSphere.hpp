#ifndef _CSPHERE
#define _CSPHERE
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

class CSphere {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CSphere(const CVector3f& pos, const float radius) : mCenter(pos), mRadius(radius){}
#else
  CSphere(const CVector3f& pos, const float radius) : mCenter(pos), mRadius(radius) {}

  explicit CSphere(CInputStream& in) : mCenter(in), mRadius(in.ReadFloat()) {}
#endif

  const CVector3f& GetCenter() const { return mCenter; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const float GetRadius() const { return mRadius; }
#else
  void SetCenter(const CVector3f& center) { mCenter = center; }
  float GetRadius() const { return mRadius; }
#endif

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
