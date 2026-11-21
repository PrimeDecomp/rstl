#ifndef _CANIMATIONMANAGER
#define _CANIMATIONMANAGER
#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#endif
#include "Kyoto/Animation/CAnimSysContext.hpp"
#include "Kyoto/Animation/CAnimationDatabase.hpp"

#include "Kyoto/TToken.hpp"

#if !RSTL_CORRUPTION
class CAnimationDatabase;
class CAnimTreeNode;
#endif

#if RSTL_CORRUPTION
class CAnimTreeNode;
#endif
class CMetaAnimTreeBuildOrders;
#if !RSTL_CORRUPTION
class CAnimTreeContext;

#endif
class IMetaAnim;
class CAnimationManager {
public:
#if RSTL_CORRUPTION
  CAnimationManager(const rstl::rc_ptr< CAnimationDatabase >& animDB, const CAnimSysContext& sysCtx)
  : mAnimDB(animDB), mSysCtx(sysCtx) {}
#else
  CAnimationManager(const TToken< CAnimationDatabase >& animDB, const CAnimSysContext& sysCtx);
#endif

  rstl::ncrc_ptr< CAnimTreeNode > GetAnimationTree(uint animIdx,
                                                   const CMetaAnimTreeBuildOrders& orders) const;

  rstl::rc_ptr< IMetaAnim > GetMetaAnimation(uint animIdx) const;
#if !RSTL_CORRUPTION
  uint GetNumMetaAnims() const; // Guessed name, stands in for a function stripped from the link.
#endif

private:
#if RSTL_CORRUPTION
  rstl::rc_ptr< CAnimationDatabase > mAnimDB;
#else
  void ForceOrdering();

  TToken< CAnimationDatabase > mAnimDB;
#endif
  CAnimSysContext mSysCtx;
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CAnimationManager, 0x24)
#else
CHECK_SIZEOF(CAnimationManager, RSTL_PRIME1 ? 0x18 : 0x20)
#endif

#if !RSTL_CORRUPTION
#if !RSTL_PRIME1
// FIXME: it's not being inlined correctly in Prime 1
inline CAnimationManager::CAnimationManager(const TToken< CAnimationDatabase >& animDB,
                                            const CAnimSysContext& sysCtx)
: mAnimDB(animDB), mSysCtx(sysCtx) {}
#endif

#endif
#endif // _CANIMATIONMANAGER
