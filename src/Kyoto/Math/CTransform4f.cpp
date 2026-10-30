#include "rstl/RstlVersions.h"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#if !RSTL_PRIME1
#include "Kyoto/Math/CUnitVector3f.hpp"
#endif

#include "Kyoto/Streams/CInputStream.hpp"

const CTransform4f CTransform4f::sIdentity(1.f, 0.f, 0.f, 0.f, //
                                           0.f, 1.f, 0.f, 0.f, //
                                           0.f, 0.f, 1.f, 0.f  //
);

CTransform4f CTransform4f::LookAt(const CVector3f& pos, const CVector3f& lookPos,
                                  const CVector3f& up) {
  CVector3f vLook = lookPos - pos;
  const float mag = vLook.Magnitude();
  if (mag <= FLT_EPSILON) {
    vLook = CVector3f(0.f, 1.f, 0.f);
  } else {
    const float invMag = 1.f / mag;
    vLook *= invMag;
  }

  CVector3f vUp = up - vLook * CMath::Limit(CVector3f::Dot(up, vLook), 1.f);
  float vMag = vUp.Magnitude();
  if (vMag <= FLT_EPSILON) {
    vUp = CVector3f(0.f, 0.f, 1.f) - vLook * vLook.GetZ();
    vMag = vUp.Magnitude();
    if (vMag <= FLT_EPSILON) {
      vUp = CVector3f(0.f, 1.f, 0.f) - vLook * vLook.GetY();
      vMag = vUp.Magnitude();
    }
  }

  vUp *= (1.f / vMag);

  const CVector3f vRight = CVector3f::Cross(vLook, vUp);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return CTransform4f(
      vRight.GetX(), vLook.GetX(), vUp.GetX(), pos.GetX(),
      vRight.GetY(), vLook.GetY(), vUp.GetY(), pos.GetY(),
      vRight.GetZ(), vLook.GetZ(), vUp.GetZ(), pos.GetZ());
#else
  return CTransform4f(vRight[kDX], vLook.GetX(), vUp.GetX(), pos.GetX(), vRight[kDY], vLook.GetY(),
                      vUp.GetY(), pos.GetY(), vRight[kDZ], vLook.GetZ(), vUp.GetZ(), pos.GetZ());
#endif

}

CTransform4f CTransform4f::MakeRotationsBasedOnY(const CUnitVector3f& yRot) {
  uint i;
  if (yRot.GetY() < yRot.GetX()) {
    if (yRot.GetZ() < yRot.GetY()) {
      i = 2;
    } else {
      i = 1;
    }
  } else if (yRot.GetZ() < yRot.GetX()) {
    i = 2;
  } else {
    i = 1;
  }

  CVector3f v(0.f, 0.f, 0.f);
  v[i] = 1.f;

  CUnitVector3f xRot(CVector3f::Cross(yRot, v));
  return CTransform4f::FromColumns(xRot, yRot, CVector3f::Cross(xRot, yRot), CVector3f::Zero());
#if !RSTL_PRIME1
}

CTransform4f CTransform4f::Shear(float xy, float xz, float yx, float yz, float zx, float zy) {
  return CTransform4f(1.f, xy, xz, 0.f, //
                      yx, 1.f, yz, 0.f, //
                      zx, zy, 1.f, 0.f  //
  );
#endif

}

CTransform4f CTransform4f::RotateX(const CRelAngle& x) {
  const float sinAngle = sin(x.AsRadians());
  const float cosAngle = cos(x.AsRadians());
  return CTransform4f(1.f, 0.f, 0.f, 0.f,            //
                      0.f, cosAngle, -sinAngle, 0.f, //
                      0.f, sinAngle, cosAngle, 0.f   //
  );
}

CTransform4f CTransform4f::RotateY(const CRelAngle& y) {
  const float sinAngle = sin(y.AsRadians());
  const float cosAngle = cos(y.AsRadians());
  return CTransform4f(cosAngle, 0.f, sinAngle, 0.f, //
                      0.f, 1.f, 0.f, 0.f,           //
                      -sinAngle, 0.f, cosAngle, 0.f //
  );
}

CTransform4f CTransform4f::RotateZ(const CRelAngle& z) {
  const float sinAngle = sin(z.AsRadians());
  const float cosAngle = cos(z.AsRadians());
  return CTransform4f(cosAngle, -sinAngle, 0.f, 0.f, //
                      sinAngle, cosAngle, 0.f, 0.f,  //
                      0.f, 0.f, 1.f, 0.f             //
  );
}

void CTransform4f::RotateLocalX(const CRelAngle& x) {
  const float sinAngle = sin(x.AsRadians());
  const float cosAngle = cos(x.AsRadians());
  // clang-format off
  float _m01 = Get01(); _m01 *= sinAngle;
  float _m11 = Get11(); _m11 *= sinAngle;
  float _m21 = Get21(); _m21 *= sinAngle;
  float _m02 = Get02(); _m02 *= sinAngle;
  float _m12 = Get12(); _m12 *= sinAngle;
  float _m22 = Get22(); _m22 *= sinAngle;
  // clang-format on
  m01 *= cosAngle;
  m11 *= cosAngle;
  m21 *= cosAngle;
  m02 *= cosAngle;
  m12 *= cosAngle;
  m22 *= cosAngle;
  m01 += _m02;
  m11 += _m12;
  m21 += _m22;
  m02 -= _m01;
  m12 -= _m11;
  m22 -= _m21;
}

void CTransform4f::RotateLocalY(const CRelAngle& y) {
  const float sinAngle = sin(y.AsRadians());
  const float cosAngle = cos(y.AsRadians());
  // clang-format off
  float _m00 = Get00(); _m00 *= sinAngle;
  float _m10 = Get10(); _m10 *= sinAngle;
  float _m20 = Get20(); _m20 *= sinAngle;
  float _m02 = Get02(); _m02 *= sinAngle;
  float _m12 = Get12(); _m12 *= sinAngle;
  float _m22 = Get22(); _m22 *= sinAngle;
  // clang-format on

  m00 *= cosAngle;
  m10 *= cosAngle;
  m20 *= cosAngle;
  m02 *= cosAngle;
  m12 *= cosAngle;
  m22 *= cosAngle;
  m00 -= _m02;
  m10 -= _m12;
  m20 -= _m22;
  m02 += _m00;
  m12 += _m10;
  m22 += _m20;
}

void CTransform4f::RotateLocalZ(const CRelAngle& z) {
  const float sinAngle = sin(z.AsRadians());
  const float cosAngle = cos(z.AsRadians());
  // clang-format off
  float _m00 = Get00(); _m00 *= sinAngle;
  float _m10 = Get10(); _m10 *= sinAngle;
  float _m20 = Get20(); _m20 *= sinAngle;
  float _m01 = Get01(); _m01 *= sinAngle;
  float _m11 = Get11(); _m11 *= sinAngle;
  float _m21 = Get21(); _m21 *= sinAngle;
  // clang-format off

  m00 *= cosAngle;
  m10 *= cosAngle;
  m20 *= cosAngle;
  m01 *= cosAngle;
  m11 *= cosAngle;
  m21 *= cosAngle;
  m00 += _m01;
  m10 += _m11;
  m20 += _m21;
  m01 -= _m00;
  m11 -= _m10;
  m21 -= _m20;
}

void CTransform4f::Orthonormalize() {
  const CVector3f xNorm = GetRight().AsNormalized();
  const CVector3f& yCol = GetForward();
  CVector3f zCross = CVector3f::Cross(xNorm, yCol);
  const CVector3f zNorm = zCross.AsNormalized();
  CVector3f yNew = CVector3f::Cross(zNorm, xNorm);
  m00 = xNorm.GetX();
  m10 = xNorm.GetY();
  m20 = xNorm.GetZ();
  m01 = yNew.GetX();
  m11 = yNew.GetY();
  m21 = yNew.GetZ();
  m02 = zNorm.GetX();
  m12 = zNorm.GetY();
  m22 = zNorm.GetZ();
}

CTransform4f::CTransform4f(CInputStream& in)
: m00(in.ReadFloat())
, m01(in.ReadFloat())
, m02(in.ReadFloat())
, m03(in.ReadFloat())
, m10(in.ReadFloat())
, m11(in.ReadFloat())
, m12(in.ReadFloat())
, m13(in.ReadFloat())
, m20(in.ReadFloat())
, m21(in.ReadFloat())
, m22(in.ReadFloat())
, m23(in.ReadFloat()) {}

CTransform4f::CTransform4f(const CMatrix3f& basis, const CVector3f& position)
: m00(basis.Get00())
, m01(basis.Get01())
, m02(basis.Get02())
, m03(position.GetX())
, m10(basis.Get10())
, m11(basis.Get11())
, m12(basis.Get12())
, m13(position.GetY())
, m20(basis.Get20())
, m21(basis.Get21())
, m22(basis.Get22())
, m23(position.GetZ()) {}

CTransform4f CTransform4f::Scale(float scale) {

  return CTransform4f(scale, 0.f, 0.f, 0.f, //
                      0.f, scale, 0.f, 0.f, //
                      0.f, 0.f, scale, 0.f  //
  );
}

CTransform4f CTransform4f::Scale(float x, float y, float z) {
  return CTransform4f(x, 0.f, 0.f, 0.f, //
                      0.f, y, 0.f, 0.f, //
                      0.f, 0.f, z, 0.f  //
  );
}

CTransform4f CTransform4f::Scale(const CVector3f& scale) {
  return CTransform4f(scale.GetX(), 0.f, 0.f, 0.f, //
                      0.f, scale.GetY(), 0.f, 0.f, //
                      0.f, 0.f, scale.GetZ(), 0.f  //
  );
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CTransform4f CTransform4f::Translate(float x, float y, float z) {
#else
CTransform4f CTransform4f::Translate(const float x, const float y, const float z) {
#endif

  return CTransform4f(1.f, 0.f, 0.f, x, //
                      0.f, 1.f, 0.f, y, //
                      0.f, 0.f, 1.f, z  //
  );
}

CTransform4f CTransform4f::Translate(const CVector3f& pos) {
  return CTransform4f(1.f, 0.f, 0.f, pos.GetX(), //
                      0.f, 1.f, 0.f, pos.GetY(), //
                      0.f, 0.f, 1.f, pos.GetZ()  //
  );
}

CMatrix3f CTransform4f::BuildMatrix3f() const {
  return CMatrix3f(m00, m01, m02, m10, m11, m12, m20, m21, m22);
}

CTransform4f CTransform4f::MultiplyIgnoreTranslation(const CTransform4f& other) const {
  const float b00 = other.m00, b01 = other.m01, b02 = other.m02;
  const float b10 = other.m10, b11 = other.m11, b12 = other.m12;
  const float b20 = other.m20, b21 = other.m21, b22 = other.m22;
  return CTransform4f(m00 * b00 + m01 * b10 + m02 * b20,
                      m00 * b01 + m01 * b11 + m02 * b21,
                      m00 * b02 + m01 * b12 + m02 * b22,
                      other.m03 + m03,
                      m10 * b00 + m11 * b10 + m12 * b20,
                      m10 * b01 + m11 * b11 + m12 * b21,
                      m10 * b02 + m11 * b12 + m12 * b22,
                      other.m13 + m13,
                      m20 * b00 + m21 * b10 + m22 * b20,
                      m20 * b01 + m21 * b11 + m22 * b21,
                      m20 * b02 + m21 * b12 + m22 * b22,
                      other.m23 + m23);
}

void CTransform4f::ScaleBy(float scale) {
  m00 *= scale;
  m01 *= scale;
  m02 *= scale;
  m10 *= scale;
  m11 *= scale;
  m12 *= scale;
  m20 *= scale;
  m21 *= scale;
  m22 *= scale;
}

CTransform4f CTransform4f::FromColumns(const CVector3f& m0, const CVector3f& m1,
                                      const CVector3f& m2, const CVector3f& pos) {
  return CTransform4f(m0.GetX(), m1.GetX(), m2.GetX(), pos.GetX(),
                      m0.GetY(), m1.GetY(), m2.GetY(), pos.GetY(),
                      m0.GetZ(), m1.GetZ(), m2.GetZ(), pos.GetZ());
}

CTransform4f CTransform4f::FromRows(const CVector3f& m0, const CVector3f& m1,
                                       const CVector3f& m2, const CVector3f& pos) {
  return CTransform4f(m0.GetX(), m0.GetY(), m0.GetZ(), pos.GetX(), //
                      m1.GetX(), m1.GetY(), m1.GetZ(), pos.GetY(), //
                      m2.GetX(), m2.GetY(), m2.GetZ(), pos.GetZ()  //
  );
}

#if RSTL_PRIME1
bool operator==(const CTransform4f& lhs, const CTransform4f& rhs) {
  return lhs.Get00() == rhs.Get00() && lhs.Get01() == rhs.Get01() && lhs.Get02() == rhs.Get02() &&
         lhs.Get03() == rhs.Get03() && // x
         lhs.Get10() == rhs.Get10() && lhs.Get11() == rhs.Get11() && lhs.Get12() == rhs.Get12() &&
         lhs.Get13() == rhs.Get13() && // y
         lhs.Get20() == rhs.Get20() && lhs.Get21() == rhs.Get21() && lhs.Get22() == rhs.Get22() &&
         lhs.Get23() == rhs.Get23(); // z
}
#endif

CTransform4f CTransform4f::GetQuickInverse() const{
  const float _m03 = -(Get20() * Get23() - (Get00() * -Get03() - Get10() * Get13()));
  const float _m13 = -(Get21() * Get23() - (Get01() * -Get03() - Get11() * Get13()));
  const float _m23 = -(Get22() * Get23() - (Get02() * -Get03() - Get12() * Get13()));

  return CTransform4f(Get00(), Get10(), Get20(), _m03, Get01(), Get11(), Get21(), _m13, Get02(), Get12(), Get22(), _m23);
}

CTransform4f CTransform4f::GetRotation() const{
  return CTransform4f(m00, m01, m02, 0.f, //
    m10, m11, m12, 0.f,//
    m20, m21, m22, 0.f);
#if RSTL_PRIME1
}

void CTransform4f::SetRotation(const CMatrix3f& rotation) {
  m00 = rotation.Get00();
  m01 = rotation.Get01();
  m02 = rotation.Get02();
  m10 = rotation.Get10();
  m11 = rotation.Get11();
  m12 = rotation.Get12();
  m20 = rotation.Get20();
  m21 = rotation.Get21();
  m22 = rotation.Get22();
#endif

}

void CTransform4f::SetRotation(const CTransform4f& rotation){
  m00 = rotation.Get00();
  m01 = rotation.Get01();
  m02 = rotation.Get02();
  m10 = rotation.Get10();
  m11 = rotation.Get11();
  m12 = rotation.Get12();
  m20 = rotation.Get20();
  m21 = rotation.Get21();
  m22 = rotation.Get22();
}

#ifdef __MWERKS__
CTransform4f::CTransform4f(register const CTransform4f& other){
  // Hack to get around compiler error.
  register CTransform4f* thiz = this;
  __asm__ {
    lfd f0, CTransform4f.m00(other);
    lfd f1, CTransform4f.m02(other);
    lfd f2, CTransform4f.m10(other);
    stfd f0, CTransform4f.m00(thiz);
    stfd f1, CTransform4f.m02(thiz);
    stfd f2, CTransform4f.m10(thiz);
    lfd f0, CTransform4f.m12(other);
    lfd f1, CTransform4f.m20(other);
    lfd f2, CTransform4f.m22(other);
    stfd f0, CTransform4f.m12(thiz);
    stfd f1, CTransform4f.m20(thiz);
    stfd f2, CTransform4f.m22(thiz);
  }
}
#else
CTransform4f::CTransform4f(const CTransform4f& other) {
  m00 = other.m00;
  m01 = other.m01;
  m02 = other.m02;
  m03 = other.m03;
  m10 = other.m10;
  m11 = other.m11;
  m12 = other.m12;
  m13 = other.m13;
  m20 = other.m20;
  m21 = other.m21;
  m22 = other.m22;
  m23 = other.m23;
}
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#ifdef __MWERKS__
CTransform4f& CTransform4f::operator=(register const CTransform4f& other) {
  register CTransform4f& thiz = *this;
  __asm__ volatile {
    lfd f0, CTransform4f.m00(other);
    lfd f1, CTransform4f.m02(other);
    lfd f2, CTransform4f.m10(other);
    stfd f0, CTransform4f.m00(thiz);
    stfd f1, CTransform4f.m02(thiz);
    stfd f2, CTransform4f.m10(thiz);
    lfd f0, CTransform4f.m12(other);
    lfd f1, CTransform4f.m20(other);
    lfd f2, CTransform4f.m22(other);
    stfd f0, CTransform4f.m12(thiz);
    stfd f1, CTransform4f.m20(thiz);
    stfd f2, CTransform4f.m22(thiz);
  }

  return *this;
}
#else
CTransform4f& CTransform4f::operator=(const CTransform4f& other) {
  m00 = other.m00;
  m01 = other.m01;
  m02 = other.m02;
  m03 = other.m03;
  m10 = other.m10;
  m11 = other.m11;
  m12 = other.m12;
  m13 = other.m13;
  m20 = other.m20;
  m21 = other.m21;
  m22 = other.m22;
  m23 = other.m23;
  return *this;
}
#endif
#else
#ifdef __MWERKS__
CTransform4f& CTransform4f::operator=(register const CTransform4f& other) {
  register CTransform4f& thiz = *this;
  __asm__ {
    lfd f0, CTransform4f.m00(other);
    lfd f1, CTransform4f.m02(other);
    lfd f2, CTransform4f.m10(other);
    stfd f0, CTransform4f.m00(thiz);
    stfd f1, CTransform4f.m02(thiz);
    stfd f2, CTransform4f.m10(thiz);
    lfd f0, CTransform4f.m12(other);
    lfd f1, CTransform4f.m20(other);
    lfd f2, CTransform4f.m22(other);
    stfd f0, CTransform4f.m12(thiz);
    stfd f1, CTransform4f.m20(thiz);
    stfd f2, CTransform4f.m22(thiz);
  }

  return *this;
}
#else
CTransform4f& CTransform4f::operator=(const CTransform4f& other) {
  m00 = other.m00;
  m01 = other.m01;
  m02 = other.m02;
  m03 = other.m03;
  m10 = other.m10;
  m11 = other.m11;
  m12 = other.m12;
  m13 = other.m13;
  m20 = other.m20;
  m21 = other.m21;
  m22 = other.m22;
  m23 = other.m23;
  return *this;
}
#endif
#endif

#ifdef __MWERKS__
CVector3f CTransform4f::operator*(register const CVector3f& vec) const {
  // Assume RVO for return value
  register const CVector3f* ret;
  // Hack to get around compiler error.
  register const CTransform4f* thiz = this;

  __asm__ {

    psq_l  f2, CVector3f.mX(vec), 0, 0;
    psq_l  f3, CVector3f.mZ(vec), 1, 0;

    psq_l  f4, CTransform4f.m00(thiz), 0, 0;
    psq_l  f5, CTransform4f.m02(thiz), 0, 0;

    ps_mul f0, f4, f2;

    psq_l  f4, CTransform4f.m10(thiz), 0, 0;

    ps_madd f0, f5, f3, f0;
    ps_mul f1, f4, f2;

    psq_l f5, CTransform4f.m12(thiz), 0, 0;
    psq_l f4, CTransform4f.m20(thiz), 0, 0;

    ps_mul f2, f4, f2;

    ps_madd f1, f5, f3, f1;

    psq_l f5, CTransform4f.m22(thiz), 0, 0;

    ps_madd f2, f5, f3, f2;

    lfs f6, 1.f;
    ps_madds1 f0, f6, f0, f0;
    ps_madds1 f1, f6, f1, f1;
    ps_madds1 f2, f6, f2, f2;

    stfs f0, CVector3f.mX(ret);
    stfs f1, CVector3f.mY(ret);
    stfs f2, CVector3f.mZ(ret);
  }
  return *ret;
}
#else
CVector3f CTransform4f::operator*(const CVector3f& vec) const {
  return CVector3f(m00 * vec.GetX() + m01 * vec.GetY() + m02 * vec.GetZ() + m03,
                   m10 * vec.GetX() + m11 * vec.GetY() + m12 * vec.GetZ() + m13,
                   m20 * vec.GetX() + m21 * vec.GetY() + m22 * vec.GetZ() + m23);
}
#endif

#ifdef __MWERKS__
CVector3f CTransform4f::Rotate(register const CVector3f& vec) const{
  // Assume RVO for return value
  register const CVector3f* ret;
  // Hack to get around compiler error.
  register const CTransform4f* thiz = this;

  __asm__ {
    lfs f7, 0.f;
    lfs f6, 1.f;

    psq_l f5, CTransform4f.m02(thiz), 1, 0;
    psq_l f2, CVector3f.mX(vec), 0, 0;
    psq_l f4, CTransform4f.m00(thiz), 0, 0;

    ps_merge00 f5, f5, f7;

    psq_l f3, CVector3f.mZ(vec), 1, 0;

    ps_mul f0, f4, f2;

    psq_l f4, CTransform4f.m10(thiz), 0, 0;

    ps_mul f1, f4, f2;

    psq_l f4, CTransform4f.m20(thiz), 0, 0;

    ps_madd f0, f5, f3, f0;

    psq_l f5, CTransform4f.m12(thiz), 1, 0;

    ps_mul f2, f4, f2;

    ps_merge00 f5, f5, f7;

    ps_madds1 f0, f6, f0, f0;
    ps_madd f1, f5, f3, f1;

    psq_l f5, CTransform4f.m22(thiz), 1, 0;

    ps_merge00 f5, f5, f7;

    ps_madds1 f1, f6, f1, f1;
    ps_madd f2, f5, f3, f2;
    ps_madds1 f2, f6, f2, f2;

    stfs f0, CVector3f.mX(ret);
    stfs f1, CVector3f.mY(ret);
    stfs f2, CVector3f.mZ(ret);
  }
  return *ret;
}
#else
CVector3f CTransform4f::Rotate(const CVector3f& vec) const {
  return CVector3f(m00 * vec.GetX() + m01 * vec.GetY() + m02 * vec.GetZ(),
                   m10 * vec.GetX() + m11 * vec.GetY() + m12 * vec.GetZ(),
                   m20 * vec.GetX() + m21 * vec.GetY() + m22 * vec.GetZ());
}
#endif

#ifdef __MWERKS__
CVector3f CTransform4f::TransposeRotate(register const CVector3f& in) const{
  // Assume RVO for return value
  register const CVector3f* ret;
  // Hack to get around compiler error.
  register const CTransform4f* thiz = this;

  __asm__ volatile {
    lfs f0, CVector3f.mX(in);
    lfs f1, CVector3f.mY(in);

    ps_merge00 f3, f0, f0;

    psq_l f4, CTransform4f.m00(thiz), 0, 0;

    lfs f2, CVector3f.mZ(in);

    ps_merge00 f1, f1, f1;

    lfs f0, CTransform4f.m12(thiz);

    ps_mul f5, f4, f3;

    psq_l f6, CTransform4f.m10(thiz), 0, 0;

    ps_merge00 f4, f2, f2;

    psq_l f7, CTransform4f.m20(thiz), 0, 0;

    fmuls f0, f0, f1;

    ps_madd f5, f6, f1, f5;

    lfs f1, CTransform4f.m02(thiz);
    lfs f2, CTransform4f.m22(thiz);

    ps_madd f5, f7, f4, f5;
    fmadds f0, f1, f3, f0;

    ps_merge11 f1, f5, f5;

    stfs f5, CVector3f.mX(ret);
    fmadds f0, f2, f4, f0;
    stfs f1, CVector3f.mY(ret);
    stfs f0, CVector3f.mZ(ret);
  }
  return *ret;
}
#else
CVector3f CTransform4f::TransposeRotate(const CVector3f& in) const {
  return CVector3f(m00 * in.GetX() + m10 * in.GetY() + m20 * in.GetZ(),
                   m01 * in.GetX() + m11 * in.GetY() + m21 * in.GetZ(),
                   m02 * in.GetX() + m12 * in.GetY() + m22 * in.GetZ());
}
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#ifdef __MWERKS__
CTransform4f CTransform4f::operator*(register const CTransform4f& xf) const {
  register CTransform4f* ret;
  // Assume RVO for return value
    // Hack to get around compiler error.
  register const CTransform4f* thiz = this;
  __asm__ volatile {
    psq_l f0, CTransform4f.m00(thiz), 0, 0;
    psq_l f7, CTransform4f.m02(xf), 0, 0;

    ps_merge11 f1, f0, f0;

    psq_l f4, CTransform4f.m00(xf), 0, 0;

    ps_merge00 f0, f0, f0;

    lfs f3, 0.f;

    psq_l f2, CTransform4f.m02(thiz), 0, 0;
    psq_l f8, CTransform4f.m12(xf), 0, 0;

    ps_mul f10, f0, f7;

    psq_l f5, CTransform4f.m10(xf), 0, 0;

    ps_mul f0, f0, f4;

    psq_l f9, CTransform4f.m22(xf), 0, 0;

    ps_merge01 f3, f3, f2;

    psq_l f6, CTransform4f.m20(xf), 0, 0;

    ps_merge00 f2, f2, f2;

    ps_madd f10, f1, f8, f10;
    ps_madd f0, f1, f5, f0;
    ps_madd f10, f2, f9, f10;
    ps_madd f0, f2, f6, f0;
    ps_add f10, f3, f10;

    psq_st f0, CTransform4f.m00(ret), 0, 0;
    psq_st f10, CTransform4f.m02(ret), 0, 0;

    psq_l f0, CTransform4f.m10(thiz), 0, 0;
    psq_l f2, CTransform4f.m12(thiz), 0, 0;

    ps_merge11 f1, f0, f0;
    ps_merge00 f0, f0, f0;
    ps_merge01 f3, f3, f2;
    ps_merge00 f2, f2, f2;
    ps_mul f10, f0, f7;
    ps_mul f0, f0, f4;
    ps_madd f10, f1, f8, f10;
    ps_madd f0, f1, f5, f0;
    ps_madd f10, f2, f9, f10;
    ps_madd f0, f2, f6, f0;
    ps_add f10, f3, f10;

    psq_st f0, CTransform4f.m10(ret), 0, 0;
    psq_st f10, CTransform4f.m12(ret), 0, 0;

    psq_l f0, CTransform4f.m20(thiz), 0, 0;
    psq_l f2, CTransform4f.m22(thiz), 0, 0;

    ps_merge11 f1, f0, f0;
    ps_merge00 f0, f0, f0;
    ps_merge01 f3, f3, f2;
    ps_merge00 f2, f2, f2;
    ps_mul f10, f0, f7;
    ps_mul f0, f0, f4;
    ps_madd f10, f1, f8, f10;
    ps_madd f0, f1, f5, f0;
    ps_madd f10, f2, f9, f10;
    ps_madd f0, f2, f6, f0;
    ps_add f10, f3, f10;

    psq_st f0, CTransform4f.m20(ret), 0, 0;
    psq_st f10, CTransform4f.m22(ret), 0, 0;
    // implicit return via RVO
  }
}
#else
CTransform4f CTransform4f::operator*(const CTransform4f& xf) const {
  return CTransform4f(m00 * xf.m00 + m01 * xf.m10 + m02 * xf.m20,
                      m00 * xf.m01 + m01 * xf.m11 + m02 * xf.m21,
                      m00 * xf.m02 + m01 * xf.m12 + m02 * xf.m22,
                      m00 * xf.m03 + m01 * xf.m13 + m02 * xf.m23 + m03,
                      m10 * xf.m00 + m11 * xf.m10 + m12 * xf.m20,
                      m10 * xf.m01 + m11 * xf.m11 + m12 * xf.m21,
                      m10 * xf.m02 + m11 * xf.m12 + m12 * xf.m22,
                      m10 * xf.m03 + m11 * xf.m13 + m12 * xf.m23 + m13,
                      m20 * xf.m00 + m21 * xf.m10 + m22 * xf.m20,
                      m20 * xf.m01 + m21 * xf.m11 + m22 * xf.m21,
                      m20 * xf.m02 + m21 * xf.m12 + m22 * xf.m22,
                      m20 * xf.m03 + m21 * xf.m13 + m22 * xf.m23 + m23);
}
#endif
#else
#ifdef __MWERKS__
CTransform4f CTransform4f::operator*(register const CTransform4f& xf) const {
  register CTransform4f* ret;
  // Assume RVO for return value
  // Hack to get around compiler error.
  register const CTransform4f* thiz = this;
  __asm__ volatile {
    psq_l f0, CTransform4f.m00(thiz), 0, 0;
    psq_l f7, CTransform4f.m02(xf), 0, 0;
    ps_merge11 f1, f0, f0;
    psq_l f4, CTransform4f.m00(xf), 0, 0;
    ps_merge00 f0, f0, f0;
    lfs f3, 0.f;
    psq_l f2, CTransform4f.m02(thiz), 0, 0;
    psq_l f8, CTransform4f.m12(xf), 0, 0;
    ps_mul f11, f0, f7;
    psq_l f5, CTransform4f.m10(xf), 0, 0;
    ps_mul f10, f0, f4;
    psq_l f9, CTransform4f.m22(xf), 0, 0;
    ps_merge01 f3, f3, f2;
    psq_l f6, CTransform4f.m20(xf), 0, 0;
    ps_merge00 f2, f2, f2;
    psq_l f0, CTransform4f.m10(thiz), 0, 0;
    ps_madd f11, f1, f8, f11;
    ps_madd f10, f1, f5, f10;
    ps_merge11 f1, f0, f0;
    ps_madd f11, f2, f9, f11;
    ps_madd f10, f2, f6, f10;
    psq_l f2, CTransform4f.m12(thiz), 0, 0;
    ps_merge00 f0, f0, f0;
    ps_add f11, f3, f11;
    psq_st f10, CTransform4f.m00(ret), 0, 0;
    ps_merge01 f3, f3, f2;
    ps_mul f10, f0, f4;
    psq_st f11, CTransform4f.m02(ret), 0, 0;
    ps_mul f11, f0, f7;
    ps_merge00 f2, f2, f2;
    psq_l f0, CTransform4f.m20(thiz), 0, 0;
    ps_madd f10, f1, f5, f10;
    ps_madd f11, f1, f8, f11;
    ps_merge11 f1, f0, f0;
    ps_madd f10, f2, f6, f10;
    ps_madd f11, f2, f9, f11;
    psq_l f2, CTransform4f.m22(thiz), 0, 0;
    ps_merge00 f0, f0, f0;
    psq_st f10, CTransform4f.m10(ret), 0, 0;
    ps_add f11, f3, f11;
    ps_mul f10, f0, f4;
    ps_merge01 f3, f3, f2;
    psq_st f11, CTransform4f.m12(ret), 0, 0;
    ps_mul f11, f0, f7;
    ps_merge00 f2, f2, f2;
    ps_madd f10, f1, f5, f10;
    ps_madd f11, f1, f8, f11;
    ps_madd f10, f2, f6, f10;
    ps_madd f11, f2, f9, f11;
    psq_st f10, CTransform4f.m20(ret), 0, 0;
    ps_add f11, f3, f11;
    psq_st f11, CTransform4f.m22(ret), 0, 0;
    // implicit return via RVO
  }
}
#else
CTransform4f CTransform4f::operator*(const CTransform4f& xf) const {
  return CTransform4f(m00 * xf.m00 + m01 * xf.m10 + m02 * xf.m20,
                      m00 * xf.m01 + m01 * xf.m11 + m02 * xf.m21,
                      m00 * xf.m02 + m01 * xf.m12 + m02 * xf.m22,
                      m00 * xf.m03 + m01 * xf.m13 + m02 * xf.m23 + m03,
                      m10 * xf.m00 + m11 * xf.m10 + m12 * xf.m20,
                      m10 * xf.m01 + m11 * xf.m11 + m12 * xf.m21,
                      m10 * xf.m02 + m11 * xf.m12 + m12 * xf.m22,
                      m10 * xf.m03 + m11 * xf.m13 + m12 * xf.m23 + m13,
                      m20 * xf.m00 + m21 * xf.m10 + m22 * xf.m20,
                      m20 * xf.m01 + m21 * xf.m11 + m22 * xf.m21,
                      m20 * xf.m02 + m21 * xf.m12 + m22 * xf.m22,
                      m20 * xf.m03 + m21 * xf.m13 + m22 * xf.m23 + m23);
}
#endif
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#ifdef __MWERKS__
asm CTransform4f CTransform4f::GetInverse() const {
  // r3 is the return storage; r4 is this.
  nofralloc
  stwu r1, -0x30(r1);
  stfd f31, 0x20(r1);
  psq_st f31, 0x28(r1), 0, 0;
  stfd f30, 0x10(r1);
  psq_st f30, 0x18(r1), 0, 0;

  lfs f6, CTransform4f.m10(r4);
  lfs f8, CTransform4f.m22(r4);
  lfs f2, CTransform4f.m03(r4);
  lfs f7, CTransform4f.m12(r4);
  fmuls f0, f6, f8;
  lfs f4, CTransform4f.m20(r4);
  fmuls f1, f2, f8;
  lfs f10, CTransform4f.m21(r4);
  fmuls f2, f2, f7;
  lfs f9, CTransform4f.m11(r4);
  fmuls f3, f7, f10;
  lfs f31, CTransform4f.m01(r4);
  fmsubs f0, f7, f4, f0;
  lfs f11, CTransform4f.m23(r4);
  fmuls f5, f9, f4;
  lfs f4, CTransform4f.m00(r4);
  fmsubs f3, f9, f8, f3;
  lfs f12, CTransform4f.m02(r4);
  fmuls f0, f31, f0;
  lfs f13, CTransform4f.m13(r4);
  fmsubs f5, f6, f10, f5;
  lfs f6, 1.f;
  fmadds f0, f4, f3, f0;
  fmuls f3, f12, f11;
  fmuls f30, f12, f13;
  fmadds f0, f12, f5, f0;
  fmuls f7, f7, f11;
  fmuls f8, f13, f8;
  fdivs f0, f6, f0;
  fneg f5, f7;
  fsubs f4, f3, f1;
  fneg f6, f30;
  fneg f3, f3;
  fadds f5, f5, f8;
  fmuls f4, f9, f4;
  fadds f6, f6, f2;
  fadds f1, f3, f1;
  fmadds f4, f31, f5, f4;
  fsubs f3, f7, f8;
  fsubs f5, f30, f2;
  fmadds f2, f10, f6, f4;
  fmuls f2, f0, f2;
  stfs f2, CTransform4f.m03(r3);

  lfs f2, CTransform4f.m10(r4);
  lfs f4, CTransform4f.m00(r4);
  fmuls f1, f2, f1;
  lfs f2, CTransform4f.m20(r4);
  fmadds f1, f4, f3, f1;
  fmadds f1, f2, f5, f1;
  fmuls f1, f0, f1;
  stfs f1, CTransform4f.m13(r3);

  lfs f1, CTransform4f.m11(r4);
  lfs f3, CTransform4f.m03(r4);
  lfs f2, CTransform4f.m21(r4);
  fneg f4, f1;
  lfs f10, CTransform4f.m13(r4);
  fmuls f6, f3, f1;
  lfs f7, CTransform4f.m01(r4);
  fmuls f1, f3, f2;
  lfs f9, CTransform4f.m23(r4);
  fmuls f3, f10, f2;
  lfs f2, CTransform4f.m10(r4);
  fmsubs f1, f7, f9, f1;
  lfs f5, CTransform4f.m00(r4);
  fneg f7, f7;
  lfs f8, CTransform4f.m20(r4);
  fmadds f3, f4, f9, f3;
  fmuls f1, f2, f1;
  fmadds f2, f7, f10, f6;
  fmadds f1, f5, f3, f1;
  fmadds f1, f8, f2, f1;
  fmuls f1, f0, f1;
  stfs f1, CTransform4f.m23(r3);

  lfs f2, CTransform4f.m12(r4);
  lfs f1, CTransform4f.m21(r4);
  lfs f3, CTransform4f.m11(r4);
  fmuls f1, f2, f1;
  lfs f2, CTransform4f.m22(r4);
  fmsubs f1, f3, f2, f1;
  fmuls f1, f0, f1;
  stfs f1, CTransform4f.m00(r3);

  lfs f2, CTransform4f.m01(r4);
  lfs f1, CTransform4f.m22(r4);
  lfs f3, CTransform4f.m02(r4);
  fmuls f1, f2, f1;
  lfs f2, CTransform4f.m21(r4);
  fmsubs f1, f3, f2, f1;
  fmuls f1, f0, f1;
  stfs f1, CTransform4f.m01(r3);

  lfs f2, CTransform4f.m02(r4);
  lfs f1, CTransform4f.m11(r4);
  lfs f3, CTransform4f.m01(r4);
  fmuls f1, f2, f1;
  lfs f2, CTransform4f.m12(r4);
  fmsubs f1, f3, f2, f1;
  fmuls f1, f0, f1;
  stfs f1, CTransform4f.m02(r3);

  lfs f2, CTransform4f.m10(r4);
  lfs f1, CTransform4f.m22(r4);
  lfs f3, CTransform4f.m12(r4);
  fmuls f1, f2, f1;
  lfs f2, CTransform4f.m20(r4);
  fmsubs f1, f3, f2, f1;
  fmuls f1, f0, f1;
  stfs f1, CTransform4f.m10(r3);

  lfs f2, CTransform4f.m02(r4);
  lfs f1, CTransform4f.m20(r4);
  lfs f3, CTransform4f.m00(r4);
  fmuls f1, f2, f1;
  lfs f2, CTransform4f.m22(r4);
  fmsubs f1, f3, f2, f1;
  fmuls f1, f0, f1;
  stfs f1, CTransform4f.m11(r3);

  lfs f2, CTransform4f.m00(r4);
  lfs f1, CTransform4f.m12(r4);
  lfs f3, CTransform4f.m02(r4);
  fmuls f1, f2, f1;
  lfs f2, CTransform4f.m10(r4);
  fmsubs f1, f3, f2, f1;
  fmuls f1, f0, f1;
  stfs f1, CTransform4f.m12(r3);

  lfs f2, CTransform4f.m11(r4);
  lfs f1, CTransform4f.m20(r4);
  lfs f3, CTransform4f.m10(r4);
  fmuls f1, f2, f1;
  lfs f2, CTransform4f.m21(r4);
  fmsubs f1, f3, f2, f1;
  fmuls f1, f0, f1;
  stfs f1, CTransform4f.m20(r3);

  lfs f2, CTransform4f.m00(r4);
  lfs f1, CTransform4f.m21(r4);
  lfs f3, CTransform4f.m01(r4);
  fmuls f1, f2, f1;
  lfs f2, CTransform4f.m20(r4);
  fmsubs f1, f3, f2, f1;
  fmuls f1, f0, f1;
  stfs f1, CTransform4f.m21(r3);

  lfs f2, CTransform4f.m01(r4);
  lfs f1, CTransform4f.m10(r4);
  lfs f3, CTransform4f.m00(r4);
  fmuls f1, f2, f1;
  lfs f2, CTransform4f.m11(r4);
  fmsubs f1, f3, f2, f1;
  fmuls f0, f0, f1;
  stfs f0, CTransform4f.m22(r3);

  psq_l f31, 0x28(r1), 0, 0;
  lfd f31, 0x20(r1);
  psq_l f30, 0x18(r1), 0, 0;
  lfd f30, 0x10(r1);
  addi r1, r1, 0x30;
  blr;
}
#else
CTransform4f CTransform4f::GetInverse() const {
  // Shared translation products: tx12 is translation.x * A[1][2], etc.
  const float tx22 = m03 * m22;
  const float tx12 = m03 * m12;
  const float tz02 = m02 * m23;
  const float ty02 = m02 * m13;
  const float tz12 = m12 * m23;
  const float ty22 = m13 * m22;

  // The inverse of [A t; 0 1] is [adj(A) / det(A), -A^-1 t; 0 1].
  const float adj00 = m11 * m22 - m12 * m21;
  const float adj01 = m02 * m21 - m01 * m22;
  const float adj02 = m01 * m12 - m02 * m11;
  const float adj10 = m12 * m20 - m10 * m22;
  const float adj11 = m00 * m22 - m02 * m20;
  const float adj12 = m02 * m10 - m00 * m12;
  const float adj20 = m10 * m21 - m11 * m20;
  const float adj21 = m01 * m20 - m00 * m21;
  const float adj22 = m00 * m11 - m01 * m10;
  const float invDet = 1.f / (m02 * adj20 + (m00 * adj00 + m01 * adj10));

  // Cramer's rule for A * inverseTranslation = -t. Cache the shared products
  // and preserve the expansion order to retain the original float rounding.
  const float invTranslationX =
      invDet * (m21 * (-ty02 + tx12) + (m01 * (-tz12 + ty22) + m11 * (tz02 - tx22)));
  const float invTranslationY =
      invDet * (m20 * (ty02 - tx12) + (m00 * (tz12 - ty22) + m10 * (-tz02 + tx22)));
  const float invTranslationZ =
      invDet * (m20 * (-m01 * m13 + m03 * m11) +
                (m00 * (-m11 * m23 + m13 * m21) + m10 * (m01 * m23 - m03 * m21)));

  return CTransform4f(invDet * adj00, invDet * adj01, invDet * adj02, invTranslationX,
                      invDet * adj10, invDet * adj11, invDet * adj12, invTranslationY,
                      invDet * adj20, invDet * adj21, invDet * adj22, invTranslationZ);
}

#endif
#else
CTransform4f CTransform4f::GetInverse() const {
#ifdef __MWERKS__
  register CTransform4f* ret;
#endif

  const float rxz = m02;
  const float ty = m13;
  const float rxy = m01;
  const float invDet = 1.f / ((m00 * (m11 * m22 - m12 * m21) + rxy * (m12 * m20 - m10 * m22)) +
                              rxz * (m10 * m21 - m11 * m20));
  const float ryzTz = m12 * m23;
  const float rxzTz = rxz * m23;
  const float tyRzz = ty * m22;
  const float rxzTy = rxz * ty;
  const float txRyz = m03 * m12;
  const float txRzz = m03 * m22;

#ifdef __MWERKS__
  ret->m03 = invDet * (m21 * (-rxzTy + txRyz) + (rxy * (-ryzTz + tyRzz) + (rxzTz - txRzz) * m11));
  ret->m13 = invDet * (m20 * (rxzTy - txRyz) + (m00 * (ryzTz - tyRzz) + m10 * (-rxzTz + txRzz)));
  ret->m23 = invDet * (m20 * (-rxy * ty + m03 * m11) + (m00 * (-m11 * m23 + ty * m21) +
                                                       m10 * (rxy * m23 - m03 * m21)));
  ret->m00 = invDet * (m11 * m22 - m21 * m12);
  ret->m01 = invDet * (rxz * m21 - rxy * m22);
  ret->m02 = invDet * (rxy * m12 - rxz * m11);
  ret->m10 = invDet * (m12 * m20 - m10 * m22);
  ret->m11 = invDet * (m00 * m22 - rxz * m20);
  ret->m12 = invDet * (rxz * m10 - m00 * m12);
  ret->m20 = invDet * (m10 * m21 - m11 * m20);
  ret->m21 = invDet * (rxy * m20 - m00 * m21);
  ret->m22 = invDet * (m00 * m11 - rxy * m10);
#else
  return CTransform4f(
      invDet * (m11 * m22 - m21 * m12), invDet * (rxz * m21 - rxy * m22),
      invDet * (rxy * m12 - rxz * m11),
      invDet * (m21 * (-rxzTy + txRyz) + (rxy * (-ryzTz + tyRzz) + (rxzTz - txRzz) * m11)),
      invDet * (m12 * m20 - m10 * m22), invDet * (m00 * m22 - rxz * m20),
      invDet * (rxz * m10 - m00 * m12),
      invDet * (m20 * (rxzTy - txRyz) + (m00 * (ryzTz - tyRzz) + m10 * (-rxzTz + txRzz))),
      invDet * (m10 * m21 - m11 * m20), invDet * (rxy * m20 - m00 * m21),
      invDet * (m00 * m11 - rxy * m10),
      invDet * (m20 * (-rxy * ty + m03 * m11) +
                (m00 * (-m11 * m23 + ty * m21) + m10 * (rxy * m23 - m03 * m21))));
#endif
}
#endif

