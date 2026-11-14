#ifndef _CLZOSUPPORT
#define _CLZOSUPPORT

#include "rstl/types.h"

class CLZOSupport {
public:
  static int Inflate(const uchar* source, uint sourceLen, uchar* dest, uint& destLen);
};

#endif // _CLZOSUPPORT
