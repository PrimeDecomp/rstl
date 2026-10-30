#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CAnimCharacterSet.hpp"
#include "Kyoto/CFactoryFnReturn.hpp"
#include "Kyoto/CFactoryMgr.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CAnimCharacterSet::CAnimCharacterSet(CInputStream& in)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
: mVersion(in.Get< short >()), mCharacterSet(in), mAnimationSet(in) {}
#else
: mVersion(in.Get< ushort >()), mCharacterSet(in), mAnimationSet(in) {}
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
const CFactoryFnReturn FAnimCharacterSet(const SObjectTag& tag, CInputStream& in,
#else
CFactoryFnReturn FAnimCharacterSet(const SObjectTag& tag, CInputStream& in,
#endif

                                   const CVParamTransfer& xfer) {
  return rs_new CAnimCharacterSet(in);
}

