#include "rstl/locked_cache_allocator.hpp"

// Guessed name. The only observed consumer is the static initializer, which clears every flag and
// the allocation count of the object; mode 1 is neither the cache-first nor the heap-first mode.
static rstl::locked_cache_allocator sLockedCacheAllocator(1);
