#ifndef _RSTL_LOCKED_CACHE_ALLOCATOR_IMPL
#define _RSTL_LOCKED_CACHE_ALLOCATOR_IMPL

#include "rstl/locked_cache_allocator.hpp"

namespace rstl {
template < typename T >
#if RSTL_LOCKED_CACHE_TYPED_ALLOCATE
inline void locked_cache_allocator::allocate(T*& out, int size)
#else
inline void locked_cache_allocator::Allocate(T*& out, int size)
#endif
{
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
    out = static_cast< T* >(rs_aligned_new_line_in("lc_allocator.h", 71, size));
  } else {
    out = static_cast< T* >(AllocateLockedCache32(size));
    if (mHeapAfterCacheAttempt)
      mPreferHeap = true;
    if (out == 0) {
      --mAllocationCount;
      mPreferHeap = true;
#if RSTL_LOCKED_CACHE_TYPED_ALLOCATE
      allocate(out, size);
#else
      Allocate(out, size);
#endif
    }
  }
}

} // namespace rstl

#endif // _RSTL_LOCKED_CACHE_ALLOCATOR_IMPL
