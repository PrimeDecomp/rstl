#ifndef _CCALLSTACK
#define _CCALLSTACK

#include <types.h>

#include "rstl/RstlVersions.h"

class CCallStack {
  static const char kUnknownType[];

public:
  CCallStack(uint lineNum, const char* lineStr, const char* type = kUnknownType);
#if RSTL_KYOTO_CALLSTACK_SIGNED_CTOR
  // Prototype assertions retain the signed-depth constructor ABI.
  CCallStack(int depth, const char* source, const char* type);
#endif

  const char* GetFileAndLineText() const;
  const char* GetTypeText() const;

private:
  const char* mLine;
  const char* mType;
};

#endif // _CCALLSTACK
