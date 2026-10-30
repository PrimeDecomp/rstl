#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"

uint CCharAnimMemoryMetrics::sTotalSize = 0;

// TODO: verify that this Prime/Echoes difference is real
void CCharAnimMemoryMetrics::AddToTotalSize(RSTL_PRIME1_CONST uint size, EAnimSubSystem subSystem) {
  sTotalSize += size;
}

void CCharAnimMemoryMetrics::SubtractFromTotalSize(uint size, EAnimSubSystem subSystem) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  sTotalSize -= size;
#else
  sTotalSize = sTotalSize > size ? sTotalSize - size : 0;
#endif
}

uint CCharAnimMemoryMetrics::GetTotalSize() { return sTotalSize; }
