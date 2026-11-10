#include "Kyoto/Alloc/CMediumAllocPool.hpp"
#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
#include "Kyoto/Basics/CBasics.hpp"
#endif

CMediumAllocPool* CMediumAllocPool::gMediumAllocPtr = nullptr;

CMediumAllocPool::CMediumAllocPool()
#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
: mLastRequestBlocks(0)
, mLastPuddle(kHeapCount, mList.end())
#if RSTL_KYOTO_ALLOCATOR_EXTENDED_MEDIUM_POOL
, mFreeBlocks(kHeapCount, 0u)
#endif
#else
: mLastNodePrev(mList.begin())
#endif
{
  gMediumAllocPtr = this;
}

void CMediumAllocPool::ClearPuddles() {
#if !RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
  mLastNodePrev = mList.end();
#endif
  for (rstl::list< SMediumAllocPuddle >::iterator it = mList.begin(); it != mList.end(); ++it) {
    // The debug build checks each puddle here before releasing the pool.
  }
  mList.clear();
#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
  mLastPuddle.clear();
  mLastPuddle.resize(kHeapCount, mList.end());
#if RSTL_KYOTO_ALLOCATOR_EXTENDED_MEDIUM_POOL
  mFreeBlocks.clear();
  mFreeBlocks.resize(kHeapCount, 0u);
#endif
#endif
  gMediumAllocPtr = nullptr;
}

bool CMediumAllocPool::HasPuddles() const { return mList.size() != 0; }

#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
void* CMediumAllocPool::Alloc(uint size, int heapIndex) {
  const uint blockCount = (size + 31) / 32;
  void* result = mLastPuddle[heapIndex]->FindFree(blockCount);
  if (result == nullptr) {
    for (Puddles::iterator it = mList.begin(); it != mList.end(); ++it) {
      if (it != mLastPuddle[heapIndex] && it->GetHeapIndex() == heapIndex) {
        result = it->FindFree(blockCount);
        if (result != nullptr) {
          mLastPuddle[heapIndex] = it;
          break;
        }
      }
    }
  }
  mLastRequestBlocks = blockCount;
#if RSTL_KYOTO_ALLOCATOR_EXTENDED_MEDIUM_POOL
  if (result != nullptr) {
    mFreeBlocks[heapIndex] -= blockCount;
  }
#endif
  return result;
}

CMediumAllocPool::Puddles::iterator
CMediumAllocPool::FindNextPuddle(int heapIndex, const Puddles::iterator& current) {
  for (Puddles::iterator it = mList.begin(); it != mList.end(); ++it) {
    if (it->GetHeapIndex() == heapIndex && it != current) {
      return it;
    }
  }
  return mList.begin();
}
#else
void* CMediumAllocPool::Alloc(uint len) {
  SMediumAllocPuddle* puddle = &*mLastNodePrev;
  void* ret;
  uint blockCount = 1;
  if (len >= 32) {
    blockCount = (len + 31) / 32;
  }

  ret = puddle->FindFree(blockCount);

  if (ret == nullptr) {
    for (rstl::list< SMediumAllocPuddle >::iterator it = mList.begin(); it != mList.end(); ++it) {
      if (it == mLastNodePrev) {
        continue;
      }

      ret = it->FindFree(blockCount);
      if (ret != nullptr) {
        mLastNodePrev = it;
        break;
      }
    }
  }

  return ret;
}
#endif

#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
int CMediumAllocPool::Free(const void* ptr) {
  Puddles::node* node = mList.begin().get_node();
  Puddles::node* end = mList.end().get_node();
  for (; node != end; node = node->get_next()) {
    SMediumAllocPuddle* puddle = node->get_value();
    if (puddle->Contains(ptr)) {
      const uint blocks = puddle->Free(ptr);
#if RSTL_KYOTO_ALLOCATOR_EXTENDED_MEDIUM_POOL
      mFreeBlocks[puddle->GetHeapIndex()] += blocks;
#endif
      if (puddle->GetNumAllocs() == 0 && puddle->CanErase()) {
        const int heapIndex = puddle->GetHeapIndex();
        if (mLastPuddle[heapIndex] == node) {
          mLastPuddle[heapIndex] = FindNextPuddle(heapIndex, Puddles::iterator(node));
        }
#if RSTL_KYOTO_ALLOCATOR_EXTENDED_MEDIUM_POOL
        mFreeBlocks[heapIndex] -= puddle->GetNumEntries();
#endif
        mList.erase(node);
      }
      return blocks;
    }
  }
  return 0;
}
#else
int CMediumAllocPool::Free(const void* ptr) {
  rstl::list< SMediumAllocPuddle >::node* node = mList.begin().get_node();
  for (; node != mList.end().get_node(); node = node->get_next()) {
    SMediumAllocPuddle* puddle = node->get_value();
    if (puddle->GetPtrOffset(ptr) < puddle->GetNumEntries() * 32) {
      puddle->Free(ptr);
      if (node->get_value()->GetNumAllocs() == 0 && node->get_value()->CanErase()) {
        if (mLastNodePrev == node) {
          mLastNodePrev = mList.begin().get_node();
        }

        mList.erase(node);
      }

      return 2;
    }
  }
  return 1;
}
#endif

uint CMediumAllocPool::GetNumAllocs() {
  uint ret = 0;
  for (rstl::list< SMediumAllocPuddle >::iterator it = mList.begin(); it != mList.end(); ++it) {
    ret += it->GetNumAllocs();
  }

  return ret;
}
uint CMediumAllocPool::GetTotalEntries() {
  uint ret = 0;
  for (rstl::list< SMediumAllocPuddle >::iterator it = mList.begin(); it != mList.end(); ++it) {
    ret += it->GetNumEntries();
  }

  return ret;
}

uint CMediumAllocPool::GetNumBlocksAvailable() {
  uint ret = 0;
  for (rstl::list< SMediumAllocPuddle >::iterator it = mList.begin(); it != mList.end(); ++it) {
    ret += it->GetNumBlocks();
  }

  return ret;
}

#if RSTL_KYOTO_ALLOCATOR_EXTENDED_MEDIUM_POOL
uint CMediumAllocPool::GetAvailableBytes(int heapIndex) const {
  return GetAllocMemoryRequired(mFreeBlocks[heapIndex]);
}
#endif

#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
void CMediumAllocPool::AddPuddle(uint numBlocks, void* data, bool canErase, int heapIndex) {
  mList.push_back(SMediumAllocPuddle(numBlocks, data, canErase, heapIndex));
  mLastPuddle[heapIndex] = mList.end();
  --mLastPuddle[heapIndex];
#if RSTL_KYOTO_ALLOCATOR_EXTENDED_MEDIUM_POOL
  mFreeBlocks[heapIndex] += numBlocks;
#endif
}
#else
void CMediumAllocPool::AddPuddle(uint len, void* data, const bool unk) {
  mList.push_back(SMediumAllocPuddle(len, data, unk));
  mLastNodePrev = mList.end();
  --mLastNodePrev;
}
#endif

#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
SMediumAllocPuddle::SMediumAllocPuddle(uint numBlocks, void* data, bool canErase, int heapIndex)
: mMainData(static_cast<uchar*>(data))
, mBookKeeping(static_cast<uchar*>(data) + CMediumAllocPool::GetAllocMemoryRequired(numBlocks), numBlocks)
, mCanErase(canErase)
, mIsMem2(heapIndex == 1) {}
#else
SMediumAllocPuddle::SMediumAllocPuddle(const uint numBlocks, void* data, const bool canErase)
: mMainData(static_cast< uchar* >(data))
, mBookKeeping(static_cast< uchar* >(data) + numBlocks * 32)
, mCachedBookKeepingAddr(nullptr)
, mUnused(-1)
, mNumBlocks(numBlocks)
, mNumAllocs(0)
, mNumEntries(numBlocks)
, mCanErase(canErase) {
  SMediumAllocPuddle::InitBookKeeping(mBookKeeping, numBlocks);
}
#endif

SMediumAllocPuddle::~SMediumAllocPuddle() {}

#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
void* SMediumAllocPuddle::FindFree(uint blockCount) {
  const int index = mBookKeeping.Alloc(blockCount != 0 ? blockCount : 1);
  if (index == -1) {
    return nullptr;
  }
  return mMainData.get() + index * 32;
}

uint SMediumAllocPuddle::Free(const void* ptr) {
  return mBookKeeping.Free((reinterpret_cast<uintptr_t>(ptr) -
                            reinterpret_cast<uintptr_t>(mMainData.get())) / 32);
}

CMediumAllocBookKeeping::CMediumAllocBookKeeping(void* data, uint numBlocks)
: mTags(static_cast<uchar*>(data) + 4)
, mNumEntries(numBlocks)
, mNumBlocks(numBlocks)
, mNumAllocs(0)
, mSearchIndex(0) {
  CBasics::ZeroMemory(mTags, numBlocks);
  WriteTags(0, mNumEntries, 0);
}

uint CMediumAllocPool::GetBookKeepingMemoryRequired(uint numBlocks) {
  return (numBlocks + 8 + 31) & ~31u;
}

namespace {
enum { kAllocated = 0x80, kExtended = 0x7f };
const uint kSizeMasks[] = {0, ~0u};

// Extended lengths occupy four bytes adjacent to a one-byte tag.
#pragma pack(push, 1)
struct STagLength {
  uint value;
};
#pragma pack(pop)

inline uint ReadSize(uint tag, const uchar* extended) {
  const uint size = tag & kExtended;
  const bool hasExtendedSize = size == kExtended;
  const uint word = reinterpret_cast<const STagLength*>(extended)->value;
  return (size & kSizeMasks[!hasExtendedSize]) | (kSizeMasks[hasExtendedSize] & word);
}
}

void CMediumAllocBookKeeping::WriteTags(uint index, uint count, uint flags) {
  if (count >= kExtended) {
    const uint tag = flags | kExtended;
    const uint end = index + count;
    mTags[index] = tag;
    reinterpret_cast<STagLength*>(mTags + index + 1)->value = count;
    mTags[end - 1] = tag;
    reinterpret_cast<STagLength*>(mTags + end - 5)->value = count;
  } else {
    const uint tag = flags | static_cast<uchar>(count);
    mTags[index] = tag;
    mTags[index + count - 1] = tag;
  }
}

void CMediumAllocBookKeeping::CountFreeBefore(uint& total, uint& runs, uint index) const {
  uint span = 0;
  uint count = 0;
  while (index != 0) {
    const uint tag = mTags[index - 1];
    const uint size = ReadSize(tag, mTags + index - 5);
    if (tag & kAllocated) {
      break;
    }
    span += size;
    index -= size;
    ++count;
  }
  runs = count;
  total = span;
}

void CMediumAllocBookKeeping::CountFreeAfter(uint& total, uint& runs, uint index) const {
  uint span = 0;
  uint count = 0;
  while (index < mNumEntries) {
    const uint tag = mTags[index];
    const uint size = ReadSize(tag, mTags + index + 1);
    if (tag & kAllocated) {
      break;
    }
    span += size;
    index += size;
    ++count;
  }
  runs = count;
  total = span;
}

int CMediumAllocBookKeeping::Alloc(uint count) {
  if (count > mNumBlocks) {
    return -1;
  }
  const uint start = mSearchIndex;
  uint index = start;
  do {
    const uint tag = mTags[index];
    const uint size = ReadSize(tag, mTags + index + 1);
    if (!(tag & kAllocated) && size >= count) {
      ++mNumAllocs;
      WriteTags(index, count, kAllocated);
      if (size != count) {
        WriteTags(index + count, size - count, 0);
      }
      mNumBlocks -= count;
      mSearchIndex = (index + count) % mNumEntries;
      return index;
    }
    index = (index + size) % mNumEntries;
    if (index >= mNumEntries) {
      index = 0;
    }
  } while (index != start);
  return -1;
}

uint CMediumAllocBookKeeping::Free(uint index) {
  const uint count = ReadSize(mTags[index], mTags + index + 1);
  uint previous, previousRuns, next, nextRuns;
  CountFreeBefore(previous, previousRuns, index);
  CountFreeAfter(next, nextRuns, index + count);
  WriteTags(index - previous, previous + count + next, 0);
  mSearchIndex = index - previous;
  mNumBlocks += count;
  --mNumAllocs;
  return count;
}
#else
void* SMediumAllocPuddle::FindFree(uint blockCount) {
  void* bookKeepingptr;
  uchar* entryPtr;
  void* ret;

  entryPtr = (uchar*)FindFreeEntry(blockCount);
  if (entryPtr == NULL) {
    return NULL;
  }

  bookKeepingptr = mBookKeeping;
  ret = mMainData.get();
  entryPtr[0] = (uchar)blockCount;
  ret = (void*)((uchar*)ret + ((uchar*)entryPtr - (uchar*)bookKeepingptr) * 0x20);
  entryPtr[blockCount - 1] = blockCount;
  mNumBlocks -= blockCount;
  mNumAllocs++;
  return ret;
}

void* SMediumAllocPuddle::FindFreeEntry(uint numBlocks) {
  if (GetNumBlocks() >= numBlocks) {
    uchar* cachedBookPtr = mCachedBookKeepingAddr;
    uchar* bookPtr = mBookKeeping;

    if (cachedBookPtr == nullptr) {
      cachedBookPtr = bookPtr;
    }

    uchar* ptr2 = bookPtr + GetNumEntries();
    uchar* ptr1 = cachedBookPtr;
    do {

      if ((*ptr1 & 0x80) == 0 || ptr1 == ptr2) {
        if (ptr1 == ptr2) {
          ptr1 = bookPtr;
        } else {
          ptr1 += *ptr1;
        }
      } else {
        uint offset = GetBlockOffset(ptr1, ptr2);

        if (offset >= numBlocks) {
          if ((ushort)(offset - numBlocks)) {
            SMediumAllocPuddle::InitBookKeeping(ptr1 + numBlocks, offset - numBlocks);
          }

          mCachedBookKeepingAddr = ptr1;
          return ptr1;
        }

        ptr1 += offset;

        if (ptr1 == cachedBookPtr) {
          break;
        }

        if (ptr1 == ptr2) {
          ptr1 = bookPtr;
        }
      }
    } while (ptr1 != cachedBookPtr);
  }
  return nullptr;
}

void SMediumAllocPuddle::Free(const void* ptr) {
  uchar* bookKeepingStart;
  uchar* bookKeepingPtr;
  ushort mergedCount;
  bool isCached;
  size_t blockOffset =
      (reinterpret_cast< uintptr_t >(ptr) - reinterpret_cast< uintptr_t >(mMainData.get())) / 32;
  uint blockCount = mBookKeeping[blockOffset];
  mergedCount = blockCount;
  isCached = false;
  mNumBlocks += blockCount;
  --mNumAllocs;

  bookKeepingStart = mBookKeeping;
  uchar* cachedBookKeep = mCachedBookKeepingAddr;
  uchar* block = bookKeepingStart + blockOffset;
  bookKeepingPtr = block;
  uchar* bookKeepingEndPtr = bookKeepingStart + GetNumEntries();
  if (cachedBookKeep == block) {
    isCached = true;
  }

  if (block > bookKeepingStart && block[-1] & 0x80) {
    const ushort previousCount =
        !(block[-1] & 0x60)
            ? static_cast< ushort >(block[-2] + (block[-1] & 0x7f) * 256)
            : static_cast< ushort >(
                  (block[-1] & 0x60) == 0x60ul ? 3 : ((block[-1] & 0x60) == 0x40 ? 2 : 1));

    bookKeepingPtr -= previousCount;
    mergedCount = static_cast< ushort >(mergedCount + previousCount);
  }

  uchar* ptr1 = block + blockCount;
  if (ptr1 < bookKeepingEndPtr && (ptr1[0] & 0x80) > 0) {
    mergedCount = static_cast< ushort >(mergedCount + GetBlockOffset(ptr1, bookKeepingEndPtr));
  }

  InitBookKeeping(bookKeepingPtr, mergedCount);
  if (!isCached) {
    return;
  }
  if (bookKeepingPtr == bookKeepingStart) {
    mCachedBookKeepingAddr = nullptr;
  } else {
    mCachedBookKeepingAddr = bookKeepingPtr - bookKeepingPtr[-1];
  }
}

ushort SMediumAllocPuddle::GetBlockOffset(const void* ptr1, const void* ptr2) {
  unsigned char tmp = (uchar*)ptr2 - (uchar*)ptr1 > 1 ? ((uchar*)ptr1)[1] : 0;

  ushort x = tmp + (*(uchar*)(ptr1) & 0x7f) * 0x100;
  if ((x & 0x6000) == 0) {
    return x;
  }
  return (x & 0x6000u) == 0x6000u ? 3 : (((x & 0x6000u) != 0x4000) ? 0 : 1) + 1;
}

void SMediumAllocPuddle::InitBookKeeping(uchar* bookKeepingPtr, ushort blockCount) {
  if (blockCount < 4) {
    const uchar tmp = (blockCount == 3 ? 96 : blockCount == 2 ? 64 : 32) | 0x80;
    bookKeepingPtr[0] = tmp;
    if (blockCount > 1) {
      bookKeepingPtr[blockCount - 1] = tmp;
    }
  } else {
    bookKeepingPtr[0] = (blockCount >> 8) | 0x80;
    bookKeepingPtr[1] = blockCount;
    bookKeepingPtr[blockCount - 2] = blockCount;
    bookKeepingPtr[blockCount - 1] = (blockCount >> 8) | 0x80;
  }
}
#endif
