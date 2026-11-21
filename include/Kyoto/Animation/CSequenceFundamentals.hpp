#ifndef _CSEQUENCEFUNDAMENTALS
#define _CSEQUENCEFUNDAMENTALS
#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Animation/CAssetPOINode.hpp"
#include "Kyoto/Animation/CAssetSoundPOINode.hpp"
#else
#include "rstl/RstlVersions.h"
#endif
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
#if RSTL_CORRUPTION
                        const rstl::vector< CSoundPOINode >& soundNodes,
                        const rstl::vector< CAssetSoundPOINode >& assetSoundNodes,
                        const rstl::vector< CAssetPOINode >& assetNodes)
#else
                        const rstl::vector< CSoundPOINode >& soundNodes)
#endif
  : mSteadyStateAnimInfo(info)
  , mBoolPOINodes(boolNodes)
  , mInt32POINodes(int32Nodes)
  , mParticlePOINodes(particleNodes)
#if RSTL_CORRUPTION
  , mSoundPOINodes(soundNodes)
  , mAssetSoundPOINodes(assetSoundNodes)
  , mAssetPOINodes(assetNodes) {}
#else
  , mSoundPOINodes(soundNodes) {}
#endif

  const CSteadyStateAnimInfo& GetSteadyStateAnimInfo() const { return mSteadyStateAnimInfo; }
  const rstl::vector< CBoolPOINode >& GetBoolPointsOfInterest() const { return mBoolPOINodes; }
  const rstl::vector< CInt32POINode >& GetInt32PointsOfInterest() const { return mInt32POINodes; }

  const rstl::vector< CParticlePOINode >& GetParticlePointsOfInterest() const {
    return mParticlePOINodes;
  }
  const rstl::vector< CSoundPOINode >& GetSoundPointsOfInterest() const { return mSoundPOINodes; }
#if RSTL_CORRUPTION
  const rstl::vector< CAssetSoundPOINode >& GetAssetSoundPointsOfInterest() const { // Guessed name
    return mAssetSoundPOINodes;
  }
  const rstl::vector< CAssetPOINode >& GetAssetPointsOfInterest() const { // Guessed name
    return mAssetPOINodes;
  }
#endif

private:
  CSteadyStateAnimInfo mSteadyStateAnimInfo;
  rstl::vector< CBoolPOINode > mBoolPOINodes;
  rstl::vector< CInt32POINode > mInt32POINodes;
  rstl::vector< CParticlePOINode > mParticlePOINodes;
  rstl::vector< CSoundPOINode > mSoundPOINodes;
#if RSTL_CORRUPTION
  rstl::vector< CAssetSoundPOINode > mAssetSoundPOINodes; // Guessed name
  rstl::vector< CAssetPOINode > mAssetPOINodes;           // Guessed name
#endif
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CSequenceFundamentals, 0x88)
#else
CHECK_SIZEOF(CSequenceFundamentals, RSTL_PRIME1 ? 0x58 : (RSTL_VERSION >= RSTL_R32J ? 0x48 : 0x58))
#endif

#endif // _CSEQUENCEFUNDAMENTALS
