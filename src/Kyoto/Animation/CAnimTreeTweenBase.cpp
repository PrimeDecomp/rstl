#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CAnimTreeTweenBase.hpp"
// TODO: verify that this Prime/Echoes difference is real
#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"
#include "Kyoto/Animation/CAnimMathUtils.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
#include "Kyoto/Animation/CJointData_LinearStorage.hpp"
#endif

#include "Kyoto/Animation/CSegIdList.hpp"
#include "Kyoto/Animation/CSegStatementSet.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/optional_object.hpp"

#if RSTL_PRIME1
const int CAnimTreeTweenBase::kBlendRoot_Offset = 1;
const int CAnimTreeTweenBase::kBlendRoot_Rotation = 2;
#endif

s32 CAnimTreeTweenBase::sAdvancementDepth = 0;

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CAnimTreeTweenBase::CAnimTreeTweenBase(bool b1, const rstl::ncrc_ptr< CAnimTreeNode >& a,
#else
CAnimTreeTweenBase::CAnimTreeTweenBase(bool characterSpaceBlend,
                                       const rstl::ncrc_ptr< CAnimTreeNode >& a,
#endif

                                       const rstl::ncrc_ptr< CAnimTreeNode >& b, int flags,
                                       const rstl::string& name)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
: CAnimTreeDoubleChild(a, b, name), mFlags(flags), mCharacterSpaceBlend(b1), mCullSelector(0) {
  CCharAnimMemoryMetrics::AddToTotalSize(8, CCharAnimMemoryMetrics::kASS_Two);
}
#else
: CAnimTreeDoubleChild(a, b, name)
, mFlags(flags)
, mCharacterSpaceBlend(characterSpaceBlend)
, mCullSelector(0) {}
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CAnimTreeTweenBase::~CAnimTreeTweenBase() {
  CCharAnimMemoryMetrics::SubtractFromTotalSize(8, CCharAnimMemoryMetrics::kASS_Two);
}
#else
CAnimTreeTweenBase::~CAnimTreeTweenBase() {}
#endif

bool CAnimTreeTweenBase::VHasOffset(const CSegId& seg) const {
  return mA->VHasOffset(seg) && mB->VHasOffset(seg);
}

CVector3f CAnimTreeTweenBase::VGetOffset(const CSegId& seg) const {
  float blend_weight = GetBlendingWeight();
  if (blend_weight >= 1.0) {
    return mB->VGetOffset(seg);
  } else {
    CVector3f start_offset = mA->VGetOffset(seg);
    CVector3f end_offset = mB->VGetOffset(seg);
    return start_offset.Lerp(start_offset, end_offset, blend_weight);
  }
}

CQuaternion CAnimTreeTweenBase::VGetRotation(const CSegId& seg) const {
  float blend_weight = GetBlendingWeight();
  if (blend_weight >= 1.0) {
    return mB->VGetRotation(seg);
  } else {
    CQuaternion start_offset = mA->VGetRotation(seg);
    CQuaternion end_offset = mB->VGetRotation(seg);
    return CAnimMathUtils::SlerpLocal(start_offset, end_offset, blend_weight);
  }
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CAnimTreeTweenBase::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const {
#else
static void GetSegStatementSet(const rstl::rc_ptr< CAnimTreeNode >& child, const CSegIdList& list,
                               CSegStatementSet& setOut,
                               rstl::optional_object< CCharAnimTime > time) {
  if (time.valid()) {
    child->VGetSegStatementSet(list, setOut, *time);
  } else {
    child->VGetSegStatementSet(list, setOut);
  }
}

// Guessed name.
void CAnimTreeTweenBase::BlendSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                                              rstl::optional_object< CCharAnimTime > time) const {
#endif

  float weight = GetBlendingWeight();
  static int sStack = 0;
  ++sStack;
  if (weight >= 1.0) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    mB->GetSegStatementSet(list, setOut);
#else
    GetSegStatementSet(mB, list, setOut, time);
#endif

  } else if (sStack > 3) {
    const rstl::ncrc_ptr< CAnimTreeNode >& child = weight > 0.5f ? mB : mA;
    rstl::rc_ptr< CAnimTreeNode > best = child->GetBestUnblendedChild();
    if (!best)
      best = child;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    best->GetSegStatementSet(list, setOut);
#else
    GetSegStatementSet(best, list, setOut, time);
#endif

  } else {
    CStackSegStatementSet setA;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    mA->GetSegStatementSet(list, setA);
#else
    GetSegStatementSet(mA, list, setA, time);
#endif

    CStackSegStatementSet setB;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    mB->GetSegStatementSet(list, setB);
#else
    GetSegStatementSet(mB, list, setB, time);
#endif

    int count = list.Size();
    for (int i = 0; i < count; ++i) {
      const CSegId& id = list[i];
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      const CQuaternion& rotationA = setA.GetData(id).Orientation();
      if (weight < CAnimMathUtils::kInterpolationThreshold) {
        setOut.Set(id, rotationA);
        if (setA.GetData(id).OffsetValid())
          setOut.Set(id, setA.GetData(id).Offset());
      } else {
        setOut.Set(id, CAnimMathUtils::SlerpLocal(rotationA, setB.GetData(id).Orientation(), weight));
        if (setA.GetData(id).OffsetValid() && setB.GetData(id).OffsetValid())
          setOut.Set(id, CVector3f::Lerp(setA.GetData(id).Offset(), setB.GetData(id).Offset(), weight));
#else
      const CQuaternion& rotationA = setA[id].Orientation();
      setOut[id].Set(CAnimMathUtils::SlerpLocal(rotationA, setB[id].Orientation(), weight));
      if (setA[id].OffsetValid() && setB[id].OffsetValid()) {
        const CVector3f& offsetA = setA[id].Offset();
        setOut[id].Set(CVector3f::Lerp(offsetA, setB[id].Offset(), weight));
      }
      if (setA[id].ScaleValid() || setB[id].ScaleValid()) {
        const CVector3f scaleA = setA[id].ScaleValid() ? setA[id].Scale() : CVector3f::One();
        const CVector3f scaleB = setB[id].ScaleValid() ? setB[id].Scale() : CVector3f::One();
        setOut[id].SetScale(CVector3f::Lerp(scaleA, scaleB, weight));
#endif

      }
    }
  }
  --sStack;
}

#if !RSTL_PRIME1
void CAnimTreeTweenBase::VGetSegStatementSet(const CSegIdList& list,
                                             CSegStatementSet& setOut) const {
  BlendSegStatementSet(list, setOut, rstl::optional_object_null());
}
#endif

void CAnimTreeTweenBase::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                                          const CCharAnimTime& time) const {
#if !RSTL_PRIME1
  BlendSegStatementSet(list, setOut, time);
}

static void GetSegData(const rstl::rc_ptr< CAnimTreeNode >& child, const CCharLayoutInfo& layout,
                       CJointData_LinearStorage& data,
                       rstl::optional_object< CCharAnimTime > time) {
  if (time.valid()) {
    child->VGetJointData_Linear(layout, data, *time);
  } else {
    child->VGetJointData_Linear(layout, data);
  }
}

// Guessed name.
void CAnimTreeTweenBase::BlendSegData(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                                      rstl::optional_object< CCharAnimTime > time) const {
#endif

  float weight = GetBlendingWeight();
  static int sStack = 0;
  ++sStack;
  if (weight >= 1.0) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    mB->GetSegStatementSet(list, setOut, time);
#else
    GetSegData(mB, layout, data, time);
#endif

  } else if (sStack > 3) {
    const rstl::ncrc_ptr< CAnimTreeNode >& child = weight > 0.5f ? mB : mA;
    rstl::rc_ptr< CAnimTreeNode > best = child->GetBestUnblendedChild();
    if (!best)
      best = child;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    best->GetSegStatementSet(list, setOut, time);
#else
    GetSegData(best, layout, data, time);
#endif

  } else {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CStackSegStatementSet setA;
    mA->GetSegStatementSet(list, setA, time);
    CStackSegStatementSet setB;
    mB->GetSegStatementSet(list, setB, time);
    int count = list.Size();
    for (int i = 0; i < count; ++i) {
      const CSegId& id = list[i];
      const CQuaternion& rotationA = setA.GetData(id).Orientation();
      setOut.Set(id, CAnimMathUtils::SlerpLocal(rotationA, setB.GetData(id).Orientation(), weight));
      if (setA.GetData(id).OffsetValid() && setB.GetData(id).OffsetValid())
        setOut.Set(id, CVector3f::Lerp(setA.GetData(id).Offset(), setB.GetData(id).Offset(), weight));
    }
#else
    GetSegData(mA, layout, data, time);
    CJointData_LinearStorage dataB(layout.GetNumSegments(), CJointData_LinearStorage::kAF_Pool);
    if (data.HasScales())
      dataB.SetHasScales(true);
    GetSegData(mB, layout, dataB, time);
    data.Blend(dataB, weight);
#endif

  }
  --sStack;
#if !RSTL_PRIME1
}

void CAnimTreeTweenBase::VGetJointData_Linear(const CCharLayoutInfo& layout,
                                              CJointData_LinearStorage& data,
                                              const CCharAnimTime& time) const {
  BlendSegData(layout, data, time);
}

void CAnimTreeTweenBase::VGetJointData_Linear(const CCharLayoutInfo& layout,
                                              CJointData_LinearStorage& data) const {
  BlendSegData(layout, data, rstl::optional_object_null());
#endif

}

float CAnimTreeTweenBase::VGetRightChildWeight() const { return GetBlendingWeight(); }

float CAnimTreeTweenBase::GetBlendingWeight() const { return VGetBlendingWeight(); }

bool CAnimTreeTweenBase::ShouldCullTree() { return sAdvancementDepth >= 3; }

rstl::optional_object< rstl::ownership_transfer< IAnimReader > > CAnimTreeTweenBase::VSimplified() {
  if (mCullSelector == 0) {
    rstl::optional_object< rstl::ownership_transfer< IAnimReader > > a = mA->Simplified();
    rstl::optional_object< rstl::ownership_transfer< IAnimReader > > b = mB->Simplified();
    const bool simplifyA = a.valid();
    const bool simplifyB = b.valid();
    if (!simplifyA && !simplifyB)
      return rstl::optional_object_null();
    CAnimTreeTweenBase* clone = static_cast< CAnimTreeTweenBase* >(Clone().take_ownership());
    if (simplifyA)
      clone->ReplaceLeftChild(static_cast< CAnimTreeNode* >(a->take_ownership()));
    if (simplifyB)
      clone->ReplaceRightChild(static_cast< CAnimTreeNode* >(b->take_ownership()));
    return rstl::ownership_transfer< IAnimReader >(clone);
  } else {
    const rstl::ncrc_ptr< CAnimTreeNode >& child = mCullSelector == 1 ? mB : mA;
    rstl::rc_ptr< CAnimTreeNode > best = child->GetBestUnblendedChild();
    if (!best)
      return child->Clone();
    else
      return best->Clone();
  }
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
rstl::optional_object< rstl::ownership_transfer< IAnimReader > > CAnimTreeTweenBase::VReverseSimplified() {
#else
rstl::optional_object< rstl::ownership_transfer< IAnimReader > >
CAnimTreeTweenBase::VReverseSimplified() {
#endif

  return CAnimTreeTweenBase::VSimplified();
}

void CAnimTreeTweenBase::VGetWeightedReaders(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    float w, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const {
  float blend_weight = GetBlendingWeight();
  mA->VGetWeightedReaders(w * (1.0f - blend_weight), out);
  mB->VGetWeightedReaders(w * blend_weight, out);
#else
    float weight, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const {
  const float blendWeight = GetBlendingWeight();
  mA->VGetWeightedReaders(weight * (1.f - blendWeight), out);
  mB->VGetWeightedReaders(weight * blendWeight, out);
#endif

}

