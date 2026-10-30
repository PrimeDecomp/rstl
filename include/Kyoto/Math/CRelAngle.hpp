#ifndef _CRELANGLE
#define _CRELANGLE
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/Math/CMath.hpp"

class CRelAngle {
public:
#if RSTL_PRIME1
  CRelAngle(float rad) : mAngle(rad) {}
#endif

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

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static CRelAngle FromDegrees(const float deg) { return CRelAngle(deg * (M_PIF / 180.f)); }
  static CRelAngle FromRadians(const float rad) { return CRelAngle(rad); }
#else
  static CRelAngle FromDegrees(float deg) { return CRelAngle(deg * (M_PIF / 180.f)); }
  static CRelAngle FromRadians(float rad) { return CRelAngle(rad); }
#endif

private:
#if !RSTL_PRIME1
  CRelAngle(float rad) : mAngle(rad) {}
#endif

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
inline float sine(const CRelAngle& angle) { return sin(angle.AsRadians()); }
inline float cosine(const CRelAngle& angle) { return cos(angle.AsRadians()); }
#else
static inline float sine(const CRelAngle& angle) { return sin(angle.AsRadians()); }
static inline float cosine(const CRelAngle& angle) { return cos(angle.AsRadians()); }

// Guessed name; native Wii portal clipping calls tan on an angle reference.
static inline float tangent(const CRelAngle& angle) { return tan(angle.AsRadians()); }
#endif

#endif // _CRELANGLE
