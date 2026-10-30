#ifndef _CELEMENTALLOCATIONCHUNK
#define _CELEMENTALLOCATIONCHUNK

#include "rstl/RstlVersions.h"
#include "types.h"

class CElementAllocationChunk {
public:
  CElementAllocationChunk();

  bool CanAllocate(uint size) const;
  bool Contains(const void* ptr) const;
  void* Allocate(uint size);
  void Free(void* ptr);
  void Rewind(uint size);
  uint GetAllocatedSize() const;
  uint GetAllocationCount() const;

private:
  uint mCapacity;
  uint mAllocatedWords;
  uint mAllocationCount;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  ALIGNAS(16) uint mData[256];
#else
  uint mData[256];
#endif
};
CHECK_SIZEOF(CElementAllocationChunk, 0x40c)

#endif // _CELEMENTALLOCATIONCHUNK
