#ifndef _CARAMMANAGER
#define _CARAMMANAGER
#include "rstl/RstlVersions.h"

#include "types.h"

#include "dolphin/arq.h"

#include "rstl/list.hpp"

class CARAMManager {
public:
  enum EDMAPriority {
    kDMAPrio_Zero,
    kDMAPrio_One,
    kDMAPrio_Two,
    kDMAPrio_Three,
    kDMAPrio_Four,
    kDMAPrio_Five,
    kDMAPrio_Six,
  };

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static uint GetAllocatedBytes() { return mChunksAllocated * mChunkSize; }
  static uint GetCapacityBytes() { return mNumChunks * mChunkSize; }
  static bool Initialize(uint);
#else
  static bool Initialize(uint chunkSize, uint size, uint secondChunkSize);
#endif

  static void Shutdown();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static void* Alloc(uint len);
  static uint FindFreeBlocks(uint, uint, uint);
  static bool Free(const void* ptr);
  static uint DMAToARAM(void*, void*, uint, EDMAPriority);
#else
  static void* Alloc(uint len, int pool = 0);
  static bool Free(const void* ptr, int pool = 0);
  static int DMAToARAM(void*, void*, uint, EDMAPriority);
#endif

  static int DMAToMRAM(void*, void*, uint, EDMAPriority);
  static bool IsDMACompleted(uint handle);
  static void WaitForDMACompletion(uint handle);
  static void WaitForAllDMAsToComplete();
  static bool CancelDMA(uint);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static void AramManagerDMACallback(uintptr_t result);
#else
  static void AramManagerDMACallback(u32 result);
#endif

  static void RefreshActiveDMAList();
  static void CollectGarbage();
#if !RSTL_PRIME1
  static bool IsValidAlloc(const void* ptr);
  static const void* GetInvalidAlloc();
#endif

  static void PreInitializeAlloc(uint size) { mPreInitializeAlloc += size; }
#if RSTL_PRIME1
  static void* GetInvalidAlloc() { return reinterpret_cast< void* >(kInvalidAlloc); }
#endif

  static const uint GetInvalidDMAHandle() { return kInvalidHandle; }
  static uint GetAndIncrementUniqueID() {
    mDMAUniqueID++;
    if (mDMAUniqueID == GetInvalidDMAHandle()) {
      mDMAUniqueID++;
    }
    return mDMAUniqueID;
  }

private:
  struct SAramDMARequest {
    ARQRequest mRequest;
    uint mUniqueID;
    bool mComplete;
  };

#if !RSTL_PRIME1
  class CAramPool;
  friend class CAramPool;
#endif

  static bool mbInitialized;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static u32 mpARAMStart;
  static uint mChunkSize;
  static uint mNumChunks;
  static uint* mpBookKeepingMemory;
#else
  static uint mDMAUniqueID;
  static CAramPool* mPools[2];
#endif

  static uint mPreInitializeAlloc;
#if RSTL_PRIME1
  static uint mDMAUniqueID;
  static uint mChunksAllocated;
#endif

  static rstl::list< SAramDMARequest* > mActiveDMAs;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static const int kInvalidAlloc;
#else
  static const uint kFreeChunk;
#endif

  static const int kInvalidHandle;
};

#endif // _CARAMMANAGER
