#include "rstl/RstlVersions.h"

#if RSTL_RC_PTR_PRIVATE_COUNT || RSTL_ALLOCATOR_GLOBAL_ALLOCATE
#include "rstl/rc_ptr.hpp"
#include "rstl/rmemory_allocator.hpp"

int rstl::rc_ptr_private::sNull = 0x00FFFFFF;

#if RSTL_ALLOCATOR_GLOBAL_ALLOCATE
void* rstl::rmemory_allocator::global_allocate(int size)
#else
void* rstl::rmemory_allocator::allocate(int size)
#endif
{
  return size == 0 ? nullptr : rs_new uchar[size];
}
#else
#include "Kyoto/Alloc/CMemory.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/math.hpp"
#include "rstl/rc_ptr.hpp"
#include "stdio.h"

#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

#include <string.h>

namespace rstl {
#if RSTL_RC_PTR_INLINE_COUNT
int rc_ptr_private::sNull = 0x1000000 - 1;
#else
CRefData CRefData::sNull(nullptr, 0x1000000 - 1);
#endif
}
#endif
