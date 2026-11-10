#ifndef _CMEDIUMALLOCPOOL
#define _CMEDIUMALLOCPOOL

#include "Kyoto/Alloc/AllocatorCommon.hpp"
#include <rstl/auto_ptr.hpp>
#include <rstl/list.hpp>
#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
#include <rstl/reserved_vector.hpp>
#endif

#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
class CMediumAllocBookKeeping { // Guessed name
public:
  CMediumAllocBookKeeping(void* data, uint numBlocks);

  uint GetNumEntries() const { return mNumEntries; }
  uint GetNumBlocks() const { return mNumBlocks; }
  uint GetNumAllocs() const { return mNumAllocs; }

  int Alloc(uint count);
  uint Free(uint index);

private:
  void WriteTags(uint index, uint count, uint flags);
  void CountFreeBefore(uint& total, uint& runs, uint index) const;
  void CountFreeAfter(uint& total, uint& runs, uint index) const;

  uchar* mTags;
  uint mNumEntries;
  uint mNumBlocks;
  uint mNumAllocs;
  uint mSearchIndex;
};
#endif

struct SMediumAllocPuddle {
#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
  SMediumAllocPuddle(uint numBlocks, void* data, bool canErase, int heapIndex);
#else
  SMediumAllocPuddle(const uint numBlocks, void* data, const bool canErase);
#endif
  ~SMediumAllocPuddle();
  void* FindFree(uint blockCount);
#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
  uint Free(const void* ptr);

  uint GetNumBlocks() const { return mBookKeeping.GetNumBlocks(); }
  uint GetNumAllocs() const { return mBookKeeping.GetNumAllocs(); }
  uint GetNumEntries() const { return mBookKeeping.GetNumEntries(); }
  bool CanErase() const { return mCanErase; }
  int GetHeapIndex() const { return mIsMem2; }
  bool Contains(const void* ptr) const {
    const uchar* data = mMainData.get();
    return ptr >= data && ptr < data + GetNumEntries() * 32;
  }
#else
  void* FindFreeEntry(uint blockCount);
  void Free(const void* ptr);

  const uint GetNumBlocks() const { return mNumBlocks; }
  const uint GetNumAllocs() const { return mNumAllocs; }
  const uint GetNumEntries() const { return mNumEntries; }
  const bool CanErase() const { return mCanErase; }
  const uint GetPtrOffset(const void* ptr) const { return (uchar*)ptr - mMainData.get(); }
  static ushort GetBlockOffset(const void* ptrA, const void* ptrB);
  static void InitBookKeeping(uchar* bookKeepingPtr, const ushort blockCount);
#endif

private:
  rstl::auto_ptr< uchar > mMainData;
#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
  CMediumAllocBookKeeping mBookKeeping;
  uint mCanErase : 1;
  uint mIsMem2 : 1;
#else
  uchar* mBookKeeping;
  uchar* mCachedBookKeepingAddr;
  uint mUnused;
  uint mNumBlocks;
  uint mNumAllocs;
  uint mNumEntries;
  bool mCanErase : 1;
#endif
};

class CMediumAllocPool {
public:
#if !RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
  rstl::list< SMediumAllocPuddle > mList;
  rstl::list< SMediumAllocPuddle >::iterator mLastNodePrev;
#endif
  CMediumAllocPool();
#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
  void* Alloc(uint size, int heapIndex);
  void AddPuddle(uint numBlocks, void* data, bool canErase, int heapIndex);
#else
  void* Alloc(uint size);
  void AddPuddle(const uint, void*, const bool);
#endif
  bool HasPuddles() const;
  void ClearPuddles();

  int Free(const void* ptr);

  uint GetTotalEntries();
  uint GetNumBlocksAvailable();
  uint GetNumAllocs();

  static uint GetAllocMemoryRequired(uint numBlocks) { return numBlocks * 32; }
#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
  static uint GetBookKeepingMemoryRequired(uint numBlocks);
#else
  static uint GetBookKeepingMemoryRequired(uint numBlocks) { return numBlocks; }
#endif

  static CMediumAllocPool* gMediumAllocPtr;
#if RSTL_KYOTO_ALLOCATOR_EXTENDED_MEDIUM_POOL
  uint GetAvailableBytes(int heapIndex) const; // Guessed name
#endif

private:
  typedef rstl::list< SMediumAllocPuddle > Puddles;
#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
  enum { kHeapCount = 2 };
  Puddles::iterator FindNextPuddle(int heapIndex, const Puddles::iterator& current); // Guessed name
  uint mLastRequestBlocks;
  Puddles mList;
  rstl::reserved_vector< Puddles::iterator, kHeapCount > mLastPuddle;
#if RSTL_KYOTO_ALLOCATOR_EXTENDED_MEDIUM_POOL
  rstl::reserved_vector< uint, kHeapCount > mFreeBlocks;
#endif
#endif
};

#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
CHECK_SIZEOF(CMediumAllocBookKeeping, 0x14)
CHECK_SIZEOF(SMediumAllocPuddle, 0x20)
CHECK_SIZEOF(CMediumAllocPool, RSTL_KYOTO_ALLOCATOR_EXTENDED_MEDIUM_POOL ? 0x30 : 0x24)
#endif

#endif // _CMEDIUMALLOCPOOL
