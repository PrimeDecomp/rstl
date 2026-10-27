#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "rstl/RstlVersions.h"

CMemoryInStream::CMemoryInStream(const void* ptr, unsigned long len, EOwnerShip ownership)
: CInputStream(ptr, len, ownership == kOS_Owned) {}

#if RSTL_KYOTO_BLOCK_INPUT_STREAM
CMemoryInStream::CMemoryInStream(const void* ptr, unsigned long len)
: CInputStream(ptr, len, false) {}

size_t CMemoryInStream::Read(void* dest, size_t len) { return 0; }
#else
CMemoryInStream::CMemoryInStream(const void* ptr, unsigned long len) : CInputStream(ptr, len) {}
#endif
