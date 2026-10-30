#include "rstl/RstlVersions.h"
#include "Kyoto/Math/CVector3d.hpp"
#include "Kyoto/Math/CMath.hpp"

#if !RSTL_PRIME1
// .bss        start:0x80417408 end:0x804174b0
CVector3d CVector3d::sZeroVector = CVector3d(0.0, 0.0, 0.0);
CVector3d CVector3d::sUpVector = CVector3d(0.0, 0.0, 1.0);
CVector3d CVector3d::sDownVector = CVector3d(0.0, 0.0, -1.0);
CVector3d CVector3d::sLeftVector = CVector3d(-1.0, 0.0, 0.0);
CVector3d CVector3d::sRightVector = CVector3d(1.0, 0.0, 0.0);
CVector3d CVector3d::sForwardVector = CVector3d(0.0, 1.0, 0.0);
CVector3d CVector3d::sBackVector = CVector3d(0.0, -1.0, 0.0);
#endif

CVector3d::CVector3d(double x, double y, double z) : mX(x), mY(y), mZ(z) {}

CVector3d::CVector3d(const CVector3f& other)
: mX(other.GetX()), mY(other.GetY()), mZ(other.GetZ()) {}

double CVector3d::Magnitude() const {
  double ret = mX * mX;
  ret += mY * mY;
  ret += mZ * mZ;
  return CMath::SqrtD(ret);
}

double CVector3d::MagSquared() const {
  double ret = mX * mX;
  ret += mY * mY;
  ret += mZ * mZ;
  return ret;
}

CVector3d CVector3d::AsNormalized() const {
  const double mag = 1.f / Magnitude();
  return CVector3d(mX * mag, mY * mag, mZ * mag);
}

CVector3f CVector3d::AsCVector3f() const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1 || RSTL_VERSION < RSTL_R32J
  float z = mZ;
  float y = mY;
  float x = mX;
  return CVector3f(x, y, z);
#else
  return CVector3f(mX, mY, mZ);
#endif
}

double CVector3d::Dot(const CVector3d& a, const CVector3d& b) {
  double ret = a.GetX() * b.GetX();
  ret += a.GetY() * b.GetY();
  ret += a.GetZ() * b.GetZ();
  return ret;
}

CVector3d CVector3d::Cross(const CVector3d& lhs, const CVector3d& rhs) {
  double x1 = lhs.GetX();
  double y1 = lhs.GetY();
  double z1 = lhs.GetZ();
  double x2 = rhs.GetX();
  double y2 = rhs.GetY();
  double z2 = rhs.GetZ();
  return CVector3d((y1 * z2) - (y2 * z1), (z1 * x2) - (z2 * x1), (x1 * y2) - (x2 * y1));
}

CVector3d operator+(const CVector3d& lhs, const CVector3d& rhs) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return CVector3d(lhs.GetX() + rhs.GetX(), lhs.GetY() + rhs.GetY(), lhs.GetZ() + rhs.GetZ());
#else
  const double x = lhs.GetX() + rhs.GetX();
  const double y = lhs.GetY() + rhs.GetY();
  const double z = lhs.GetZ() + rhs.GetZ();
  return CVector3d(x, y, z);
#endif
}

CVector3d operator-(const CVector3d& lhs, const CVector3d& rhs) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return CVector3d(lhs.GetX() - rhs.GetX(), lhs.GetY() - rhs.GetY(), lhs.GetZ() - rhs.GetZ());
#else
  const double x = lhs.GetX() - rhs.GetX();
  const double y = lhs.GetY() - rhs.GetY();
  const double z = lhs.GetZ() - rhs.GetZ();
  return CVector3d(x, y, z);
#endif
}

#if !RSTL_PRIME1
CVector3d operator-(const CVector3d& v) { return CVector3d(-v.GetX(), -v.GetY(), -v.GetZ()); }
#endif

CVector3d operator*(double lhs, const CVector3d& rhs) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return CVector3d(lhs * rhs.GetX(), lhs * rhs.GetY(), lhs * rhs.GetZ());
#else
  const double x = lhs * rhs.GetX();
  const double y = lhs * rhs.GetY();
  const double z = lhs * rhs.GetZ();
  return CVector3d(x, y, z);
#endif
}

