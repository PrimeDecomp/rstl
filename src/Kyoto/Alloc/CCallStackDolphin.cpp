#include "Kyoto/Alloc/CCallStack.hpp"

#if !RSTL_KYOTO_DEBUG_BUILD
extern "C" const char gkUnknownType[] = "UnknownType";
#endif

CCallStack::CCallStack(uint lineNum, const char* fileAndLine, const char* type)
: mLine(fileAndLine), mType(type) {}

#if RSTL_KYOTO_CALLSTACK_SIGNED_CTOR
CCallStack::CCallStack(int depth, const char* fileAndLine, const char* type)
: mLine(fileAndLine), mType(type) {}
#endif

const char* CCallStack::GetFileAndLineText() const { return mLine; }

const char* CCallStack::GetTypeText() const { return mType; }
