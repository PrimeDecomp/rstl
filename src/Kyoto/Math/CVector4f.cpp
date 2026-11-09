#include "Kyoto/Math/CVector4f.hpp"

#include "Kyoto/Math/CMath.hpp"

CVector4f CVector4f::sZeroVector(0.f, 0.f, 0.f, 0.f);
CVector4f CVector4f::sOneRGBVector(1.f, 1.f, 1.f, 0.f);

float CVector4f::Magnitude() const {
  float mag = mX * mX;
  mag += mY * mY;
  mag += mZ * mZ;
  mag += mW * mW;
  return CMath::SqrtF(mag);
}

