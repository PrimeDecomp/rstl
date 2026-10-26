#include "rstl/RstlVersions.h"

#if RSTL_STRING_EXTRAS_IN_RSTL_EXTRAS
#include "rstl/StringExtras.hpp"

#include "Kyoto/Alloc/Assert.hpp"
#include "Corruption/Kyoto/CFactoryMgr.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "rstl/vector.hpp"

char CStringExtras::ConvertToLowerCase(char c) {
  if (c >= 'A' && c <= 'Z') {
    return (c + ('a' - 'A')) & 0xff;
  }
  return c;
}

int CStringExtras::ConvertToInteger(const rstl::string& str) {
  rstl::string::const_iterator begin = str.begin();
  rstl::string::const_iterator end = str.end();
  while (begin != end && *begin == ' ') {
    ++begin;
  }
  while (end != begin && *(end - 1) == ' ') {
    end = end - 1;
  }
  if (end == begin) {
    RS_VERIFY_THROW_IN("CStringExtras.cpp", 447, false, kException_NotInt, "kException_NotInt");
  }

  int result = 0;
  int scale = 1;
  do {
    end -= 1;
    const char c = *end;
    if (c >= '0' && c <= '9') {
      result += scale * (c - '0');
      scale *= 10;
    } else if (c == '-' || c == '+') {
      if (c == '-') {
        result *= -1;
      }
      if (end == begin) {
        return result;
      }
      RS_VERIFY_THROW_IN("CStringExtras.cpp", 482, false, kException_NotInt, "kException_NotInt");
    } else {
      RS_VERIFY_THROW_IN("CStringExtras.cpp", 488, false, kException_NotInt, "kException_NotInt");
    }
  } while (end != begin);
  return result;
}

CFactoryFnReturn FAudioTranslationTableFactory(const SObjectTag&,
                                               const rstl::auto_ptr< uchar >& buffer, int size,
                                               const CVParamTransfer&) {
  CMemoryInStream in(buffer.get(), size);
  return rs_new_line_in("CStringExtras.cpp", 1141) rstl::vector< short >(in);
}
#elif RSTL_RC_PTR_PRIVATE_COUNT
#include "rstl/rc_ptr.hpp"
#include "rstl/rmemory_allocator.hpp"

int rstl::rc_ptr_private::sNull = 0x00FFFFFF;

void* rstl::rmemory_allocator::allocate(int size) {
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
