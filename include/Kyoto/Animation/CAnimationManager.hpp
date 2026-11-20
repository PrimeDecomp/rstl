#ifndef _CANIMATIONMANAGER
#define _CANIMATIONMANAGER
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CAnimSysContext.hpp"
#include "Kyoto/Animation/CAnimationDatabase.hpp"

#include "Kyoto/TToken.hpp"

class CAnimationDatabase;
class CAnimTreeNode;

class CMetaAnimTreeBuildOrders;
class CAnimTreeContext;

class IMetaAnim;
class CAnimationManager {
public:
  CAnimationManager(const TToken< CAnimationDatabase >& animDB, const CAnimSysContext& sysCtx);

  rstl::ncrc_ptr< CAnimTreeNode > GetAnimationTree(uint animIdx,
                                                   const CMetaAnimTreeBuildOrders& orders) const;

  rstl::rc_ptr< IMetaAnim > GetMetaAnimation(uint animIdx) const;
  uint GetNumMetaAnims() const; // Guessed name, stands in for a function stripped from the link.

private:
  void ForceOrdering();

  TToken< CAnimationDatabase > mAnimDB;
  CAnimSysContext mSysCtx;
};
CHECK_SIZEOF(CAnimationManager, RSTL_PRIME1 ? 0x18 : 0x20)

#if !RSTL_PRIME1
// FIXME: it's not being inlined correctly in Prime 1
inline CAnimationManager::CAnimationManager(const TToken< CAnimationDatabase >& animDB,
                                            const CAnimSysContext& sysCtx)
: mAnimDB(animDB), mSysCtx(sysCtx) {}
#endif

#endif // _CANIMATIONMANAGER
