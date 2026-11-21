#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CPoseAsTransforms_Linear.hpp"

#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CJointData_LinearStorage.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Animation/CVisibilityAnimationData.hpp"
#endif
#include "Kyoto/Math/CQuaternion.hpp"

static rstl::vector< CSegId >::const_iterator ConnectedPartsEnd(const CCharLayoutInfo& layout,
                                                                const CSegId& seg);

CPoseAsTransforms_Linear::CPoseAsTransforms_Linear(int count, int withScale, int withOffsets)
#if RSTL_CORRUPTION
: mElements(count, CElementType(CMatrix3f::Identity(), CVector3f::Zero(), CVector3f::Zero(),
                                CVector3f::Zero()))
#else
: mElements(count, CElementType(CMatrix3f::Identity(), CVector3f::Zero(), CVector3f::Zero()))
#endif
, mScales(withScale == 1 ? count : 0, CVector3f::One())
, mUnscaledRotations(withScale == 1 ? count : 0, CMatrix3f::Identity())
, x30_(withOffsets == 1 ? count : 0, CVector3f::Zero())
, x40_24_(false)
, mUniformScale(0) {}

const CMatrix3f& CPoseAsTransforms_Linear::GetTransformMinusOffset(const CSegId& seg) const {
  return mElements[seg.val()].mRotation;
}

CMatrix3f CPoseAsTransforms_Linear::GetRotation(const CSegId& seg) {
  if (mScales.size() != 0) {
    return mUnscaledRotations[seg.val()];
  }
  return GetTransformMinusOffset(seg);
}

const CVector3f& CPoseAsTransforms_Linear::GetOffset(const CSegId& seg) const {
  return mElements[seg.val()].mOffset;
}

#if RSTL_CORRUPTION
const CVector3f& CPoseAsTransforms_Linear::GetSkinningOffset(const CSegId& seg) const {
  return mElements[seg.val()].mSkinningOffset;
}

const CVector3f& CPoseAsTransforms_Linear::GetScale(const CSegId& seg) const {
  return mScales[seg.val()];
}

#endif
CTransform4f CPoseAsTransforms_Linear::GetTransform(const CSegId& seg) {
  const CElementType& elem = mElements[seg.val()];
  if (mScales.size() != 0) {
    return CTransform4f(mUnscaledRotations[seg.val()], elem.mOffset);
  }
  return CTransform4f(elem.mRotation, elem.mOffset);
}

void CPoseAsTransforms_Linear::BuildPose(const CCharLayoutInfo& layout,
                                         const CJointData_LinearStorage& data) {
  x40_24_ = false;
  const uchar* rotations = data.GetRotations();
  const uchar* translations = data.GetTranslations();
  const uchar* scales = data.GetScales();
  int stride = data.GetStride();
#if RSTL_CORRUPTION
  const CVector3f* referenceOffset = layout.GetLinearReferenceStanceOffsets().data();
  if (mScales.size() != 0 && data.HasScales()) {
    CElementType* elem = mElements.data();
    const CSegId* parent = layout.GetLinearParents().data();
    CMatrix3f* unscaled = mUnscaledRotations.data();
    CVector3f* scale = mScales.data();
#else

  if (mScales.size() != 0) {
    rstl::vector< CElementType >::iterator elem = mElements.begin();
    rstl::vector< CSegId >::const_iterator parent = layout.GetLinearParents().begin();
    rstl::vector< CMatrix3f >::iterator unscaled = mUnscaledRotations.begin();
    rstl::vector< CVector3f >::iterator scale = mScales.begin();

#endif
    elem->mRotation = CMatrix3f::Identity();
    elem->mOffset = CVector3f::Zero();
    *unscaled = CMatrix3f::Identity();
    *scale++ = CVector3f::One();

    const uchar* scaleBase = data.GetScales();
    rotations += stride;
    translations += stride;
    scales += stride;
    ++elem;
    ++unscaled;
    ++parent;
#if RSTL_CORRUPTION
    ++referenceOffset;
    long count = mElements.size();
#else

    int count = mElements.size();
#endif
    for (int i = 1; i < count; ++i) {
      uchar parentId = parent->val();
      const CElementType& parentElem = mElements[parentId];
      const CMatrix3f& parentRotation = mUnscaledRotations[parentId];
      *scale = *reinterpret_cast< const CVector3f* >(scales);
      *unscaled =
          parentRotation * reinterpret_cast< const CQuaternion* >(rotations)->BuildTransform();
      elem->mRotation = *unscaled * CMatrix3f::Scale(scale->GetX(), scale->GetY(), scale->GetZ());
      elem->mLocalOffset = *reinterpret_cast< const CVector3f* >(translations);
#if RSTL_CORRUPTION
      const CVector3f& parentScale =
          *reinterpret_cast< const CVector3f* >(scaleBase + stride * parentId);
      elem->mOffset = parentElem.mOffset + parentRotation * (elem->mLocalOffset * parentScale);
      elem->mSkinningOffset = elem->mOffset - elem->mRotation * *referenceOffset;
#else
      elem->mOffset = parentElem.mOffset +
                      parentRotation *
                          CVector3f::ByElementMultiply(
                              elem->mLocalOffset,
                              *reinterpret_cast< const CVector3f* >(scaleBase + stride * parentId));

#endif
      rotations += stride;
      translations += stride;
      scales += stride;
      ++parent;
#if RSTL_CORRUPTION
      ++referenceOffset;
#endif
      ++unscaled;
      ++scale;
      ++elem;
    }
  } else {
#if RSTL_CORRUPTION
    const CSegId* parent = layout.GetLinearParents().data();
    CElementType* elem = mElements.data();
#else
    rstl::vector< CElementType >::iterator elem = mElements.begin();
    rstl::vector< CSegId >::const_iterator parent = layout.GetLinearParents().begin();

#endif
    elem->mRotation = CMatrix3f::Identity();
    elem->mOffset = CVector3f::Zero();
    rotations += stride;
    translations += stride;
    ++parent;
    ++elem;
#if RSTL_CORRUPTION
    ++referenceOffset;
    long count = mElements.size();
#else

    int count = mElements.size();
#endif
    for (int i = 1; i < count; ++i) {
      const CElementType& parentElem = mElements[parent->val()];
      elem->mRotation = parentElem.mRotation *
                        reinterpret_cast< const CQuaternion* >(rotations)->BuildTransform();
      elem->mLocalOffset = *reinterpret_cast< const CVector3f* >(translations);
      elem->mOffset = parentElem.mOffset + parentElem.mRotation * elem->mLocalOffset;
#if RSTL_CORRUPTION
      elem->mSkinningOffset = elem->mOffset - elem->mRotation * *referenceOffset;
#endif
      rotations += stride;
      translations += stride;
      ++parent;
      ++elem;
#if RSTL_CORRUPTION
      ++referenceOffset;
#endif
    }
  }
}
#if RSTL_CORRUPTION

void CPoseAsTransforms_Linear::SetRotationAndUpdateChildOffsets(
    const CCharLayoutInfo& layout, const CSegId& seg, const CMatrix3f& rotation) {
  int id = seg.val();
  x40_24_ = false;
  CElementType& elem = mElements[id];
  if (mScales.size() == 0) {
    elem.mRotation = rotation;
  } else {
    const CVector3f& scale = mScales[id];
    elem.mRotation = rotation * CMatrix3f::Scale(scale.GetX(), scale.GetY(), scale.GetZ());
    mUnscaledRotations[id] = rotation;
  }

  if (layout.GetSegmentData(CSegId(seg)).GetNumConnectedParts() >= 2) {
    rstl::vector< CSegId >::const_iterator it =
        layout.GetSegmentData(seg).GetConnectedParts().begin();
    rstl::vector< CSegId >::const_iterator end = ConnectedPartsEnd(layout, seg);
    ++it;
    bool hasScale = mScales.size() != 0;
    const CVector3f& scale = hasScale ? mScales[id] : CVector3f::One();
    const CMatrix3f& parentRotation = hasScale ? mUnscaledRotations[id] : elem.mRotation;
    for (; it != end; ++it) {
      CElementType& child = mElements[it->val()];
      child.mOffset = elem.mOffset + parentRotation * (child.mLocalOffset * scale);
    }
  }
}

void CPoseAsTransforms_Linear::UpdateVisibility(const CVisibilityAnimationDataReader& data) {
  uint count = data.GetBitFlagsArraySize();
  mVisibility.resize_uninitialized(count);
  for (uchar i = 0; i < count; ++i) {
    mVisibility[i] = data.GetVisibilityBitFlags(i);
  }
}

void CPoseAsTransforms_Linear::ClearVisibility() { mVisibility.clear(); }
#endif

void CPoseAsTransforms_Linear::SetRotation(const CCharLayoutInfo& layout, const CSegId& seg,
                                           const CMatrix3f& rotation) {
  x40_24_ = false;
  CMatrix3f delta = rotation * GetRotation(seg).GetTranspose();
  RotateHierarchy(layout, seg, delta, 1);
}

void CPoseAsTransforms_Linear::RotateHierarchy(const CCharLayoutInfo& layout, const CSegId& seg,
                                               const CMatrix3f& rotation, int order) {
  int id = seg.val();
  x40_24_ = false;
  CElementType& elem = mElements[id];
  if (mScales.size() == 0) {
    if (order == 1) {
      elem.mRotation = rotation * elem.mRotation;
    } else {
      elem.mRotation = elem.mRotation * rotation;
    }
    elem.mRotation = elem.mRotation.Orthonormalized();
  } else {
    CMatrix3f unscaled(CMatrix3f::Identity());
    if (order == 1) {
      unscaled = rotation * elem.mRotation;
    } else {
      unscaled = elem.mRotation * rotation;
    }
    const CVector3f& scale = mScales[id];
    elem.mRotation = unscaled * CMatrix3f::Scale(scale.GetX(), scale.GetY(), scale.GetZ());
    mUnscaledRotations[id] = unscaled;
  }

#if RSTL_CORRUPTION
  if (layout.GetSegmentData(seg).GetNumConnectedParts() >= 2) {
#else
  if (layout.GetSegmentData(CSegId(seg)).GetNumConnectedParts() >= 2) {
#endif
    rstl::vector< CSegId >::const_iterator it =
        layout.GetSegmentData(seg).GetConnectedParts().begin();
    rstl::vector< CSegId >::const_iterator end = ConnectedPartsEnd(layout, seg);
    ++it;
    bool hasScale = mScales.size() != 0;
    const CVector3f& scale = hasScale ? mScales[id] : CVector3f::One();
    const CMatrix3f& parentRotation = hasScale ? mUnscaledRotations[id] : elem.mRotation;
    for (; it != end; ++it) {
      CElementType& child = mElements[it->val()];
#if RSTL_CORRUPTION
      child.mOffset = elem.mOffset + parentRotation * (child.mLocalOffset * scale);
#else
      child.mOffset =
          elem.mOffset + parentRotation * CVector3f::ByElementMultiply(child.mLocalOffset, scale);
#endif
      RotateHierarchy(layout, *it, rotation, order);
    }
  }
}

static rstl::vector< CSegId >::const_iterator ConnectedPartsEnd(const CCharLayoutInfo& layout,
                                                                const CSegId& seg) {
#if RSTL_CORRUPTION
  return layout.GetSegmentData(seg).GetConnectedParts().end();
#else
  const CCharLayoutNode& node = layout.GetSegmentData(seg);
  return node.GetConnectedParts().begin() + node.GetNumConnectedParts();
#endif
}

void CPoseAsTransforms_Linear::AllocateScale() {
#if RSTL_CORRUPTION
  mScales.resize(mElements.size(), CVector3f::Zero());
#else
  mScales.resize(mElements.size(), CVector3f::One());
#endif
  mUnscaledRotations.resize(mElements.size(), CMatrix3f::Identity());
}
