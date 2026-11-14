#ifndef _CSMALLALLOCPOOL
#define _CSMALLALLOCPOOL

#include "rstl/types.h"

#include "Kyoto/Alloc/AllocatorCommon.hpp"
#include "rstl/RstlVersions.h"

class CSmallAllocPool {
public:
  CSmallAllocPool(uint len, void* mainData, void* bookKeeping);
  void* FindFree(int len);
  void* Alloc(uint size);
#if RSTL_KYOTO_SMALL_ALLOC_POOL_BLOCK_COUNTS
  int Free(const void* ptr);
#else
  bool Free(const void* ptr);
#endif

  bool PtrWithinPool(const void* ptr) const {
    return static_cast< uint >(
               (static_cast< const uchar* >(ptr) - static_cast< uchar* >(mMainData)) /
               kAllocatorPointerSize) < mNumBlocks;
  }

  uint GetIndexFromPtr(const void* ptr) const {
    return (static_cast< const uchar* >(ptr) - static_cast< const uchar* >(mMainData)) /
           kAllocatorPointerSize;
  }
  intptr_t GetEntryValue(const uint idx) const {
    return *(static_cast< uchar* >(mBookKeeping) + idx);
  }
  uchar* GetPtrFromIndex(const uint idx) const {
    return static_cast< uchar* >(mMainData) + (idx * (kAllocatorPointerSize * 2));
  }

  uint GetNumBlocksAvailable() const { return mNumBlocksAvailable; }
  uint GetTotalEntries() const { return mNumBlocks; }
  uint GetAllocatedSize() const { return mNumBlocks - mNumBlocksAvailable; }
  uint GetNumAllocs() const { return mNumAllocs; }

private:
  void* mMainData;
  void* mBookKeeping;
  int mNumBlocks;
  void* mCachedBookKeepingOffset;
  int x10_;
  int x14_;
#if RSTL_KYOTO_SMALL_ALLOC_POOL_BLOCK_COUNTS
  uint mLastAllocBlockCount;
#endif
  uint mNumBlocksAvailable;
  uint mNumAllocs;
};

#endif // _CSMALLALLOCPOOL
