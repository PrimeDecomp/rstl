#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CParticlePOINode.hpp"
#if !RSTL_PRIME1
#include "float.h"
#endif

CParticlePOINode::CParticlePOINode(CInputStream& in) : CPOINode(in), mData(in) {}

CParticlePOINode CParticlePOINode::CopyNodeMinusStartTime(const CParticlePOINode& node,
                                                          const CCharAnimTime& startTime) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return CParticlePOINode(node.GetString(), node.GetPoiType(), node.GetTime() - startTime,
#else
  return CParticlePOINode(node.GetNameHash(), node.GetPoiType(), node.GetTime() - startTime,
#endif

                          node.GetIndex(), node.GetSaveState(), node.GetWeight(),
                          node.GetCharacterIndex(), node.GetFlags(), node.GetParticleData());
}

#if !RSTL_PRIME1
float CParticlePOINode::GetMaximumDistance() const {
  if (GetFlags() & 0x8000000) {
    return 60.f;
  }
  if (GetFlags() & 0x4000000) {
    return 40.f;
  }
  if (GetFlags() & 0x2000000) {
    return 20.f;
  }
  return FLT_MAX;
}
#endif

