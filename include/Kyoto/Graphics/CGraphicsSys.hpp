#ifndef _CGRAPHICSSYS
#define _CGRAPHICSSYS
#include "rstl/RstlVersions.h"

#include "types.h"

class COsContext;
class CMemorySys;

class CGraphicsSys {
public:
  CGraphicsSys(const COsContext& osContext, const CMemorySys& memorySys, uint, void*);
  CGraphicsSys(const COsContext& osContext, const CMemorySys& memorySys);
  CGraphicsSys(const COsContext& osContext, const CMemorySys& memorySys, bool progressive);
  ~CGraphicsSys();

private:
#if RSTL_PRIME1
  uint pad;
#endif

  static bool mGraphicsInitialized;
};

#endif // _CGRAPHICSSYS
