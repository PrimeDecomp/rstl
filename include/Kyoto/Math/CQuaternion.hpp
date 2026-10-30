#ifndef _CQUATERNION
#define _CQUATERNION
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CTransform4f.hpp"

class CRelAngle;
class CUnitVector3f;
class CNUQuaternion;

class CQuaternion {
public:
  CQuaternion() {}
  CQuaternion(CInputStream& in);
  CQuaternion(float w, float x, float y, float z) : w(w), imaginary(x, y, z) {}
  CQuaternion(float w, const CVector3f& imaginary) : w(w), imaginary(imaginary) {}

  CQuaternion operator*(const CQuaternion&) const;
  CQuaternion& operator*=(const CQuaternion& other) {
    *this = *this * other;
    return *this;
  }
  static CQuaternion ScalarVector(float w, const CVector3f& imaginary) {
    return CQuaternion(w, imaginary);
  }
  static CQuaternion ShortestRotationArc(const CVector3f&, const CVector3f&);

  static CQuaternion LookAt(const CUnitVector3f&, const CUnitVector3f&, const CRelAngle&);
  static CQuaternion ShortestRotationArcClamped(const CVector3f&, const CVector3f&,
                                                const CRelAngle&);

  bool IsValidQuaternion(float epsilon) const;
  static CQuaternion Slerp(const CQuaternion& a, const CQuaternion& b, float t);
  static CQuaternion SlerpLocal(const CQuaternion& from, const CQuaternion& to, float t);
  CRelAngle AngleFrom(const CQuaternion& other) const;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CQuaternion BuildEquivalent() const;
#else
  CQuaternion BuildEquivalent() const { return CQuaternion(-w, -imaginary); }
#endif

  CQuaternion BuildNormalized() const;
  static CQuaternion AxisAngle(const CUnitVector3f&, const CRelAngle&);
  CVector3f Transform(const CVector3f&) const;
  static CQuaternion XRotation(const CRelAngle&);
  static CQuaternion YRotation(const CRelAngle&);
  static CQuaternion ZRotation(const CRelAngle&);
  static CQuaternion YXZRotation(const CRelAngle& y, const CRelAngle& x, const CRelAngle& z) {
    return ZRotation(z) * XRotation(x) * YRotation(y);
  }
  CMatrix3f BuildTransform() const;
  CTransform4f BuildTransform4f() const;
  CTransform4f BuildTransform4f(const CVector3f&) const;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CQuaternion BuildInverted() const { return ScalarVector(GetScalar(), -GetVector()); }
#else
  CQuaternion BuildInverted() const {
    return CQuaternion(GetScalar(), CVector3f(-AxisX(), -AxisY(), -AxisZ()));
  }
#endif

  static CQuaternion FromMatrixRows(const CVector3f&, const CVector3f&, const CVector3f&);
#if !RSTL_PRIME1
  static CQuaternion FromMatrixColumns(const CVector3f&, const CVector3f&, const CVector3f&);
#endif

  static CQuaternion FromMatrix(const CMatrix3f&);
  static CQuaternion FromMatrix(const CTransform4f&);
  static CQuaternion FromNUQuaternion(const CNUQuaternion&);

  static const CQuaternion& NoRotation() { return sNoRotation; }

  static float Dot(const CQuaternion& a, const CQuaternion& b) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return a.GetScalar() * b.GetScalar() + CVector3f::Dot(a.GetVector(), b.GetVector());
#else
    return CVector3f::Dot(a.GetVector(), b.GetVector()) + a.GetScalar() * b.GetScalar();
#endif

  }

  bool LocalTo(const CQuaternion& other) const { return Dot(other, *this) >= 0.f; }

  static CQuaternion MadeLocalToFirst(const CQuaternion& first, const CQuaternion& second) {
    return second.LocalTo(first) ? second : second.BuildEquivalent();
  }

  float GetScalar() const { return w; }
  const CVector3f& GetVector() const { return imaginary; }
  float AxisX() const { return imaginary.GetX(); }
  float AxisY() const { return imaginary.GetY(); }
  float AxisZ() const { return imaginary.GetZ(); }

private:
  float w;
  CVector3f imaginary;

  static const CQuaternion sNoRotation;
};
CHECK_SIZEOF(CQuaternion, 0x10)

#if !RSTL_PRIME1
namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CQuaternion)
} // namespace rstl
#endif

#endif // _CQUATERNION
