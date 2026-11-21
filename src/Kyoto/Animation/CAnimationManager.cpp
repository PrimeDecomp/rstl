#include "rstl/RstlVersions.h"
#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CAnimationManager.hpp"
#include "Kyoto/Animation/CAnimationDatabase.hpp"

#include "Kyoto/Animation/IMetaAnim.hpp"

rstl::ncrc_ptr< CAnimTreeNode >
CAnimationManager::GetAnimationTree(uint animIdx, const CMetaAnimTreeBuildOrders& orders) const {
#if RSTL_CORRUPTION
  return mAnimDB->GetMetaAnim(animIdx)->GetAnimationTree(mSysCtx, orders);
#else
  const rstl::rc_ptr< IMetaAnim >& anim = mAnimDB.NonConstCopy()->GetMetaAnim(animIdx);
  return anim->GetAnimationTree(mSysCtx, orders);
#endif
}

rstl::rc_ptr< IMetaAnim > CAnimationManager::GetMetaAnimation(uint animIdx) const {
#if RSTL_CORRUPTION
  return mAnimDB->GetMetaAnim(animIdx);
#else
  return mAnimDB.NonConstCopy()->GetMetaAnim(animIdx);
#endif
}
#if !RSTL_CORRUPTION

#if !RSTL_PRIME1
uint CAnimationManager::GetNumMetaAnims() const {
  return mAnimDB.NonConstCopy()->GetNumMetaAnims();
}
#endif

#if RSTL_PRIME1
#if RSTL_VERSION < RSTL_R3ME_00
void CAnimationManager::ForceOrdering() {
  mAnimDB.NonConstCopy();
}
#endif
#endif
#endif
