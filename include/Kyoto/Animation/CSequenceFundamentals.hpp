#ifndef _CSEQUENCEFUNDAMENTALS
#define _CSEQUENCEFUNDAMENTALS
#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CBoolPOINode.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"
#include "Kyoto/Animation/CSteadyStateAnimInfo.hpp"
#include "Kyoto/Animation/IAnimReader.hpp"
#include "rstl/vector.hpp"

class CSequenceFundamentals {
public:
  CSequenceFundamentals(const CSteadyStateAnimInfo& info,
                        const rstl::vector< CBoolPOINode >& boolNodes,
                        const rstl::vector< CInt32POINode >& int32Nodes,
                        const rstl::vector< CParticlePOINode >& particleNodes,
                        const rstl::vector< CSoundPOINode >& soundNodes)
  : mSteadyStateAnimInfo(info)
  , mBoolPOINodes(boolNodes)
  , mInt32POINodes(int32Nodes)
  , mParticlePOINodes(particleNodes)
  , mSoundPOINodes(soundNodes) {}

  const CSteadyStateAnimInfo& GetSteadyStateAnimInfo() const { return mSteadyStateAnimInfo; }
  const rstl::vector< CBoolPOINode >& GetBoolPointsOfInterest() const { return mBoolPOINodes; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::vector< CInt32POINode >& GetInt32PointsOfInterest() const {
    return mInt32POINodes;
  }
#else
  const rstl::vector< CInt32POINode >& GetInt32PointsOfInterest() const { return mInt32POINodes; }
#endif

  const rstl::vector< CParticlePOINode >& GetParticlePointsOfInterest() const {
    return mParticlePOINodes;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::vector< CSoundPOINode >& GetSoundPointsOfInterest() const {
    return mSoundPOINodes;
  }
#else
  const rstl::vector< CSoundPOINode >& GetSoundPointsOfInterest() const { return mSoundPOINodes; }
#endif

private:
  CSteadyStateAnimInfo mSteadyStateAnimInfo;
  rstl::vector< CBoolPOINode > mBoolPOINodes;
  rstl::vector< CInt32POINode > mInt32POINodes;
  rstl::vector< CParticlePOINode > mParticlePOINodes;
  rstl::vector< CSoundPOINode > mSoundPOINodes;
};
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHECK_SIZEOF(CSequenceFundamentals, 0x58)
#else
CHECK_SIZEOF(CSequenceFundamentals, RSTL_VERSION >= RSTL_R32J ? 0x48 : 0x58)
#endif

#endif // _CSEQUENCEFUNDAMENTALS
