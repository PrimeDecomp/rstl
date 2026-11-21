#ifndef _CANIMPOIDATA
#define _CANIMPOIDATA
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#endif

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "Kyoto/Animation/CAssetPOINode.hpp"
#include "Kyoto/Animation/CAssetSoundPOINode.hpp"
#else
#include "Kyoto/CFactoryFnReturn.hpp"
#include "rstl/optional_object.hpp"
#endif
#include "Kyoto/Animation/CBoolPOINode.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"

#include "rstl/vector.hpp"

#if !RSTL_CORRUPTION
class CInputStream;
#endif

#if !RSTL_CORRUPTION
class CInt32POINode;
class CBoolPOINode;
class CSoundPOINode;
class CParticlePOINode;
#endif

class CAnimPOIData {
public:
  explicit CAnimPOIData(CInputStream& in);

  const rstl::vector< CBoolPOINode >& GetBoolPOIStream() const { return mBoolNodes; }
  const rstl::vector< CInt32POINode >& GetInt32POIStream() const { return mInt32Nodes; }
  const rstl::vector< CParticlePOINode >& GetParticlePOIStream() const { return mParticleNodes; }
  const rstl::vector< CSoundPOINode >& GetSoundPOIStream() const { return mSoundNodes; }
#if RSTL_CORRUPTION
  const rstl::vector< CAssetSoundPOINode >& GetAssetSoundPOIStream() const {
    return mAssetSoundNodes;
  }
  const rstl::vector< CAssetPOINode >& GetAssetPOIStream() const { return mAssetNodes; }
#endif

private:
  uint mVersion;
  rstl::vector< CBoolPOINode > mBoolNodes;
  rstl::vector< CInt32POINode > mInt32Nodes;
  rstl::vector< CParticlePOINode > mParticleNodes;
  rstl::vector< CSoundPOINode > mSoundNodes;
#if RSTL_CORRUPTION
  rstl::vector< CAssetSoundPOINode > mAssetSoundNodes;
  rstl::vector< CAssetPOINode > mAssetNodes;
#endif
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CAnimPOIData, 0x64)
#endif
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_CORRUPTION
#if RSTL_PRIME1
const CFactoryFnReturn AnimPOIDataFactory(const SObjectTag& tag, CInputStream& in,
                                          const CVParamTransfer& xfer);
#else
CHECK_SIZEOF(CAnimPOIData, RSTL_VERSION >= RSTL_R32J ? 0x34 : 0x44)
#endif
#endif

#endif // _CANIMPOIDATA
