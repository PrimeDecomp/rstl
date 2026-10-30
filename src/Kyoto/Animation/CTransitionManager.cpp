#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CTransitionManager.hpp"
#include "Kyoto/Animation/CTreeUtils.hpp"
#if !RSTL_PRIME1
#include "Kyoto/Animation/IMetaTrans.hpp"
#endif

rstl::ncrc_ptr< CAnimTreeNode >
CTransitionManager::GetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                      const rstl::ncrc_ptr< CAnimTreeNode >& b) const {

  return CTreeUtils::GetTransitionTree(a, b, mContext);
}

#if !RSTL_PRIME1
rstl::rc_ptr< IMetaTrans >
CTransitionManager::GetMetaTrans(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                 const rstl::ncrc_ptr< CAnimTreeNode >& b) const {
  return CTreeUtils::GetMetaTrans(a, b, mContext);
}
#endif

