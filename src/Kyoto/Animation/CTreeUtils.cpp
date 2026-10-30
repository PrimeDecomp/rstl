#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CTreeUtils.hpp"

#include "Kyoto/Animation/CAnimSysContext.hpp"
#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "Kyoto/Animation/CTransitionDatabase.hpp"
#include "Kyoto/Animation/IMetaTrans.hpp"

rstl::ncrc_ptr< CAnimTreeNode >
CTreeUtils::GetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                              const rstl::ncrc_ptr< CAnimTreeNode >& b,
                              const CAnimSysContext& animCtx) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CAnimTreeEffectiveContribution contribA = a->GetContributionOfHighestInfluence();
  CAnimTreeEffectiveContribution contribB = b->GetContributionOfHighestInfluence();
  const rstl::rc_ptr< IMetaTrans >& trans =
      animCtx.GetTransitionDatabase().NonConstCopy()->GetMetaTrans(
          contribA.GetAnimDatabaseIndex(), contribB.GetAnimDatabaseIndex());
#else
  rstl::rc_ptr< IMetaTrans > trans = GetMetaTrans(a, b, animCtx);
#endif
  return trans->GetTransitionTree(a, b, animCtx);
}

#if !RSTL_PRIME1
rstl::rc_ptr< IMetaTrans > CTreeUtils::GetMetaTrans(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                    const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                                    const CAnimSysContext& animCtx) {
  CAnimTreeEffectiveContribution contribA = a->GetContributionOfHighestInfluence();
  CAnimTreeEffectiveContribution contribB = b->GetContributionOfHighestInfluence();
  return animCtx.GetTransitionDatabase().NonConstCopy()->GetMetaTrans(
      contribA.GetAnimDatabaseIndex(), contribB.GetAnimDatabaseIndex());
}
#endif

