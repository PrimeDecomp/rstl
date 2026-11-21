#ifndef _CMETATRANSMETAANIM
#define _CMETATRANSMETAANIM
#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#endif
#include "Kyoto/Animation/IMetaTrans.hpp"
#include "rstl/rc_ptr.hpp"

#if !RSTL_CORRUPTION
class CInputStream;
class COutputStream;
#endif

class IMetaAnim;
#if RSTL_CORRUPTION
class CInputStream;
class COutputStream;
#endif

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
#if RSTL_CORRUPTION
CHECK_SIZEOF(CMetaTransMetaAnim, 0xc)
#else
CHECK_SIZEOF(CMetaTransMetaAnim, RSTL_PRIME1 ? 0x8 : 0xc)
#endif

#endif // _CMETATRANSMETAANIM
