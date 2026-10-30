#ifndef _CMETATRANSSNAP
#define _CMETATRANSSNAP
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/IMetaTrans.hpp"

class CMetaTransSnap : public IMetaTrans {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  EMetaTransType GetType() const override { return kMTT_Snap; }
#else
  // IMetaTrans
  EMetaTransType GetType() const override;
#endif

  rstl::ncrc_ptr< CAnimTreeNode > VGetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                     const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                                     const CAnimSysContext& animSys) const override;

  void WriteTransData(COutputStream&) const override;
};

#if !RSTL_PRIME1
CHECK_SIZEOF(CMetaTransSnap, 0x4)
#endif

#endif // _CMETATRANSSNAP
