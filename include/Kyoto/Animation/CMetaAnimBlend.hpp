#ifndef _CMETAANIMBLEND
#define _CMETAANIMBLEND
#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#endif
#include "Kyoto/Animation/IMetaAnim.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/vector.hpp"

class CMetaAnimBlend : public IMetaAnim {
public:
  EMetaAnimType GetType() const override { return kMAT_Blend; }

  void GetUniquePrimitives(rstl::set< CPrimitive >& primsOut) const override;
  rstl::ncrc_ptr< CAnimTreeNode >
  VGetAnimationTree(const CAnimSysContext& animSys,
                    const CMetaAnimTreeBuildOrders& orders) const override;

  void WriteAnimData(COutputStream& out) const override;

  explicit CMetaAnimBlend(CInputStream& in);

private:
  rstl::rc_ptr< IMetaAnim > mAnimA;
  rstl::rc_ptr< IMetaAnim > mAnimB;
  float mBlend;
  bool mCharacterSpaceBlend;
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CMetaAnimBlend, 0x1c)
#else
CHECK_SIZEOF(CMetaAnimBlend, RSTL_PRIME1 ? 0x14 : 0x1c)
#endif

#endif // _CMETAANIMBLEND
