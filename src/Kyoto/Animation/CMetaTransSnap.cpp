#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CMetaTransSnap.hpp"

rstl::ncrc_ptr< CAnimTreeNode >
CMetaTransSnap::VGetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                   const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                   const CAnimSysContext& animSys) const {
  return b;
}

void CMetaTransSnap::WriteTransData(COutputStream& out) const {}

#if !RSTL_PRIME1
EMetaTransType CMetaTransSnap::GetType() const { return kMTT_Snap; }
#endif

