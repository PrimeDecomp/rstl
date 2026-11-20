#include "Kyoto/Alloc/CCallStack.hpp"

extern "C" const char gkUnknownType[] = "UnknownType";

CCallStack::CCallStack(uint lineNum, const char* fileAndLine, const char* type)
: mLine(fileAndLine), mType(type) {}

#if RSTL_KYOTO_CALLSTACK_SIGNED_CTOR
CCallStack::CCallStack(int depth, const char* fileAndLine, const char* type)
: mLine(fileAndLine), mType(type) {}
#endif

const char* CCallStack::GetFileAndLineText() const { return mLine; }

const char* CCallStack::GetTypeText() const { return mType; }
