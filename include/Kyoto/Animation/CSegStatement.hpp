#ifndef _CSEGSTATEMENT
#define _CSEGSTATEMENT
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "rstl/types.h"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
struct
#else
class
#endif
CSegStatement {
public:
  CSegStatement()
#if RSTL_PRIME1
  : mStatus(0)
#else
  : mRotationValid(false)
  , mOffsetValid(false)
  , mScaleValid(false)
#endif
  {
  }
#if RSTL_PRIME1
  CQuaternion GetOrientation() const { return Orientation(); }
  CVector3f GetOffset() const { return Offset(); }
  CQuaternion& Orientation() { return mRotation; }
#endif

  const CQuaternion& Orientation() const { return mRotation; }
#if RSTL_PRIME1
  CVector3f& Offset() { return mOffset; }
#endif

  const CVector3f& Offset() const { return mOffset; }
#if !RSTL_PRIME1
  const CVector3f& Scale() const { return mScale; }
  bool RotationValid() const { return mRotationValid; }
#endif
  bool OffsetValid() const {
#if RSTL_PRIME1
    return mStatus != 0;
#else
    return mOffsetValid;
#endif
  }
#if RSTL_PRIME1
  void SetStatus(uint status) { mStatus = status; }
#else
  bool ScaleValid() const { return mScaleValid; }
#endif

  void Set(const CQuaternion& rotation) {
#if RSTL_PRIME1
    Orientation() = rotation;
#else
    mRotation = rotation;
    mRotationValid = true;
#endif
  }

  void Set(const CVector3f& offset) {
    mOffset = offset;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    mStatus = 1;
#else
    mOffsetValid = true;
#endif
  }
#if !RSTL_PRIME1
  void SetScale(const CVector3f& scale) {
    mScale = scale;
    mScaleValid = true;
  }
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
public:
#else
private:
#endif

  CQuaternion mRotation;
  CVector3f mOffset;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  uint mStatus; // 0 free, 1 used
#else
  CVector3f mScale;
  bool mRotationValid : 1;
  bool mOffsetValid : 1;
  bool mScaleValid : 1;
#endif
};
CHECK_SIZEOF(CSegStatement, RSTL_PRIME1 ? 0x20 : 0x2c)

#endif // _CSEGSTATEMENT
