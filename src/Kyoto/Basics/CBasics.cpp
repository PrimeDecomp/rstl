#include "Kyoto/Basics/CBasics.hpp"

#include "rstl/RstlVersions.h"

#include <stdarg.h>
#include <stdio.h>

char* CBasics::Stringize(const char* fmt, ...) {
  static char stringizeBuffer[512];

  va_list args;
  va_start(args, fmt);
#if RSTL_KYOTO_STRINGIZE_VSNPRINTF || NONMATCHING
  vsnprintf(stringizeBuffer, sizeof(stringizeBuffer), fmt, args);
#else
  vsprintf(stringizeBuffer, fmt, args);
#endif
  va_end(args);

  return stringizeBuffer;
}
