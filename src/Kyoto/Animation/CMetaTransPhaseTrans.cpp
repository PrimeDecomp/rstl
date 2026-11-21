#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Animation/CMetaTransPhaseTrans.hpp"
#include "Kyoto/Animation/CAnimTreeTimeScale.hpp"
#include "Kyoto/Animation/CAnimTreeTransition.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"

CMetaTransPhaseTrans::CMetaTransPhaseTrans(CInputStream& in)
: mTransDur(CCharAnimTime(in))
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, xc_(in.ReadBool())
#else
, mCharacterSpaceBlend(in.ReadBool())
#endif
, mRunA(in.ReadBool())
#if RSTL_CORRUPTION
, mFlags(in.ReadInt32()) {}
#else
, mFlags(
      in.ReadLong()
      ) {}

#if RSTL_PRIME1
CAnimTreeTimeScale::CAnimTreeTimeScale(
    const rstl::ncrc_ptr< CAnimTreeNode >& node,
    const rstl::ownership_transfer< IVaryingAnimationTimeScale >& timeScale,
    const CCharAnimTime& time, const rstl::string& name)
: CAnimTreeSingleChild(node, name)
, mTimeScale(timeScale)
, mCurAccelTime(0.f)
, mTargetAccelTime(time)
, mInitialTime(node->GetSteadyStateAnimInfo().GetDuration() - node->GetTimeRemaining()) {}
#endif
#endif

rstl::ncrc_ptr< CAnimTreeNode >
CMetaTransPhaseTrans::VGetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                         const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                         const CAnimSysContext& animSys) const {
  CSteadyStateAnimInfo infoA = a->GetContributionOfHighestInfluence().GetSteadyStateAnimInfo();
  CCharAnimTime durationA = infoA.GetDuration();
  CSteadyStateAnimInfo infoB = b->GetContributionOfHighestInfluence().GetSteadyStateAnimInfo();
  CCharAnimTime durationB = infoB.GetDuration();
  float scaleA = durationA / durationB;
  float scaleB = durationB / durationA;
  CAnimTreeEffectiveContribution contribution = a->GetContributionOfHighestInfluence();
  rstl::optional_object< float > phase = contribution.GetPhase();
#if RSTL_CORRUPTION
  b->VSetPhase(*phase);
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  b->SetPhase(*phase);
#else
  b->VSetPhase(*phase);
#endif
  rstl::ownership_transfer< IVaryingAnimationTimeScale > timeScaleA =
      rs_new CLinearAnimationTimeScale(CCharAnimTime::ZeroFlat(), 1.f, mTransDur, scaleA);
  rstl::ownership_transfer< IVaryingAnimationTimeScale > timeScaleB =
      rs_new CLinearAnimationTimeScale(CCharAnimTime::ZeroFlat(), scaleB, mTransDur, 1.f);
  rstl::ncrc_ptr< CAnimTreeNode > treeA = rs_new CAnimTreeTimeScale(
      a, timeScaleA, mTransDur, CAnimTreeTimeScale::CreatePrimitiveName(a, 1.f, mTransDur, scaleA));
  rstl::ncrc_ptr< CAnimTreeNode > treeB = rs_new CAnimTreeTimeScale(
      b, timeScaleB, mTransDur, CAnimTreeTimeScale::CreatePrimitiveName(b, scaleB, mTransDur, 1.f));
  return rs_new CAnimTreeTransition(
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_CORRUPTION
  rstl::ownership_transfer< IVaryingAnimationTimeScale > timeScaleA =
      rs_new_line(87) CLinearAnimationTimeScale(CCharAnimTime::ZeroFlat(), 1.f, mTransDur, scaleA);
  rstl::ownership_transfer< IVaryingAnimationTimeScale > timeScaleB =
      rs_new_line(92) CLinearAnimationTimeScale(CCharAnimTime::ZeroFlat(), scaleB, mTransDur, 1.f);
  rstl::ncrc_ptr< CAnimTreeNode > treeA = rs_new_line(97) CAnimTreeTimeScale(
      a, timeScaleA, mTransDur, CAnimTreeTimeScale::CreatePrimitiveName(a, 1.f, mTransDur, scaleA));
  rstl::ncrc_ptr< CAnimTreeNode > treeB = rs_new_line(105) CAnimTreeTimeScale(
      b, timeScaleB, mTransDur, CAnimTreeTimeScale::CreatePrimitiveName(b, scaleB, mTransDur, 1.f));
  return rs_new_line(114) CAnimTreeTransition(
      mCharacterSpaceBlend, treeA, treeB, mTransDur, 0, mRunA, mFlags,
#else
#if RSTL_PRIME1
      xc_,
#else
      mCharacterSpaceBlend,
#endif
      treeA, treeB, mTransDur, mRunA, mFlags,
#endif
      CAnimTreeTransition::CreatePrimitiveName(treeA, treeB, mTransDur.GetSeconds()));
}

void CMetaTransPhaseTrans::WriteTransData(COutputStream& out) const {
  mTransDur.PutTo(out);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  out.WriteChar(bool(xc_));
#else
  out.WriteChar(bool(mCharacterSpaceBlend));
#endif
  out.WriteChar(bool(mRunA));
  out.WriteLong(mFlags);
}
