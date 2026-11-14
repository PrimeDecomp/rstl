#ifndef _CCRC32
#define _CCRC32
#include "rstl/types.h"

class CCRC32 {
public:
  static uint Calculate(const void* data, uint length);
  static uint Calculate(const void* data, uint length, uint checksum);
  static uint CalculateString(const char* str, uint checksum);
};

#endif // _CCRC32
