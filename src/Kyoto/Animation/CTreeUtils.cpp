#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Animation/CTreeUtils.hpp"

#include "Kyoto/Animation/CAnimSysContext.hpp"
#include "Kyoto/Animation/CAnimTreeNode.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Animation/CMetaTransPhaseTrans.hpp"
#include "Kyoto/Animation/CMetaTransTrans.hpp"
#endif
#include "Kyoto/Animation/CTransitionDatabase.hpp"
#include "Kyoto/Animation/IMetaTrans.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/CAssetId.hpp"


#endif

rstl::ncrc_ptr< CAnimTreeNode >
CTreeUtils::GetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                              const rstl::ncrc_ptr< CAnimTreeNode >& b,
#if RSTL_CORRUPTION
                              const CAnimSysContext& animCtx, const bool convertPhaseTrans) {
  rstl::rc_ptr< IMetaTrans > trans = GetMetaTrans(a, b, animCtx, convertPhaseTrans);
#else
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
#endif
  return trans->GetTransitionTree(a, b, animCtx);
}

#if RSTL_CORRUPTION
rstl::rc_ptr< IMetaTrans > CTreeUtils::GetMetaTrans(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                    const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                                    const CAnimSysContext& animCtx,
                                                    const bool convertPhaseTrans) {
  CAnimTreeEffectiveContribution contribA = a->GetContributionOfHighestInfluence();
  CAnimTreeEffectiveContribution contribB = b->GetContributionOfHighestInfluence();
  rstl::rc_ptr< IMetaTrans > trans = animCtx.GetTransitionDatabase()->GetMetaTrans(
      contribA.GetPrimitive().GetResourceId(), contribB.GetPrimitive().GetResourceId());

  if (convertPhaseTrans && trans->GetType() == kMTT_PhaseTrans) {
    const CMetaTransPhaseTrans* phase = static_cast< const CMetaTransPhaseTrans* >(trans.GetPtr());
    if (phase != nullptr) {
      trans.Assign(rs_new_line(89) CMetaTransTrans(phase->GetTransDur(),
                                                   phase->GetCharacterSpaceBlend(),
                                                   phase->GetRunA(), phase->GetFlags(), 0));
}
  }

  return trans;
}
#else
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
#endif
