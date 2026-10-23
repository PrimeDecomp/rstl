#include "rstl/RstlVersions.h"

#if RSTL_RC_PTR_PRIVATE_COUNT
#include "rstl/rc_ptr.hpp"
#include "rstl/rmemory_allocator.hpp"

int rstl::rc_ptr_private::sNull = 0x00FFFFFF;

void* rstl::rmemory_allocator::allocate(int size) {
  return size == 0 ? nullptr : rs_new uchar[size];
}
#else
#if RSTL_VERSION == RSTL_G2MEAB
#include "Corruption/Kyoto/Alloc/CMemory.hpp"
#else
#include "Kyoto/Alloc/CMemory.hpp"
#endif
#include "rstl/StringExtras.hpp"
#include "rstl/math.hpp"
#include "rstl/rc_ptr.hpp"
#include "stdio.h"

#if RSTL_VERSION == RSTL_G2MEAB
#include "Corruption/Kyoto/Basics/CCast.hpp"
#else
#include "Kyoto/Basics/CCast.hpp"
#endif
#if RSTL_VERSION == RSTL_G2MEAB
#include "Corruption/Kyoto/Streams/CInputStream.hpp"
#else
#include "Kyoto/Streams/CInputStream.hpp"
#endif

#include <string.h>

namespace rstl {
#if RSTL_RC_PTR_INLINE_COUNT
int CRefData::sNull = 0x1000000 - 1;
#else
CRefData CRefData::sNull(nullptr, 0x1000000 - 1);
#endif
}
#endif
