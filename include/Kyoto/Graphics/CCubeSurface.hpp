#ifndef _CCUBESURFACE
#define _CCUBESURFACE
#include "rstl/RstlVersions.h"

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"

#include "rstl/construct.hpp"

class CCubeModel;
class CCubeSurface {
  friend class CCubeModel;
  friend class CModel;

public:
  struct SSurfaceData {
    CVector3f mCenter;
    uint mMaterialIndex;
    uint mDisplayListSizeAndNormalHint;
    mutable CCubeModel* mParent;
    mutable const void* mNextSurface;
    uint mExtraSize;
    CUnitVector3f mNormal;
#if !RSTL_PRIME1
    short mMatrixBank;
    short x2e_;
#endif
    CAABox mBounds;
  };

  CCubeSurface(const void* data) : mData(static_cast< const SSurfaceData* >(data)) {}

  const CVector3f& GetCenter() const { return mData->mCenter; }
  const CUnitVector3f& GetNormalHint() const { return mData->mNormal; }
  uint GetMaterialIndex() const { return mData->mMaterialIndex; }
  CAABox GetBounds() const;
  CCubeModel* GetParent() const { return mData->mParent; }
#if !RSTL_PRIME1
  short GetMatrixBank() const { return mData->mMatrixBank; } // Guessed name.
  short GetShadowBank() const { return mData->x2e_; }        // Target-derived grouping key.
#endif
  uint GetDisplayListSize() const { return mData->mDisplayListSizeAndNormalHint & 0x7fffffff; }
  uint GetSurfaceHeaderSize() const { return (sizeof(SSurfaceData) + 7 + mData->mExtraSize) & ~31; }
  const void* GetDisplayList() const {
    return reinterpret_cast< const uchar* >(mData) + GetSurfaceHeaderSize();
  }
  CCubeSurface GetNextSurface() const { return CCubeSurface(mData->mNextSurface); }
  bool IsValid() const { return mData != nullptr; }

  static const CVector3f skDefaultNormal;
  union {
    uchar* mRawdata;
    const SSurfaceData* mData;
  };
};
CHECK_SIZEOF(CCubeSurface, 4)

#if RSTL_KYOTO_CONSTRUCTION_POLICIES
namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CCubeSurface)
} // namespace rstl
#endif

#endif // _CCUBESURFACE
