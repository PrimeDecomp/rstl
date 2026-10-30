#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CAnimPOIData.hpp"

#if RSTL_PRIME1
#include "Kyoto/Animation/CBoolPOINode.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"
#endif

#include "Kyoto/Streams/CInputStream.hpp"
#if RSTL_PRIME1
#include "dolphin/types.h"
#include "rstl/vector.hpp"
#endif

CAnimPOIData::CAnimPOIData(CInputStream& in)
: mVersion(in.Get< uint >()), mBoolNodes(in), mInt32Nodes(in), mParticleNodes(in) {
  if (mVersion > 1) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    mSoundNodes = in;
#else
    mSoundNodes = rstl::vector< CSoundPOINode >(in);
#endif

  }
}

#if RSTL_PRIME1
const CFactoryFnReturn AnimPOIDataFactory(const SObjectTag& tag, CInputStream& in,
                                    const CVParamTransfer& xfer) {
  return rs_new CAnimPOIData(in);
}
#endif

