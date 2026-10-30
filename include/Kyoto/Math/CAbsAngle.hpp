#ifndef _CABSANGLE
#define _CABSANGLE
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"

#if RSTL_PRIME1
class CRelAngle;
#endif

class CAbsAngle {
public:
  float AsDegrees() const { return mAngle * (180.f / M_PIF); }
  float AsRadians() const { return mAngle; }
  // ArcCosine__9CAbsAngleFf weak
  // -> calls ArcCosineR__5CMathFf

  CAbsAngle& operator+=(const CAbsAngle& v) {
    mAngle += v.mAngle;
    return *this;
  }

  CAbsAngle& operator-=(const CAbsAngle& v) {
    mAngle -= v.mAngle;
    return *this;
  }
  CAbsAngle& operator*=(float v) {
    mAngle *= v;
    return *this;
  }
  CAbsAngle& operator/=(float v) {
    mAngle /= v;
    return *this;
  }

  CAbsAngle& operator+=(const CRelAngle& v) {
    mAngle = CMath::ClampRadians(mAngle + v.AsRadians());
    return *this;
  }

  CAbsAngle& operator-=(const CRelAngle& v) {
    mAngle = CMath::ClampRadians(mAngle - v.AsRadians());
    return *this;
  }

  static CAbsAngle FromDegrees(float deg) {
    return CAbsAngle(CMath::ClampRadians(deg * (M_PIF / 180.f)));
  }
  static CAbsAngle FromRadians(float rad) { return CAbsAngle(CMath::ClampRadians(rad)); }

private:
  CAbsAngle(float rad) : mAngle(rad) {}

  float mAngle;
};
CHECK_SIZEOF(CAbsAngle, 0x4)

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
inline CRelAngle operator-(const CAbsAngle& lhs, const CAbsAngle& rhs) {
  return CRelAngle::FromRadians(lhs.AsRadians() - rhs.AsRadians());
#else
// __mi__FRC9CAbsAngleRC9CAbsAngle
inline CRelAngle operator-(const CAbsAngle& a, const CAbsAngle& b) {
  return CRelAngle::FromRadians(a.AsRadians() - b.AsRadians());
#endif

}

static inline float cosine(const CAbsAngle& angle) { return cos(angle.AsRadians()); }

#endif // _CABSANGLE
