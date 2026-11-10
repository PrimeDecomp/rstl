#include "rstl/RstlVersions.h"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Basics/CBasics.hpp"

#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"
#include "types.h"

#ifndef __MWERKS__
// TODO
#define __HI(x) (*(int*)&x)
#endif

CVector3f CVector3f::sZeroVector(0.f, 0.f, 0.f);
CUnitVector3f CVector3f::sUpVector(0.f, 0.f, 1.f);
CUnitVector3f CVector3f::sDownVector(0.f, 0.f, -1.f);
CUnitVector3f CVector3f::sLeftVector(-1.f, 0.f, 0.f);
CUnitVector3f CVector3f::sRightVector(1.f, 0.f, 0.f);
CUnitVector3f CVector3f::sForwardVector(0.f, 1.f, 0.f);
CUnitVector3f CVector3f::sBackVector(0.f, -1.f, 0.f);
#if !RSTL_PRIME1
CVector3f CVector3f::sOneVector(1.f, 1.f, 1.f);
#endif

CVector3f::CVector3f(CInputStream& in) {
  in.Get(this, sizeof(CVector3f));
  mX = CBasics::SwapBytes(mX);
  mY = CBasics::SwapBytes(mY);
  mZ = CBasics::SwapBytes(mZ);
}

void CVector3f::PutTo(COutputStream& out) const {
  out.WriteReal32(mX);
  out.WriteReal32(mY);
  out.WriteReal32(mZ);
}

CVector3f CVector3f::Slerp(const CVector3f& a, const CVector3f& b, const CRelAngle& angle) {
#if RSTL_KYOTO_VECTOR3F_NORMALIZE_MAGNITUDE
  CVector3f axb = CVector3f::Cross(a, b);
  if (!axb.IsNormalizable()) {
    return a;
  }
  axb.Normalize();
#else
  CVector3f axb = CVector3f::Cross(a, b).AsNormalized();
#endif
  CVector3f crs = CVector3f::Cross(axb, a);
  float asin = sine(angle);
  float acos = cosine(angle);
  return acos * a + asin * crs;
}

#if RSTL_KYOTO_VECTOR3F_NORMALIZE_MAGNITUDE
float CVector3f::Normalize() {
  const float mag = Magnitude();
  const float invMag = 1.f / mag;
  mX *= invMag;
  mY *= invMag;
  mZ *= invMag;
  return mag;
}
#else
CVector3f& CVector3f::Normalize() {
  float mag = 1.f / Magnitude();
  mX *= mag;
  mY *= mag;
  mZ *= mag;
  return *this;
}
#endif

float CVector3f::NormalizeWithMagnitude() {
  const float magSquared = MagSquared();
  const float invMag = CMath::InvSqrtF(magSquared);
  mX *= invMag;
  mY *= invMag;
  mZ *= invMag;
  return magSquared * invMag;
}

float CVector3f::Magnitude() const {
  float mag = mX * mX;
  mag += mY * mY;
  mag += mZ * mZ;
  return CMath::SqrtF(mag);
}

CVector3f CVector3f::AsNormalized() const {
  float mag = 1.f / Magnitude();
  float x = mX * mag;
  float y = mY * mag;
  float z = mZ * mag;
  return CVector3f(x, y, z);
}

bool CVector3f::IsNotInf() const {
  int x = __HI(mX);
  int y = __HI(mY);
  int z = __HI(mZ);
  if ((x & 0x7f800000) == 0x7f800000 || (y & 0x7f800000) == 0x7f800000 ||
      (z & 0x7f800000) == 0x7f800000) {
    return false;
  }

  return true;
}

bool CVector3f::IsNormalizable() const { return IsNotInf() && MagSquared() >= 9.9999995e-29f; }

bool CVector3f::CanBeNormalized() const {
  int x = __HI(mX);
  int y = __HI(mY);
  int z = __HI(mZ);
  if ((x & 0x7f800000) == 0x7f800000 || (y & 0x7f800000) == 0x7f800000 ||
      (z & 0x7f800000) == 0x7f800000) {
    return false;
  }

  if (CMath::AbsF(mX) < FLT_EPSILON && CMath::AbsF(mY) < FLT_EPSILON &&
      CMath::AbsF(mZ) < FLT_EPSILON) {
    return false;
  }

  return true;
}

float CVector3f::GetAngleDiff(const CVector3f& a, const CVector3f& b) {
  float mag1 = a.Magnitude();
  float mag2 = b.Magnitude();

  float ret = 0.f;
  if (mag1 > FLT_EPSILON && mag2 > FLT_EPSILON) {
    float dist = CVector3f::Dot(a, b) / (mag1 * mag2);
    if (dist < -1.f) {
      dist = -1.f;
    } else if (dist > 1.f) {
      dist = 1.f;
    }
    ret = CMath::ArcCosineR(dist);
  }
  return ret;
}

bool CVector3f::IsEqu(const CVector3f& vec, float epsilon) const {
  return CMath::AbsF(mX - vec.mX) <= epsilon && CMath::AbsF(mY - vec.mY) <= epsilon &&
         CMath::AbsF(mZ - vec.mZ) <= epsilon;
}

