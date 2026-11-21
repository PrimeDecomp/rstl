#ifndef _CTREEUTILS
#define _CTREEUTILS
#include "rstl/RstlVersions.h"

#include "rstl/RstlVersions.h"
#include "rstl/rc_ptr.hpp"

class CAnimTreeNode;
class CAnimSysContext;
class IMetaTrans;
class CTreeUtils {
public:
  // Guessed name; selects the transition between the highest-contributing animations.
  static rstl::rc_ptr< IMetaTrans > GetMetaTrans(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                 const rstl::ncrc_ptr< CAnimTreeNode >& b,
#if RSTL_CORRUPTION
                                                 const CAnimSysContext& animCtx,
                                                 bool convertPhaseTrans);
#else
                                                 const CAnimSysContext& animCtx);
#endif
  static rstl::ncrc_ptr< CAnimTreeNode > GetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                           const rstl::ncrc_ptr< CAnimTreeNode >& b,
#if RSTL_CORRUPTION
                                                           const CAnimSysContext& animCtx,
                                                           bool convertPhaseTrans);
#else
                                                           const CAnimSysContext& animCtx);
#endif
};

#endif // _CTREEUTILS
