#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"

uint CCharAnimMemoryMetrics::sTotalSize = 0;

#if RSTL_PRIME1
void CCharAnimMemoryMetrics::AddToTotalSize(const uint size, EAnimSubSystem subSystem) {
  sTotalSize += size;
}

void CCharAnimMemoryMetrics::SubtractFromTotalSize(uint size, EAnimSubSystem subSystem) {
  sTotalSize -= size;
}
#else
void CCharAnimMemoryMetrics::AddToTotalSize(uint size, EAnimSubSystem subSystem) {
  sTotalSize += size;
}

void CCharAnimMemoryMetrics::SubtractFromTotalSize(uint size, EAnimSubSystem subSystem) {
  sTotalSize = sTotalSize > size ? sTotalSize - size : 0;
}
#endif

uint CCharAnimMemoryMetrics::GetTotalSize() { return sTotalSize; }
