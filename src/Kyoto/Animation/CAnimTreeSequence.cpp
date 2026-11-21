#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Animation/CAnimTreeSequence.hpp"
#if RSTL_CORRUPTION

#include "Kyoto/Animation/CSegIdMaskMap.hpp"
#endif

#include "Kyoto/Animation/CBoolPOINode.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CSequenceHelper.hpp"

#include "Kyoto/Animation/CSoundPOINode.hpp"
#include "Kyoto/Animation/CTreeUtils.hpp"
#include "Kyoto/Animation/IMetaAnim.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#endif

#if RSTL_PRIME1
template < class T >
static inline T& GetOutputNode(T* list, uint idx) { return list[idx]; }
#endif

template < class T >
#if RSTL_CORRUPTION
uint _getPOIList(const CCharAnimTime& time, T* listOut, uint capacity, uint iterator, const CSegIdMaskMap& mask,
#else
uint _getPOIList(const CCharAnimTime& time, T* listOut, uint capacity, uint iterator, int additive,
#endif
                 rstl::vector< T > stream, const CCharAnimTime& curTime) {
  CCharAnimTime curTimeCopy(curTime);
  uint count = stream.size();
  CCharAnimTime tmpTime = curTime + time;
  uint ret = 0;
  uint it = iterator;
  while (it < count) {
    T node(stream[it]);
    if (node.GetTime() > tmpTime)
      break;
#if RSTL_CORRUPTION
    if ((node.GetUnk2C() & mask.GetMask(node.GetSegId())) && node.GetTime() >= curTimeCopy) {
#else
    if (node.GetTime() >= curTimeCopy) {
#endif
      T adjustedNode = T::CopyNodeMinusStartTime(node, curTimeCopy);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      uint idx = iterator + ret;
      if (idx < capacity) {
        ++ret;
        GetOutputNode(listOut, idx) = adjustedNode;
      }
#else
      if (iterator + ret < capacity) {
        listOut[iterator + ret++] = adjustedNode;
      }
#endif
    }
    ++it;
  }
  return ret;
}

CAnimTreeSequence::CAnimTreeSequence(const rstl::vector< rstl::rc_ptr< IMetaAnim > >& seq,
                                     const CAnimSysContext& animSys, const rstl::string& name)
: CAnimTreeSingleChild(
      seq[0]->GetAnimationTree(animSys, CMetaAnimTreeBuildOrders::NoSpecialOrders()), name)
, mAnimCtx(animSys)
, mSequence(seq)
, mCurIdx(0)
, mFundamentals(CSequenceHelper(seq, animSys).ComputeSequenceFundamentals())
, mCurTime(0.f) {}

CAnimTreeSequence::CAnimTreeSequence(const rstl::ncrc_ptr< CAnimTreeNode >& curNode,
                                     const rstl::vector< rstl::rc_ptr< IMetaAnim > >& metaAnims,
                                     const CAnimSysContext& animSys, int curIdx,
                                     const rstl::string& name,
                                     const CSequenceFundamentals& fundamentals,
                                     const CCharAnimTime& time)
: CAnimTreeSingleChild(curNode, name)
, mAnimCtx(animSys)
, mSequence(metaAnims)
, mCurIdx(curIdx)
, mFundamentals(fundamentals)
#if RSTL_CORRUPTION
, mCurTime(time) {}
#else
, mCurTime(time) {
  mChild->VGetTimeRemaining();
}
#endif

SAdvancementResults CAnimTreeSequence::VAdvanceView(const CCharAnimTime& dt) {

  CCharAnimTime totalDelta(0.f);
  CVector3f posDelta(0.f, 0.f, 0.f);
  CQuaternion rotDelta = CQuaternion::NoRotation();

  rstl::ncrc_ptr< CAnimTreeNode > curChild(mChild);
  int seqSize = mSequence.size();

  if (mCurIdx >= seqSize && curChild->VGetTimeRemaining().EqualsZero()) {
    mFundamentals = CSequenceHelper(mSequence, mAnimCtx).ComputeSequenceFundamentals();
    mCurIdx = 0;
    mCurTime = CCharAnimTime::ZeroFlat();
    rstl::ncrc_ptr< CAnimTreeNode > newChild = CTreeUtils::GetTransitionTree(
        curChild,
        mSequence[mCurIdx]->GetAnimationTree(mAnimCtx, CMetaAnimTreeBuildOrders::NoSpecialOrders()),
#if RSTL_CORRUPTION
        mAnimCtx, true);
#else
        mAnimCtx);
#endif
    mChild = newChild;
    curChild = mChild;
  }

  CCharAnimTime remTime = dt;
  while (remTime.GreaterThanZero() && mCurIdx < seqSize) {
    CCharAnimTime chRem = curChild->VGetTimeRemaining();
    if (chRem.EqualsZero()) {
      ++mCurIdx;
      if (mCurIdx < seqSize) {
        rstl::ncrc_ptr< CAnimTreeNode > newChild = CTreeUtils::GetTransitionTree(
            curChild,
            mSequence[mCurIdx]->GetAnimationTree(mAnimCtx,
                                                       CMetaAnimTreeBuildOrders::NoSpecialOrders()),
#if RSTL_CORRUPTION
            mAnimCtx, true);
#else
            mAnimCtx);
#endif
        mChild = newChild;
      }
    }
    curChild = mChild;
    if (mCurIdx < seqSize) {
      SAdvancementResults res = curChild->VAdvanceView(remTime);

      rstl::optional_object< rstl::ownership_transfer< IAnimReader > > simp =
          curChild->Simplified();
      if (simp) {
        curChild = Cast(*simp);
        mChild = curChild;
        curChild = mChild;
      }
      CCharAnimTime prevRemTime = remTime;
      remTime = res.mRemTime;
      totalDelta += prevRemTime - remTime;
#if RSTL_CORRUPTION
      const CAdvancementDeltas& deltas = res.mDeltas;
#else
      const CAdvancementDeltas& deltas = res.mDeltas;

      posDelta += deltas.mPosDelta;
#endif
      rotDelta *= deltas.mRotDelta;
#if RSTL_CORRUPTION
      const CMatrix3f rotation = deltas.mRotDelta.BuildTransform();
      posDelta += rotation * deltas.mPosDelta;
#endif
    }
  }

  mCurTime += totalDelta;
#if RSTL_CORRUPTION
  const CTransform4f rotation = rotDelta.BuildTransform4f();
  return SAdvancementResults(dt - totalDelta,
                            CAdvancementDeltas(rotation.TransposeRotate(posDelta), rotDelta));
#else
  return SAdvancementResults(dt - totalDelta, CAdvancementDeltas(posDelta, rotDelta));

#endif
}

CCharAnimTime CAnimTreeSequence::VGetTimeRemaining() const {
#if RSTL_CORRUPTION
  CCharAnimTime remaining = mChild->GetTimeRemaining();
  remaining += mFundamentals.GetSteadyStateAnimInfo().GetDurationFromIndex(mCurIdx + 1);
  return remaining;
#else
  if (mCurIdx == mSequence.size() - 1) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return mChild->VGetTimeRemaining();
#else
    return mChild->GetTimeRemaining();
#endif
  }
  CCharAnimTime dur = mFundamentals.GetSteadyStateAnimInfo().GetDuration();
  return dur - mCurTime;
#endif
}

CSteadyStateAnimInfo CAnimTreeSequence::VGetSteadyStateAnimInfo() const {
  return mFundamentals.GetSteadyStateAnimInfo();
}

uint CAnimTreeSequence::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
#if RSTL_CORRUPTION
                                        uint capacity, uint iterator, const CSegIdMaskMap& mask) const {
  return _getPOIList(time, listOut, capacity, iterator, mask,
#else
                                        uint capacity, uint iterator, int additive) const {
  return _getPOIList(time, listOut, capacity, iterator, additive,
#endif
                     mFundamentals.GetBoolPointsOfInterest(), mCurTime);
}

#if !RSTL_CORRUPTION
uint CAnimTreeSequence::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut,
                                         uint capacity, uint iterator, int additive) const {
  return _getPOIList(time, listOut, capacity, iterator, additive,
                     mFundamentals.GetInt32PointsOfInterest(), mCurTime);
}

#endif
uint CAnimTreeSequence::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
#if RSTL_CORRUPTION
                                            uint capacity, uint iterator, const CSegIdMaskMap& mask) const {
  return _getPOIList(time, listOut, capacity, iterator, mask,
#else
                                            uint capacity, uint iterator, int additive) const {
  return _getPOIList(time, listOut, capacity, iterator, additive,
#endif
                     mFundamentals.GetParticlePointsOfInterest(), mCurTime);
}

uint CAnimTreeSequence::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut,
#if RSTL_CORRUPTION
                                         uint capacity, uint iterator, const CSegIdMaskMap& mask) const {
  return _getPOIList(time, listOut, capacity, iterator, mask,
#else
                                         uint capacity, uint iterator, int additive) const {
  return _getPOIList(time, listOut, capacity, iterator, additive,
#endif
                     mFundamentals.GetSoundPointsOfInterest(), mCurTime);
}

rstl::ownership_transfer< IAnimReader > CAnimTreeSequence::VClone() const {
  return rs_new_line(293) CAnimTreeSequence(Cast(mChild->Clone()), mSequence, mAnimCtx, mCurIdx,
                                            mName, mFundamentals, mCurTime);
}

rstl::rc_ptr< CAnimTreeNode > CAnimTreeSequence::VGetBestUnblendedChild() const {
  rstl::rc_ptr< CAnimTreeNode > ch = mChild->GetBestUnblendedChild();
  if (ch)
    return rs_new_line(308) CAnimTreeSequence(Cast(ch->Clone()), mSequence, mAnimCtx, mCurIdx,
                                              mName, mFundamentals, mCurTime);
  return ch;
}

bool CAnimTreeSequence::VSupportsReverseView() const { return false; }

CAnimTreeEffectiveContribution CAnimTreeSequence::VGetContributionOfHighestInfluence() const {
  return mChild->GetContributionOfHighestInfluence();
}

rstl::string CAnimTreeSequence::CreatePrimitiveName(const rstl::vector< rstl::string >& names) {
  return rstl::string_l("");
}
