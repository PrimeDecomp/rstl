#ifndef _CPARTICLEPOINODE
#define _CPARTICLEPOINODE
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CPOINode.hpp"
#include "Kyoto/Particles/CParticleData.hpp"

class CParticlePOINode : public CPOINode {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CParticlePOINode(const rstl::string name = rstl::string_l(""), const EPOIType type = kPT_Particle,
#else
  CParticlePOINode(const uint nameHash = -1, const EPOIType type = kPT_Particle,
#endif

                   const CCharAnimTime& time = CCharAnimTime(), const int index = -1,
                   const bool unique = false, const float weight = 1.f, const int charIdx = -1,
                   const int flags = 0, const CParticleData& data = CParticleData())
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  : CPOINode(name, type, time, index, unique, weight, charIdx, flags), mData(data) {}
#else
  : CPOINode(nameHash, type, time, index, unique, weight, charIdx, flags), mData(data) {}
#endif

  explicit CParticlePOINode(CInputStream& in);

  const CParticleData& GetParticleData() const { return mData; }
#if !RSTL_PRIME1
  float GetMaximumDistance() const; // Guessed name
#endif

  static CParticlePOINode CopyNodeMinusStartTime(const CParticlePOINode& node,
                                                 const CCharAnimTime& startTime);

private:
  CParticleData mData;
};
#if !RSTL_PRIME1
CHECK_CHILD_SIZEOF(CParticlePOINode, CPOINode, 0x18)
#endif

#endif // _CPARTICLEPOINODE
