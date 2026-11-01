#include "Kyoto/Basics/CBasics.hpp"

#include "rstl/RstlVersions.h"

#include <stdarg.h>
#include <stdio.h>

char* CBasics::Stringize(const char* fmt, va_list args) {
  static char stringizeBuffer[512];
  vsnprintf(stringizeBuffer, sizeof(stringizeBuffer), fmt, args);
  return stringizeBuffer;
}

#if RSTL_VERSION >= RSTL_G2MEAB
char* CBasics::Stringize(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char* ret = Stringize(fmt, args);
  va_end(args);

  return ret;
}
#else
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
#endif
