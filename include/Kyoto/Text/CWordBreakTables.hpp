#ifndef _CWORDBREAKTABLES
#define _CWORDBREAKTABLES

#include "rstl/types.h"

class CWordBreakTables {
public:
  static int GetBeginRank(wchar_t ch);
  static int GetEndRank(wchar_t ch);
};

#endif // _CWORDBREAKTABLES
