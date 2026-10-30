#ifndef _CUNITVECTOR3F
#define _CUNITVECTOR3F

#include "types.h"

#include "Kyoto/Math/CVector3f.hpp"
#include "rstl/RstlVersions.h"

class CUnitVector3f : public CVector3f {
public:
  enum EDontNormalize {
    kN_No,
  };

  enum ENormalize {
    kN_Yes,
  };

#if !RSTL_PRIME1
  CUnitVector3f(CInputStream& in) : CVector3f(in) { Normalize(); }
#endif
  CUnitVector3f(const float x, const float y, const float z) : CVector3f(x, y, z) {}
#if RSTL_PRIME1
  CUnitVector3f(const float x, const float y, const float z, const ENormalize)
  : CVector3f(x, y, z) {
    Normalize();
  }
  CUnitVector3f(const float x, const float y, const float z, const EDontNormalize)
  : CVector3f(x, y, z) {}
#else
  CUnitVector3f(const float x, const float y, const float z, const ENormalize);
  CUnitVector3f(const float x, const float y, const float z, const EDontNormalize);
#endif

  CUnitVector3f(const CVector3f& vec, const ENormalize) : CVector3f(vec) { Normalize(); }

  CUnitVector3f(const CVector3f& vec, const EDontNormalize) : CVector3f(vec) {}

  CUnitVector3f(const CVector3f& vec);
  CUnitVector3f operator-() const { return CUnitVector3f(-GetX(), -GetY(), -GetZ()); }

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
#if RSTL_PRIME1
CHECK_SIZEOF(CUnitVector3f, 0xc)
#else
CHECK_CHILD_SIZEOF(CUnitVector3f, CVector3f, 0x0)
#endif

inline const CUnitVector3f& CVector3f::Up() { return sUpVector; }
inline const CUnitVector3f& CVector3f::Down() { return sDownVector; }
inline const CUnitVector3f& CVector3f::Left() { return sLeftVector; }
inline const CUnitVector3f& CVector3f::Right() { return sRightVector; }
inline const CUnitVector3f& CVector3f::Forward() { return sForwardVector; }
inline const CUnitVector3f& CVector3f::Back() { return sBackVector; }

#if !RSTL_PRIME1
#ifdef __MWERKS__
#pragma cpp_extensions on
#endif
inline CUnitVector3f::CUnitVector3f(const float x, const float y, const float z, const ENormalize)
: CVector3f(x, y, z) {
  Normalize();
}

inline CUnitVector3f::CUnitVector3f(const float x, const float y, const float z,
                                    const EDontNormalize)
: CVector3f(x, y, z) {}
#ifdef __MWERKS__
#pragma cpp_extensions reset
#endif
#endif

#endif // _CUNITVECTOR3F
