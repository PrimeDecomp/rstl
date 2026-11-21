#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif
#include "Kyoto/Animation/CMetaTransFactory.hpp"
#include "Kyoto/Alloc/CMemory.hpp"

#include "Kyoto/Animation/CMetaTransMetaAnim.hpp"
#include "Kyoto/Animation/CMetaTransPhaseTrans.hpp"
#include "Kyoto/Animation/CMetaTransSnap.hpp"
#include "Kyoto/Animation/CMetaTransTrans.hpp"
#include "Kyoto/Animation/IMetaTrans.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

rstl::rc_ptr< IMetaTrans > CMetaTransFactory::CreateMetaTrans(CInputStream& in) {
#if RSTL_CORRUPTION
  EMetaTransType type = static_cast< EMetaTransType >(in.ReadInt32());
#else
  EMetaTransType type = static_cast< EMetaTransType >(in.ReadLong());
#endif
  switch (type) {
  case kMTT_MetaAnim:
#if RSTL_CORRUPTION
    return rs_new_line(19) CMetaTransMetaAnim(in);
  case 1:
#else
    return rs_new CMetaTransMetaAnim(in);
#endif
  case kMTT_Trans:
#if RSTL_CORRUPTION
    return rs_new_line(26) CMetaTransTrans(in, type);
#else
    return rs_new CMetaTransTrans(in);
#endif
  case kMTT_PhaseTrans:
#if RSTL_CORRUPTION
    return rs_new_line(32) CMetaTransPhaseTrans(in);
#else
    return rs_new CMetaTransPhaseTrans(in);
#endif
  case kMTT_Snap:
#if RSTL_CORRUPTION
    return rs_new_line(38) CMetaTransSnap;
#else
    return rs_new CMetaTransSnap;
#endif
  default:
    return rstl::rc_ptr< IMetaTrans >();
  }
}
