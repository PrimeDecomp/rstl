#ifndef _CTRANSFORM4F
#define _CTRANSFORM4F
#include "rstl/RstlVersions.h"

#include "types.h"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "Kyoto/Math/CUnitVector3f.hpp"
#else
#include "Kyoto/Math/CVector3f.hpp"
#include "rstl/construct.hpp"
#endif

class CInputStream;

typedef const float (*ConstMtxPtr)[4];

class CMatrix3f;
class CRelAngle;
class CUnitVector3f;

class CTransform4f {
public:
  CTransform4f(const float _m00, const float _m01, const float _m02, const float _m03,
               const float _m10, const float _m11, const float _m12, const float _m13,
               const float _m20, const float _m21, const float _m22, const float _m23)
  : m00(_m00)
  , m01(_m01)
  , m02(_m02)
  , m03(_m03)
  , m10(_m10)
  , m11(_m11)
  , m12(_m12)
  , m13(_m13)
  , m20(_m20)
  , m21(_m21)
  , m22(_m22)
  , m23(_m23) {}

  CTransform4f(CInputStream& in);
  CTransform4f(const CMatrix3f& rotation, const CVector3f& translation);
  CTransform4f(const CTransform4f& other);
  CTransform4f& operator=(const CTransform4f& other);

  CVector3f GetTranslation() const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_R3ME_00
    float x, y, z;
    z = m23;
    y = m13;
    x = m03;
    return CVector3f(x, y, z);
#else
    return CVector3f(m03, m13, m23);
#endif
#else
#if RSTL_VERSION >= RSTL_R32J
    float x, y, z;
    z = m23;
    y = m13;
    x = m03;
    return CVector3f(x, y, z);
#else
    return CVector3f(m03, m13, m23);
#endif
#endif

  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CVector3f GetRight() const {
#if RSTL_VERSION >= RSTL_R3IJ
    float x, y, z;
    z = m20;
    y = m10;
    x = m00;
    return CVector3f(x, y, z);
#else
    return CVector3f(m00, m10, m20);
#endif
#else
  CVector3f GetRight() const { return CVector3f(m00, m10, m20); }
  const CVector3f GetForward() const {
#if RSTL_VERSION >= RSTL_R32J
    float x, y, z;
    z = m21;
    y = m11;
    x = m01;
    return CVector3f(x, y, z);
#else
    return CVector3f(m01, m11, m21);
#endif
#endif

  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const CVector3f GetForward() const {
#if RSTL_VERSION >= RSTL_R3IJ
    float x, y, z;
    z = m21;
    y = m11;
    x = m01;
    return CVector3f(x, y, z);
#else
    return CVector3f(m01, m11, m21);
#endif
  }
  CVector3f GetUp() const {
#if RSTL_VERSION >= RSTL_R3IJ
    float x, y, z;
    z = m22;
    y = m12;
    x = m02;
    return CVector3f(x, y, z);
#else
    return CVector3f(m02, m12, m22);
#endif
  }
#else
  CVector3f GetUp() const { return CVector3f(m02, m12, m22); }
#endif

  ConstMtxPtr GetCStyleMatrix() const { return reinterpret_cast< ConstMtxPtr >(this); }

  CMatrix3f BuildMatrix3f() const;
  const float Get00() const { return m00; }
  const float Get01() const { return m01; }
  const float Get02() const { return m02; }
  const float Get03() const { return m03; }
  const float Get10() const { return m10; }
  const float Get11() const { return m11; }
  const float Get12() const { return m12; }
  const float Get13() const { return m13; }
  const float Get20() const { return m20; }
  const float Get21() const { return m21; }
  const float Get22() const { return m22; }
  const float Get23() const { return m23; }
  const CVector3f GetColumn(EDimX dim) const { return CVector3f(m00, m10, m20); }
  void SetColumn(EDimX dim, const CVector3f& col) {
    m00 = col.GetX();
    m10 = col.GetY();
    m20 = col.GetZ();
  }

  void SetColumn(EDimY dim, const CVector3f& col) {
    m01 = col.GetX();
    m11 = col.GetY();
    m21 = col.GetZ();
  }

  void SetColumn(EDimZ dim, const CVector3f& col) {
    m02 = col.GetX();
    m12 = col.GetY();
    m22 = col.GetZ();
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const CVector3f GetColumn(EDimY dim) const { return CVector3f(m01, m11, m21); }
#else
  const CVector3f GetColumn(EDimY dim) const {
#if RSTL_VERSION >= RSTL_R32J
    float x, y, z;
    z = m21;
    y = m11;
    x = m01;
    return CVector3f(x, y, z);
#else
    return CVector3f(m01, m11, m21);
#endif
  }
#endif

  const CVector3f GetColumn(EDimZ dim) const { return CVector3f(m02, m12, m22); }
  CVector3f GetColumn(int column) const {
    ConstMtxPtr matrix = GetCStyleMatrix();
    return static_cast< uint >(column) < 4
               ? CVector3f(matrix[0][column], matrix[1][column], matrix[2][column])
               : CVector3f::Zero();
  }

  void ScaleBy(const float scale);
  // GetCStyleMatrix__12CTransform4fCFv
  CTransform4f GetInverse() const;
  CTransform4f GetQuickInverse() const;
  CTransform4f GetRotation() const;
  inline const CVector3f& GetRow(EDimX dim) const {
    return *reinterpret_cast< const CVector3f* >(&m00);
  }
  inline const CVector3f& GetRow(EDimY dim) const {
    return *reinterpret_cast< const CVector3f* >(&m10);
  }
  inline const CVector3f& GetRow(EDimZ dim) const {
    return *reinterpret_cast< const CVector3f* >(&m20);
  }
  inline const CVector3f& GetRow(int i) const {
    return *(reinterpret_cast< const CVector3f* >(reinterpret_cast< const float* >(&m00) + i * 4));
  }
  // GetUp__12CTransform4fCFv
  static CTransform4f LookAt(const CVector3f& pos, const CVector3f& lookPos,
                             const CVector3f& up = CVector3f::Up());
  static CTransform4f MakeRotationsBasedOnY(const CUnitVector3f& yRot);
  CTransform4f MultiplyIgnoreTranslation(const CTransform4f& other) const;
  void Orthonormalize();
  CVector3f Rotate(const CVector3f& in) const;
  void RotateLocalX(const CRelAngle& angle);
  void RotateLocalY(const CRelAngle& angle);
  void RotateLocalZ(const CRelAngle& angle);
  static CTransform4f RotateX(const CRelAngle&);
#if !RSTL_PRIME1
  // Guessed name
  static CTransform4f Shear(float xy, float xz, float yx, float yz, float zx, float zy);
#endif

  static CTransform4f RotateY(const CRelAngle&);
  static CTransform4f RotateZ(const CRelAngle&);
  static CTransform4f Scale(float);
  static CTransform4f Scale(float, float, float);
  static CTransform4f Scale(const CVector3f&);
  // SetRotation__12CTransform4fFRC12CTransform4f
  void SetRotation(const CMatrix3f& rotation);
  void SetRotation(const CTransform4f& rotation);
  CVector3f TransposeMultiply(const CVector3f& in) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_R3IJ
    float z = in.GetZ();
    float y = in.GetY();
    float x = in.GetX();
#else
    float x = in.GetX();
    float y = in.GetY();
    float z = in.GetZ();
#endif
    return TransposeRotate(CVector3f(x - m03, y - m13, z - m23));
#else
    return TransposeRotate(CVector3f(in.GetX() - m03, in.GetY() - m13, in.GetZ() - m23));
#endif

  }
  CVector3f TransposeRotate(const CVector3f& in) const;

  void SetTranslation(const CVector3f& vec) {
    m03 = vec.GetX();
    m13 = vec.GetY();
    m23 = vec.GetZ();
  }
  void AddTranslation(const CVector3f& vec) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    m03 = GetCStyleMatrix()[0][3] + vec.GetX();
    m13 = GetCStyleMatrix()[1][3] + vec.GetY();
    m23 = GetCStyleMatrix()[2][3] + vec.GetZ();
#else
    m03 += vec.GetX();
    m13 += vec.GetY();
    m23 += vec.GetZ();
#endif

  }
#if !RSTL_PRIME1
  void AddTranslationX(float x) { m03 += x; }
  void AddTranslationY(float y) { m13 += y; }
#endif

  void AddTranslationZ(float z) { m23 += z; }

  CTransform4f& operator*=(const CTransform4f& other) {
    *this = *this * other;
    return *this;
  }
  CTransform4f operator*(const CTransform4f& vec) const;
  CVector3f operator*(const CVector3f& vec) const;

  static CTransform4f FromRows(const CVector3f&, const CVector3f&, const CVector3f&,
                               const CVector3f&);
  static CTransform4f FromColumns(const CVector3f&, const CVector3f&, const CVector3f&,
                                  const CVector3f&);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static CTransform4f Translate(float x, float y, float z);
#else
  static CTransform4f Translate(const float x, const float y, const float z);
#endif

  static CTransform4f Translate(const CVector3f& vec);

  static const CTransform4f& Identity() { return sIdentity; }

private:
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

  static const CTransform4f sIdentity;
};
#if !RSTL_PRIME1
CHECK_SIZEOF(CTransform4f, 0x30)
#endif

bool operator==(const CTransform4f& lhs, const CTransform4f& rhs);

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHECK_SIZEOF(CTransform4f, 0x30)
#else
namespace rstl {
RSTL_DECLARE_TRIVIALLY_DESTRUCTIBLE(CTransform4f)
RSTL_DECLARE_BITWISE_CONSTRUCTION(CTransform4f)
} // namespace rstl
#endif

#endif // _CTRANSFORM4F
