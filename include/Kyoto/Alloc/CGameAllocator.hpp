#ifndef _CGAMEALLOCATOR
#define _CGAMEALLOCATOR

#include "Kyoto/Alloc/AllocatorCommon.hpp"
#include "Kyoto/Alloc/CMediumAllocPool.hpp"
#include "Kyoto/Alloc/IAllocator.hpp"

#include <types.h>

#include "rstl/RstlVersions.h"

class CSmallAllocPool;
class CMediumAllocPool;
class COsContext;
class CGameAllocator : public IAllocator {
public:
  class SGameMemInfo {
    friend class CGameAllocator;

  public:
#if !RSTL_KYOTO_PROTO_SOURCE_FORMS
    // Guessed name
    static size_t PostGuard() {
      enum EGuard { kG_Value = EXPAND_PATTERN(0xEF) };
      EGuard guard = kG_Value;
      return guard;
    }

    SGameMemInfo(SGameMemInfo* const& prev, SGameMemInfo* next, SGameMemInfo* nextFree, size_t len,
                 const char* fileAndLine, const char* type)
    : mPostGuard(PostGuard())
#else
    SGameMemInfo(SGameMemInfo* prev, SGameMemInfo* next, SGameMemInfo* nextFree, size_t len,
                 const char* fileAndLine, const char* type)
    : mPostGuard(0xefefefef)
#endif
    , mSize(len)
    , mFileAndLine(fileAndLine)
    , mType(type)
    , mPrev(prev)
    , mNext(next)
    , mNextFree(nextFree)
    , mGuardBlockStart(kAllocatorPriorGuard) {
    }

    SGameMemInfo* GetPrev() const {
      return reinterpret_cast< SGameMemInfo* >(reinterpret_cast< uintptr_t >(mPrev) &
                                               ~(kAllocatorPointerBits - 1));
    }
    void SetPrev(SGameMemInfo* prev) {
      void* ptr = mPrev;
      mPrev = prev;
      mPrev = reinterpret_cast< SGameMemInfo* >(
          (reinterpret_cast< uintptr_t >(ptr) & (kAllocatorPointerBits - 1)) |
          (reinterpret_cast< uintptr_t >(mPrev) & ~(kAllocatorPointerBits - 1)));
    }
    SGameMemInfo* GetNext() const {
      return reinterpret_cast< SGameMemInfo* >(reinterpret_cast< uintptr_t >(mNext) &
                                               ~(kAllocatorPointerBits - 1));
    }
    void SetNext(SGameMemInfo* next) {
      void* ptr = mNext;
      mNext = next;
      mNext = reinterpret_cast< SGameMemInfo* >(
          (reinterpret_cast< uintptr_t >(ptr) & (kAllocatorPointerBits - 1)) |
          (reinterpret_cast< uintptr_t >(mNext) & ~(kAllocatorPointerBits - 1)));
    }
    uint GetPrevMaskedFlags() const {
      return reinterpret_cast< uintptr_t >(mPrev) & (kAllocatorPointerBits - 1);
    }
    void SetPrevMaskedFlags(const uint flags) {
      mPrev = reinterpret_cast< SGameMemInfo* >(
          (reinterpret_cast< uintptr_t >(mPrev) & ~(kAllocatorPointerBits - 1)) | flags);
    }
    void SetAllocated(const bool allocated) {
      if (allocated) {
        SetPrevMaskedFlags((GetPrevMaskedFlags() & ~1) | 1);
      } else {
        mPrev = reinterpret_cast< SGameMemInfo* >(reinterpret_cast< uintptr_t >(mPrev) & ~1);
      }
    }
    uint GetNextMaskedFlags();
    void SetTopOfHeapAllocated(const bool topOfHeap) {
      const uint flags = GetPrevMaskedFlags();
      SGameMemInfo* prev = GetPrev();
      uint topFlag = 0;
      if (topOfHeap) {
        topFlag = 2;
      }
      mPrev = reinterpret_cast< SGameMemInfo* >(reinterpret_cast< uintptr_t >(prev) |
                                                (topFlag | (flags & ~2)));
    }
    size_t GetLength() const { return mSize; }
    void SetLength(const size_t len) { mSize = len; }
    SGameMemInfo* GetNextFree() const {
      return reinterpret_cast< SGameMemInfo* >(reinterpret_cast< uintptr_t >(mNextFree) &
                                               ~(kAllocatorPointerBits - 1));
    }
    void SetNextFree(SGameMemInfo* info) {
      void* ptr = mNextFree;
      mNextFree = info;
      mNextFree = reinterpret_cast< SGameMemInfo* >(
          (reinterpret_cast< uintptr_t >(ptr) & (kAllocatorPointerBits - 1)) |
          (reinterpret_cast< uintptr_t >(mNextFree) & ~(kAllocatorPointerBits - 1)));
    }

    bool IsAllocated() const { return reinterpret_cast< uintptr_t >(mPrev) & 1; }
    void SetNotAllocated() {
      void* ptr = mPrev;
      mPrev = reinterpret_cast< SGameMemInfo* >(reinterpret_cast< uintptr_t >(ptr) & ~1);
    }

    bool IsPostGuardBlockIntact() const { return mPostGuard == kAllocatorPostGuard; }
    bool IsPriorGuardBlockIntact() const { return mGuardBlockStart == kAllocatorPriorGuard; }

  private:
    // Guards the end of the previous block's data.
    size_t mPostGuard; // Guessed name
    size_t mSize;
    const char* mFileAndLine;
    const char* mType;
    SGameMemInfo* mPrev;
    SGameMemInfo* mNext;
    SGameMemInfo* mNextFree;
    // Guards the start of this block's data.
    size_t mGuardBlockStart;
  };

  SGameMemInfo* GetMemInfoFromBlockPtr(const void* ptr) const;
  CGameAllocator();

  // IAllocator
  ~CGameAllocator();
  bool Initialize(COsContext& ctx) override;
  void Shutdown() override;
  void* Alloc(size_t size, EHint hint, EScope scope, EType type, const CCallStack& cs) override;
  SGameMemInfo* FindFreeBlock(uint);
  SGameMemInfo* FindFreeBlockFromTopOfHeap(uint);
  uint FixupAllocPtrs(SGameMemInfo*, const uint, uint, EHint, const CCallStack&);
  void UpdateAllocDebugStats(unsigned int, unsigned int, unsigned int);
  bool Free(const void* ptr) override;
  bool FreeNormalAllocation(const void* ptr);
  void ReleaseAll() override;
  void* AllocSecondary(size_t size, EHint hint, EScope scope, EType type,
                       const CCallStack& cs) override;
  bool FreeSecondary(const void* ptr) override;
  void ReleaseAllSecondary() override;
  void SetOutOfMemoryCallback(FOutOfMemoryCb cb, const void* target) override;
  int EnumAllocations(FEnumAllocationsCb func, const void* ptr, bool b) const override;
  SAllocInfo GetAllocInfo(const void* ptr) const override;
#if RSTL_KYOTO_ALLOCATOR_METRICS_RESET
  SMetrics GetMetrics(bool resetPeriodicPeak, bool resetPeak) const override;
#else
  SMetrics GetMetrics() const override;
#endif
  void OffsetFakeStatics(int offset) override;

  static uint GetFreeBinEntryForSize(uint);
  void AddFreeEntryToFreeList(SGameMemInfo*);
  void RemoveFreeEntryFromFreeList(SGameMemInfo*);
  void DumpAllocations() const;
  size_t GetLargestFreeChunk() const;

private:
  SGameMemInfo** GetBinPtr(uint bin) { return &mBins[bin]; }
  uchar mInitialized; // Guessed name
  // Unresolved bytes; the target does not initialize these with mInitialized.
  uchar x5_;
  uchar x6_;
  uchar x7_;
  uint mHeapSize;
  SGameMemInfo* mFirst;
  SGameMemInfo* mLast;
  SGameMemInfo* mBins[16];
  uint x54_; // Only zeroed by construction and Shutdown; purpose unresolved.
  FOutOfMemoryCb mOomCallback;
  const void* mOomTarget;
  CSmallAllocPool* mSmallAllocPool;
  void* mSmallAllocMainData;
  void* mSmallAllocBookKeeping;
  bool mSmallAllocFailed;    // Guessed name
  int mSmallAllocRetryDelay; // Guessed name
  CMediumAllocPool* mMediumPool;
  void* mInitialMediumPoolData;         // Guessed name
  bool mMediumAllocFailed;              // Guessed name
  uint mTotalHeapAllocations;           // Guessed name
  uint mCurrentHeapAllocations;         // Guessed name
  uint mRequestedHeapBytes;             // Guessed name
  uint mAllocatedHeapBytes;             // Guessed name
  uint mFreeHeapBytes;                  // Guessed name
  uint mPeakHeapAllocations;            // Guessed name
  mutable uint mPeakAllocatedHeapBytes; // Guessed name
#if RSTL_KYOTO_ALLOCATOR_METRICS_RESET
  mutable uint mPeriodicPeakAllocatedHeapBytes; // Guessed name
#endif
  uint mMinAllocationSize;                    // Guessed name
  uint mMaxAllocationSize;                    // Guessed name
  uint mAverageAllocationSize;                // Guessed name
  uint mSmallHeapAllocations;                 // Guessed name
  uint mPeakSmallPoolEntries;                 // Guessed name
  mutable uint mPreviousTotalHeapAllocations; // Guessed name
  void* mPhysicalAddr;
  uint mFakeStatics;
  uint mSmallPoolReservedBytes; // Guessed name
};

CHECK_SIZEOF(CGameAllocator, RSTL_KYOTO_ALLOCATOR_METRICS_RESET ? 0xc4 : 0xc0)
NESTED_CHECK_SIZEOF(CGameAllocator, SGameMemInfo, 0x20)

#endif // _CGAMEALLOCATOR
