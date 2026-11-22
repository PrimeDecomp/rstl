#ifndef _CGRAPHICSSYS
#define _CGRAPHICSSYS

#include "rstl/RstlVersions.h"
#include "rstl/types.h"
#include "Kyoto/Compat.hpp"

class COsContext;
class CMemorySys;

class CGraphicsSys {
public:
  explicit CGraphicsSys(const COsContext& osContext); // PC-specific
  CGraphicsSys(const COsContext& osContext MEMORY_SYS_ARGUMENT_DECL, uint, void*);
  CGraphicsSys(const COsContext& osContext, const CMemorySys& memorySys);
  CGraphicsSys(const COsContext& osContext, const CMemorySys& memorySys, bool progressive);
  ~CGraphicsSys();
  // Guessed name
  static bool IsInitialized() { return mGraphicsInitialized; }

private:
#if RSTL_PRIME1
  uint pad;
#endif

  static bool mGraphicsInitialized;
};

#endif // _CGRAPHICSSYS
