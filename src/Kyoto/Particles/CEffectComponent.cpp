#include "rstl/RstlVersions.h"
#include "Kyoto/Particles/CEffectComponent.hpp"

#include "Kyoto/Animation/CPOINode.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CEffectComponent::CEffectComponent(CInputStream& in)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
: mName(in)
#else
: mNameHash(CPOINode::GetHashForString(rstl::string(in).data()))
#endif

, mTag(GetSObjectTagFromStream(in))
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mBoneName(in)
#else
, mBone(in)
#endif
, mScale(in.Get< float >())
, mParentedMode(CParticleData::EParentedMode(in.Get< uint >()))
, mFlags(in.Get< uint >()) {}

SObjectTag CEffectComponent::GetSObjectTagFromStream(CInputStream& in) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const FourCC type = in.Get< CAssetId >();
  const CAssetId id = in.Get< CAssetId >();
#else
  const FourCC type = in.Get< FourCC >();
  const CAssetId id = in.ReadUint32();
#endif

  return SObjectTag(type, id);
}

