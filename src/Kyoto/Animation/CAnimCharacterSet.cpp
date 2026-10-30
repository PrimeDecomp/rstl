#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CAnimCharacterSet.hpp"
#include "Kyoto/CFactoryFnReturn.hpp"
#include "Kyoto/CFactoryMgr.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CAnimCharacterSet::CAnimCharacterSet(CInputStream& in)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
: mVersion(in.Get< short >())
#else
: mVersion(in.Get< ushort >())
#endif
, mCharacterSet(in)
, mAnimationSet(in) {}

// TODO: verify that this Prime/Echoes difference is real
RSTL_PRIME1_CONST CFactoryFnReturn FAnimCharacterSet(const SObjectTag& tag, CInputStream& in,
                                   const CVParamTransfer& xfer) {
  return rs_new CAnimCharacterSet(in);
}

