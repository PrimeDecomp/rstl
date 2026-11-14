#ifndef _CRELANGLE
#define _CRELANGLE
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#include "rstl/types.h"

#include "Kyoto/Math/CMath.hpp"

class CRelAngle {
public:
  CRelAngle(float rad) : mAngle(rad) {}

  float AsDegrees() const { return mAngle * (180.f / M_PIF); }
  float AsRadians() const { return mAngle; }
#if RSTL_PRIME1
  static CRelAngle ArcCosine(float v) { return FromRadians(acosf(v)); }
#endif

  CRelAngle& operator+=(const CRelAngle& v) {
    mAngle += v.mAngle;
    return *this;
  }
  CRelAngle& operator-=(const CRelAngle& v) {
    mAngle -= v.mAngle;
    return *this;
  }
  CRelAngle& operator*=(float v) {
    mAngle *= v;
    return *this;
  }
  CRelAngle& operator/=(float v) {
    mAngle /= v;
    return *this;
  }

#if RSTL_PRIME1
  bool operator<(const CRelAngle& rhs) const { return mAngle < rhs.mAngle; }
#endif

  static CRelAngle FromDegrees(RSTL_PRIME1_CONST float deg) {
    return CRelAngle(deg * (M_PIF / 180.f));
  }
  static CRelAngle FromRadians(RSTL_PRIME1_CONST float rad) { return CRelAngle(rad); }

private:
  float mAngle;
};
CHECK_SIZEOF(CRelAngle, 0x4)

#if RSTL_PRIME1
inline CRelAngle operator-(const CRelAngle& a, const CRelAngle& b) {
  CRelAngle out(a);
  out -= b;
  return out;
}
inline CRelAngle operator+(const CRelAngle& a, const CRelAngle& b) {
  CRelAngle out(a);
  out += b;
  return out;
}
#endif

inline CRelAngle operator/(const CRelAngle& a, float b) {
  CRelAngle out(a);
  out /= b;
  return out;
}
inline CRelAngle operator*(const CRelAngle& a, float b) {
  CRelAngle out(a);
  out *= b;
  return out;
}
inline float sine(const CRelAngle& angle) { return sin(angle.AsRadians()); }
inline float cosine(const CRelAngle& angle) { return cos(angle.AsRadians()); }
#if !RSTL_PRIME1

// Guessed name; native Wii portal clipping calls tan on an angle reference.
static inline float tangent(const CRelAngle& angle) { return tan(angle.AsRadians()); }
#endif

#endif // _CRELANGLE
