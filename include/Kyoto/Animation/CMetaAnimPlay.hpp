#ifndef _CMETAANIMPLAY
#define _CMETAANIMPLAY
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#endif

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#endif
#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Animation/CPrimitive.hpp"
#include "Kyoto/Animation/IMetaAnim.hpp"

class CMetaAnimPlay : public IMetaAnim {
public:
#if RSTL_CORRUPTION
  explicit CMetaAnimPlay(CInputStream& in) : mPrimitive(in), mStartTime(in) {}

  // IMetaAnim
  ~CMetaAnimPlay() override {}
  void GetUniquePrimitives(rstl::set< CPrimitive >& primsOut) const override;
#else
#if !RSTL_PRIME1
  explicit CMetaAnimPlay(CInputStream& in) : mPrimitive(in), mStartTime(in) {}

  // IMetaAnim
  ~CMetaAnimPlay() override {}
  void GetUniquePrimitives(rstl::set< CPrimitive >& primsOut) const override;
#endif

#endif
  EMetaAnimType GetType() const override { return kMAT_Play; }
#if RSTL_CORRUPTION
  void WriteAnimData(COutputStream& out) const override;
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void GetUniquePrimitives(rstl::set< CPrimitive >& primsOut) const override;
#else
  void WriteAnimData(COutputStream& out) const override;
#endif

#endif
  rstl::ncrc_ptr< CAnimTreeNode >
  VGetAnimationTree(const CAnimSysContext& animSys,
                    const CMetaAnimTreeBuildOrders& orders) const override;
#if !RSTL_CORRUPTION
#if RSTL_PRIME1
  void WriteAnimData(COutputStream& out) const override;

  explicit CMetaAnimPlay(CInputStream& in) : mPrimitive(in), mStartTime(in) {}
#endif
#endif

private:
  CPrimitive mPrimitive;
  CCharAnimTime mStartTime;
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CMetaAnimPlay, 0x30)
#else
CHECK_SIZEOF(CMetaAnimPlay, 0x24)
#endif

#endif // _CMETAANIMPLAY
