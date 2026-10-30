#ifndef _CSEGSTATEMENT
#define _CSEGSTATEMENT
#include "rstl/RstlVersions.h"

#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "types.h"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
struct CSegStatement {
#else
class CSegStatement {
#endif

public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CSegStatement() : mStatus(0) {}
  CQuaternion GetOrientation() const { return Orientation(); }
  CVector3f GetOffset() const { return Offset(); }
  CQuaternion& Orientation() { return mRotation; }
#else
  CSegStatement() : mRotationValid(false), mOffsetValid(false), mScaleValid(false) {}
#endif

  const CQuaternion& Orientation() const { return mRotation; }
#if RSTL_PRIME1
  CVector3f& Offset() { return mOffset; }
#endif

  const CVector3f& Offset() const { return mOffset; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  bool OffsetValid() const { return mStatus != 0; }
  void Set(const CQuaternion& rotation) { Orientation() = rotation; }
#else
  const CVector3f& Scale() const { return mScale; }
  bool RotationValid() const { return mRotationValid; }
  bool OffsetValid() const { return mOffsetValid; }
  bool ScaleValid() const { return mScaleValid; }
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void SetStatus(uint status) { mStatus = status; }
#else
  void Set(const CQuaternion& rotation) {
    mRotation = rotation;
    mRotationValid = true;
  }
#endif

  void Set(const CVector3f& offset) {
    mOffset = offset;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    mStatus = 1;
#else
    mOffsetValid = true;
  }
  void SetScale(const CVector3f& scale) {
    mScale = scale;
    mScaleValid = true;
#endif

  }

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHECK_SIZEOF(CSegStatement, 0x20)
#else
CHECK_SIZEOF(CSegStatement, 0x2c)
#endif

#endif // _CSEGSTATEMENT
