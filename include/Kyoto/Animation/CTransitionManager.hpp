#ifndef _CTRANSITIONMANAGER
#define _CTRANSITIONMANAGER
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CAnimSysContext.hpp"
#include "Kyoto/Animation/CAnimTreeNode.hpp"

#if !RSTL_PRIME1
class CAnimTreeNode;
class IMetaTrans;
#endif

class CTransitionManager {
public:
  CTransitionManager(const CAnimSysContext& context);
#if !RSTL_PRIME1
  ~CTransitionManager() {}
  // Guessed name, corresponding to CTreeUtils::GetMetaTrans.
  rstl::rc_ptr< IMetaTrans > GetMetaTrans(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                          const rstl::ncrc_ptr< CAnimTreeNode >& b) const;
#endif

  rstl::ncrc_ptr< CAnimTreeNode > GetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                    const rstl::ncrc_ptr< CAnimTreeNode >& b) const;

private:
  CAnimSysContext mContext;
};

#if RSTL_PRIME1
CHECK_SIZEOF(CTransitionManager, 0x10)
#endif

#endif // _CTRANSITIONMANAGER
