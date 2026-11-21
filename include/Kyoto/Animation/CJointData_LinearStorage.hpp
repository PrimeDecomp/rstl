#ifndef _CJOINTDATA_LINEARSTORAGE
#define _CJOINTDATA_LINEARSTORAGE
#include "rstl/RstlVersions.h"

#include "rstl/static_assert.hpp"
#include "rstl/types.h"

#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#if !RSTL_CORRUPTION
class CCharLayoutInfo;
#endif

// Guessed name. A strided cursor into the joint pose arrays.
#if RSTL_CORRUPTION
class CCharLayoutInfo;
#else
union UJointCursor {
  UJointCursor(uchar* bytes) : mBytes(bytes) {}

  uchar* mBytes;
  CQuaternion* mRotation;
  CVector3f* mVector;
};
#endif

class CJointData_LinearStorage {
public:
  enum EAllocateFrom {
    kAF_Pool, // Guessed name: falls back to heap allocation when the pool is exhausted.
    kAF_Heap, // Guessed name.
  };

  CJointData_LinearStorage(int count, EAllocateFrom allocateFrom);
  ~CJointData_LinearStorage();
  void SetZeroRotation();
  void SetReferenceOffsets(const CCharLayoutInfo& layout);
  // Guessed names; the native operations preserve the existing flags and use this count.
  void Add(const CJointData_LinearStorage& other, float weight);
  void Blend(const CJointData_LinearStorage& other, float weight);
  // Guessed name; clears flags without modifying the stored pose.
  void ResetFlags();
#if RSTL_CORRUPTION
  void ResetTransforms(); // Guessed name.
#endif

#if RSTL_CORRUPTION
  uchar* GetRotations() { return mRotations; }
  const uchar* GetRotations() const { return mRotations; }
#else
  UJointCursor GetRotations() { return mRotations; }
  const uchar* GetRotations() const { return mRotations.mBytes; }
#endif

#if RSTL_CORRUPTION
  uchar* GetTranslations() { return mTranslations; }
  const uchar* GetTranslations() const { return mTranslations; }
#else
  UJointCursor GetTranslations() { return mTranslations; }
  const uchar* GetTranslations() const { return mTranslations.mBytes; }
#endif

#if RSTL_CORRUPTION
  uchar* GetScales() { return mScales; }
  const uchar* GetScales() const { return mScales; }
#else
  UJointCursor GetScales() { return mScales; }
  const uchar* GetScales() const { return mScales.mBytes; }
#endif

#if RSTL_CORRUPTION
  int GetNumJoints() const { return mCount; }
#endif
  int GetStride() const { return mStride; }

  CQuaternion& Rotation(int index) {
#if RSTL_CORRUPTION
    return *reinterpret_cast< CQuaternion* >(mRotations + mStride * index);
#else
    return *reinterpret_cast< CQuaternion* >(mRotations.mBytes + mStride * index);
#endif
  }

  const CQuaternion& Rotation(int index) const {
#if RSTL_CORRUPTION
    return *reinterpret_cast< const CQuaternion* >(mRotations + mStride * index);
#else
    return *reinterpret_cast< const CQuaternion* >(mRotations.mBytes + mStride * index);
#endif
  }

  CVector3f& Translation(int index) {
#if RSTL_CORRUPTION
    return *reinterpret_cast< CVector3f* >(mTranslations + mStride * index);
#else
    return *reinterpret_cast< CVector3f* >(mTranslations.mBytes + mStride * index);
#endif
  }

  const CVector3f& Translation(int index) const {
#if RSTL_CORRUPTION
    return *reinterpret_cast< const CVector3f* >(mTranslations + mStride * index);
#else
    return *reinterpret_cast< const CVector3f* >(mTranslations.mBytes + mStride * index);
#endif
  }

#if RSTL_CORRUPTION
  CVector3f& Scale(int index) { return *reinterpret_cast< CVector3f* >(mScales + index * mStride); }
#else
  CVector3f& Scale(int index) {
    return *reinterpret_cast< CVector3f* >(mScales.mBytes + mStride * index);
  }
#endif

  const CVector3f& Scale(int index) const {
#if RSTL_CORRUPTION
    return *reinterpret_cast< const CVector3f* >(mScales + index * mStride);
#else
    return *reinterpret_cast< const CVector3f* >(mScales.mBytes + index * mStride);
#endif
  }

  bool HasScales() const { return mHasScales; }

  bool UsesZeroOffsets() const { return mUseZeroOffsets; }
  void SetUseZeroOffsets(bool value) { mUseZeroOffsets = value; }

  void SetHasOffsets(bool value) { mHasOffsets = value; }

  void SetHasScales(bool value) { mHasScales = value; }

  void ResetScales(); // Guessed name.

private:
  // Guessed name; may change the selected allocation policy on pool exhaustion.
  static void* Allocate(int count, EAllocateFrom requested, EAllocateFrom& selected);

  EAllocateFrom mAllocationType;
  void* mStorage;
  int mCount;
  bool mHasScales : 1;
  bool mHasOffsets : 1;
  bool mUseZeroOffsets : 1;
#if RSTL_CORRUPTION
  uchar* mRotations;
  uchar* mTranslations;
  uchar* mScales;
#else
  UJointCursor mRotations;
  UJointCursor mTranslations;
  UJointCursor mScales;
#endif
  int mStride;
};
CHECK_SIZEOF(CJointData_LinearStorage, 0x20)

#endif // _CJOINTDATA_LINEARSTORAGE
