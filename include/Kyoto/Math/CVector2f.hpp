#ifndef _CVECTOR2F
#define _CVECTOR2F

#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"
#include "types.h"
#include "rstl/RstlVersions.h"
#include "rstl/construct.hpp"

class CVector2f {
  static const CVector2f skZeroVector;

public:
#if RSTL_PRIME1
  CVector2f() {}
#endif
  CVector2f(float x, float y);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CVector2f(CInputStream& in) : mX(in.Get< float >()), mY(in.Get< float >()) {}
#else
  CVector2f(CInputStream& in) : mX(in.ReadFloat()), mY(in.ReadFloat()) {}
#endif

  float GetX() const { return mX; }
  float GetY() const { return mY; }
  void SetX(float x) { mX = x; }
  void SetY(float y) { mY = y; }

  CVector2f& operator+=(const CVector2f& rhs);
  CVector2f& operator-=(const CVector2f& rhs);
  CVector2f& operator*=(float rhs);
  CVector2f& operator/=(float rhs);

  CVector2f& Normalize();

  float Magnitude() const;
  bool IsNormalizable() const;
  float MagSquared() const;
  CVector2f AsNormalized() const;

  float& operator[](int idx) { return *(&mX + idx); }

  const float& operator[](int idx) const { return reinterpret_cast< const float* >(this)[idx]; }

  static float GetAngleDiff(const CVector2f& a, const CVector2f& b);
  static float Dot(const CVector2f& a, const CVector2f& b);

  static const CVector2f& Zero() { return skZeroVector; }

  static CVector2f Lerp(const CVector2f& a, const CVector2f& b, RSTL_PRIME1_CONST float t) {
    const float negT = 1.f - t;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1 && RSTL_VERSION >= RSTL_R3IJ
    // Wii rounds each product before adding the two contributions.
    return CVector2f(static_cast< float >(a.mX * negT) + static_cast< float >(b.mX * t),
                     static_cast< float >(a.mY * negT) + static_cast< float >(b.mY * t));
#elif RSTL_PRIME1
    return CVector2f(a.mX * negT + b.mX * t, a.mY * negT + b.mY * t);
#else
    const float aX = a.GetX();
    const float bX = b.GetX();
    const float aY = a.GetY();
    const float bY = b.GetY();
    return CVector2f(aX * negT + bX * t, aY * negT + bY * t);
#endif
  }

  void PutTo(COutputStream& out) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    out.Put(GetX());
    out.Put(GetY());
#else
    out.Put(mX);
    out.Put(mY);
#endif
  }

private:
  float mX;
  float mY;
};

CVector2f operator+(const CVector2f& lhs, const CVector2f& rhs);
CVector2f operator-(const CVector2f& lhs, const CVector2f& rhs);
bool operator==(const CVector2f& lhs, const CVector2f& rhs);
CVector2f operator*(const CVector2f& lhs, const float& rhs);
CVector2f operator*(const float& lhs, const CVector2f& rhs);
CVector2f operator/(const CVector2f& lhs, const float& rhs);

#if RSTL_KYOTO_CONSTRUCTION_POLICIES
namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CVector2f)
RSTL_DECLARE_RESERVED_VECTOR_PUSH_CONSTRUCTION(CVector2f)
RSTL_DECLARE_OPTIONAL_VALUE_CONSTRUCTION(CVector2f)
} // namespace rstl
#endif

#endif // _CVECTOR2F
