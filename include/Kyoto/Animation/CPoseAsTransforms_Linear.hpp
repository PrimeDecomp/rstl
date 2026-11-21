#ifndef _CPOSEASTRANSFORMS_LINEAR
#define _CPOSEASTRANSFORMS_LINEAR
#include "rstl/RstlVersions.h"

#include "rstl/static_assert.hpp"
#include "rstl/types.h"

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/vector.hpp"

class CCharLayoutInfo;
class CJointData_LinearStorage;
#if RSTL_CORRUPTION
class CVisibilityAnimationDataReader;
#endif

class CPoseAsTransforms_Linear {
public:
  class CElementType {
  public:
#if RSTL_CORRUPTION
    CElementType(const CMatrix3f& rotation, const CVector3f& offset, const CVector3f& localOffset,
                 const CVector3f& skinningOffset)
    : mRotation(rotation), mOffset(offset), mLocalOffset(localOffset),
      mSkinningOffset(skinningOffset) {}
#else
    CElementType(const CMatrix3f& rotation, const CVector3f& offset, const CVector3f& localOffset)
    : mRotation(rotation), mOffset(offset), mLocalOffset(localOffset) {}
#endif

    CMatrix3f mRotation;
    CVector3f mOffset;
    CVector3f mLocalOffset;
#if RSTL_CORRUPTION
    CVector3f mSkinningOffset; // Guessed name
#endif
  };

  CPoseAsTransforms_Linear(int count, int withScale, int withOffsets);

  void AllocateScale();
  void SetUniformScale(bool uniform) { mUniformScale = uniform; } // Guessed name.
  void RotateHierarchy(const CCharLayoutInfo& layout, const CSegId& seg, const CMatrix3f& rotation,
                       int order);
  void SetRotation(const CCharLayoutInfo& layout, const CSegId& seg, const CMatrix3f& rotation);
  void BuildPose(const CCharLayoutInfo& layout, const CJointData_LinearStorage& data);
#if RSTL_CORRUPTION
  // Guessed names
  void SetRotationAndUpdateChildOffsets(const CCharLayoutInfo& layout, const CSegId& seg,
                                        const CMatrix3f& rotation);
  void UpdateVisibility(const CVisibilityAnimationDataReader& data);
  void ClearVisibility();
  const CVector3f& GetSkinningOffset(const CSegId& seg) const;
  const CVector3f& GetScale(const CSegId& seg) const;
#endif
  CTransform4f GetTransform(const CSegId& seg);
  const CVector3f& GetOffset(const CSegId& seg) const;
  CMatrix3f GetRotation(const CSegId& seg);
  const CMatrix3f& GetTransformMinusOffset(const CSegId& seg) const;
#if !RSTL_CORRUPTION
  rstl::vector< CElementType >& GetElements() { return mElements; }
#endif
  const rstl::vector< CElementType >& GetElements() const { return mElements; }
#if RSTL_CORRUPTION
  const rstl::vector< CVector3f >& GetScales() const { return mScales; } // Guessed name
#endif
  bool HasScale() const { return !mScales.empty(); }
  bool HasUniformScale() const { return mUniformScale; }
#if RSTL_CORRUPTION
  rstl::vector< uint > GetVisibility() const { return mVisibility; } // Guessed name
#endif

private:
  rstl::vector< CElementType > mElements;
  rstl::vector< CVector3f > mScales;
  rstl::vector< CMatrix3f > mUnscaledRotations;
  rstl::vector< CVector3f > x30_;
  bool x40_24_ : 1;
  uchar mUniformScale;
#if RSTL_CORRUPTION
  rstl::vector< uint > mVisibility; // Guessed name
#endif
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CPoseAsTransforms_Linear, 0x54)
NESTED_CHECK_SIZEOF(CPoseAsTransforms_Linear, CElementType, 0x48)
#else
CHECK_SIZEOF(CPoseAsTransforms_Linear, 0x44)
#endif

#endif // _CPOSEASTRANSFORMS_LINEAR
