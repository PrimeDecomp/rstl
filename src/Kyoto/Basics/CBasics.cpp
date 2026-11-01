#include "Kyoto/Basics/CBasics.hpp"

#include "rstl/RstlVersions.h"

#include <stdarg.h>
#include <stdio.h>

char* CBasics::Stringize(const char* fmt, va_list args) {
  static char stringizeBuffer[512];
  vsnprintf(stringizeBuffer, sizeof(stringizeBuffer), fmt, args);
  return stringizeBuffer;
}

char* CBasics::Stringize(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
#if RSTL_KYOTO_STRINGIZE_DELEGATES
  char* ret = Stringize(fmt, args);
#else
  static char stringizeBuffer[512];
#if RSTL_KYOTO_STRINGIZE_VSNPRINTF || NONMATCHING
  vsnprintf(stringizeBuffer, sizeof(stringizeBuffer), fmt, args);
#else
  vsprintf(stringizeBuffer, fmt, args);
#endif
  char* ret = stringizeBuffer;
#endif
  va_end(args);
  return ret;
}
