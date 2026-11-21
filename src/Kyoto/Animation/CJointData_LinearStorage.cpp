#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CJointData_LinearStorage.hpp"

#if !RSTL_CORRUPTION
#include "Kyoto/Animation/CAnimMathUtils.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#endif
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Alloc/LockedCache.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Animation/CAnimMathUtils.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"


#endif

void* CJointData_LinearStorage::Allocate(int count, EAllocateFrom requested,
                                         EAllocateFrom& selected) {
  void* storage = nullptr;
  selected = requested;
  if (selected == kAF_Pool) {
    storage = AllocateLockedCache4(count * (sizeof(CQuaternion) + 2 * sizeof(CVector3f)));
    if (storage == nullptr) {
      selected = kAF_Heap;
    }
  }

  if (selected != kAF_Pool) {
#if RSTL_CORRUPTION
    storage = CMemory::Alloc(count * (sizeof(CQuaternion) + 2 * sizeof(CVector3f)),
                             IAllocator::kHI_None, IAllocator::kSC_Unk1, IAllocator::kTP_Heap,
                             CCallStack(-1, "CJointData_LinearStorage.cpp(27) : ", gkUnknownType));
#else
    storage = CMemory::Alloc(count * (sizeof(CQuaternion) + 2 * sizeof(CVector3f)));
#endif
  }
  return storage;
}

CJointData_LinearStorage::CJointData_LinearStorage(int count, EAllocateFrom allocateFrom)
: mStorage(Allocate(count, allocateFrom, mAllocationType))
, mCount(count)
, mHasScales(false)
, mHasOffsets(false)
, mUseZeroOffsets(false)
, mRotations(static_cast< uchar* >(mStorage))
#if RSTL_CORRUPTION
, mTranslations(mRotations + sizeof(CQuaternion))
, mScales(mTranslations + sizeof(CVector3f))
#else
, mTranslations(mRotations.mBytes + sizeof(CQuaternion))
, mScales(mTranslations.mBytes + sizeof(CVector3f))
#endif
, mStride(sizeof(CQuaternion) + 2 * sizeof(CVector3f)) {}

CJointData_LinearStorage::~CJointData_LinearStorage() {
  if (mAllocationType == kAF_Pool) {
    rs_lc_free(mStorage);
  } else if (mAllocationType == kAF_Heap) {
    CMemory::Free(mStorage);
  }
}

void CJointData_LinearStorage::SetZeroRotation() {
#if RSTL_CORRUPTION
  uchar* destination = mRotations;
#else
  uchar* destination = mRotations.mBytes;
#endif
  for (int i = 0; i < mCount; ++i) {
    *reinterpret_cast< CQuaternion* >(destination) = CQuaternion::NoRotation();
    destination += mStride;
  }
}

void CJointData_LinearStorage::ResetScales() {
#if RSTL_CORRUPTION
  uchar* destination = mScales;
#else
  uchar* destination = mScales.mBytes;
#endif
  for (int i = 0; i < mCount; ++i) {
    *reinterpret_cast< CVector3f* >(destination) = CVector3f::One();
    destination += mStride;
  }
#if RSTL_CORRUPTION
  mHasScales = true;
}

void CJointData_LinearStorage::ResetTransforms() {
  uchar* rotations = mRotations;
  uchar* translations = mTranslations;
  uchar* scales = mScales;
  for (int i = 0; i < mCount; ++i) {
    *reinterpret_cast< CQuaternion* >(rotations) = CQuaternion::NoRotation();
    rotations += mStride;
    *reinterpret_cast< CVector3f* >(translations) = CVector3f::Zero();
    translations += mStride;
    *reinterpret_cast< CVector3f* >(scales) = CVector3f::One();
    scales += mStride;
  }
  mHasOffsets = true;
  mHasScales = true;
#endif
}

void CJointData_LinearStorage::SetReferenceOffsets(const CCharLayoutInfo& layout) {
  const CVector3f* source = layout.GetLinearParentOffsets().data();
#if RSTL_CORRUPTION
  uchar* destination = mTranslations;
#else
  uchar* destination = mTranslations.mBytes;
#endif
  for (int i = 0; i < mCount; ++i) {
    *reinterpret_cast< CVector3f* >(destination) = *source++;
    destination += mStride;
  }
#if RSTL_CORRUPTION
  mHasOffsets = true;
#endif
}

void CJointData_LinearStorage::ResetFlags() {
  mHasOffsets = false;
  mHasScales = false;
  mUseZeroOffsets = false;
}

void CJointData_LinearStorage::Blend(const CJointData_LinearStorage& other, float weight) {
#if RSTL_CORRUPTION
  uchar* rotations = mRotations;
#else
  uchar* rotations = mRotations.mBytes;
  uchar* translations = mTranslations.mBytes;
#endif
  const int stride = mStride;
#if RSTL_CORRUPTION
  const uchar* sourceRotations = other.mRotations;
#else
  const uchar* sourceRotations = other.mRotations.mBytes;
  const uchar* sourceTranslations = other.mTranslations.mBytes;
#endif
  const int sourceStride = other.mStride;
#if RSTL_CORRUPTION
  const bool blendScales = mHasScales && other.mHasScales;
  const bool blendOffsets = mHasOffsets && other.mHasOffsets;
#endif

#if RSTL_CORRUPTION
  if (!blendScales && !blendOffsets) {
    for (int i = 0; i < mCount; ++i) {
      CQuaternion& rotation = *reinterpret_cast< CQuaternion* >(rotations);
      const CQuaternion& sourceRotation = *reinterpret_cast< const CQuaternion* >(sourceRotations);
      rotation = CAnimMathUtils::SlerpLocal(rotation, sourceRotation, weight);
      rotations += stride;
      sourceRotations += sourceStride;
    }
  } else if (!blendScales) {
    uchar* translations = mTranslations;
    const uchar* sourceTranslations = other.mTranslations;
#else
  if (!mHasScales) {
#endif
    for (int i = 0; i < mCount; ++i) {
      CQuaternion& rotation = *reinterpret_cast< CQuaternion* >(rotations);
      const CQuaternion& sourceRotation = *reinterpret_cast< const CQuaternion* >(sourceRotations);
      rotation = CAnimMathUtils::SlerpLocal(rotation, sourceRotation, weight);
      rotations += stride;
      sourceRotations += sourceStride;

      CVector3f& translation = *reinterpret_cast< CVector3f* >(translations);
      const CVector3f& sourceTranslation =
          *reinterpret_cast< const CVector3f* >(sourceTranslations);
      translation = CVector3f::Lerp(translation, sourceTranslation, weight);
      translations += stride;
      sourceTranslations += sourceStride;
    }
#if RSTL_CORRUPTION
  } else if (!blendOffsets) {
    uchar* scales = mScales;
    const uchar* sourceScales = other.mScales;
    for (int i = 0; i < mCount; ++i) {
      CQuaternion& rotation = *reinterpret_cast< CQuaternion* >(rotations);
      const CQuaternion& sourceRotation = *reinterpret_cast< const CQuaternion* >(sourceRotations);
      rotation = CAnimMathUtils::SlerpLocal(rotation, sourceRotation, weight);
      rotations += stride;
      sourceRotations += sourceStride;

      CVector3f& scale = *reinterpret_cast< CVector3f* >(scales);
      const CVector3f& sourceScale = *reinterpret_cast< const CVector3f* >(sourceScales);
      scale = CVector3f::Lerp(scale, sourceScale, weight);
      scales += stride;
      sourceScales += sourceStride;
    }
#endif
  } else {
#if RSTL_CORRUPTION
    uchar* scales = mScales;
    const uchar* sourceScales = other.mScales;
    uchar* translations = mTranslations;
    const uchar* sourceTranslations = other.mTranslations;
#else
    uchar* scales = mScales.mBytes;
    const uchar* sourceScales = other.mScales.mBytes;
#endif
    for (int i = 0; i < mCount; ++i) {
      CQuaternion& rotation = *reinterpret_cast< CQuaternion* >(rotations);
      const CQuaternion& sourceRotation = *reinterpret_cast< const CQuaternion* >(sourceRotations);
      rotation = CAnimMathUtils::SlerpLocal(rotation, sourceRotation, weight);
      rotations += stride;
      sourceRotations += sourceStride;

      CVector3f& translation = *reinterpret_cast< CVector3f* >(translations);
      const CVector3f& sourceTranslation =
          *reinterpret_cast< const CVector3f* >(sourceTranslations);
      translation = CVector3f::Lerp(translation, sourceTranslation, weight);
      translations += stride;
      sourceTranslations += sourceStride;

      CVector3f& scale = *reinterpret_cast< CVector3f* >(scales);
      const CVector3f& sourceScale = *reinterpret_cast< const CVector3f* >(sourceScales);
      scale = CVector3f::Lerp(scale, sourceScale, weight);
      scales += stride;
      sourceScales += sourceStride;
    }
  }
}

void CJointData_LinearStorage::Add(const CJointData_LinearStorage& other, float weight) {
#if RSTL_CORRUPTION
  uchar* rotations = mRotations;
  uchar* translations = mTranslations;
#else
  uchar* rotations = mRotations.mBytes;
  uchar* translations = mTranslations.mBytes;
#endif
  const int stride = mStride;
#if RSTL_CORRUPTION
  const uchar* sourceRotations = other.mRotations;
  const uchar* sourceTranslations = other.mTranslations;
#else
  const uchar* sourceRotations = other.mRotations.mBytes;
  const uchar* sourceTranslations = other.mTranslations.mBytes;
#endif
  const int sourceStride = other.mStride;

#if RSTL_CORRUPTION
  if (!mHasScales || !other.mHasScales) {
#else
  if (!mHasScales) {
#endif
    for (int i = 0; i < mCount; ++i) {
      CQuaternion& rotation = *reinterpret_cast< CQuaternion* >(rotations);
      const CQuaternion& sourceRotation = *reinterpret_cast< const CQuaternion* >(sourceRotations);
      rotation =
          rotation * CAnimMathUtils::Slerp(CQuaternion::NoRotation(), sourceRotation, weight);
      rotations += stride;
      sourceRotations += sourceStride;

      CVector3f& translation = *reinterpret_cast< CVector3f* >(translations);
      const CVector3f& sourceTranslation =
          *reinterpret_cast< const CVector3f* >(sourceTranslations);
      translation = translation + weight * sourceTranslation;
      translations += stride;
      sourceTranslations += sourceStride;
    }
  } else {
#if RSTL_CORRUPTION
    uchar* scales = mScales;
    const uchar* sourceScales = other.mScales;
#else
    uchar* scales = mScales.mBytes;
    const uchar* sourceScales = other.mScales.mBytes;
#endif
    for (int i = 0; i < mCount; ++i) {
      CQuaternion& rotation = *reinterpret_cast< CQuaternion* >(rotations);
      const CQuaternion& sourceRotation = *reinterpret_cast< const CQuaternion* >(sourceRotations);
      rotation =
          rotation * CAnimMathUtils::Slerp(CQuaternion::NoRotation(), sourceRotation, weight);
      rotations += stride;
      sourceRotations += sourceStride;

      CVector3f& translation = *reinterpret_cast< CVector3f* >(translations);
      const CVector3f& sourceTranslation =
          *reinterpret_cast< const CVector3f* >(sourceTranslations);
      translation = translation + weight * sourceTranslation;
      translations += stride;
      sourceTranslations += sourceStride;

      CVector3f& scale = *reinterpret_cast< CVector3f* >(scales);
      const CVector3f& sourceScale = *reinterpret_cast< const CVector3f* >(sourceScales);
      scale = CVector3f::ByElementMultiply(scale,
                                           CVector3f::Lerp(CVector3f::One(), sourceScale, weight));
      scales += stride;
      sourceScales += sourceStride;
    }
  }
}
