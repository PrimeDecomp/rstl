#include "Kyoto/Alloc/CSmallAllocPool.hpp"
#include <string.h>

CSmallAllocPool::CSmallAllocPool(uint len, void* mainData, void* bookKeeping)
: mMainData(mainData)
, mBookKeeping(bookKeeping)
, mNumBlocks(len)
, mCachedBookKeepingOffset(NULL)
, x10_(-1)
, x14_(-1)
#if RSTL_KYOTO_SMALL_ALLOC_POOL_BLOCK_COUNTS
, mLastAllocBlockCount(0)
#endif
, mNumBlocksAvailable(len)
, mNumAllocs(0) {
  memset(bookKeeping, 0, len / 2);
}

void* CSmallAllocPool::FindFree(int len) {
  uchar* bookKeepingPtr;
  int size = (int)len / 2;
  if (mCachedBookKeepingOffset == nullptr) {
    mCachedBookKeepingOffset = mBookKeeping;
  }
  uchar* curKeepingOffset = static_cast< uchar* >(mCachedBookKeepingOffset);
  bookKeepingPtr = static_cast< uchar* >(mBookKeeping);
  uchar* bookKeepingEndPtr = bookKeepingPtr + ((uint)mNumBlocks >> 1);
  uchar* curKeepingIter = curKeepingOffset;
  while (true) {
    uchar* iter;
    if (static_cast< uchar* >(curKeepingIter)[0] != 0 || curKeepingIter == bookKeepingEndPtr) {
      if (curKeepingIter == bookKeepingEndPtr) {
        curKeepingIter = bookKeepingPtr;
      } else {
        int tmp = static_cast< uchar* >(curKeepingIter)[0];
        int reg = tmp >> 4;
        curKeepingIter += (reg / 2);
      }
    } else {
      uchar* tempIter = curKeepingIter + size;
      iter = curKeepingIter + 1;
      while (iter != curKeepingOffset && iter != bookKeepingEndPtr && iter != tempIter) {
        if (static_cast< uchar* >(iter)[0] == 0) {
          iter++;
        } else {
          break;
        }
      }

      if (iter == curKeepingIter + size) {
        if (iter == bookKeepingEndPtr) {
          mCachedBookKeepingOffset = bookKeepingPtr;
        } else {
          mCachedBookKeepingOffset = curKeepingIter;
        }
        return curKeepingIter;
      }

      if (iter == curKeepingOffset) {
        return nullptr;
      }
      if (iter == bookKeepingEndPtr) {
        curKeepingIter = bookKeepingPtr;
      } else {
        curKeepingIter = iter;
      }
    }
    if (curKeepingIter == curKeepingOffset) {
      return nullptr;
    }
  };
}

void* CSmallAllocPool::Alloc(const uint size) {
  uint len = size >= 4 ? (size + (kAllocatorPointerSize - 1)) / kAllocatorPointerSize : 1;

#if RSTL_KYOTO_SMALL_ALLOC_POOL_BLOCK_COUNTS
  len = (len + 1) & ~1u;
#else
  if ((len & 1) != 0) {
    len += 1;
  }
#endif

  uchar* freePtr = static_cast< uchar* >(FindFree(len));
  if (freePtr == nullptr) {
    return nullptr;
  }

#if RSTL_KYOTO_SMALL_ALLOC_POOL_BLOCK_COUNTS
  uint sub = len - 2;
#else
  int sub = len - 2;
#endif
  uchar* bufPtr = GetPtrFromIndex(freePtr - static_cast< uchar* >(mBookKeeping));
  *freePtr = (len << 4) | 0xf;
  int blockSize = sub / 2;
  uchar* freePtrIter = freePtr + 1;
  while (blockSize--) {
    *freePtrIter = 0xff;
    ++freePtrIter;
  }

  mNumBlocksAvailable -= len;
  ++mNumAllocs;
#if RSTL_KYOTO_SMALL_ALLOC_POOL_BLOCK_COUNTS
  mLastAllocBlockCount = len;
#endif

  return bufPtr;
}

#if RSTL_KYOTO_SMALL_ALLOC_POOL_BLOCK_COUNTS
int CSmallAllocPool::Free(const void* ptr)
#else
bool CSmallAllocPool::Free(const void* ptr)
#endif
{
  const int ptrIndex = GetIndexFromPtr(ptr);
  const int bitShift = (ptrIndex & 1) ? 0 : 4;
  const size_t entryIndex = static_cast< size_t >(ptrIndex) / 2;
  long entryValue = GetEntryValue(entryIndex);

  entryValue = (entryValue >> bitShift) & 0xF;
  mNumBlocksAvailable += entryValue;
#if !RSTL_KYOTO_SMALL_ALLOC_POOL_BLOCK_COUNTS
  int blocksToClear = entryValue;
#endif
  mNumAllocs -= 1;
  x14_ = ptrIndex;

  if (static_cast< size_t >(ptrIndex) == static_cast< size_t >(x10_)) {
    x10_ = -1;
  }

#if RSTL_KYOTO_SMALL_ALLOC_POOL_BLOCK_COUNTS
  int blocksToClear = entryValue;
#endif
  for (uchar* bookkeepingPtr = static_cast< uchar* >(mBookKeeping) + entryIndex; blocksToClear != 0;
       ++bookkeepingPtr) {
    *bookkeepingPtr = 0;
    blocksToClear -= 2;
  }
#if RSTL_KYOTO_SMALL_ALLOC_POOL_BLOCK_COUNTS
  return entryValue;
#else
  return true;
#endif
}
