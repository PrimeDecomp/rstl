#ifndef _CMETATRANSMETAANIM
#define _CMETATRANSMETAANIM
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/IMetaTrans.hpp"
#include "rstl/rc_ptr.hpp"

class CInputStream;
class COutputStream;

class IMetaAnim;

class CMetaTransMetaAnim : public IMetaTrans {
public:
  explicit CMetaTransMetaAnim(CInputStream& in);
  // IMetaTrans
  EMetaTransType GetType() const override { return kMTT_MetaAnim; }

  rstl::ncrc_ptr< CAnimTreeNode > VGetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                     const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                                     const CAnimSysContext& animSys) const override;

  void WriteTransData(COutputStream&) const override;

private:
  rstl::rc_ptr< IMetaAnim > mMetaAnim;
};
CHECK_SIZEOF(CMetaTransMetaAnim, RSTL_PRIME1 ? 0x8 : 0xc)

#endif // _CMETATRANSMETAANIM
