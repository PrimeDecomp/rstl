#ifndef _CCUBESURFACE
#define _CCUBESURFACE
#include "rstl/RstlVersions.h"

#if !RSTL_CORRUPTION
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "rstl/RstlVersions.h"
#include "rstl/construct.hpp"
#endif
#include "rstl/static_assert.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Math/CAABox.hpp"
#endif

#if !RSTL_CORRUPTION
#if defined(TARGET_PC)
#include "Metaforce/Game/CModelSectionReader.hpp"
#endif
#endif

class CCubeModel;
class CCubeSurface {
  friend class CCubeModel;
#if !RSTL_CORRUPTION
  friend class CModel;
#endif

#if !RSTL_CORRUPTION
public:
#endif
  struct SSurfaceData {
    CVector3f mCenter;
    uint mMaterialIndex;
    uint mDisplayListSizeAndNormalHint;
#if RSTL_CORRUPTION
    CCubeModel* mParent;
    const void* mNextSurface;
#else
    mutable CCubeModel* mParent;
    mutable const void* mNextSurface;
#endif
    uint mExtraSize;
#if RSTL_CORRUPTION
    CVector3f mNormal;
    short mMatrixBank;
    short x2e_;
#else
    CUnitVector3f mNormal;
#if !RSTL_PRIME1
    short mMatrixBank;
    short x2e_;
#endif
#endif
    CAABox mBounds;
#if !RSTL_CORRUPTION
#if defined(TARGET_PC)
    const void* displayList;
    uint headerSize;
#endif
#endif
  };

#if RSTL_CORRUPTION
  static const CVector3f skDefaultNormal;
  const SSurfaceData* mData;
#else
#if defined(TARGET_PC)
  static SSurfaceData ReadData(TModelData data, CCubeModel* parent, uint materialCount);
#endif
#endif

#if RSTL_CORRUPTION
public:
  explicit CCubeSurface(const void* data) : mData(static_cast< const SSurfaceData* >(data)) {}
#else
  CCubeSurface(const void* data) : mData(static_cast< const SSurfaceData* >(data)) {}
#endif

#if !RSTL_CORRUPTION
  const CVector3f& GetCenter() const { return mData->mCenter; }
  const CUnitVector3f& GetNormalHint() const { return mData->mNormal; }
  uint GetMaterialIndex() const { return mData->mMaterialIndex; }
#endif
  CAABox GetBounds() const;
#if RSTL_CORRUPTION
  uchar GetDrawMaskBitIndex() const; // Guessed name
  const CVector3f& GetCenter() const { return mData->mCenter; }
  const CVector3f& GetNormalHint() const { return mData->mNormal; }
  uint GetMaterialIndex() const { return mData->mMaterialIndex; }
#endif
  CCubeModel* GetParent() const { return mData->mParent; }
#if !RSTL_PRIME1
  short GetMatrixBank() const { return mData->mMatrixBank; } // Guessed name.
  short GetShadowBank() const { return mData->x2e_; }        // Target-derived grouping key.
#endif
  uint GetDisplayListSize() const { return mData->mDisplayListSizeAndNormalHint & 0x7fffffff; }
  uint GetSurfaceHeaderSize() const {
#if defined(TARGET_PC)
    return mData->headerSize;
#else
    return (sizeof(SSurfaceData) + 7 + mData->mExtraSize) & ~31;
#endif
  }
  const void* GetDisplayList() const {
#if defined(TARGET_PC)
    return mData->displayList;
#else
    return reinterpret_cast< const uchar* >(mData) + GetSurfaceHeaderSize();
#endif
  }
  CCubeSurface GetNextSurface() const { return CCubeSurface(mData->mNextSurface); }
  bool IsValid() const { return mData != nullptr; }
#if !RSTL_CORRUPTION

  static const CVector3f skDefaultNormal;
  union {
    uchar* mRawdata;
    const SSurfaceData* mData;
  };
#endif
};
CHECK_SIZEOF(CCubeSurface, 4)

#if !RSTL_CORRUPTION
#if RSTL_KYOTO_CONSTRUCTION_POLICIES
namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CCubeSurface)
RSTL_DECLARE_DIRECT_CONSTRUCTION(CCubeSurface)
} // namespace rstl
#endif
#endif

#endif // _CCUBESURFACE
