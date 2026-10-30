#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CMetaTransFactory.hpp"
#if RSTL_PRIME1
#include "Kyoto/Alloc/CMemory.hpp"
#endif

#include "Kyoto/Animation/CMetaTransMetaAnim.hpp"
#include "Kyoto/Animation/CMetaTransPhaseTrans.hpp"
#include "Kyoto/Animation/CMetaTransSnap.hpp"
#include "Kyoto/Animation/CMetaTransTrans.hpp"
#include "Kyoto/Animation/IMetaTrans.hpp"
#if !RSTL_PRIME1
#include "Kyoto/Alloc/CMemory.hpp"
#endif

#include "Kyoto/Streams/CInputStream.hpp"

rstl::rc_ptr< IMetaTrans > CMetaTransFactory::CreateMetaTrans(CInputStream& in) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  EMetaTransType type = static_cast< EMetaTransType >(in.ReadLong());
#else
  EMetaTransType type = static_cast< EMetaTransType >(in.ReadInt32());
#endif

  switch (type) {
  case kMTT_MetaAnim:
    return rs_new CMetaTransMetaAnim(in);
  case kMTT_Trans:
    return rs_new CMetaTransTrans(in);
  case kMTT_PhaseTrans:
    return rs_new CMetaTransPhaseTrans(in);
  case kMTT_Snap:
    return rs_new CMetaTransSnap;
  default:
    return rstl::rc_ptr< IMetaTrans >();
  }
}

