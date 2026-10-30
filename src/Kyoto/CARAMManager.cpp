#include "rstl/RstlVersions.h"
#include "Kyoto/CARAMManager.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "rstl/math.hpp"
#if !RSTL_PRIME1
#include "dolphin/ar.h"
#endif

#include "dolphin/arq.h"
#include "dolphin/os.h"

#if !RSTL_PRIME1
class CARAMManager::CAramPool {
public:
  CAramPool(uint chunkSize, uint numChunks)
  : mpARAMStart(ARAlloc(chunkSize * numChunks))
  , mChunkSize(chunkSize)
  , mNumChunks(numChunks)
  , mChunksAllocated(0)
  , mMaxChunksAllocated(0)
  , mpBookKeepingMemory((uint*)CMemory::Alloc(numChunks * 4, IAllocator::kHI_None,
                                              IAllocator::kSC_Unk1, IAllocator::kTP_Heap,
                                              CCallStack(-1, "\?\?(\?\?)"))) {
    for (uint i = 0; i < mNumChunks; ++i) {
      mpBookKeepingMemory[i] = kFreeChunk;
    }
    CMemory::OffsetFakeStatics(mNumChunks * 4);
  }
  ~CAramPool();

  void* Alloc(uint len);
  uint FindFreeBlocks(uint start, uint end, uint count);
  bool Free(const void* ptr);

private:
  u32 mpARAMStart;
  uint mChunkSize;
  uint mNumChunks;
  uint mChunksAllocated;
  uint mMaxChunksAllocated;
  uint* mpBookKeepingMemory;
};
#endif

bool CARAMManager::mbInitialized = false;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
u32 CARAMManager::mpARAMStart = 0;
uint CARAMManager::mChunkSize = 0;
uint CARAMManager::mNumChunks = 0;
uint* CARAMManager::mpBookKeepingMemory;
#else
uint CARAMManager::mDMAUniqueID = 0;
CARAMManager::CAramPool* CARAMManager::mPools[2];
#endif

uint CARAMManager::mPreInitializeAlloc = 16 * 1024;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
uint CARAMManager::mDMAUniqueID = 0;
uint CARAMManager::mChunksAllocated = 0;
const int CARAMManager::kInvalidAlloc = -1;
#else
const uint CARAMManager::kFreeChunk = 0;
#endif

const int CARAMManager::kInvalidHandle = -1;

rstl::list< CARAMManager::SAramDMARequest* > CARAMManager::mActiveDMAs;

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
bool CARAMManager::Initialize(uint chunkSize) {
  uint numChunks = (ARGetSize() - mPreInitializeAlloc) / chunkSize;
  mChunkSize = chunkSize;
  mNumChunks = numChunks;
  mpARAMStart = ARAlloc(chunkSize * numChunks);
  mpBookKeepingMemory = (uint*)CMemory::Alloc(numChunks * 4, IAllocator::kHI_None,
                                              IAllocator::kSC_Unk1, IAllocator::kTP_Heap,
                                              CCallStack(-1, "\?\?(\?\?)"));
  CMemory::OffsetFakeStatics(mNumChunks * 4);

  for (uint i = 0; i < numChunks; ++i) {
    mpBookKeepingMemory[i] = 0;
  }

  mDMAUniqueID = 0;
  mbInitialized = true;
  return true;
}

void CARAMManager::Shutdown()
#else
CARAMManager::CAramPool::~CAramPool()
#endif
{
#if RSTL_PRIME1
  WaitForAllDMAsToComplete();
  CMemory::Free(mpBookKeepingMemory);
#endif

  u32 unk = 0;
  CMemory::OffsetFakeStatics(-mNumChunks * 4);
  ARFree(&unk);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mbInitialized = false;
#else
  CMemory::Free(mpBookKeepingMemory);
#endif
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void* CARAMManager::Alloc(const uint len)
#else
void* CARAMManager::CAramPool::Alloc(uint len)
#endif
{
  uint chunkCount = (mChunkSize - 1 + len) / mChunkSize;
  uint block = FindFreeBlocks(0, mNumChunks, chunkCount);

  if (block == -1) {
    return (void*)-1;
  }

  mChunksAllocated += chunkCount;
#if !RSTL_PRIME1
  mMaxChunksAllocated = rstl::max_val(mMaxChunksAllocated, mChunksAllocated);
#endif

  uint blockOffset = mpARAMStart + block * mChunkSize;
  mpBookKeepingMemory[block] = chunkCount;

  while (--chunkCount != 0) {
    ++block;
    mpBookKeepingMemory[block] = -1;
  }

  return (void*)blockOffset;
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
uint CARAMManager::FindFreeBlocks(uint start, uint end, uint count)
#else
uint CARAMManager::CAramPool::FindFreeBlocks(uint start, uint end, uint count)
#endif
{
  while (start < end) {
    if (mpBookKeepingMemory[start] == 0) {
      if (count == 1) {
        return start;
      }

      ++start;
      int found = 1;
      while (start < end) {
        uint tmp = mpBookKeepingMemory[start];
        if (tmp != 0) {
          start += tmp;
          break;
        }

        found++;
        if (found == count) {
          return start - (count - 1);
        }
        ++start;
      }
    } else {
      start += mpBookKeepingMemory[start];
    }
  }
  return -1;
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
bool CARAMManager::Free(const void* ptr)
#else
bool CARAMManager::CAramPool::Free(const void* ptr)
#endif
{
  if (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      GetInvalidAlloc() == ptr
#else
      !IsValidAlloc(ptr)
#endif
  ) {
    return false;
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  uint blockStart = (reinterpret_cast< uintptr_t >(ptr) - mpARAMStart) / mChunkSize;
#else
  uint blockStart = (reinterpret_cast< u32 >(ptr) - mpARAMStart) / mChunkSize;
#endif

  uint blockCount = mpBookKeepingMemory[blockStart];
  mChunksAllocated -= blockCount;
  while (blockCount--) {
    mpBookKeepingMemory[blockStart++] = 0;
  }
  return true;
}

#if !RSTL_PRIME1
bool CARAMManager::Initialize(uint chunkSize, uint size, uint secondChunkSize) {
  uint aramSize = ARGetSize() - mPreInitializeAlloc;
  uint numChunks = size / chunkSize;
  uint secondNumChunks = (aramSize - size) / secondChunkSize;
  mPools[1] = rs_new CAramPool(chunkSize, numChunks);
  mPools[0] = rs_new CAramPool(secondChunkSize, secondNumChunks);
  mDMAUniqueID = 0;
  mbInitialized = true;
  return true;
}

void CARAMManager::Shutdown() {
  WaitForAllDMAsToComplete();
  delete mPools[0];
  delete mPools[1];
  mbInitialized = false;
}

void* CARAMManager::Alloc(uint len, int pool) { return mPools[pool]->Alloc(len); }

bool CARAMManager::Free(const void* ptr, int pool) { return mPools[pool]->Free(ptr); }
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
uint
#else
int
#endif
CARAMManager::DMAToARAM(void* src, void* dest, uint len, EDMAPriority priority) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  DCStoreRange(src, len);
#else
  DCFlushRange(src, len);
#endif

  SAramDMARequest* req = rs_new SAramDMARequest();
  req->mComplete = false;
  req->mUniqueID = mDMAUniqueID;
  mActiveDMAs.push_back(req);
  ARQPostRequest(&req->mRequest, req->mUniqueID, ARQ_TYPE_MRAM_TO_ARAM,
                 (priority == kDMAPrio_One) ? ARQ_PRIORITY_HIGH : ARQ_PRIORITY_LOW,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                 reinterpret_cast< uintptr_t >(src),
                 reinterpret_cast< uintptr_t >(dest),
#else
                 reinterpret_cast< u32 >(src),
                 reinterpret_cast< u32 >(dest),
#endif
                 len,
                 AramManagerDMACallback);

  GetAndIncrementUniqueID();
  return req->mUniqueID;
}

int CARAMManager::DMAToMRAM(void* src, void* dest, uint len, EDMAPriority priority) {
  DCInvalidateRange(dest, len);
  SAramDMARequest* req = rs_new SAramDMARequest();
  req->mComplete = false;
  req->mUniqueID = mDMAUniqueID;
  mActiveDMAs.push_back(req);
  ARQPostRequest(&req->mRequest, req->mUniqueID, ARQ_TYPE_ARAM_TO_MRAM,
                 (priority == kDMAPrio_One) ? ARQ_PRIORITY_HIGH : ARQ_PRIORITY_LOW,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                 reinterpret_cast< uintptr_t >(src),
                 reinterpret_cast< uintptr_t >(dest),
#else
                 reinterpret_cast< u32 >(src),
                 reinterpret_cast< u32 >(dest),
#endif
                 len,
                 AramManagerDMACallback);

  GetAndIncrementUniqueID();
  return req->mUniqueID;
}

bool CARAMManager::IsDMACompleted(uint handle) {
  rstl::list< SAramDMARequest* >::iterator it = mActiveDMAs.begin();
  for (; it != mActiveDMAs.end(); ++it) {
    uint uniqueId = (*it)->mUniqueID;
    if (uniqueId == handle) {
      if ((*it)->mComplete) {
        delete (*it);
        mActiveDMAs.erase(it);
        return true;
      }
      return false;
    }
  }

  return true;
}

void CARAMManager::WaitForDMACompletion(uint handle) {
  rstl::list< SAramDMARequest* >::iterator it = mActiveDMAs.begin();
  for (; it != mActiveDMAs.end(); ++it) {
    uint uniqueId = (*it)->mUniqueID;
    if (uniqueId == handle) {
      // Spin until complete!
      while (!(*it)->mComplete)
        ;
      delete (*it);
      mActiveDMAs.erase(it);
      return;
    }
  }
}

void CARAMManager::WaitForAllDMAsToComplete() {
  while ((int)mActiveDMAs.size() > 0) {
    RefreshActiveDMAList();
  }
}

bool CARAMManager::CancelDMA(uint handle) {
  rstl::list< SAramDMARequest* >::iterator it = mActiveDMAs.begin();
  for (; it != mActiveDMAs.end(); ++it) {
    uint uniqueId = (*it)->mUniqueID;
    if (uniqueId == handle) {
      return (*it)->mComplete != false;
    }
  }
  return true;
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CARAMManager::AramManagerDMACallback(uintptr_t result)
#else
void CARAMManager::AramManagerDMACallback(u32 result)
#endif
{
  SAramDMARequest* req = reinterpret_cast< SAramDMARequest* >(result);
  req->mComplete = true;
  if (req->mRequest.type == ARQ_TYPE_ARAM_TO_MRAM) {
    DCInvalidateRange(reinterpret_cast< void* >(req->mRequest.dest), req->mRequest.length);
  }
}

void CARAMManager::RefreshActiveDMAList() {
  rstl::list< SAramDMARequest* >::iterator it = mActiveDMAs.begin();
  while (it != mActiveDMAs.end()) {
    if ((*it)->mComplete) {
      delete (*it);
      it = mActiveDMAs.erase(it);
      continue;
    }
    ++it;
  }
}

void CARAMManager::CollectGarbage() { RefreshActiveDMAList(); }

#if !RSTL_PRIME1
bool CARAMManager::IsValidAlloc(const void* ptr) {
  return reinterpret_cast< const void* >(-1) != ptr;
}

const void* CARAMManager::GetInvalidAlloc() { return reinterpret_cast< const void* >(-1); }
#endif

