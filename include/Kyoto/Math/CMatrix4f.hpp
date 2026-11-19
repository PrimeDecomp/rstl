#ifndef _CMATRIX4F
#define _CMATRIX4F
#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"


#include "rstl/types.h"

#include "Kyoto/Math/CVector3f.hpp"

class CMatrix4f {
public:
  CMatrix4f(float, float, float, float, float, float, float, float, float, float, float, float,
            float, float, float, float);
#if !RSTL_PRIME1
  CMatrix4f(const CMatrix4f& other)
  : m00(other.m00)
  , m01(other.m01)
  , m02(other.m02)
  , m03(other.m03)
  , m10(other.m10)
  , m11(other.m11)
  , m12(other.m12)
  , m13(other.m13)
  , m20(other.m20)
  , m21(other.m21)
  , m22(other.m22)
  , m23(other.m23)
  , m30(other.m30)
  , m31(other.m31)
  , m32(other.m32)
  , m33(other.m33) {}
#endif

  CVector3f operator*(const CVector3f& vec) const;

  CVector3f MultiplyOneOverW(const CVector3f& vec) const;
  float MultiplyGetW(const CVector3f& vec) const;
  float Determinant() const {
    return m00 * (m13 * (m21 * m32 - m22 * m31) - m11 * (m23 * m32 - m22 * m33) +
                  m12 * (m23 * m31 - m21 * m33)) -
           m01 * (m10 * (m22 * m33 - m23 * m32) - m12 * (m20 * m33 - m23 * m30) +
                  m13 * (m20 * m32 - m22 * m30)) +
           m02 * (m10 * (m21 * m33 - m23 * m31) - m11 * (m20 * m33 - m23 * m30) +
                  m13 * (m20 * m31 - m21 * m30)) -
           m03 * (m10 * (m21 * m32 - m22 * m31) - m11 * (m20 * m32 - m22 * m30) +
                  m12 * (m20 * m31 - m21 * m30));
  }
  
  CMatrix4f GetInverse() const {
    const float invDet = 1.f / Determinant();
    // minorRC is the determinant of this matrix without row R and column C.
    const float minor00 =
        m13 * (m21 * m32 - m22 * m31) - m11 * (m23 * m32 - m22 * m33) + m12 * (m23 * m31 - m21 * m33);
    const float minor10 =
        m01 * (m22 * m33 - m23 * m32) - m02 * (m21 * m33 - m23 * m31) + m03 * (m21 * m32 - m22 * m31);
    const float minor20 =
        m01 * (m12 * m33 - m13 * m32) - m02 * (m11 * m33 - m13 * m31) + m03 * (m11 * m32 - m12 * m31);
    const float minor30 =
        m01 * (m12 * m23 - m13 * m22) - m02 * (m11 * m23 - m13 * m21) + m03 * (m11 * m22 - m12 * m21);
    const float minor01 =
        m10 * (m22 * m33 - m23 * m32) - m12 * (m20 * m33 - m23 * m30) + m13 * (m20 * m32 - m22 * m30);
    const float minor11 =
        m00 * (m22 * m33 - m23 * m32) - m02 * (m20 * m33 - m23 * m30) + m03 * (m20 * m32 - m22 * m30);
    const float minor21 =
        m00 * (m12 * m33 - m13 * m32) - m02 * (m10 * m33 - m13 * m30) + m03 * (m10 * m32 - m12 * m30);
    const float minor31 =
        m00 * (m12 * m23 - m13 * m22) - m02 * (m10 * m23 - m13 * m20) + m03 * (m10 * m22 - m12 * m20);
    const float minor02 =
        m10 * (m21 * m33 - m23 * m31) - m11 * (m20 * m33 - m23 * m30) + m13 * (m20 * m31 - m21 * m30);
    const float minor12 =
        m00 * (m21 * m33 - m23 * m31) - m01 * (m20 * m33 - m23 * m30) + m03 * (m20 * m31 - m21 * m30);
    const float minor22 =
        m00 * (m11 * m33 - m13 * m31) - m01 * (m10 * m33 - m13 * m30) + m03 * (m10 * m31 - m11 * m30);
    const float minor32 =
        m00 * (m11 * m23 - m13 * m21) - m01 * (m10 * m23 - m13 * m20) + m03 * (m10 * m21 - m11 * m20);
    const float minor03 =
        m10 * (m21 * m32 - m22 * m31) - m11 * (m20 * m32 - m22 * m30) + m12 * (m20 * m31 - m21 * m30);
    const float minor13 =
        m00 * (m21 * m32 - m22 * m31) - m01 * (m20 * m32 - m22 * m30) + m02 * (m20 * m31 - m21 * m30);
    const float minor23 =
        m00 * (m11 * m32 - m12 * m31) - m01 * (m10 * m32 - m12 * m30) + m02 * (m10 * m31 - m11 * m30);
    const float minor33 =
        m00 * (m11 * m22 - m12 * m21) - m01 * (m10 * m22 - m12 * m20) + m02 * (m10 * m21 - m11 * m20);

    return CMatrix4f(invDet * minor00, invDet * -minor10, invDet * minor20, invDet * -minor30,
                    invDet * -minor01, invDet * minor11, invDet * -minor21, invDet * minor31,
                    invDet * minor02, invDet * -minor12, invDet * minor22, invDet * -minor32,
                    invDet * -minor03, invDet * minor13, invDet * -minor23, invDet * minor33);
  }

  static const CMatrix4f& Identity() { return sIdentity; }

private:
  static const CMatrix4f sIdentity;
  float m00;
  float m01;
  float m02;
  float m03;
  float m10;
  float m11;
  float m12;
  float m13;
  float m20;
  float m21;
  float m22;
  float m23;
  float m30;
  float m31;
  float m32;
  float m33;
};
CHECK_SIZEOF(CMatrix4f, 0x40);

#endif // _CMATRIX4F
