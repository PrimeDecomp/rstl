#ifndef _CMETATRANSTRANS
#define _CMETATRANSTRANS
#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#endif
#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Animation/IMetaTrans.hpp"
#include "rstl/rc_ptr.hpp"

#if RSTL_CORRUPTION


class IMetaAnim;
#endif
class CInputStream;
class COutputStream;

#if !RSTL_CORRUPTION
class IMetaAnim;
#endif
class CMetaTransTrans : public IMetaTrans {
public:
#if RSTL_CORRUPTION
  CMetaTransTrans(CInputStream& in, int type);
  CMetaTransTrans(const CCharAnimTime& transDur, const bool characterSpaceBlend, const bool runA,
                  const u32 flags, const char blendCurve)
  : mTransDur(transDur)
  , mCharacterSpaceBlend(characterSpaceBlend)
  , mRunA(runA)
  , mFlags(flags)
  , mBlendCurve(blendCurve) {}

#else
  explicit CMetaTransTrans(CInputStream& in);
#endif
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
#if RSTL_CORRUPTION
  char mBlendCurve; // Guessed name
#endif
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CMetaTransTrans, 0x18)
#else
CHECK_SIZEOF(CMetaTransTrans, 0x14)
#endif

#endif // _CMETATRANSTRANS
