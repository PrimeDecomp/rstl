#ifndef _CVECTOR3F
#define _CVECTOR3F

#include "types.h"

#include "Kyoto/Math/CVector2f.hpp"
#include "rstl/RstlVersions.h"
#include "rstl/construct.hpp"

#include "float.h"

class CInputStream;
class COutputStream;

class CRelAngle;
class CUnitVector3f;

enum EDimX { kDX = 0 };
enum EDimY { kDY = 1 };
enum EDimZ { kDZ = 2 };

class CVector3f {
public:
  CVector3f() {}
  explicit CVector3f(float x, float y, float z) : mX(x), mY(y), mZ(z) {}
  CVector3f(const CVector2f& v, float z) : mX(v.GetX()), mY(v.GetY()), mZ(z) {}

  CVector3f(CInputStream& in);
  void PutTo(COutputStream& out) const;

#if RSTL_KYOTO_CONST_SCALAR_RETURNS
  const float GetX() const { return mX; }
  const float GetY() const { return mY; }
  const float GetZ() const { return mZ; }
#else
  float GetX() const { return mX; }
  float GetY() const { return mY; }
  float GetZ() const { return mZ; }
#endif

  void SetX(float x) { mX = x; }
  void SetY(float y) { mY = y; }
  void SetZ(float z) { mZ = z; }

  static CVector3f ByElementMultiply(const CVector3f& lhs, const CVector3f& rhs) {
    return CVector3f(lhs.GetX() * rhs.GetX(), lhs.GetY() * rhs.GetY(), lhs.GetZ() * rhs.GetZ());
  }

  static CVector3f Slerp(const CVector3f& a, const CVector3f& b, const CRelAngle& angle);
  CVector3f& Normalize();
  float Magnitude() const;
  CVector3f AsNormalized() const;
  bool IsNotInf() const;
  bool IsNormalizable() const;
  bool CanBeNormalized() const;
  static float GetAngleDiff(const CVector3f& a, const CVector3f& b);
  bool IsEqu(const CVector3f& other, float epsilon = FLT_EPSILON) const;
  static CVector3f Lerp(const CVector3f& a, const CVector3f& b, float v) {
    float inv = 1.f - v;
    float x = a.mX * inv + b.mX * v;
    float y = a.mY * inv + b.mY * v;
    float z = a.mZ * inv + b.mZ * v;
    return CVector3f(x, y, z);
  }
  inline float MagSquared() const { return GetX() * GetX() + GetY() * GetY() + GetZ() * GetZ(); }
  static CVector3f Cross(const CVector3f& lhs, const CVector3f& rhs) {
    float lX = lhs.GetX();
    float lY = lhs.GetY();
    float lZ = lhs.GetZ();
    float rX = rhs.GetX();
    float rY = rhs.GetY();
    float rZ = rhs.GetZ();
    return CVector3f(lY * rZ - rY * lZ, lZ * rX - rZ * lX, lX * rY - rX * lY);
  }

  float& operator[](EDimX dim) { return mX; }
  float& operator[](EDimY dim) { return mY; }
  float& operator[](EDimZ dim) { return mZ; }
  const float& operator[](EDimX) const { return mX; }
  const float& operator[](EDimY) const { return mY; }
  const float& operator[](EDimZ) const { return mZ; }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  float& operator[](const int i) { return reinterpret_cast< float* >(this)[i]; }
  const float operator[](const int i) const { return (&mX)[i]; }
#else
  float& operator[](int i) { return (&mX)[i]; }
  float operator[](int i) const { return (&mX)[i]; }
#endif
  const float* GetArrayPtr() const { return &mX; } // Guessed name
  bool IsNonZero() const { return mX != 0.f || mY != 0.f || mZ != 0.f; }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CVector2f DropZ() const { return CVector2f(mX, mY); }
#else
  CVector3f DropZ() const { return CVector3f(mX, mY, 0.f); }
#endif

  CVector3f& operator+=(const CVector3f& other) {
    mX += other.mX;
    mY += other.mY;
    mZ += other.mZ;
    return *this;
  }
  CVector3f& operator-=(const CVector3f& other) {
    mX -= other.mX;
    mY -= other.mY;
    mZ -= other.mZ;
    return *this;
  }
  CVector3f& operator*=(const float v) {
    mX *= v;
    mY *= v;
    mZ *= v;
    return *this;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CVector3f& operator/=(const float v) { return *this *= (1.f / v); }
#else
  CVector3f& operator/=(const float v) {
    mX /= v;
    mY /= v;
    mZ /= v;
    return *this;
  }
#endif

  CVector2f ToVec2f() const { return CVector2f(mX, mY); }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static const float Dot(const CVector3f& a, const CVector3f& b) {
#if RSTL_VERSION >= RSTL_R3IJ
    return a.mX * b.mX + a.mY * b.mY + a.mZ * b.mZ;
#else
    return (a.GetX() * b.GetX()) + (a.GetY() * b.GetY()) + (a.GetZ() * b.GetZ());
#endif
#else
  static float Dot(const CVector3f& a, const CVector3f& b) {
    return (a.mX * b.mX) + (a.mY * b.mY) + (a.mZ * b.mZ);
#endif
  }

  static const CVector3f& Zero() { return sZeroVector; }
#if RSTL_PRIME1
  static const CVector3f& One() { return sOneVector; }
#endif
  static const CUnitVector3f& Up();
  static const CUnitVector3f& Down();
  static const CUnitVector3f& Left();
  static const CUnitVector3f& Right();
  static const CUnitVector3f& Forward();
  static const CUnitVector3f& Back();
#if !RSTL_PRIME1
  static const CVector3f& One() { return sOneVector; }
#endif

#if RSTL_PRIME1
  friend CVector3f operator-(const CVector3f& lhs, const CVector3f& rhs);
  friend CVector3f operator+(const CVector3f& lhs, const CVector3f& rhs);
  friend CVector3f operator*(const CVector3f& lhs, const CVector3f& rhs);
  friend CVector3f operator*(const CVector3f& vec, const float f);
  friend CVector3f operator*(const float f, const CVector3f& vec);
  friend CVector3f operator/(const CVector3f& vec, const float f);
  friend CVector3f operator-(const CVector3f& vec);
  friend bool operator==(const CVector3f& lhs, const CVector3f& rhs);
  friend bool operator!=(const CVector3f& lhs, const CVector3f& rhs);
#endif

protected:
  float mX;
  float mY;
  float mZ;

  static CVector3f sZeroVector;
#if RSTL_PRIME1
  static CVector3f sOneVector;
#endif
  static CUnitVector3f sUpVector;
  static CUnitVector3f sDownVector;
  static CUnitVector3f sLeftVector;
  static CUnitVector3f sRightVector;
  static CUnitVector3f sForwardVector;
  static CUnitVector3f sBackVector;
#if !RSTL_PRIME1
  static CVector3f sOneVector;
#endif
};
DECLARE_FULL_SIZE_FOR(CVector3f, 0xc)
CHECK_SIZEOF(CVector3f, CVector3f_FULL_SIZE)

#if RSTL_KYOTO_CONSTRUCTION_POLICIES
namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CVector3f)
RSTL_DECLARE_RESERVED_VECTOR_PUSH_CONSTRUCTION(CVector3f)
RSTL_DECLARE_OPTIONAL_VALUE_CONSTRUCTION(CVector3f)
} // namespace rstl
#endif

// ClassifyVector__FRC9CVector3f
// TGetType<9CVector3f>__FRC9CVector3f
// close_enough__FRC9CVector3fRC9CVector3ff in CloseEnough.cpp

inline bool operator==(const CVector3f& lhs, const CVector3f& rhs) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return lhs.mX == rhs.mX && lhs.mY == rhs.mY && lhs.mZ == rhs.mZ;
#else
  return lhs.GetX() == rhs.GetX() && lhs.GetY() == rhs.GetY() && lhs.GetZ() == rhs.GetZ();
#endif
}
inline bool operator!=(const CVector3f& lhs, const CVector3f& rhs) {
  return lhs.GetX() != rhs.GetX() || lhs.GetY() != rhs.GetY() || lhs.GetZ() != rhs.GetZ();
}

inline CVector3f operator-(const CVector3f& lhs, const CVector3f& rhs) {
#if !RSTL_PRIME1 || RSTL_VERSION >= RSTL_R3IJ
  float x = lhs.GetX() - rhs.GetX();
  float y = lhs.GetY() - rhs.GetY();
  float z = lhs.GetZ() - rhs.GetZ();
#else
  float x = lhs.mX - rhs.mX;
  float y = lhs.mY - rhs.mY;
  float z = lhs.mZ - rhs.mZ;
#endif
  return CVector3f(x, y, z);
}

inline CVector3f operator+(const CVector3f& lhs, const CVector3f& rhs) {
#if !RSTL_PRIME1 || RSTL_VERSION >= RSTL_R3IJ
  float x = lhs.GetX() + rhs.GetX();
  float y = lhs.GetY() + rhs.GetY();
  float z = lhs.GetZ() + rhs.GetZ();
#else
  float x = lhs.mX + rhs.mX;
  float y = lhs.mY + rhs.mY;
  float z = lhs.mZ + rhs.mZ;
#endif
  return CVector3f(x, y, z);
}

#if RSTL_PRIME1
inline CVector3f operator*(const CVector3f& lhs, const CVector3f& rhs) {
  CVector3f ret(lhs);
  ret.mX *= rhs.mX;
  ret.mY *= rhs.mY;
  ret.mZ *= rhs.mZ;
  return ret;
}
#endif

inline CVector3f operator*(const CVector3f& vec, const float f) {
  float x = vec.GetX() * f;
  float y = vec.GetY() * f;
  float z = vec.GetZ() * f;
  return CVector3f(x, y, z);
}

inline CVector3f operator*(const float f, const CVector3f& vec) {
  float x = f * vec.GetX();
  float y = f * vec.GetY();
  float z = f * vec.GetZ();
  return CVector3f(x, y, z);
}

inline CVector3f operator/(const CVector3f& vec, const float f) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  float n = (1.f / f);
  float x = vec.GetX() * n;
  float y = vec.GetY() * n;
  float z = vec.GetZ() * n;
#else
  float x = vec.GetX() / f;
  float y = vec.GetY() / f;
  float z = vec.GetZ() / f;
#endif
  return CVector3f(x, y, z);
}

inline CVector3f operator-(const CVector3f& vec) {
#if !RSTL_PRIME1 && RSTL_VERSION >= RSTL_R32J
  const float x = -vec.GetX();
  const float y = -vec.GetY();
  const float z = -vec.GetZ();
  return CVector3f(x, y, z);
#else
  return CVector3f(-vec.GetX(), -vec.GetY(), -vec.GetZ());
#endif
}

#include "Kyoto/Math/CUnitVector3f.hpp"

#endif // _CVECTOR3F
