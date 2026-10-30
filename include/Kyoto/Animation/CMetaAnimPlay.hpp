#ifndef _CMETAANIMPLAY
#define _CMETAANIMPLAY
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Animation/CPrimitive.hpp"
#include "Kyoto/Animation/IMetaAnim.hpp"

class CMetaAnimPlay : public IMetaAnim {
public:
#if !RSTL_PRIME1
  explicit CMetaAnimPlay(CInputStream& in) : mPrimitive(in), mStartTime(in) {}

  // IMetaAnim
  ~CMetaAnimPlay() override {}
  void GetUniquePrimitives(rstl::set< CPrimitive >& primsOut) const override;
#endif

  EMetaAnimType GetType() const override { return kMAT_Play; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void GetUniquePrimitives(rstl::set< CPrimitive >& primsOut) const override;
#else
  void WriteAnimData(COutputStream& out) const override;
#endif

  rstl::ncrc_ptr< CAnimTreeNode >
  VGetAnimationTree(const CAnimSysContext& animSys,
                    const CMetaAnimTreeBuildOrders& orders) const override;
#if RSTL_PRIME1
  void WriteAnimData(COutputStream& out) const override;

  explicit CMetaAnimPlay(CInputStream& in) : mPrimitive(in), mStartTime(in) {}
#endif

private:
  CPrimitive mPrimitive;
  CCharAnimTime mStartTime;
};
CHECK_SIZEOF(CMetaAnimPlay, 0x24)

#endif // _CMETAANIMPLAY
