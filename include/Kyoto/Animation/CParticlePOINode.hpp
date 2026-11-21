#ifndef _CPARTICLEPOINODE
#define _CPARTICLEPOINODE
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#endif

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#endif
#include "Kyoto/Animation/CPOINode.hpp"
#include "Kyoto/Particles/CParticleData.hpp"

class CParticlePOINode : public CPOINode {
public:
#if RSTL_CORRUPTION
  CParticlePOINode(uint nameHash = -1, EPOIType type = kPT_Particle,
                   const CCharAnimTime& time = CCharAnimTime(), int index = -1,
                   const bool unique = false, float weight = 1.f, int charIdx = -1, int flags = 0,
                   int unk2C = CPOINode::skDefaultUnk2C, const CSegId& segId = CSegId(),
                   int duration = 0, FourCC particleType = -1,
                   const CAssetId& particleId = kInvalidAssetId, float scale = 0.f,
                   CParticleData::EParentedMode mode = CParticleData::kPM_Invalid)
  : CPOINode(nameHash, type, time, index, unique, weight, charIdx, flags, unk2C, segId)
  , mDuration(duration)
  , mParticleType(particleType)
  , mParticleId(particleId)
  , mScale(scale)
  , mParentMode(mode) {}
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CParticlePOINode(const rstl::string name = rstl::string_l("")
#else
  CParticlePOINode(const uint name = -1
#endif
                   , const EPOIType type = kPT_Particle,
                   const CCharAnimTime& time = CCharAnimTime(), const int index = -1,
                   const bool unique = false, const float weight = 1.f, const int charIdx = -1,
                   const int flags = 0, const CParticleData& data = CParticleData())
  : CPOINode(name, type, time, index, unique, weight, charIdx, flags), mData(data) {}
#endif

  explicit CParticlePOINode(CInputStream& in);

#if RSTL_CORRUPTION
  int GetDuration() const { return mDuration; }
  SObjectTag GetParticleAssetInfo() const { // Guessed name
    return SObjectTag(mParticleType, mParticleId);
  }
  float GetScale() const { return mScale; } // Guessed name
  CParticleData::EParentedMode GetParentedMode() const { return mParentMode; } // Guessed name
#else
  const CParticleData& GetParticleData() const { return mData; }
#endif
  float GetMaximumDistance() const; // Guessed name
#if RSTL_CORRUPTION
  int GetAnimMask() const { return GetUnk2C(); } // Guessed name
#endif

  static CParticlePOINode CopyNodeMinusStartTime(const CParticlePOINode& node,
                                                 const CCharAnimTime& startTime);

private:
#if RSTL_CORRUPTION
  int mDuration; // Guessed name
  FourCC mParticleType; // Guessed name
  CAssetId mParticleId; // Guessed name
  float mScale; // Guessed name
  CParticleData::EParentedMode mParentMode; // Guessed name
#else
  CParticleData mData;
#endif
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CParticlePOINode, 0x50)
#else
#if !RSTL_PRIME1
CHECK_CHILD_SIZEOF(CParticlePOINode, CPOINode, 0x18)
#endif
#endif

#endif // _CPARTICLEPOINODE
