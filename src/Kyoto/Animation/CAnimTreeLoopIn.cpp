#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif
#include "Kyoto/Animation/CAnimTreeLoopIn.hpp"
#if RSTL_CORRUPTION

#include "Kyoto/Animation/CSegIdMaskMap.hpp"
#endif

#include "Kyoto/Animation/CSequenceHelper.hpp"
#include "Kyoto/Animation/CTreeUtils.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

template < class T >
#if RSTL_CORRUPTION
uint _getPOIList(const CCharAnimTime& time, T* listOut, uint capacity, uint iterator, const CSegIdMaskMap& mask,
#else
uint _getPOIList(const CCharAnimTime& time, T* listOut, uint capacity, uint iterator, int additive,
#endif
                 const rstl::vector< T >& stream, const CCharAnimTime& curTime) {
  uint ret = 0;
  CCharAnimTime curTimeCopy(curTime);
  uint count = stream.size();
  CCharAnimTime tmpTime = curTime + time;
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
      if (iterator + ret < capacity) {
        T adjustedNode = T::CopyNodeMinusStartTime(node, curTimeCopy);
        listOut[iterator + ret] = adjustedNode;
        ++ret;
      }
    }
    ++it;
  }
  return ret;
}

CAnimTreeLoopIn::CAnimTreeLoopIn(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                 const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                 const rstl::ncrc_ptr< CAnimTreeNode >& c,
                                 const CAnimSysContext& animCtx, const rstl::string& name)
#if RSTL_CORRUPTION
: CAnimTreeSingleChild(CTreeUtils::GetTransitionTree(a, c, animCtx, false), name)
#else
: CAnimTreeSingleChild(CTreeUtils::GetTransitionTree(a, c, animCtx), name)
#endif
, mNextAnim(b)
, mDidLoopIn(false)
, mAnimCtx(animCtx)
, mFundamentals(CSequenceHelper(mChild, mNextAnim, animCtx).ComputeSequenceFundamentals())
, mCurTime(0.f) {}

CAnimTreeLoopIn::CAnimTreeLoopIn(const rstl::ncrc_ptr< CAnimTreeNode >& a,
#if RSTL_CORRUPTION
                                 const rstl::ncrc_ptr< CAnimTreeNode >& b, const bool didLoopIn,
                                 const CAnimSysContext& animCtx, const rstl::string& name,
#else
                                 const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                 RSTL_ECHOES_CONST bool didLoopIn, const CAnimSysContext& animCtx,
                                 const rstl::string& name,
#endif
                                 const CSequenceFundamentals& fundamentals,
                                 const CCharAnimTime& time)
: CAnimTreeSingleChild(a, name)
, mNextAnim(b)
, mDidLoopIn(didLoopIn)
, mAnimCtx(animCtx)
, mFundamentals(fundamentals)
, mCurTime(time) {}

SAdvancementResults CAnimTreeLoopIn::VAdvanceView(const CCharAnimTime& dt) {

  rstl::ncrc_ptr< CAnimTreeNode > origChild = mChild;
  SAdvancementResults res = origChild->AdvanceView(dt);

  CCharAnimTime remainder = res.GetRemainder();
  mCurTime += dt - remainder;
  CCharAnimTime remTime = origChild->GetTimeRemaining();
  if ((close_enough(remTime.GetSeconds(), 0.f) ||
       close_enough(dt.GetSeconds() - remainder.GetSeconds(), 0.f)) &&
      !mDidLoopIn) {
    rstl::ncrc_ptr< CAnimTreeNode > newChild =
#if RSTL_CORRUPTION
        CTreeUtils::GetTransitionTree(origChild, mNextAnim, mAnimCtx, false);
#else
        CTreeUtils::GetTransitionTree(origChild, mNextAnim, mAnimCtx);
#endif
    ReplaceChild(newChild);
    mDidLoopIn = true;
  }
  return res;
}

CCharAnimTime CAnimTreeLoopIn::VGetTimeRemaining() const {
  CCharAnimTime duration = mFundamentals.GetSteadyStateAnimInfo().GetDuration();
  return duration - mCurTime;
}

CSteadyStateAnimInfo CAnimTreeLoopIn::VGetSteadyStateAnimInfo() const {
  return mFundamentals.GetSteadyStateAnimInfo();
}

uint CAnimTreeLoopIn::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
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
uint CAnimTreeLoopIn::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut,
                                       uint capacity, uint iterator, int additive) const {
  return _getPOIList(time, listOut, capacity, iterator, additive,
                     mFundamentals.GetInt32PointsOfInterest(), mCurTime);
}

#endif
uint CAnimTreeLoopIn::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
#if RSTL_CORRUPTION
                                          uint capacity, uint iterator, const CSegIdMaskMap& mask) const {
  return _getPOIList(time, listOut, capacity, iterator, mask,
#else
                                          uint capacity, uint iterator, int additive) const {
  return _getPOIList(time, listOut, capacity, iterator, additive,
#endif
                     mFundamentals.GetParticlePointsOfInterest(), mCurTime);
}

uint CAnimTreeLoopIn::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut,
#if RSTL_CORRUPTION
                                       uint capacity, uint iterator, const CSegIdMaskMap& mask) const {
  return _getPOIList(time, listOut, capacity, iterator, mask,
#else
                                       uint capacity, uint iterator, int additive) const {
  return _getPOIList(time, listOut, capacity, iterator, additive,
#endif
                     mFundamentals.GetSoundPointsOfInterest(), mCurTime);
}

#if RSTL_CORRUPTION
const CVisibilityAnimationDataReader* CAnimTreeLoopIn::VGetVisibilityDataAtCurrentTime() const {
  return mChild->GetVisibilityDataAtCurrentTime();
}

const CVisibilityAnimationDataReader*
CAnimTreeLoopIn::VGetVisibilityDataAtTime(const CCharAnimTime& time) const {
  return mChild->GetVisibilityDataAtTime(time);
}

#endif
rstl::ownership_transfer< IAnimReader > CAnimTreeLoopIn::VClone() const {
#if RSTL_CORRUPTION
  return rs_new_line(255) CAnimTreeLoopIn(Cast(mChild->Clone()), Cast(mNextAnim->Clone()),
                                          mDidLoopIn, mAnimCtx, mName, mFundamentals, mCurTime);
#else
  return rs_new CAnimTreeLoopIn(Cast(mChild->Clone()), mNextAnim, mDidLoopIn, mAnimCtx, mName,
                                mFundamentals, mCurTime);
#endif
}

rstl::rc_ptr< CAnimTreeNode > CAnimTreeLoopIn::VGetBestUnblendedChild() const {
  rstl::rc_ptr< CAnimTreeNode > child = mChild->GetBestUnblendedChild();
  if (child)
#if RSTL_CORRUPTION
    return rs_new_line(272) CAnimTreeLoopIn(Cast(child->Clone()), mNextAnim, mDidLoopIn, mAnimCtx,
                                            mName, mFundamentals, mCurTime);
#else
    return rs_new CAnimTreeLoopIn(Cast(child->Clone()), mNextAnim, mDidLoopIn, mAnimCtx, mName,
                                  mFundamentals, mCurTime);
#endif
  return child;
}

rstl::optional_object< rstl::ownership_transfer< IAnimReader > > CAnimTreeLoopIn::VSimplified() {
  CCharAnimTime remTime = mChild->GetTimeRemaining();
  if (remTime.GreaterThanZero() && !close_enough(remTime.GetSeconds(), 0.f)) {
    rstl::ncrc_ptr< CAnimTreeNode > child = mChild;
    rstl::optional_object< rstl::ownership_transfer< IAnimReader > > simp = child->Simplified();
    if (simp)
      ReplaceChild(Cast(*simp));
#if RSTL_CORRUPTION
  } else if (mDidLoopIn && mChild->GetTimeRemaining().EqualsZero()) {
    return mChild->Clone();
  }
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  } else if (mDidLoopIn && mChild->GetTimeRemaining().EqualsZero()) {
    return mChild->Clone();
  }
#else
  } else if (mDidLoopIn) {
    if (mChild->GetTimeRemaining().EqualsZero()) {
      return mChild->Clone();
    }
  }
#endif
#endif
  return rstl::optional_object_null();
}

bool CAnimTreeLoopIn::VSupportsReverseView() const { return false; }

CAnimTreeEffectiveContribution CAnimTreeLoopIn::VGetContributionOfHighestInfluence() const {
  return mChild->GetContributionOfHighestInfluence();
}

rstl::string CAnimTreeLoopIn::CreatePrimitiveName(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                  const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                                  const rstl::ncrc_ptr< CAnimTreeNode >& c) {
  return rstl::string_l("");
}
