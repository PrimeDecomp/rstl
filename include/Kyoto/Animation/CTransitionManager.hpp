#ifndef _CTRANSITIONMANAGER
#define _CTRANSITIONMANAGER
#include "rstl/RstlVersions.h"

#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CAnimSysContext.hpp"
#include "Kyoto/Animation/CAnimTreeNode.hpp"

#if !RSTL_CORRUPTION
class IMetaTrans;
#endif

class CTransitionManager {
public:
#if RSTL_CORRUPTION
  explicit CTransitionManager(const CAnimSysContext& context) : mContext(context) {}
#else
  CTransitionManager(const CAnimSysContext& context);

#if !RSTL_PRIME1
  // Guessed name, corresponding to CTreeUtils::GetMetaTrans.
  rstl::rc_ptr< IMetaTrans > GetMetaTrans(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                          const rstl::ncrc_ptr< CAnimTreeNode >& b) const;
#endif

  rstl::ncrc_ptr< CAnimTreeNode > GetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                    const rstl::ncrc_ptr< CAnimTreeNode >& b) const;
#endif

private:
  CAnimSysContext mContext;
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CTransitionManager, 0x1c)
#else
CHECK_SIZEOF(CTransitionManager, RSTL_PRIME1 ? 0x10 : 0x18)
#endif

#if !RSTL_CORRUPTION
#if !RSTL_PRIME1
// FIXME: Combined with CAnimationManager, it's not being inlined correctly in Prime 1
inline CTransitionManager::CTransitionManager(const CAnimSysContext& context) : mContext(context) {}
#endif

#endif
#endif // _CTRANSITIONMANAGER
