#ifndef _CANIMATIONMANAGER
#define _CANIMATIONMANAGER
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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CAnimationManager(const TToken< CAnimationDatabase >& animDB, const CAnimSysContext& sysCtx);
#else
  CAnimationManager(TToken< CAnimationDatabase > animDB, const CAnimSysContext& sysCtx)
  : mAnimDB(animDB), mSysCtx(sysCtx) {}
  ~CAnimationManager() {}
#endif

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

#endif // _CANIMATIONMANAGER
