#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CAnimTreeDoubleChild.hpp"
#if RSTL_PRIME1
#include "Kyoto/Animation/CAnimTreeNode.hpp"
#endif

#include "Kyoto/Animation/CBoolPOINode.hpp"
#if RSTL_PRIME1
#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"
#endif

#include "Kyoto/Animation/CInt32POINode.hpp"
#if RSTL_PRIME1
#include "Kyoto/Animation/CPOINode.hpp"
#endif

#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"
#if RSTL_PRIME1
#include "Kyoto/Animation/IAnimReader.hpp"
#endif

#include "Kyoto/Math/CloseEnough.hpp"
#include "stdlib.h"

CAnimTreeDoubleChild::CAnimTreeDoubleChild(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                           const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                           const rstl::string& name)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
: CAnimTreeNode(name), mA(a), mB(b) {
  CCharAnimMemoryMetrics::AddToTotalSize(8, CCharAnimMemoryMetrics::kASS_Two);
#else
: CAnimTreeNode(name), mA(a), mB(b) {}

SAdvancementResults CAnimTreeDoubleChild::VAdvanceView(const CCharAnimTime& time) {
  SAdvancementResults a = mA->VAdvanceView(time);
  SAdvancementResults b = mB->VAdvanceView(time);
  return a.mRemTime > b.mRemTime ? a : b;
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CAdvancementResults CAnimTreeDoubleChild::VAdvanceView(const CCharAnimTime& dt) {
  CAdvancementResults resA = mA->AdvanceView(dt);
  CAdvancementResults resB = mB->AdvanceView(dt);
  return resA.GetRemainder() > resB.GetRemainder() ? resA : resB;
}

CAnimTreeDoubleChild::~CAnimTreeDoubleChild() {
  CCharAnimMemoryMetrics::SubtractFromTotalSize(8, CCharAnimMemoryMetrics::kASS_Two);
}
#else
CAnimTreeDoubleChild::~CAnimTreeDoubleChild() {}
#endif

uint CAnimTreeDoubleChild::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                           uint capacity, uint iterator, int unk) const {
  int x = mA->GetBoolPOIList(time, listOut, capacity, iterator, unk);
  x += mB->GetBoolPOIList(time, listOut, capacity, x + iterator, unk);
  if (x > capacity)
    x = capacity;
  qsort(listOut, x, sizeof(CBoolPOINode), CPOINode::compare);
  return x;
#else
                                           uint capacity, uint iterator, int additive) const {
  uint count = mA->GetBoolPOIList(time, listOut, capacity, iterator, additive);
  count += mB->GetBoolPOIList(time, listOut, capacity, count + iterator, additive);
  if (count > capacity) {
    count = capacity;
  }
  qsort(listOut, count, sizeof(CBoolPOINode), CPOINode::compare);
  return count;
#endif

}

uint CAnimTreeDoubleChild::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                            uint capacity, uint iterator, int unk) const {
  int x = mA->GetInt32POIList(time, listOut, capacity, iterator, unk);
  x += mB->GetInt32POIList(time, listOut, capacity, x + iterator, unk);
  if (x > capacity)
    x = capacity;
  qsort(listOut, x, sizeof(CInt32POINode), CPOINode::compare);
  return x;
#else
                                            uint capacity, uint iterator, int additive) const {
  uint count = mA->GetInt32POIList(time, listOut, capacity, iterator, additive);
  count += mB->GetInt32POIList(time, listOut, capacity, count + iterator, additive);
  if (count > capacity) {
    count = capacity;
  }
  qsort(listOut, count, sizeof(CInt32POINode), CPOINode::compare);
  return count;
#endif

}

uint CAnimTreeDoubleChild::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                               uint capacity, uint iterator, int unk) const {
  int x = mA->GetParticlePOIList(time, listOut, capacity, iterator, unk);
  x += mB->GetParticlePOIList(time, listOut, capacity, x + iterator, unk);
  if (x > capacity)
    x = capacity;
  qsort(listOut, x, sizeof(CParticlePOINode), CPOINode::compare);
  return x;
#else
                                               uint capacity, uint iterator, int additive) const {
  uint count = mA->GetParticlePOIList(time, listOut, capacity, iterator, additive);
  count += mB->GetParticlePOIList(time, listOut, capacity, count + iterator, additive);
  if (count > capacity) {
    count = capacity;
  }
  qsort(listOut, count, sizeof(CParticlePOINode), CPOINode::compare);
  return count;
#endif

}

uint CAnimTreeDoubleChild::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                            uint capacity, uint iterator, int unk) const {
  int x = mA->GetSoundPOIList(time, listOut, capacity, iterator, unk);
  x += mB->GetSoundPOIList(time, listOut, capacity, x + iterator, unk);
  if (x > capacity)
    x = capacity;
  qsort(listOut, x, sizeof(CSoundPOINode), CPOINode::compare);
  return x;
#else
                                            uint capacity, uint iterator, int additive) const {
  uint count = mA->GetSoundPOIList(time, listOut, capacity, iterator, additive);
  count += mB->GetSoundPOIList(time, listOut, capacity, count + iterator, additive);
  if (count > capacity) {
    count = capacity;
  }
  qsort(listOut, count, sizeof(CSoundPOINode), CPOINode::compare);
  return count;
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
bool CAnimTreeDoubleChild::VGetBoolPOIState(const char* name) const {
  return mB->VGetBoolPOIState(name);
#else
bool CAnimTreeDoubleChild::VGetBoolPOIState(uint nameHash) const {
  return mB->VGetBoolPOIState(nameHash);
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
s32 CAnimTreeDoubleChild::VGetInt32POIState(const char* name) const {
  return mB->VGetInt32POIState(name);
#else
s32 CAnimTreeDoubleChild::VGetInt32POIState(uint nameHash) const {
  return mB->VGetInt32POIState(nameHash);
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CParticleData::EParentedMode CAnimTreeDoubleChild::VGetParticlePOIState(const char* name) const {
  return mB->VGetParticlePOIState(name);
#else
CParticleData::EParentedMode CAnimTreeDoubleChild::VGetParticlePOIState(uint nameHash) const {
  return mB->VGetParticlePOIState(nameHash);
#endif

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const CCharAnimTime& trueAdvancement, const CAdvancementDeltas& leftDeltas,
    const CAdvancementDeltas& rightDeltas)
: mTrueAdvancement(trueAdvancement), mLeftDeltas(leftDeltas), mRightDeltas(rightDeltas) {}
#else
    const CCharAnimTime& time, const SAdvancementDeltas& left, const SAdvancementDeltas& right)
: mTrueAdvancement(time), mLeftDeltas(left), mRightDeltas(right) {}
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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      CAdvancementResults result = mA->AdvanceView(leftRemaining);
#else
      SAdvancementResults result = mA->AdvanceView(leftRemaining);
#endif

      rstl::optional_object< rstl::ownership_transfer< IAnimReader > > simplified =
          mA->Simplified();
      if (simplified.valid())
        mA = Cast(*simplified);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      CAdvancementDeltas deltas = result.GetAdvancementDeltas();
#else
      SAdvancementDeltas deltas = result.mDeltas;
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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      CAdvancementResults result = mB->AdvanceView(rightRemaining);
#else
      SAdvancementResults result = mB->AdvanceView(rightRemaining);
#endif

      rstl::optional_object< rstl::ownership_transfer< IAnimReader > > simplified =
          mB->Simplified();
      if (simplified.valid())
        mB = Cast(*simplified);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      CAdvancementDeltas deltas = result.GetAdvancementDeltas();
#else
      SAdvancementDeltas deltas = result.mDeltas;
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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return CDoubleChildAdvancementResult(time, CAdvancementDeltas(leftOffset, leftRotation),
                                       CAdvancementDeltas(rightOffset, rightRotation));
#else
  return CDoubleChildAdvancementResult(time, SAdvancementDeltas(leftOffset, leftRotation),
                                       SAdvancementDeltas(rightOffset, rightRotation));
#endif

}

void CAnimTreeDoubleChild::VSetPhase(float phase) {
  mA->VSetPhase(phase);
  mB->VSetPhase(phase);
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CAdvancementResults CAnimTreeDoubleChild::VGetAdvancementResults(const CCharAnimTime& a,
                                                                 const CCharAnimTime& b) const {
  CAdvancementResults resA = mA->GetAdvancementResults(a, b);
  CAdvancementResults resB = mB->GetAdvancementResults(a, b);
  return resA.GetRemainder() > resB.GetRemainder() ? resA : resB;
#else
SAdvancementResults
CAnimTreeDoubleChild::VGetAdvancementResults(const CCharAnimTime& time,
                                             const CCharAnimTime& startOffset) const {
  SAdvancementResults a = mA->VGetAdvancementResults(time, startOffset);
  SAdvancementResults b = mB->VGetAdvancementResults(time, startOffset);
  return a.mRemTime > b.mRemTime ? a : b;
#endif

}

rstl::rc_ptr< CAnimTreeNode > CAnimTreeDoubleChild::VGetBestUnblendedChild() const {
  rstl::rc_ptr< CAnimTreeNode > child = GetRightChildWeight() > 0.5f ? mB : mA;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (!child)
#else
  if (!child) {
#endif

    return child;
#if !RSTL_PRIME1
  }
#endif

  rstl::rc_ptr< CAnimTreeNode > best = child->GetBestUnblendedChild();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (!best)
#else
  if (!best) {
#endif

    return child;
#if !RSTL_PRIME1
  }
#endif

  return best;
}

void CAnimTreeDoubleChild::VGetWeightedReaders(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    float w, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const {
  mA->VGetWeightedReaders(w, out);
  mB->VGetWeightedReaders(w, out);
#else
    float weight, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const {
  mA->VGetWeightedReaders(weight, out);
  mB->VGetWeightedReaders(weight, out);
#endif

}

