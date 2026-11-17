#include "Kyoto/Math/CVector2d.hpp"

#include "Kyoto/Math/CMath.hpp"

CVector2d::CVector2d(double x, double y) : mX(x), mY(y) {}

double CVector2d::Magnitude() const {
  double ret = mX * mX;
  ret += mY * mY;
  return CMath::SqrtD(ret);
}

double CVector2d::MagSquared() const {
  double ret = mX * mX;
  ret += mY * mY;
  return ret;
}

double CVector2d::Cross(const CVector2d& a, const CVector2d& b) {
  return a.GetX() * b.GetY() - a.GetY() * b.GetX();
}

CVector2d operator-(const CVector2d& lhs, const CVector2d& rhs) {
  return CVector2d(lhs.GetX() - rhs.GetX(), lhs.GetY() - rhs.GetY());
}

