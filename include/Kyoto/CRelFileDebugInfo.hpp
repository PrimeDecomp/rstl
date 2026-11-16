#ifndef _CRELFILEDEBUGINFO
#define _CRELFILEDEBUGINFO

#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#include "rstl/types.h"
#include <stdint.h>

#if RSTL_KYOTO_REL_DEBUG_INT_ADDRESS
// Guessed name
typedef int TRelFileDebugAddress;
// Guessed name
typedef uint TRelFileDebugStart;
#else
// Guessed name
typedef uintptr_t TRelFileDebugAddress;
// Guessed name
typedef uintptr_t TRelFileDebugStart;
#endif

// Guessed name
class CRelFileDebugInfo {
public:
  CRelFileDebugInfo();
  ~CRelFileDebugInfo();

  void Register(const char* name, const void* start, uint size);
  void Unregister();
  bool Contains(TRelFileDebugAddress address) const;

  const char* GetName() const { return mName; }
  TRelFileDebugStart GetStart() const { return mStart; }

  static CRelFileDebugInfo* FindByAddress(TRelFileDebugAddress address);

private:
  const char* mName;
  TRelFileDebugStart mStart;
  uint mSize;
  CRelFileDebugInfo* mNext;
  CRelFileDebugInfo* mPrev;

  static CRelFileDebugInfo* mspHead;
};
CHECK_SIZEOF(CRelFileDebugInfo, 0x14)

#endif // _CRELFILEDEBUGINFO
