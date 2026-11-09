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

  struct SPoolStatistics { // Guessed name
    SPoolStatistics(uint chunkSize, uint numChunks, uint freeChunks, uint peakChunks)
    : mChunkSize(chunkSize)
    , mNumChunks(numChunks)
    , mFreeChunks(freeChunks)
    , mPeakChunks(peakChunks) {}

    uint mChunkSize;  // Guessed name
    uint mNumChunks;  // Guessed name
    uint mFreeChunks; // Guessed name
    uint mPeakChunks; // Guessed name
  };
  static SPoolStatistics GetPoolAllocationStatistics(int pool);

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static uint GetAllocatedBytes() { return mChunksAllocated * mChunkSize; }
  static uint GetCapacityBytes() { return mNumChunks * mChunkSize; }
  static bool Initialize(uint);
#else
  static bool Initialize(uint chunkSize, uint size, uint secondChunkSize);
#endif

  static void Shutdown();

#if !defined(TARGET_PC)
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
#endif // !defined(TARGET_PC)

  static bool IsDMACompleted(uint handle);
  static void WaitForDMACompletion(uint handle);
  static void WaitForAllDMAsToComplete();
  static bool CancelDMA(uint);
  static void AramManagerDMACallback(uintptr_t result);

  static void RefreshActiveDMAList();
  static void CollectGarbage();
  static bool IsValidAlloc(const void* ptr);
  static RSTL_ECHOES_CONST void* GetInvalidAlloc();

  static void PreInitializeAlloc(uint size) { mPreInitializeAlloc += size; }

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
#endif
  static uint mDMAUniqueID;
#if !RSTL_PRIME1
  static CAramPool* mPools[2];
#endif

  static uint mPreInitializeAlloc;
#if RSTL_PRIME1
  static uint mChunksAllocated;
#endif

  static rstl::list< SAramDMARequest* > mActiveDMAs;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static const ptrdiff_t kInvalidAlloc;
#else
  static const uint kFreeChunk;
#endif

  static const int kInvalidHandle;
};

#if RSTL_PRIME1
inline void* CARAMManager::GetInvalidAlloc() { return reinterpret_cast< void* >(kInvalidAlloc); }
#endif

#if defined(TARGET_PC)
inline bool CARAMManager::Initialize(uint) { return true; }
inline void CARAMManager::Shutdown() {}
inline bool CARAMManager::IsDMACompleted(uint handle) { return true; }
inline void CARAMManager::WaitForDMACompletion(uint handle) {}
inline void CARAMManager::WaitForAllDMAsToComplete() {}
inline bool CARAMManager::CancelDMA(uint) { return true; }
inline void CARAMManager::AramManagerDMACallback(uintptr_t result) {}
inline void CARAMManager::RefreshActiveDMAList() {}
inline void CARAMManager::CollectGarbage() {}
#endif // TARGET_PC

#endif // _CARAMMANAGER
