#ifndef _CANIMPOIDATA
#define _CANIMPOIDATA
#include "rstl/RstlVersions.h"

#include "Kyoto/CFactoryFnReturn.hpp"
#include "rstl/optional_object.hpp"
#include "Kyoto/Animation/CBoolPOINode.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"

#include "rstl/vector.hpp"

#if RSTL_PRIME1
class CInputStream;
#endif

#if RSTL_PRIME1
class CInt32POINode;
class CBoolPOINode;
class CSoundPOINode;
class CParticlePOINode;
#endif

class CAnimPOIData {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CAnimPOIData(CInputStream& in);
#else
  explicit CAnimPOIData(CInputStream& in);
#endif

  const rstl::vector< CBoolPOINode >& GetBoolPOIStream() const { return mBoolNodes; }
  const rstl::vector< CInt32POINode >& GetInt32POIStream() const { return mInt32Nodes; }
  const rstl::vector< CParticlePOINode >& GetParticlePOIStream() const { return mParticleNodes; }
  const rstl::vector< CSoundPOINode >& GetSoundPOIStream() const { return mSoundNodes; }

private:
  uint mVersion;
  rstl::vector< CBoolPOINode > mBoolNodes;
  rstl::vector< CInt32POINode > mInt32Nodes;
  rstl::vector< CParticlePOINode > mParticleNodes;
  rstl::vector< CSoundPOINode > mSoundNodes;
};
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
const CFactoryFnReturn AnimPOIDataFactory(const SObjectTag& tag, CInputStream& in,
                                    const CVParamTransfer& xfer);
#else
CHECK_SIZEOF(CAnimPOIData, RSTL_VERSION >= RSTL_R32J ? 0x34 : 0x44)
#endif

#endif // _CANIMPOIDATA
