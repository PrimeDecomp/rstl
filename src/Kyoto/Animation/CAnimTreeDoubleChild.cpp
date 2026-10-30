#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CAnimTreeDoubleChild.hpp"
#include "Kyoto/Animation/CAnimTreeNode.hpp"

#include "Kyoto/Animation/CBoolPOINode.hpp"
#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"

#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CPOINode.hpp"

#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"
#include "Kyoto/Animation/IAnimReader.hpp"

#include "Kyoto/Math/CloseEnough.hpp"
#include "stdlib.h"

CAnimTreeDoubleChild::CAnimTreeDoubleChild(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                           const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                           const rstl::string& name)
: CAnimTreeNode(name)
, mA(a)
, mB(b) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CCharAnimMemoryMetrics::AddToTotalSize(8, CCharAnimMemoryMetrics::kASS_Two);
#endif
}

SAdvancementResults CAnimTreeDoubleChild::VAdvanceView(const CCharAnimTime& time) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  SAdvancementResults resA = mA->AdvanceView(time);
  SAdvancementResults resB = mB->AdvanceView(time);
  return resA.GetRemainder() > resB.GetRemainder() ? resA : resB;
#else
  SAdvancementResults resA = mA->VAdvanceView(time);
  SAdvancementResults resB = mB->VAdvanceView(time);
  return resA.mRemTime > resB.mRemTime ? resA : resB;
#endif
}

CAnimTreeDoubleChild::~CAnimTreeDoubleChild() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CCharAnimMemoryMetrics::SubtractFromTotalSize(8, CCharAnimMemoryMetrics::kASS_Two);
#endif
}

uint CAnimTreeDoubleChild::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
                                           uint capacity, uint iterator, int additive) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int
#else
  uint
#endif
      count = mA->GetBoolPOIList(time, listOut, capacity, iterator, additive);
  count += mB->GetBoolPOIList(time, listOut, capacity, count + iterator, additive);
  if (count > capacity) {
    count = capacity;
  }
  qsort(listOut, count, sizeof(CBoolPOINode), CPOINode::compare);
  return count;
}

uint CAnimTreeDoubleChild::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut,
                                            uint capacity, uint iterator, int additive) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int
#else
  uint
#endif
      count = mA->GetInt32POIList(time, listOut, capacity, iterator, additive);
  count += mB->GetInt32POIList(time, listOut, capacity, count + iterator, additive);
  if (count > capacity) {
    count = capacity;
  }
  qsort(listOut, count, sizeof(CInt32POINode), CPOINode::compare);
  return count;
}

uint CAnimTreeDoubleChild::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
                                               uint capacity, uint iterator, int additive) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int
#else
  uint
#endif
      count = mA->GetParticlePOIList(time, listOut, capacity, iterator, additive);
  count += mB->GetParticlePOIList(time, listOut, capacity, count + iterator, additive);
  if (count > capacity) {
    count = capacity;
  }
  qsort(listOut, count, sizeof(CParticlePOINode), CPOINode::compare);
  return count;
}

uint CAnimTreeDoubleChild::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut,
                                            uint capacity, uint iterator, int additive) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int
#else
  uint
#endif
      count = mA->GetSoundPOIList(time, listOut, capacity, iterator, additive);
  count += mB->GetSoundPOIList(time, listOut, capacity, count + iterator, additive);
  if (count > capacity) {
    count = capacity;
  }
  qsort(listOut, count, sizeof(CSoundPOINode), CPOINode::compare);
  return count;
}

bool CAnimTreeDoubleChild::VGetBoolPOIState(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const char* name
#else
    uint name
#endif
    ) const {
  return mB->VGetBoolPOIState(name);
}

s32 CAnimTreeDoubleChild::VGetInt32POIState(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const char* name
#else
    uint name
#endif
    ) const {
  return mB->VGetInt32POIState(name);
}

CParticleData::EParentedMode CAnimTreeDoubleChild::VGetParticlePOIState(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const char* name
#else
    uint name
#endif
    ) const {
  return mB->VGetParticlePOIState(name);
}

CAnimTreeEffectiveContribution CAnimTreeDoubleChild::VGetContributionOfHighestInfluence() const {
  CAnimTreeEffectiveContribution a = mA->GetContributionOfHighestInfluence();
  CAnimTreeEffectiveContribution b = mB->GetContributionOfHighestInfluence();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  float leftWeight = a.GetContributionWeight() * GetLeftChildWeight();
  float rightWeight = b.GetContributionWeight() * GetRightChildWeight();
#else
  const float leftWeight = a.GetContributionWeight() * GetLeftChildWeight();
  const float rightWeight = b.GetContributionWeight() * GetRightChildWeight();
#endif
  return leftWeight > rightWeight
             ? CAnimTreeEffectiveContribution(leftWeight, a.GetPrimitiveName(),
                                              a.GetSteadyStateAnimInfo(), a.GetTimeRemaining(),
                                              a.GetAnimDatabaseIndex())
             : CAnimTreeEffectiveContribution(rightWeight, b.GetPrimitiveName(),
                                              b.GetSteadyStateAnimInfo(), b.GetTimeRemaining(),
                                              b.GetAnimDatabaseIndex());
}

uint CAnimTreeDoubleChild::VGetNumChildren() const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int num_children = mB->VGetNumChildren();
  num_children += mA->VGetNumChildren() + 2;
  return num_children;
#else
  const uint rightCount = mB->VGetNumChildren();
  return mA->VGetNumChildren() + 2 + rightCount;
#endif
}

CAnimTreeDoubleChild::CDoubleChildAdvancementResult::CDoubleChildAdvancementResult(
    const CCharAnimTime& time, const CAdvancementDeltas& left, const CAdvancementDeltas& right)
: mTrueAdvancement(time)
, mLeftDeltas(left)
, mRightDeltas(right) {}

CAnimTreeDoubleChild::CDoubleChildAdvancementResult
CAnimTreeDoubleChild::AdvanceViewBothChildren(const CCharAnimTime& time, bool runLeft,
                                              bool loopLeft) {
  CCharAnimTime leftRemaining = time;
  CCharAnimTime totalTime = !runLeft   ? CCharAnimTime::ZeroFlat()
                            : loopLeft ? CCharAnimTime::Infinity()
                                       : mA->GetTimeRemaining();
  CVector3f leftOffset(0.f, 0.f, 0.f);
  CQuaternion leftRotation = CQuaternion::NoRotation();
  CCharAnimTime rightRemaining = time;
  CVector3f rightOffset(0.f, 0.f, 0.f);
  CQuaternion rightRotation = CQuaternion::NoRotation();
  if (time.GreaterThanZero()) {
    while (leftRemaining.GreaterThanZero() && !close_enough(leftRemaining.GetSeconds(), 0.f) &&
           totalTime.GreaterThanZero() &&
           (loopLeft || !close_enough(totalTime.GetSeconds(), 0.f))) {
      SAdvancementResults result = mA->AdvanceView(leftRemaining);

      rstl::optional_object< rstl::ownership_transfer< IAnimReader > > simplified =
          mA->Simplified();
      if (simplified.valid())
        mA = Cast(*simplified);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      CAdvancementDeltas deltas = result.GetAdvancementDeltas();
#else
      CAdvancementDeltas deltas = result.mDeltas;
#endif
      leftOffset += deltas.GetOffsetDelta();
      CQuaternion rotation = deltas.GetOrientationDelta();
      leftRotation *= rotation;
      if (!loopLeft)
        totalTime = mA->GetTimeRemaining();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      leftRemaining = result.GetRemainder();
#else
      leftRemaining = result.mRemTime;
#endif
    }
    while (rightRemaining.GreaterThanZero() && !close_enough(rightRemaining.GetSeconds(), 0.f)) {
      SAdvancementResults result = mB->AdvanceView(rightRemaining);

      rstl::optional_object< rstl::ownership_transfer< IAnimReader > > simplified =
          mB->Simplified();
      if (simplified.valid())
        mB = Cast(*simplified);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      CAdvancementDeltas deltas = result.GetAdvancementDeltas();
#else
      CAdvancementDeltas deltas = result.mDeltas;
#endif
      rightOffset += deltas.GetOffsetDelta();
      CQuaternion rotation = deltas.GetOrientationDelta();
      rightRotation *= rotation;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      rightRemaining = result.GetRemainder();
#else
      rightRemaining = result.mRemTime;
#endif
    }
  }
  return CDoubleChildAdvancementResult(time, CAdvancementDeltas(leftOffset, leftRotation),
                                       CAdvancementDeltas(rightOffset, rightRotation));
}

void CAnimTreeDoubleChild::VSetPhase(float phase) {
  mA->VSetPhase(phase);
  mB->VSetPhase(phase);
}

SAdvancementResults
CAnimTreeDoubleChild::VGetAdvancementResults(const CCharAnimTime& time,
                                             const CCharAnimTime& startOffset) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  SAdvancementResults resA = mA->GetAdvancementResults(time, startOffset);
  SAdvancementResults resB = mB->GetAdvancementResults(time, startOffset);
  return resA.GetRemainder() > resB.GetRemainder() ? resA : resB;
#else
  SAdvancementResults resA = mA->VGetAdvancementResults(time, startOffset);
  SAdvancementResults resB = mB->VGetAdvancementResults(time, startOffset);
  return resA.mRemTime > resB.mRemTime ? resA : resB;
#endif
}

rstl::rc_ptr< CAnimTreeNode > CAnimTreeDoubleChild::VGetBestUnblendedChild() const {
  rstl::rc_ptr< CAnimTreeNode > child = GetRightChildWeight() > 0.5f ? mB : mA;
  if (!child) {
    return child;
  }

  rstl::rc_ptr< CAnimTreeNode > best = child->GetBestUnblendedChild();
  if (!best) {
    return child;
  }

  return best;
}

void CAnimTreeDoubleChild::VGetWeightedReaders(
    float weight, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const {
  mA->VGetWeightedReaders(weight, out);
  mB->VGetWeightedReaders(weight, out);
}
