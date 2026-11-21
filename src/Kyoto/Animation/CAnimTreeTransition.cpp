#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Animation/CAnimTreeTransition.hpp"
#if !RSTL_CORRUPTION

#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"
#endif

#include "Kyoto/Math/CloseEnough.hpp"

#if RSTL_CORRUPTION


uint CAnimTreeTransition::GetLoopPOIHash() {
  static uint hash = CPOINode::GetHashForString("Loop");
  return hash;
#else
#if !RSTL_PRIME1
uint CAnimTreeTransition::GetLoopPOIHash() {
  static uint hash = CPOINode::GetHashForString("Loop");
  return hash;
}
#endif

CAnimTreeTransition::CAnimTreeTransition(const bool characterSpaceBlend,
                                         const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                         const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                         const CCharAnimTime& duration,
                                         RSTL_PRIME1_CONST bool runA, RSTL_PRIME1_CONST int flags,
                                         const rstl::string& name)
: CAnimTreeTweenBase(characterSpaceBlend, a, b, flags, name)
, mTransDur(duration)
, mTimeInTrans(0.f)
, mRunA(runA)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mLoopA(a->GetBoolPOIState("Loop"))
#else
, mLoopA(a->GetBoolPOIState(GetLoopPOIHash()))
#endif
, mInitialized(false) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CCharAnimMemoryMetrics::AddToTotalSize(19, CCharAnimMemoryMetrics::kASS_Two);
#endif
#endif
}

CAnimTreeTransition::CAnimTreeTransition(const bool characterSpaceBlend,
                                         const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                         const rstl::ncrc_ptr< CAnimTreeNode >& b,
#if RSTL_CORRUPTION
                                         const CCharAnimTime& duration, int blendCurve, bool runA,
                                         int flags, const rstl::string& name)
: CAnimTreeTweenBase(characterSpaceBlend, a, b, flags, name)
, mTransDur(duration)
, mTimeInTrans(0.f)
, mRunA(runA)
, mLoopA(a->GetBoolPOIState(GetLoopPOIHash()))
, mInitialized(false)
, mBlendCurve(blendCurve) {}

CAnimTreeTransition::CAnimTreeTransition(const bool characterSpaceBlend,
                                         const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                         const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                         const CCharAnimTime& duration, int blendCurve,
                                         const CCharAnimTime& timeInTrans, const bool runA, bool loopA,
                                         int flags, const rstl::string& name, bool initialized)
#else
                                         const CCharAnimTime& duration,
                                         const CCharAnimTime& timeInTrans,
                                         RSTL_PRIME1_CONST bool runA, RSTL_PRIME1_CONST bool loopA,
                                         RSTL_PRIME1_CONST int flags, const rstl::string& name,
                                         RSTL_PRIME1_CONST bool initialized)
#endif
: CAnimTreeTweenBase(characterSpaceBlend, a, b, flags, name)
, mTransDur(duration)
, mTimeInTrans(timeInTrans)
, mRunA(runA)
, mLoopA(loopA)
#if RSTL_CORRUPTION
, mInitialized(initialized)
, mBlendCurve(blendCurve) {}
#else
, mInitialized(initialized) {
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_CORRUPTION
CAnimTreeTransition::~CAnimTreeTransition() {}
#else
#if RSTL_PRIME1
  CCharAnimMemoryMetrics::AddToTotalSize(19, CCharAnimMemoryMetrics::kASS_Two);
#endif
}

CAnimTreeTransition::~CAnimTreeTransition() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CCharAnimMemoryMetrics::SubtractFromTotalSize(19, CCharAnimMemoryMetrics::kASS_Two);
#endif
}
#endif

rstl::optional_object< rstl::ownership_transfer< IAnimReader > >
CAnimTreeTransition::VSimplified() {
  if (close_enough(GetBlendingWeight(), 1.f)) {
    rstl::optional_object< rstl::ownership_transfer< IAnimReader > > simp = mB->Simplified();
    if (simp)
      return simp;
    return mB->Clone();
  }
  return CAnimTreeTweenBase::VSimplified();
}

rstl::optional_object< rstl::ownership_transfer< IAnimReader > >
CAnimTreeTransition::VReverseSimplified() {
  if (close_enough(GetBlendingWeight(), 0.f))
    return mA->Clone();
  return CAnimTreeTweenBase::VReverseSimplified();
}

rstl::pair< CCharAnimTime, CAdvancementDeltas >
CAnimTreeTransition::AdvanceViewForTransitionalPeriod(const CCharAnimTime& time) {
  IncAdvancementDepth();
  CDoubleChildAdvancementResult res = AdvanceViewBothChildren(time, mRunA, mLoopA);
  DecAdvancementDepth();
  const CCharAnimTime& trueAdvancement = res.GetTrueAdvancement();
  if (trueAdvancement.EqualsZero())
    return rstl::pair< CCharAnimTime, CAdvancementDeltas >(
        CCharAnimTime::ZeroFlat(),
        CAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));

  float oldWeight = GetBlendingWeight();
  mTimeInTrans += trueAdvancement;
  float newWeight = GetBlendingWeight();
  if (ShouldCullTree()) {
    if (newWeight < 0.5f)
      mCullSelector = 1;
    else
      mCullSelector = 2;
  }
#if !RSTL_CORRUPTION
  const CAdvancementDeltas& leftDeltas = res.GetLeftAdvancementDeltas();
  const CAdvancementDeltas& rightDeltas = res.GetRightAdvancementDeltas();
#endif

#if RSTL_CORRUPTION
  const CAdvancementDeltas& leftDeltas = res.GetLeftAdvancementDeltas();
  const CAdvancementDeltas& rightDeltas = res.GetRightAdvancementDeltas();
  switch (GetBlendRoot() & (kBlendRoot_Offset | kBlendRoot_Rotation)) {
  case kBlendRoot_Offset:
    return rstl::pair< CCharAnimTime, CAdvancementDeltas >(
#else
  if (GetBlendRoot() & kBlendRoot_Offset)
    return rstl::pair< CCharAnimTime, CAdvancementDeltas >(
#endif
        trueAdvancement,
#if RSTL_CORRUPTION
        CAdvancementDeltas::InterpolateTranslationOnly(leftDeltas, rightDeltas, oldWeight, newWeight));
  case kBlendRoot_Rotation:
    return rstl::pair< CCharAnimTime, CAdvancementDeltas >(
        trueAdvancement,
        CAdvancementDeltas::InterpolateRotationOnly(leftDeltas, rightDeltas, oldWeight, newWeight));
  case kBlendRoot_Offset | kBlendRoot_Rotation:
    return rstl::pair< CCharAnimTime, CAdvancementDeltas >(
        trueAdvancement,
        CAdvancementDeltas::Interpolate(leftDeltas, rightDeltas, oldWeight, newWeight));
  default:
    return rstl::pair< CCharAnimTime, CAdvancementDeltas >(trueAdvancement, rightDeltas);
  }
#else
        CAdvancementDeltas::Interpolate(leftDeltas, rightDeltas, oldWeight, newWeight));
  return rstl::pair< CCharAnimTime, CAdvancementDeltas >(trueAdvancement, rightDeltas);

#endif
}

SAdvancementResults CAnimTreeTransition::VAdvanceView(const CCharAnimTime& time) {

  if (time.EqualsZero()) {
    IncAdvancementDepth();
    mB->AdvanceView(time);
    if (mRunA)
      mA->AdvanceView(time);
    DecAdvancementDepth();
    if (ShouldCullTree())
      mCullSelector = 1;
    return SAdvancementResults(CCharAnimTime::ZeroFlat(),
                               CAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));

  }
  if (!mInitialized)
    mInitialized = true;
  if (mTimeInTrans + time < mTransDur) {
    rstl::pair< CCharAnimTime, CAdvancementDeltas > res = AdvanceViewForTransitionalPeriod(time);
    return SAdvancementResults(time - res.first, res.second);

  }

  CCharAnimTime transTimeRem = mTransDur - mTimeInTrans;
  rstl::pair< CCharAnimTime, CAdvancementDeltas > res(
      CCharAnimTime(0.f), CAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
  if (transTimeRem.GreaterThanZero()) {
    res = AdvanceViewForTransitionalPeriod(transTimeRem);
    if (res.first != transTimeRem)
      return SAdvancementResults(res.first, res.second);

  }
  CCharAnimTime remainder = time - transTimeRem;
  return SAdvancementResults(remainder, res.second);
#if RSTL_CORRUPTION
}
#endif

#if RSTL_CORRUPTION
SAdvancementResults
CAnimTreeTransition::VGetAdvancementResults(const CCharAnimTime& time,
                                            const CCharAnimTime& startOffset) const {
  if (GetBlendRoot() && startOffset + time < mTransDur) {
    SAdvancementResults a = mLoopA ? mA->VGetAdvancementResults(time, startOffset)
                                 : SAdvancementResults(
                                       CCharAnimTime::ZeroFlat(),
                                       CAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
    SAdvancementResults b = mB->VGetAdvancementResults(time, startOffset);
    float inverseDuration = 1.f / mTransDur.GetSeconds();
    float startWeight = inverseDuration * startOffset.GetSeconds();
    float endWeight = inverseDuration * (startOffset + time).GetSeconds();
    switch (GetBlendRoot() & (kBlendRoot_Offset | kBlendRoot_Rotation)) {
    case kBlendRoot_Offset:
      return SAdvancementResults(
          b.mRemTime,
          CAdvancementDeltas::InterpolateTranslationOnly(a.mDeltas, b.mDeltas, startWeight, endWeight));
    case kBlendRoot_Rotation:
      return SAdvancementResults(
          b.mRemTime,
          CAdvancementDeltas::InterpolateRotationOnly(a.mDeltas, b.mDeltas, startWeight, endWeight));
    case kBlendRoot_Offset | kBlendRoot_Rotation:
      return SAdvancementResults(
          b.mRemTime,
          CAdvancementDeltas::Interpolate(a.mDeltas, b.mDeltas, startWeight, endWeight));
    }
  }
  return mB->VGetAdvancementResults(time, startOffset);
#endif
}

rstl::ownership_transfer< IAnimReader > CAnimTreeTransition::VClone() const {
#if RSTL_CORRUPTION
  return rs_new_line(339) CAnimTreeTransition(
      CharacterSpaceBlend(), Cast(mA->Clone()), Cast(mB->Clone()), mTransDur, mBlendCurve,
      mTimeInTrans, mRunA, mLoopA, GetBlendRoot(), mName, mInitialized);
#else
  return rs_new CAnimTreeTransition(CharacterSpaceBlend(), Cast(mA->Clone()), Cast(mB->Clone()),
                                    mTransDur, mTimeInTrans, mRunA, mLoopA, GetBlendRoot(), mName,
                                    mInitialized);
#endif
}

float CAnimTreeTransition::VGetBlendingWeight() const {
  if (mTransDur.GreaterThanZero()) {
#if RSTL_CORRUPTION
    float t = 1.f / mTransDur.GetSeconds() * mTimeInTrans.GetSeconds();
    if (mBlendCurve == 1)
      return 0.5f - 0.5f * CMath::FastCosR(3.1415927f * t);
    return t;
#else
    return (1.f / mTransDur.GetSeconds()) * mTimeInTrans.GetSeconds();
#endif
  }
  return 1.f;
}

CCharAnimTime CAnimTreeTransition::VGetTimeRemaining() const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return rstl::max_val(mB->GetTimeRemaining(), mTransDur - mTimeInTrans);
#else
  return rstl::max_val< const CCharAnimTime& >(mB->VGetTimeRemaining(), mTransDur - mTimeInTrans);
#endif
}

CSteadyStateAnimInfo CAnimTreeTransition::VGetSteadyStateAnimInfo() const {
#if RSTL_CORRUPTION
  CSteadyStateAnimInfo info = mB->GetSteadyStateAnimInfo();
  return CSteadyStateAnimInfo(info.IsLooping(),
                              rstl::max_val< const CCharAnimTime& >(mTransDur, info.GetDuration()),
                              info.GetOffset());
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CSteadyStateAnimInfo bInfo = mB->GetSteadyStateAnimInfo();
  return CSteadyStateAnimInfo(bInfo.IsLooping(), rstl::max_val(mTransDur, bInfo.GetDuration()),
                              bInfo.GetOffset());
#else
  CSteadyStateAnimInfo info = mB->VGetSteadyStateAnimInfo();
  return CSteadyStateAnimInfo(info.IsLooping(),
                              rstl::max_val< const CCharAnimTime& >(mTransDur, info.GetDuration()),
                              info.GetOffset());
#endif
#endif
}

rstl::string CAnimTreeTransition::CreatePrimitiveName(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                      const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                                      float duration) {
  return rstl::string_l("");
}

void CAnimTreeTransition::SetBlendingWeight(float weight) {
  rstl::rc_ptr< CAnimTreeNode > right = GetRightChild();
  static_cast< CAnimTreeTweenBase* >(right.GetPtr())->SetBlendingWeight(weight);
}

rstl::rc_ptr< CAnimTreeNode > CAnimTreeTransition::VGetBestUnblendedChild() const {
  rstl::rc_ptr< CAnimTreeNode > right = GetRightChild();
  rstl::rc_ptr< CAnimTreeNode > best = right->GetBestUnblendedChild();
  if (!best)
    return right;
  return best;
}

#if RSTL_CORRUPTION
uint CAnimTreeTransition::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
                                          uint capacity, uint iterator, const CSegIdMaskMap& mask) const {
  if (mRunA || mLoopA) {
    return CAnimTreeDoubleChild::VGetBoolPOIList(time, listOut, capacity, iterator, mask);
  }
  return mB->GetBoolPOIList(time, listOut, capacity, iterator, mask);
}

uint CAnimTreeTransition::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
                                              uint capacity, uint iterator, const CSegIdMaskMap& mask) const {
  if (mRunA || mLoopA) {
    return CAnimTreeDoubleChild::VGetParticlePOIList(time, listOut, capacity, iterator, mask);
  }
  return mB->GetParticlePOIList(time, listOut, capacity, iterator, mask);
}

uint CAnimTreeTransition::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut,
                                           uint capacity, uint iterator, const CSegIdMaskMap& mask) const {
  if (mRunA || mLoopA) {
    return CAnimTreeDoubleChild::VGetSoundPOIList(time, listOut, capacity, iterator, mask);
  }
  return mB->GetSoundPOIList(time, listOut, capacity, iterator, mask);
}

uint CAnimTreeTransition::VGetAssetSoundPOIList(const CCharAnimTime& time,
                                                CAssetSoundPOINode* listOut, uint capacity,
                                                uint iterator, const CSegIdMaskMap& mask) const {
  if (mRunA || mLoopA) {
    return CAnimTreeDoubleChild::VGetAssetSoundPOIList(time, listOut, capacity, iterator, mask);
  }
  return mB->GetAssetSoundPOIList(time, listOut, capacity, iterator, mask);
}

uint CAnimTreeTransition::VGetAssetPOIList(const CCharAnimTime& time, CAssetPOINode* listOut,
                                           uint capacity, uint iterator, const CSegIdMaskMap& mask) const {
  if (mRunA || mLoopA) {
    return CAnimTreeDoubleChild::VGetAssetPOIList(time, listOut, capacity, iterator, mask);
  }
  return mB->GetAssetPOIList(time, listOut, capacity, iterator, mask);
}
#else
#if !RSTL_PRIME1
const int CAnimTreeTweenBase::kBlendRoot_Offset = 1;
const int CAnimTreeTweenBase::kBlendRoot_Rotation = 2;
#endif
#endif
