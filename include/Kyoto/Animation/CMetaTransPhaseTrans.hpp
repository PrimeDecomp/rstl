#ifndef _CMETATRANSPHASETRANS
#define _CMETATRANSPHASETRANS
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Animation/IMetaTrans.hpp"

class CMetaTransPhaseTrans : public IMetaTrans {
public:
  explicit CMetaTransPhaseTrans(CInputStream& in);
  // IMetaTrans
  EMetaTransType GetType() const override { return kMTT_PhaseTrans; }

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
  uint mFlags;
};
CHECK_SIZEOF(CMetaTransPhaseTrans, 0x14)

#endif // _CMETATRANSPHASETRANS
