#ifndef _CVECTOR4F
#define _CVECTOR4F

#include "rstl/static_assert.hpp"
#include "rstl/types.h"

// Guessed name
class CVector4f {
public:
  CVector4f() {}
  CVector4f(float x, float y, float z, float w) : mX(x), mY(y), mZ(z), mW(w) {}

  float GetX() const { return mX; }
  float GetY() const { return mY; }
  float GetZ() const { return mZ; }
  float GetW() const { return mW; }
  void SetX(float x) { mX = x; }
  void SetY(float y) { mY = y; }
  void SetZ(float z) { mZ = z; }
  void SetW(float w) { mW = w; }
  float Magnitude() const;
  float& operator[](int index) { return (&mX)[index]; }
  float operator[](int index) const { return (&mX)[index]; }

  CVector4f& operator+=(const CVector4f& other) {
    mX += other.mX;
    mY += other.mY;
    mZ += other.mZ;
    mW += other.mW;
    return *this;
  }

  CVector4f& operator*=(float scale) {
    mX *= scale;
    mY *= scale;
    mZ *= scale;
    mW *= scale;
    return *this;
  }

  static const CVector4f& Zero() { return sZeroVector; }

private:
  float mX;
  float mY;
  float mZ;
  float mW;
  static CVector4f sZeroVector;
  static CVector4f sOneRGBVector; // Guessed name
};
CHECK_SIZEOF(CVector4f, 0x10)

inline CVector4f operator+(const CVector4f& a, const CVector4f& b) {
  return CVector4f(a.GetX() + b.GetX(), a.GetY() + b.GetY(), a.GetZ() + b.GetZ(),
                   a.GetW() + b.GetW());
}

inline CVector4f operator*(float scale, const CVector4f& v) {
  float x = scale * v.GetX();
  float y = scale * v.GetY();
  float z = scale * v.GetZ();
  float w = scale * v.GetW();
  return CVector4f(x, y, z, w);
}

inline CVector4f operator*(const CVector4f& v, float scale) {
  float x = v.GetX() * scale;
  float y = v.GetY() * scale;
  float z = v.GetZ() * scale;
  float w = v.GetW() * scale;
  return CVector4f(x, y, z, w);
}

#endif // _CVECTOR4F
