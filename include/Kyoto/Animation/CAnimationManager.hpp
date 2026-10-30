#ifndef _CANIMATIONMANAGER
#define _CANIMATIONMANAGER
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CAnimSysContext.hpp"
#include "Kyoto/Animation/CAnimationDatabase.hpp"

#include "Kyoto/TToken.hpp"

#if !RSTL_PRIME1
class CAnimationDatabase;
class CAnimTreeNode;
#endif

class CMetaAnimTreeBuildOrders;
#if RSTL_PRIME1
class CAnimTreeContext;
#endif

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

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::ncrc_ptr< CAnimTreeNode >
  GetAnimationTree(uint animIdx, const CMetaAnimTreeBuildOrders& orders) const;
#else
  rstl::ncrc_ptr< CAnimTreeNode > GetAnimationTree(uint animIdx,
                                                   const CMetaAnimTreeBuildOrders& orders) const;
#endif

  rstl::rc_ptr< IMetaAnim > GetMetaAnimation(uint animIdx) const;
#if !RSTL_PRIME1
  uint GetNumMetaAnims() const; // Guessed name, stands in for a function stripped from the link.
#endif

private:
#if RSTL_PRIME1
  void ForceOrdering();
#endif

  TToken< CAnimationDatabase > mAnimDB;
  CAnimSysContext mSysCtx;
};
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION < RSTL_R3IJ
CHECK_SIZEOF(CAnimationManager, 0x18)
#endif
#else
CHECK_SIZEOF(CAnimationManager, 0x20)
#endif

#endif // _CANIMATIONMANAGER
