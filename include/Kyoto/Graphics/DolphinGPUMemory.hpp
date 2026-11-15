#ifndef _DOLPHINGPUMEMORY
#define _DOLPHINGPUMEMORY

#include "rstl/types.h"

// Guessed name
class GPUMemory {
public:
  static void SetBuffer(void* buffer, uint size);
  static void* EnsureAllocation(int size);
  static void ReleaseAllocation();
  static void TickAllocations();
  static uint GetPeakAllocatedAmount(); // Guessed name
};

#endif // _DOLPHINGPUMEMORY
