#ifndef _CMETATRANSTRANS
#define _CMETATRANSTRANS
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Animation/IMetaTrans.hpp"
#include "rstl/rc_ptr.hpp"

class CInputStream;
class COutputStream;

class IMetaAnim;
class CMetaTransTrans : public IMetaTrans {
public:
  explicit CMetaTransTrans(CInputStream& in);
  // IMetaTrans
  EMetaTransType GetType() const override { return kMTT_Trans; }

  rstl::ncrc_ptr< CAnimTreeNode > VGetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                     const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                                     const CAnimSysContext& animSys) const override;

  void WriteTransData(COutputStream&) const override;

private:
  CCharAnimTime mTransDur;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  bool xc_;
#else
  bool mCharacterSpaceBlend;
#endif

  bool mRunA;
  u32 mFlags;
};
CHECK_SIZEOF(CMetaTransTrans, 0x14)

#endif // _CMETATRANSTRANS
