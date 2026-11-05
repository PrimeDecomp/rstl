#ifndef _CUNITVECTOR3F
#define _CUNITVECTOR3F

#include "types.h"

#include "Kyoto/Math/CVector3f.hpp"
#include "rstl/RstlVersions.h"

class CUnitVector3f : public CVector3f {
public:
#if RSTL_KYOTO_UNIT_VECTOR_FLAG_ENUM
  enum ENormalize {
    kN_No,
    kN_Yes,
  };
#else
  enum EDontNormalize {
    kN_No,
  };

  enum ENormalize {
    kN_Yes,
  };
#endif

#if !RSTL_PRIME1
  CUnitVector3f(CInputStream& in) : CVector3f(in) { Normalize(); }
#endif
  CUnitVector3f(RSTL_KYOTO_VECTOR_SCALAR_CONST float x, RSTL_KYOTO_VECTOR_SCALAR_CONST float y,
                RSTL_KYOTO_VECTOR_SCALAR_CONST float z)
  : CVector3f(x, y, z) {}
#if RSTL_KYOTO_UNIT_VECTOR_FLAG_ENUM
  CUnitVector3f(float x, float y, float z, ENormalize normalize) : CVector3f(x, y, z) {
    if (normalize == kN_Yes) {
      Normalize();
    }
  }
  CUnitVector3f(const CVector3f& vec, ENormalize normalize) : CVector3f(vec) {
    if (normalize == kN_Yes) {
      Normalize();
    }
  }
#else
  CUnitVector3f(const float x, const float y, const float z, const ENormalize);
  CUnitVector3f(const float x, const float y, const float z, const EDontNormalize);

  CUnitVector3f(const CVector3f& vec, const ENormalize) : CVector3f(vec) { Normalize(); }

  CUnitVector3f(const CVector3f& vec, const EDontNormalize) : CVector3f(vec) {}
#endif

  CUnitVector3f(const CVector3f& vec);
#if !RSTL_KYOTO_UNIT_VECTOR_FLAG_ENUM
  CUnitVector3f operator-() const { return CUnitVector3f(-GetX(), -GetY(), -GetZ()); }
#endif

#if !RSTL_PRIME1
  // Views the unit vector as a plain CVector3f, selecting CVector3f's operators over the
  // CUnitVector3f ones.
  const CVector3f& AsVector() const { return *this; }

  static CUnitVector3f Forward() {
    return CUnitVector3f(CVector3f::Forward().GetX(), CVector3f::Forward().GetY(),
                         CVector3f::Forward().GetZ(), kN_No);
  }
#endif
};

CHECK_CHILD_SIZEOF(CUnitVector3f, CVector3f, 0x0)

#if RSTL_KYOTO_UNIT_VECTOR_FLAG_ENUM
inline CUnitVector3f operator-(const CUnitVector3f& vec) {
  return CUnitVector3f(-vec.GetX(), -vec.GetY(), -vec.GetZ(), CUnitVector3f::kN_No);
}
#endif

inline const CUnitVector3f& CVector3f::Up() { return sUpVector; }
inline const CUnitVector3f& CVector3f::Down() { return sDownVector; }
inline const CUnitVector3f& CVector3f::Left() { return sLeftVector; }
inline const CUnitVector3f& CVector3f::Right() { return sRightVector; }
inline const CUnitVector3f& CVector3f::Forward() { return sForwardVector; }
inline const CUnitVector3f& CVector3f::Back() { return sBackVector; }

#if !RSTL_KYOTO_UNIT_VECTOR_FLAG_ENUM
#if !RSTL_PRIME1 && defined(__MWERKS__)
#pragma cpp_extensions on
#endif
inline CUnitVector3f::CUnitVector3f(const float x, const float y, const float z, const ENormalize)
: CVector3f(x, y, z) {
  Normalize();
}

inline CUnitVector3f::CUnitVector3f(const float x, const float y, const float z,
                                    const EDontNormalize)
: CVector3f(x, y, z) {}
#if !RSTL_PRIME1 && defined(__MWERKS__)
#pragma cpp_extensions reset
#endif
#endif

#endif // _CUNITVECTOR3F
