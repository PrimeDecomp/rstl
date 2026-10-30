#ifndef _CVECTOR3I
#define _CVECTOR3I

#include "rstl/RstlVersions.h"
#include "types.h"
#include "rstl/construct.hpp"

class CVector3i {
public:
  CVector3i(int x, int y, int z);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int operator[](int index) const { return (&mX)[index]; }
#else
  const int& operator[](int index) const { return (&mX)[index]; }
  int& operator[](int index) { return (&mX)[index]; }

  int GetX() const { return mX; }
  int GetY() const { return mY; }
  int GetZ() const { return mZ; }

  static const CVector3i& Zero() { return sZeroVector; }
#endif

private:
  int mX;
  int mY;
  int mZ;

#if !RSTL_PRIME1
  static CVector3i sZeroVector;
  static CVector3i sUpVector;
  static CVector3i sDownVector;
  static CVector3i sLeftVector;
  static CVector3i sRightVector;
  static CVector3i sForwardVector;
  static CVector3i sBackVector;
  static CVector3i sOneVector;
#endif
};

#if !RSTL_PRIME1
bool operator==(const CVector3i& lhs, const CVector3i& rhs);
#endif

#if RSTL_KYOTO_CONSTRUCTION_POLICIES
namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CVector3i)
} // namespace rstl
#endif

#endif // _CVECTOR3I
