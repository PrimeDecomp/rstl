#ifndef _CGRAPHICSSYS
#define _CGRAPHICSSYS
#include "rstl/RstlVersions.h"

#include "types.h"

class COsContext;
class CMemorySys;

class CGraphicsSys {
public:
#if RSTL_PRIME1
  CGraphicsSys(const COsContext& osContext, const CMemorySys& memorySys, uint, void*);
#endif

  CGraphicsSys(const COsContext& osContext, const CMemorySys& memorySys, bool progressive);
  ~CGraphicsSys();

private:
#if RSTL_PRIME1
  uint pad;
#endif

  static bool mGraphicsInitialized;
};

#endif // _CGRAPHICSSYS
