#include "rstl/RstlVersions.h"
#if RSTL_CORRUPTION
#define RS_TTOKEN_FILE "TToken.H"

#else
#include "rstl/RstlVersions.h"
#endif
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Animation/CAnimPOIData.hpp"

#if RSTL_CORRUPTION
#include "Kyoto/CFactoryMgr.hpp"
#else
#include "Kyoto/Animation/CBoolPOINode.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"

#endif
#include "Kyoto/Streams/CInputStream.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Streams/CMemoryInStream.hpp"


#else
#include "rstl/vector.hpp"
#if RSTL_PRIME1
#include "dolphin/types.h"
#endif
#endif

CAnimPOIData::CAnimPOIData(CInputStream& in)
: mVersion(in.Get< uint >()), mBoolNodes(in), mInt32Nodes(in), mParticleNodes(in) {
  if (mVersion > 1) {
#if RSTL_CORRUPTION
    mSoundNodes = rstl::vector< CSoundPOINode >(in);
  }
  if (mVersion > 2) {
    mAssetSoundNodes = rstl::vector< CAssetSoundPOINode >(in);
  }
  if (mVersion > 3) {
    mAssetNodes = rstl::vector< CAssetPOINode >(in);
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    mSoundNodes = in;
#else
    mSoundNodes = rstl::vector< CSoundPOINode >(in);
#endif
#endif
  }
}

#if RSTL_CORRUPTION
// Guessed name
static CFactoryFnReturn AnimPOIDataFactory(const SObjectTag& tag,
                                         const rstl::auto_ptr< uchar >& buffer, int size,
                                    const CVParamTransfer& xfer) {
  CMemoryInStream in(buffer.get(), size);
  return rs_new_line(238) CAnimPOIData(in);
}
#else
#if RSTL_PRIME1
const CFactoryFnReturn AnimPOIDataFactory(const SObjectTag& tag, CInputStream& in,
                                    const CVParamTransfer& xfer) {
  return rs_new CAnimPOIData(in);
}
#endif
#endif
