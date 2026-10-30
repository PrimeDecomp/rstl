#ifndef _CCUBESURFACE
#define _CCUBESURFACE
#include "rstl/RstlVersions.h"

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"

#include "rstl/construct.hpp"

class CCubeModel;
class CCubeSurface {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
public:
  CCubeSurface(void* ptr) { mRawdata = static_cast< uchar* >(ptr); }
#else
  friend class CCubeModel;
  friend class CModel;
#endif

  struct SSurfaceData {
    CVector3f mCenter;
    uint mMaterialIndex;
    uint mDisplayListSizeAndNormalHint;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CCubeModel* mParent;
    void* mNextSurface;
#else
    // Runtime links installed after the serialized surface is loaded.
    mutable CCubeModel* mParent;
    mutable const void* mNextSurface;
#endif

    uint mExtraSize;
    CUnitVector3f mNormal;
#if !RSTL_PRIME1
    short mMatrixBank;
    short x2e_;
#endif

    CAABox mBounds;
  };

  static const CVector3f skDefaultNormal;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  union {
    uchar* mRawdata;
    SSurfaceData* mData;
  };
#else
  const SSurfaceData* mData;
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  uint GetDisplayListSize() const { return mData->mDisplayListSizeAndNormalHint & 0x7fffffff; }
  const void* GetDisplayList() const {
    return reinterpret_cast< const SSurfaceData* >(mRawdata + GetSurfaceHeaderSize());
  }
  uint GetSurfaceHeaderSize() const {
    return (sizeof(SSurfaceData) + 7 + mData->mExtraSize) & ~31;
  }
#else
public:
  explicit CCubeSurface(const void* data) : mData(static_cast< const SSurfaceData* >(data)) {}

  CAABox GetBounds() const;
#endif

  const CVector3f& GetCenter() const { return mData->mCenter; }
  const CUnitVector3f& GetNormalHint() const { return mData->mNormal; }
  uint GetMaterialIndex() const { return mData->mMaterialIndex; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CAABox GetBounds() const;
#else
  CCubeModel* GetParent() const { return mData->mParent; }
  short GetMatrixBank() const { return mData->mMatrixBank; } // Guessed name.
  short GetShadowBank() const { return mData->x2e_; }        // Target-derived grouping key.
  uint GetDisplayListSize() const { return mData->mDisplayListSizeAndNormalHint & 0x7fffffff; }
  uint GetSurfaceHeaderSize() const { return (sizeof(SSurfaceData) + 7 + mData->mExtraSize) & ~31; }
  const void* GetDisplayList() const {
    return reinterpret_cast< const uchar* >(mData) + GetSurfaceHeaderSize();
  }
#endif

  CCubeSurface GetNextSurface() const { return CCubeSurface(mData->mNextSurface); }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  bool IsValid() const { return mRawdata != nullptr; }

private:
#else
  bool IsValid() const { return mData != nullptr; }
#endif

};
#if !RSTL_PRIME1
CHECK_SIZEOF(CCubeSurface, 4)
#endif

#if !RSTL_PRIME1
namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CCubeSurface)
} // namespace rstl
#endif

#endif // _CCUBESURFACE
