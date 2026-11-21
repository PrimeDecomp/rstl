#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif
#include "Kyoto/Animation/CAnimTreeDoubleChild.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Animation/CAssetPOINode.hpp"
#include "Kyoto/Animation/CAssetSoundPOINode.hpp"
#else
#include "Kyoto/Animation/CAnimTreeNode.hpp"

#endif
#include "Kyoto/Animation/CBoolPOINode.hpp"
#if !RSTL_CORRUPTION
#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"

#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CPOINode.hpp"

#endif
#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"
#if !RSTL_CORRUPTION
#include "Kyoto/Animation/IAnimReader.hpp"

#endif
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
#if RSTL_CORRUPTION
  SAdvancementResults a = mA->VAdvanceView(time);
  SAdvancementResults b = mB->VAdvanceView(time);
  return a.mRemTime > b.mRemTime ? a : b;
#else
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
#endif
}

CAnimTreeDoubleChild::~CAnimTreeDoubleChild() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CCharAnimMemoryMetrics::SubtractFromTotalSize(8, CCharAnimMemoryMetrics::kASS_Two);
#endif
}

uint CAnimTreeDoubleChild::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
#if RSTL_CORRUPTION
                                           uint capacity, uint iterator, const CSegIdMaskMap& mask) const {
  uint count = mA->GetBoolPOIList(time, listOut, capacity, iterator, mask);
  count += mB->GetBoolPOIList(time, listOut, capacity, count + iterator, mask);
#else
                                           uint capacity, uint iterator, int additive) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int
#else
  uint
#endif
      count = mA->GetBoolPOIList(time, listOut, capacity, iterator, additive);
  count += mB->GetBoolPOIList(time, listOut, capacity, count + iterator, additive);
#endif
  if (count > capacity) {
    count = capacity;
  }
  qsort(listOut, count, sizeof(CBoolPOINode), CPOINode::compare);
  return count;
}

#if !RSTL_CORRUPTION
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

#endif
uint CAnimTreeDoubleChild::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
#if RSTL_CORRUPTION
                                               uint capacity, uint iterator, const CSegIdMaskMap& mask) const {
  uint count = mA->GetParticlePOIList(time, listOut, capacity, iterator, mask);
  count += mB->GetParticlePOIList(time, listOut, capacity, count + iterator, mask);
#else
                                               uint capacity, uint iterator, int additive) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int
#else
  uint
#endif
      count = mA->GetParticlePOIList(time, listOut, capacity, iterator, additive);
  count += mB->GetParticlePOIList(time, listOut, capacity, count + iterator, additive);
#endif
  if (count > capacity) {
    count = capacity;
  }
  qsort(listOut, count, sizeof(CParticlePOINode), CPOINode::compare);
  return count;
}

uint CAnimTreeDoubleChild::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut,
#if RSTL_CORRUPTION
                                            uint capacity, uint iterator, const CSegIdMaskMap& mask) const {
  uint count = mA->GetSoundPOIList(time, listOut, capacity, iterator, mask);
  count += mB->GetSoundPOIList(time, listOut, capacity, count + iterator, mask);
#else
                                            uint capacity, uint iterator, int additive) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int
#else
  uint
#endif
      count = mA->GetSoundPOIList(time, listOut, capacity, iterator, additive);
  count += mB->GetSoundPOIList(time, listOut, capacity, count + iterator, additive);
#endif
  if (count > capacity) {
    count = capacity;
  }
  qsort(listOut, count, sizeof(CSoundPOINode), CPOINode::compare);
  return count;
}

#if RSTL_CORRUPTION
uint CAnimTreeDoubleChild::VGetAssetSoundPOIList(const CCharAnimTime& time,
                                                 CAssetSoundPOINode* listOut, uint capacity,
                                                 uint iterator, const CSegIdMaskMap& mask) const {
  uint count = mA->GetAssetSoundPOIList(time, listOut, capacity, iterator, mask);
  count += mB->GetAssetSoundPOIList(time, listOut, capacity, count + iterator, mask);
  if (count > capacity) {
    count = capacity;
  }
  qsort(listOut, count, sizeof(CAssetSoundPOINode), CPOINode::compare);
  return count;
#else
bool CAnimTreeDoubleChild::VGetBoolPOIState(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const char* name
#else
    uint name
#endif
    ) const {
  return mB->VGetBoolPOIState(name);
#endif
}

#if RSTL_CORRUPTION
uint CAnimTreeDoubleChild::VGetAssetPOIList(const CCharAnimTime& time, CAssetPOINode* listOut,
                                            uint capacity, uint iterator, const CSegIdMaskMap& mask) const {
  uint count = mA->GetAssetPOIList(time, listOut, capacity, iterator, mask);
  count += mB->GetAssetPOIList(time, listOut, capacity, count + iterator, mask);
  if (count > capacity) {
    count = capacity;
  }
  qsort(listOut, count, sizeof(CAssetPOINode), CPOINode::compare);
  return count;
#else
s32 CAnimTreeDoubleChild::VGetInt32POIState(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const char* name
#else
    uint name
#endif
    ) const {
  return mB->VGetInt32POIState(name);
#endif
}

#if RSTL_CORRUPTION
bool CAnimTreeDoubleChild::VGetBoolPOIState(uint nameHash) const {
  return mB->VGetBoolPOIState(nameHash);
}
#else
CParticleData::EParentedMode CAnimTreeDoubleChild::VGetParticlePOIState(
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_CORRUPTION
CParticleData::EParentedMode CAnimTreeDoubleChild::VGetParticlePOIState(uint nameHash) const {
  return mB->VGetParticlePOIState(nameHash);
#else
#if RSTL_PRIME1
    const char* name
#else
    uint name
#endif
    ) const {
  return mB->VGetParticlePOIState(name);
#endif
}

CAnimTreeEffectiveContribution CAnimTreeDoubleChild::VGetContributionOfHighestInfluence() const {
  CAnimTreeEffectiveContribution a = mA->GetContributionOfHighestInfluence();
  CAnimTreeEffectiveContribution b = mB->GetContributionOfHighestInfluence();
#if RSTL_CORRUPTION
  const float leftWeight = a.GetContributionWeight() * GetLeftChildWeight();
  const float rightWeight = b.GetContributionWeight() * GetRightChildWeight();
#else
  RSTL_ECHOES_CONST float leftWeight = a.GetContributionWeight() * GetLeftChildWeight();
  RSTL_ECHOES_CONST float rightWeight = b.GetContributionWeight() * GetRightChildWeight();
#endif
  return leftWeight > rightWeight
#if RSTL_CORRUPTION
             ? CAnimTreeEffectiveContribution(leftWeight, a.GetPrimitive(),
                                              a.GetSteadyStateAnimInfo(), a.GetTimeRemaining())
             : CAnimTreeEffectiveContribution(rightWeight, b.GetPrimitive(),
                                              b.GetSteadyStateAnimInfo(), b.GetTimeRemaining());
#else
             ? CAnimTreeEffectiveContribution(leftWeight, a.GetPrimitiveName(),
                                              a.GetSteadyStateAnimInfo(), a.GetTimeRemaining(),
                                              a.GetAnimDatabaseIndex())
             : CAnimTreeEffectiveContribution(rightWeight, b.GetPrimitiveName(),
                                              b.GetSteadyStateAnimInfo(), b.GetTimeRemaining(),
                                              b.GetAnimDatabaseIndex());
#endif
}

uint CAnimTreeDoubleChild::VGetNumChildren() const {
#if RSTL_CORRUPTION
  const uint rightCount = mB->VGetNumChildren();
  return mA->VGetNumChildren() + 2 + rightCount;
}

uint CAnimTreeDoubleChild::VGetNumDirectChildren() const { return 2; }

rstl::rc_ptr< CAnimTreeNode > CAnimTreeDoubleChild::VGetDirectChild(uint index) const {
  if (index < 2) {
    return index == 0 ? GetLeftChild() : GetRightChild();
  }
  return rstl::rc_ptr< CAnimTreeNode >(nullptr);
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int num_children = mB->VGetNumChildren();
  num_children += mA->VGetNumChildren() + 2;
  return num_children;
#else
  const uint rightCount = mB->VGetNumChildren();
  return mA->VGetNumChildren() + 2 + rightCount;
#endif
#endif
}

CAnimTreeDoubleChild::CDoubleChildAdvancementResult::CDoubleChildAdvancementResult(
#if RSTL_CORRUPTION
    const CCharAnimTime& time, const SAdvancementDeltas& left, const SAdvancementDeltas& right)
: mTrueAdvancement(time), mLeftDeltas(left), mRightDeltas(right) {}
#else
    const CCharAnimTime& time, const CAdvancementDeltas& left, const CAdvancementDeltas& right)
: mTrueAdvancement(time)
, mLeftDeltas(left)
, mRightDeltas(right) {}
#endif

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
#if RSTL_CORRUPTION
      SAdvancementDeltas deltas = result.mDeltas;
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      CAdvancementDeltas deltas = result.GetAdvancementDeltas();
#else
      CAdvancementDeltas deltas = result.mDeltas;
#endif
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
#if RSTL_CORRUPTION
      SAdvancementDeltas deltas = result.mDeltas;
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      CAdvancementDeltas deltas = result.GetAdvancementDeltas();
#else
      CAdvancementDeltas deltas = result.mDeltas;
#endif
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
#if RSTL_CORRUPTION

  return CDoubleChildAdvancementResult(time, SAdvancementDeltas(leftOffset, leftRotation),
                                       SAdvancementDeltas(rightOffset, rightRotation));
#else
  return CDoubleChildAdvancementResult(time, CAdvancementDeltas(leftOffset, leftRotation),
                                       CAdvancementDeltas(rightOffset, rightRotation));
#endif
}

void CAnimTreeDoubleChild::VSetPhase(float phase) {
  mA->VSetPhase(phase);
  mB->VSetPhase(phase);
}

SAdvancementResults
CAnimTreeDoubleChild::VGetAdvancementResults(const CCharAnimTime& time,
                                             const CCharAnimTime& startOffset) const {
#if RSTL_CORRUPTION
  SAdvancementResults a = mA->VGetAdvancementResults(time, startOffset);
  SAdvancementResults b = mB->VGetAdvancementResults(time, startOffset);
  return a.mRemTime > b.mRemTime ? a : b;
#else
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
