#ifndef _RSTL_LOCKED_CACHE_ALLOCATOR_IMPL
#define _RSTL_LOCKED_CACHE_ALLOCATOR_IMPL

#include "rstl/locked_cache_allocator.hpp"

namespace rstl {
inline void locked_cache_allocator::Allocate(void*& out, int size) {
  if (size == 0) {
    out = 0;
    return;
  }
  ++mAllocationCount;
  if (mPreferHeap) {
    if (mAllocationCount == 2)
      mPreviousHeapAllocation = true;
    else
      mHeapAllocation = true;
    out = rs_aligned_new_line_in("lc_allocator.h", 71, size);
  } else {
    out = AllocateLockedCache32(size);
    if (mHeapAfterCacheAttempt)
      mPreferHeap = true;
    if (out == 0) {
      --mAllocationCount;
      mPreferHeap = true;
      Allocate(out, size);
    }
  }
}

} // namespace rstl

#endif // _RSTL_LOCKED_CACHE_ALLOCATOR_IMPL
