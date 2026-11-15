#include "rstl/static_assert.hpp"
#include "Kyoto/Graphics/DolphinGPUMemory.hpp"
#include "rstl/RstlVersions.h"

#include "Kyoto/Alloc/Assert.hpp"

#include "Kyoto/Alloc/CCircularBuffer.hpp"
#if RSTL_KYOTO_GPU_MEMORY_EXTENDED_TOKENS
#include "Kyoto/Graphics/CGraphics.hpp"
#endif

#include "rstl/list.hpp"
#include "rstl/optional_object.hpp"

#include "dolphin/gx.h"
#include "dolphin/os.h"

namespace {
// Guessed name
struct SAllocation {
  void* mPointer;
  int mSize;
#if RSTL_KYOTO_GPU_MEMORY_EXTENDED_TOKENS
  rstl::optional_object< u64 > mToken;

  SAllocation(void* pointer, int size) : mPointer(pointer), mSize(size) {}
#else
  ushort mToken;

  SAllocation(void* pointer, int size, ushort token)
  : mPointer(pointer), mSize(size), mToken(token) {}
#endif
};
CHECK_SIZEOF(SAllocation, RSTL_KYOTO_GPU_MEMORY_EXTENDED_TOKENS ? 0x14 : 0xc)

static void* sBufferBase;
#if !RSTL_KYOTO_GPU_MEMORY_EXTENDED_TOKENS
static ushort sCurrentToken;
#endif
static bool sInitialized;
static bool sDumpedSpinLockMessage;
static bool sWithinAllocation;
#if RSTL_KYOTO_GPU_MEMORY_EXTENDED_TOKENS
static int sPeakAllocatedAmount; // Guessed name
#else
// Guessed name
static int sBufferResetState;
#endif
static int sBufferSize = 0x80000;
static rstl::optional_object< CCircularBuffer > sBuffer;
static rstl::list< SAllocation > sAllocations;
} // namespace

void GPUMemory::SetBuffer(void* buffer, uint size) {
  sBufferSize = size;
  sAllocations.clear();
  sBufferBase = buffer;
#if RSTL_KYOTO_GPU_MEMORY_EXTENDED_TOKENS
  sPeakAllocatedAmount = 0;
#else
  sBufferResetState = 0;
#endif
  if (buffer != nullptr) {
    sBuffer = CCircularBuffer(buffer, sBufferSize, CCircularBuffer::kOS_NotOwned);
  }
}

void* GPUMemory::EnsureAllocation(int size) {
  if (!sInitialized) {
#if !RSTL_KYOTO_GPU_MEMORY_EXTENDED_TOKENS
    GXSetDrawSync(0xffff);
    while (GXReadDrawSync() != 0xffff) {
    }
    sCurrentToken = 1;
#endif
    sInitialized = true;
  }

  RS_VERIFY_THROW(103, !sWithinAllocation, false, "Trying to nest GPU allocations!");
#if !RSTL_KYOTO_GPU_MEMORY_LATE_ALLOCATION_FLAG
  sWithinAllocation = true;
#endif
  TickAllocations();
  int alignedSize = (size + 31) & ~31u;
  void* data = sBuffer->Alloc(alignedSize);
  if (data == nullptr && !sDumpedSpinLockMessage) {
    RS_DEBUGGER_PRINT("Skinning Warning: Spin-locking while allocating, size:%d, allocated:%d\n",
                      alignedSize, sBuffer->GetAllocatedAmount());
    sDumpedSpinLockMessage = true;
  }

  s32 startTick = OSGetTick();
  while (data == nullptr) {
    TickAllocations();
    data = sBuffer->Alloc(alignedSize);
#if !RSTL_KYOTO_GPU_MEMORY_EXTENDED_TOKENS
    if (data == nullptr) {
      s32 currentTick = OSGetTick();
      if (OSTicksToMilliseconds(static_cast< uint >(currentTick - startTick)) > 60) {
        ushort token = GXReadDrawSync();
        for (rstl::list< SAllocation >::iterator it = sAllocations.begin();
             it != sAllocations.end(); ++it) {
        }
        sCurrentToken = token;
        startTick = currentTick;
        GXSetDrawSync(sCurrentToken);
        sAllocations.clear();
      }
    }
#endif
  }

#if RSTL_KYOTO_GPU_MEMORY_EXTENDED_TOKENS
  int allocated = sBuffer->GetAllocatedAmount();
  if (allocated > sPeakAllocatedAmount) {
    sPeakAllocatedAmount = allocated;
  }
#endif
#if RSTL_KYOTO_GPU_MEMORY_LATE_ALLOCATION_FLAG
  sWithinAllocation = true;
#endif
#if RSTL_KYOTO_GPU_MEMORY_EXTENDED_TOKENS
  sAllocations.push_back(SAllocation(data, alignedSize));
#else
  sAllocations.push_back(SAllocation(data, alignedSize, sCurrentToken));
#endif
  if (data == sBufferBase) {
    GXInvalidateVtxCache();
  }
  return data;
}

void GPUMemory::ReleaseAllocation() {
  RS_VERIFY_THROW(164, sWithinAllocation, false, "Not within a skinning allocation!");
#if RSTL_KYOTO_GPU_MEMORY_EXTENDED_TOKENS
  sAllocations.back().mToken = CGraphics::IssueCurrentDrawSyncToken();
  sWithinAllocation = false;
#else
  sWithinAllocation = false;
  GXSetDrawSync(sCurrentToken);
  ++sCurrentToken;
#endif
}

#if RSTL_KYOTO_GPU_MEMORY_EXTENDED_TOKENS
uint GPUMemory::GetPeakAllocatedAmount() { return sPeakAllocatedAmount; }
#endif

void GPUMemory::TickAllocations() {
  do {
#if !RSTL_KYOTO_GPU_MEMORY_EXTENDED_TOKENS
    int syncVal = GXReadDrawSync();
    if (syncVal > static_cast< int >(sCurrentToken)) {
      syncVal -= 0x10000;
    }
#endif
    while (sAllocations.size() != 0) {
      SAllocation& front = sAllocations.front();
#if RSTL_KYOTO_GPU_MEMORY_EXTENDED_TOKENS
      if (!CGraphics::QueryCurrentDrawSyncToken(front.mToken.data())) {
        break;
      }
#else
      int tokenVal = static_cast< int >(front.mToken);
      if (tokenVal > static_cast< int >(sCurrentToken)) {
        tokenVal -= 0x10000;
      }
      if (syncVal < tokenVal) {
        break;
      }
#endif
      sBuffer->Free(front.mPointer, front.mSize);
      sAllocations.pop_front();
    }
  } while (sAllocations.size() > 150);
}

