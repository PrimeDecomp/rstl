#include "rstl/RstlVersions.h"
#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CParticlePOINode.hpp"
#if !RSTL_CORRUPTION
#if !RSTL_PRIME1
#include "float.h"
#endif
#endif

#if RSTL_CORRUPTION


CParticlePOINode::CParticlePOINode(CInputStream& in)
: CPOINode(in)
, mDuration(in.Get< int >())
, mParticleType(in.ReadInt32())
, mParticleId(in.Get< CAssetId >())
, mScale(in.ReadFloat())
, mParentMode(CParticleData::EParentedMode(in.Get< int >())) {}
#else
CParticlePOINode::CParticlePOINode(CInputStream& in) : CPOINode(in), mData(in) {}
#endif

CParticlePOINode CParticlePOINode::CopyNodeMinusStartTime(const CParticlePOINode& node,
                                                          const CCharAnimTime& startTime) {
#if RSTL_CORRUPTION
  const SObjectTag tag = node.GetParticleAssetInfo();
  return CParticlePOINode(node.GetNameHash(), node.GetPoiType(), node.GetTime() - startTime,
                          node.GetIndex(), node.GetSaveState(), node.GetWeight(),
                          node.GetCharacterIndex(), node.GetFlags(), node.GetUnk2C(),
                          node.GetSegId(), node.GetDuration(), tag.GetType(), tag.GetId(),
                          node.GetScale(), node.GetParentedMode());
#else
  return CParticlePOINode(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      node.GetString(),
#else
      node.GetNameHash(),
#endif
      node.GetPoiType(), node.GetTime() - startTime, node.GetIndex(), node.GetSaveState(),
      node.GetWeight(), node.GetCharacterIndex(), node.GetFlags(), node.GetParticleData());
#endif
}

#if RSTL_CORRUPTION
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
  return 3.4028235e38f;
}
#else
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
  return RSTL_KYOTO_FLT_MAX;
}
#endif
#endif
