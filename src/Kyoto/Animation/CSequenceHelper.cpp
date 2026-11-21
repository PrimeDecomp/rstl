#include "rstl/RstlVersions.h"
#include "rstl/RstlVersions.h"
#include "rstl/macros.hpp"
#include "Kyoto/Animation/CSequenceHelper.hpp"
#if RSTL_CORRUPTION

#include "Kyoto/Animation/CSegIdMaskMap.hpp"
#endif

#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "Kyoto/Animation/CTreeUtils.hpp"
#include "Kyoto/Animation/IMetaAnim.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

CSequenceHelper::CSequenceHelper(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                 const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                 const CAnimSysContext& context)
: mContext(context) {
  mNodes.reserve(2);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mNodes.push_back(a);
  mNodes.push_back(b);
#else
  mNodes.push_back_unsafe(a);
  mNodes.push_back_unsafe(b);
#endif
}

CSequenceHelper::CSequenceHelper(const rstl::vector< rstl::rc_ptr< IMetaAnim > >& anims,
                                 const CAnimSysContext& context)
: mContext(context) {
  mNodes.reserve(anims.size());

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  AUTO(it, anims.begin());
  AUTO(end, anims.end());
#else
  rstl::vector< rstl::rc_ptr< IMetaAnim > >::const_iterator it = anims.begin();
  rstl::vector< rstl::rc_ptr< IMetaAnim > >::const_iterator end = anims.end();
#endif
  for (; it != end; ++it) {
    const rstl::rc_ptr< IMetaAnim >& anim = *it;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    mNodes.push_back(
#else
    mNodes.push_back_unsafe(
#endif
        anim->GetAnimationTree(context, CMetaAnimTreeBuildOrders::NoSpecialOrders()));
  }
}

CSequenceFundamentals CSequenceHelper::ComputeSequenceFundamentals() const {
#if !RSTL_CORRUPTION
  const int numNodes = mNodes.size();
#endif
  CCharAnimTime elapsed = CCharAnimTime::ZeroFlat();
#if !RSTL_CORRUPTION
  CVector3f offset = CVector3f::Zero();
#endif
  CCharAnimTime duration = CCharAnimTime::ZeroFlat();
#if RSTL_CORRUPTION
  CVector3f offset = CVector3f::Zero();
#endif
  rstl::vector< CBoolPOINode > boolNodes;
  rstl::vector< CInt32POINode > int32Nodes;
  rstl::vector< CParticlePOINode > partNodes;
  rstl::vector< CSoundPOINode > soundNodes;
#if RSTL_CORRUPTION
  rstl::vector< CAssetSoundPOINode > assetSoundNodes;
  rstl::vector< CAssetPOINode > assetNodes;
  CSegIdMaskMap mask(150, CPOINode::skDefaultUnk2C);
  const int numNodes = mNodes.size();
  rstl::vector< CCharAnimTime > segmentDurations(numNodes, CCharAnimTime::ZeroFlat());
#endif

  if (numNodes > 0) {
    rstl::ncrc_ptr< CAnimTreeNode > node = Cast(mNodes[0]->Clone());
    for (int i = 0; i < numNodes; ++i) {
      CBoolPOINode boolArray[64];
#if RSTL_CORRUPTION
      const uint numBools = node->GetBoolPOIList(CCharAnimTime::Infinity(), boolArray, 64, 0, mask);
#else
      const uint numBools = node->GetBoolPOIList(CCharAnimTime::Infinity(), boolArray, 64, 0, 0);
#endif
      boolNodes.reserve(boolNodes.size() + numBools);
      for (uint j = 0; j < numBools; ++j) {
        CBoolPOINode& poi = boolArray[j];
        poi.SetTime(poi.GetTime() + duration);
#if RSTL_CORRUPTION
        boolNodes.push_back_unsafe(poi);
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        boolNodes.push_back(poi);
#else
        boolNodes.push_back_unsafe(poi);
#endif
      }

      CInt32POINode int32Array[64];
      const uint numInt32s = node->GetInt32POIList(CCharAnimTime::Infinity(), int32Array, 64, 0, 0);
      int32Nodes.reserve(int32Nodes.size() + numInt32s);
      for (uint j = 0; j < numInt32s; ++j) {
        CInt32POINode& poi = int32Array[j];
        poi.SetTime(poi.GetTime() + duration);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        int32Nodes.push_back(poi);
#else
        int32Nodes.push_back_unsafe(poi);
#endif
#endif
      }

      CParticlePOINode partArray[64];
      const uint numParts =
#if RSTL_CORRUPTION
          node->GetParticlePOIList(CCharAnimTime::Infinity(), partArray, 64, 0, mask);
#else
          node->GetParticlePOIList(CCharAnimTime::Infinity(), partArray, 64, 0, 0);
#endif
      partNodes.reserve(partNodes.size() + numParts);
      for (uint j = 0; j < numParts; ++j) {
        CParticlePOINode& poi = partArray[j];
        poi.SetTime(poi.GetTime() + duration);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        partNodes.push_back(poi);
#else
        partNodes.push_back_unsafe(poi);
#endif
      }

      CSoundPOINode soundArray[64];
#if RSTL_CORRUPTION
      const uint numSounds = node->GetSoundPOIList(CCharAnimTime::Infinity(), soundArray, 64, 0, mask);
#else
      const uint numSounds = node->GetSoundPOIList(CCharAnimTime::Infinity(), soundArray, 64, 0, 0);
#endif
      soundNodes.reserve(soundNodes.size() + numSounds);
      for (uint j = 0; j < numSounds; ++j) {
        CSoundPOINode& poi = soundArray[j];
        poi.SetTime(poi.GetTime() + duration);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        soundNodes.push_back(poi);
#else
        soundNodes.push_back_unsafe(poi);
#endif
      }

#if RSTL_CORRUPTION
      CAssetSoundPOINode assetSoundArray[64];
      const uint numAssetSounds =
          node->GetAssetSoundPOIList(CCharAnimTime::Infinity(), assetSoundArray, 64, 0, mask);
      assetSoundNodes.reserve(assetSoundNodes.size() + numAssetSounds);
      for (uint j = 0; j < numAssetSounds; ++j) {
        CAssetSoundPOINode& poi = assetSoundArray[j];
        poi.SetTime(poi.GetTime() + duration);
        assetSoundNodes.push_back_unsafe(poi);
      }

      CAssetPOINode assetArray[64];
      const uint numAssets = node->GetAssetPOIList(CCharAnimTime::Infinity(), assetArray, 64, 0, mask);
      assetNodes.reserve(assetNodes.size() + numAssets);
      for (uint j = 0; j < numAssets; ++j) {
        CAssetPOINode& poi = assetArray[j];
        poi.SetTime(poi.GetTime() + duration);
        assetNodes.push_back_unsafe(poi);
      }

      const CCharAnimTime segmentDuration = node->GetTimeRemaining();
      segmentDurations[i] = segmentDuration;
      duration += segmentDuration;
#else
      duration += node->GetTimeRemaining();
#endif
      CCharAnimTime remaining = node->GetTimeRemaining();
      while (remaining != CCharAnimTime::ZeroFlat() && !close_enough(remaining.GetSeconds(), 0.f)) {
        CCharAnimTime previous = remaining;
        SAdvancementResults result = node->AdvanceView(previous);

        rstl::optional_object< rstl::ownership_transfer< IAnimReader > > simplified =
            node->Simplified();
        if (simplified.valid()) {
          node = Cast(*simplified);
        }

        remaining = result.GetRemainder();
        elapsed += previous - remaining;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        const rstl::optional_object< CVector3f > delta =
            result.GetAdvancementDeltas().GetOffsetDelta();
#else
        const rstl::optional_object< CVector3f > delta = result.mDeltas.mPosDelta;
#endif
        offset += *delta;
      }

      if (i < numNodes - 1) {
#if RSTL_CORRUPTION
        node = CTreeUtils::GetTransitionTree(node, Cast(mNodes[i + 1]->Clone()), mContext, true);
#else
        node = CTreeUtils::GetTransitionTree(node, Cast(mNodes[i + 1]->Clone()), mContext);
#endif
      }
    }
  }

#if RSTL_CORRUPTION
  return CSequenceFundamentals(CSteadyStateAnimInfo(false, offset, segmentDurations), boolNodes,
                               int32Nodes, partNodes, soundNodes, assetSoundNodes, assetNodes);
#else
  return CSequenceFundamentals(CSteadyStateAnimInfo(false, duration, offset), boolNodes, int32Nodes,
                               partNodes, soundNodes);
#endif
}
