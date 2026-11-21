#include "Kyoto/Animation/CNamedAnimPOIData.hpp"

#include "Kyoto/Streams/CInputStream.hpp"



CNamedAnimPOIData::SParticleEntry::SParticleEntry(CInputStream& in)
: mName(in), mNameHash(CPOINode::GetHashForString(mName.c_str())), mNode(in) {}

CNamedAnimPOIData::SAssetSoundEntry::SAssetSoundEntry(CInputStream& in)
: mName(in), mNameHash(CPOINode::GetHashForString(mName.c_str())), mNode(in) {}

CNamedAnimPOIData::CNamedAnimPOIData(CInputStream& in)
: mType(in.Get< int >()), mName(in), mParticleEntries(in), mAssetSoundEntries(in) {}

bool CNamedAnimPOIData::operator<(const CNamedAnimPOIData& other) const {
  if (mType == other.mType) {
    return mName < other.mName;
  }
  return mType < other.mType;
}

CNamedAnimPOIData::CNamedAnimPOIData(int type, const rstl::string& name,
                                     const rstl::vector< SParticleEntry >& particles,
                                     const rstl::vector< SAssetSoundEntry >& assetSounds)
: mType(type), mName(name), mParticleEntries(particles), mAssetSoundEntries(assetSounds) {}

CNamedAnimPOIData CNamedAnimPOIData::MakeKey(int type, const rstl::string& name) {
  return CNamedAnimPOIData(type, name, rstl::vector< SParticleEntry >(),
                           rstl::vector< SAssetSoundEntry >());
}
