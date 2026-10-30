#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CAnimTreeTransition.hpp"

#if RSTL_PRIME1
#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"
#endif

#include "Kyoto/Math/CloseEnough.hpp"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CAnimTreeTransition::CAnimTreeTransition(const bool b1, const rstl::ncrc_ptr< CAnimTreeNode >& a,
#else
uint CAnimTreeTransition::GetLoopPOIHash() {
  static uint hash = CPOINode::GetHashForString("Loop");
  return hash;
}

CAnimTreeTransition::CAnimTreeTransition(const bool characterSpaceBlend,
                                         const rstl::ncrc_ptr< CAnimTreeNode >& a,
#endif

                                         const rstl::ncrc_ptr< CAnimTreeNode >& b,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                         const CCharAnimTime& transDur, const bool runA,
                                         const int flags, const rstl::string& name)
: CAnimTreeTweenBase(b1, a, b, flags, name)
, mTransDur(transDur)
#else
                                         const CCharAnimTime& duration, bool runA, int flags,
                                         const rstl::string& name)
: CAnimTreeTweenBase(characterSpaceBlend, a, b, flags, name)
, mTransDur(duration)
#endif

, mTimeInTrans(0.f)
, mRunA(runA)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mLoopA(a->GetBoolPOIState("Loop"))
, mInitialized(false) {
  CCharAnimMemoryMetrics::AddToTotalSize(19, CCharAnimMemoryMetrics::kASS_Two);
}
#else
, mLoopA(a->GetBoolPOIState(GetLoopPOIHash()))
, mInitialized(false) {}
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CAnimTreeTransition::CAnimTreeTransition(const bool b1, const rstl::ncrc_ptr< CAnimTreeNode >& a,
#else
CAnimTreeTransition::CAnimTreeTransition(const bool characterSpaceBlend,
                                         const rstl::ncrc_ptr< CAnimTreeNode >& a,
#endif

                                         const rstl::ncrc_ptr< CAnimTreeNode >& b,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                         const CCharAnimTime& transDur,
                                         const CCharAnimTime& timeInTrans, const bool runA,
                                         const bool loopA, const int flags,
                                         const rstl::string& name, const bool initialized)
: CAnimTreeTweenBase(b1, a, b, flags, name)
, mTransDur(transDur)
#else
                                         const CCharAnimTime& duration,
                                         const CCharAnimTime& timeInTrans, bool runA, bool loopA,
                                         int flags, const rstl::string& name, bool initialized)
: CAnimTreeTweenBase(characterSpaceBlend, a, b, flags, name)
, mTransDur(duration)
#endif

, mTimeInTrans(timeInTrans)
, mRunA(runA)
, mLoopA(loopA)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mInitialized(initialized) {
  CCharAnimMemoryMetrics::AddToTotalSize(19, CCharAnimMemoryMetrics::kASS_Two);
}
#else
, mInitialized(initialized) {}
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CAnimTreeTransition::~CAnimTreeTransition() {
  CCharAnimMemoryMetrics::SubtractFromTotalSize(19, CCharAnimMemoryMetrics::kASS_Two);
}
#else
CAnimTreeTransition::~CAnimTreeTransition() {}
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

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
rstl::pair< CCharAnimTime, CAdvancementDeltas >
#else
rstl::pair< CCharAnimTime, SAdvancementDeltas >
#endif

CAnimTreeTransition::AdvanceViewForTransitionalPeriod(const CCharAnimTime& time) {
  IncAdvancementDepth();
  CDoubleChildAdvancementResult res = AdvanceViewBothChildren(time, mRunA, mLoopA);
  DecAdvancementDepth();
  const CCharAnimTime& trueAdvancement = res.GetTrueAdvancement();
  if (trueAdvancement.EqualsZero())
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return rstl::pair< CCharAnimTime, CAdvancementDeltas >(
#else
    return rstl::pair< CCharAnimTime, SAdvancementDeltas >(
#endif

        CCharAnimTime::ZeroFlat(),
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        CAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
#else
        SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
#endif

  float oldWeight = GetBlendingWeight();
  mTimeInTrans += trueAdvancement;
  float newWeight = GetBlendingWeight();
  if (ShouldCullTree()) {
    if (newWeight < 0.5f)
      mCullSelector = 1;
    else
      mCullSelector = 2;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const CAdvancementDeltas& leftDeltas = res.GetLeftAdvancementDeltas();
  const CAdvancementDeltas& rightDeltas = res.GetRightAdvancementDeltas();
#else
  const SAdvancementDeltas& leftDeltas = res.GetLeftAdvancementDeltas();
  const SAdvancementDeltas& rightDeltas = res.GetRightAdvancementDeltas();
#endif

  if (GetBlendRoot() & kBlendRoot_Offset)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return rstl::pair< CCharAnimTime, CAdvancementDeltas >(
#else
    return rstl::pair< CCharAnimTime, SAdvancementDeltas >(
#endif

        trueAdvancement,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        CAdvancementDeltas::Interpolate(leftDeltas, rightDeltas, oldWeight, newWeight));
  return rstl::pair< CCharAnimTime, CAdvancementDeltas >(trueAdvancement, rightDeltas);
#else
        SAdvancementDeltas::Interpolate(leftDeltas, rightDeltas, oldWeight, newWeight));
  return rstl::pair< CCharAnimTime, SAdvancementDeltas >(trueAdvancement, rightDeltas);
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CAdvancementResults CAnimTreeTransition::VAdvanceView(const CCharAnimTime& time) {
#else
SAdvancementResults CAnimTreeTransition::VAdvanceView(const CCharAnimTime& time) {
#endif

  if (time.EqualsZero()) {
    IncAdvancementDepth();
    mB->AdvanceView(time);
    if (mRunA)
      mA->AdvanceView(time);
    DecAdvancementDepth();
    if (ShouldCullTree())
      mCullSelector = 1;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return CAdvancementResults(CCharAnimTime::ZeroFlat(),
                               CAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
#else
    return SAdvancementResults(CCharAnimTime::ZeroFlat(),
                               SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
#endif

  }
  if (!mInitialized)
    mInitialized = true;
  if (mTimeInTrans + time < mTransDur) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    rstl::pair< CCharAnimTime, CAdvancementDeltas > res = AdvanceViewForTransitionalPeriod(time);
    return CAdvancementResults(time - res.first, res.second);
#else
    rstl::pair< CCharAnimTime, SAdvancementDeltas > res = AdvanceViewForTransitionalPeriod(time);
    return SAdvancementResults(time - res.first, res.second);
#endif

  }
  CCharAnimTime transTimeRem = mTransDur - mTimeInTrans;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::pair< CCharAnimTime, CAdvancementDeltas > res(
      CCharAnimTime(0.f), CAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
#else
  rstl::pair< CCharAnimTime, SAdvancementDeltas > res(
      CCharAnimTime(0.f), SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
#endif

  if (transTimeRem.GreaterThanZero()) {
    res = AdvanceViewForTransitionalPeriod(transTimeRem);
    if (res.first != transTimeRem)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      return CAdvancementResults(res.first, res.second);
#else
      return SAdvancementResults(res.first, res.second);
#endif

  }
  CCharAnimTime remainder = time - transTimeRem;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return CAdvancementResults(remainder, res.second);
#else
  return SAdvancementResults(remainder, res.second);
#endif

}

rstl::ownership_transfer< IAnimReader > CAnimTreeTransition::VClone() const {
  return rs_new CAnimTreeTransition(CharacterSpaceBlend(), Cast(mA->Clone()), Cast(mB->Clone()),
                                    mTransDur, mTimeInTrans, mRunA, mLoopA, GetBlendRoot(), mName,
                                    mInitialized);
}

float CAnimTreeTransition::VGetBlendingWeight() const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (mTransDur.GreaterThanZero())
#else
  if (mTransDur.GreaterThanZero()) {
#endif

    return (1.f / mTransDur.GetSeconds()) * mTimeInTrans.GetSeconds();
#if !RSTL_PRIME1
  }
#endif

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::rc_ptr< CAnimTreeNode > child = right->GetBestUnblendedChild();
  if (!child)
#else
  rstl::rc_ptr< CAnimTreeNode > best = right->GetBestUnblendedChild();
  if (!best) {
#endif

    return right;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return child;
#else
  }
  return best;
#endif

}

#if !RSTL_PRIME1
const int CAnimTreeTweenBase::kBlendRoot_Offset = 1;
#endif

#if !RSTL_PRIME1
const int CAnimTreeTweenBase::kBlendRoot_Rotation = 2;
#endif
