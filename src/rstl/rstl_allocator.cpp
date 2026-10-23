
#include "rstl/RstlVersions.h"
#include "rstl/rmemory_allocator.hpp"

// Echoes defines rmemory_allocator::allocate in RstlExtras.cpp.
#if !RSTL_ALLOCATOR_EXTERN_ALLOCATE
#if defined(__MWERKS__) && RSTL_EMIT_OUTLINED_ALLOCATE
void* rstl::rmemory_allocator::allocate(int size) {
  return size == 0 ? nullptr : rs_new uchar[size];
}
#endif
#endif
