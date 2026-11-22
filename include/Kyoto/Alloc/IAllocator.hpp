#ifndef _IALLOCATOR
#define _IALLOCATOR

#include "Kyoto/Alloc/AllocatorCommon.hpp"
#include "rstl/types.h"
#include <stddef.h>

#include "rstl/RstlVersions.h"

class COsContext;
class CCallStack;

class IAllocator {
public:
  enum EHint {
    kHI_None = 0,
    kHI_TopOfHeap = (1 << 0),
    kHI_RoundUpLen = (1 << 1),
    kHI_MEM2 = (1 << 2),
    kHI_NoMEM2Fallback = (1 << 3),
  };

  enum EScope {
    kSC_Unk0,
    kSC_Unk1,
  };

  enum EType {
    kTP_Heap,
    kTP_Array,
  };

  struct SMetrics {
    uint mHeapSize;
    uint mTotalHeapAllocations;
    uint mCurrentHeapAllocations;
    uint mRequestedHeapBytes;
    uint mAllocatedHeapBytes;
    uint mFreeHeapBytes;
    uint mPeakHeapAllocations;
    uint mPeakAllocatedHeapBytes;
#if RSTL_KYOTO_ALLOCATOR_METRICS_RESET
    uint mPeriodicPeakAllocatedHeapBytes;
#endif
    uint mMinAllocationSize;
    uint mMaxAllocationSize;
    uint mAverageAllocationSize;
    uint mSmallNumAllocs;
    uint mSmallAllocatedBlocks;
    uint mSmallAvailableBlocks;
    uint mMediumNumAllocs;
    uint mMediumAllocatedBlocks;
    uint mMediumBlocksAvailable;
    uint mAllocationsSinceLastSnapshot;
    uint mHeapPhysicalAddress;
    uint mSmallPoolReservedBytes;
    uint mMediumPoolDataBytes;
    uint mFakeStatics;

    SMetrics(uint heapSize, uint totalHeapAllocations, uint currentHeapAllocations,
             uint requestedHeapBytes, uint allocatedHeapBytes, uint freeHeapBytes,
             uint peakHeapAllocations, uint peakAllocatedHeapBytes,
#if RSTL_KYOTO_ALLOCATOR_METRICS_RESET
             uint periodicPeakAllocatedHeapBytes,
#endif
             uint minAllocationSize, uint maxAllocationSize, uint averageAllocationSize,
             uint smallNumAllocs, uint smallAllocatedBlocks, uint smallAvailableBlocks,
             uint mediumNumAllocs, uint mediumAllocatedBlocks, uint mediumAvailableBlocks,
             uint allocationsSinceLastSnapshot, uint heapPhysicalAddress, uint smallPoolReservedBytes,
             uint mediumPoolDataBytes, uint fakeStatics);
#if RSTL_KYOTO_METRICS_COPY_CTOR
    SMetrics(const SMetrics& other);
#endif
  };

  struct SAllocatorMetrics { // Guessed name
    uint mSmallNumAllocs;
    uint mSmallAllocatedBlocks;
    uint mSmallAvailableBlocks;
    uint mMediumNumAllocs;
    uint mMediumAllocatedBlocks;
    uint mMediumAvailableBlocks;
    uint mMem1HeapPhysicalAddress;
    uint mSmallPoolReservedBytes;
    uint mMediumPoolReservedBytes;
    uint mHeapSize;
    uint mAllocatedHeapBytes;

    SAllocatorMetrics(uint smallNumAllocs, uint smallAllocatedBlocks, uint smallAvailableBlocks,
                      uint mediumNumAllocs, uint mediumAllocatedBlocks, uint mediumAvailableBlocks,
                      uint mem1HeapPhysicalAddress, uint smallPoolReservedBytes,
                      uint mediumPoolReservedBytes, uint heapSize, uint allocatedHeapBytes);
  };

  struct SHeapMetrics { // Guessed name
    uint mHeapSize;
    uint mTotalHeapAllocations;
    uint mCurrentHeapAllocations;
    uint mRequestedHeapBytes;
    uint mAllocatedHeapBytes;
    uint mFreeHeapBytes;
    uint mPeakHeapAllocations;
    uint mPeakAllocatedHeapBytes;
    uint mPeriodicPeakAllocatedHeapBytes;
    uint mMinAllocationSize;
    uint mMaxAllocationSize;
    uint mAverageAllocationSize;
    uint x30_;

    SHeapMetrics(uint heapSize, uint totalHeapAllocations, uint currentHeapAllocations,
                 uint requestedHeapBytes, uint allocatedHeapBytes, uint freeHeapBytes,
                 uint peakHeapAllocations, uint peakAllocatedHeapBytes,
                 uint periodicPeakAllocatedHeapBytes, uint minAllocationSize,
                 uint maxAllocationSize, uint averageAllocationSize, uint unk30);
  };

  struct SAllocInfo {
    const void* mInfoPtr;
    uint mLen;
    bool mIsAllocated;
    bool x9_;
    const char* mFileAndLine;
    const char* mType;

    SAllocInfo(const void* ptr, uint len, bool isAllocated, bool b2, const char* fileAndLine,
               const char* type)
    : mInfoPtr(ptr)
    , mLen(len)
    , mIsAllocated(isAllocated)
    , x9_(b2)
    , mFileAndLine(fileAndLine)
    , mType(type) {}
  };

  typedef const bool (*FOutOfMemoryCb)(const void*, uint);
#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
  typedef const bool (*FEnumAllocationsCb)(int heapIndex, const SAllocInfo& info, const void* ptr);
#else
  typedef const bool (*FEnumAllocationsCb)(const SAllocInfo& info, const void* ptr);
#endif

  virtual ~IAllocator();

  virtual bool Initialize(COsContext& ctx) = 0;
  virtual void Shutdown() = 0;
  virtual void* Alloc(size_t size, EHint hint, EScope scope, EType type, const CCallStack& cs) = 0;
  virtual bool Free(const void* ptr) = 0;
  virtual void ReleaseAll() = 0;
  virtual void* AllocSecondary(size_t size, EHint hint, EScope scope, EType type,
                               const CCallStack& cs) = 0;
  virtual bool FreeSecondary(const void* ptr) = 0;
  virtual void ReleaseAllSecondary() = 0;
  virtual void SetOutOfMemoryCallback(FOutOfMemoryCb cb, const void* data) = 0;
  virtual int EnumAllocations(FEnumAllocationsCb func, const void* ptr, bool b) const = 0;
  virtual SAllocInfo GetAllocInfo(const void* ptr) const = 0;
#if RSTL_KYOTO_ALLOCATOR_OFFSET_FAKE_STATICS
  virtual void OffsetFakeStatics(int offset) = 0;
#endif
#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
  virtual SAllocatorMetrics GetMetrics() const = 0;
  virtual int GetHeapCount() const = 0;
  virtual SHeapMetrics GetHeapMetrics(int index, uint flags) const = 0;
  virtual const char* GetHeapName(int index) const = 0;
#elif RSTL_KYOTO_ALLOCATOR_METRICS_RESET
  virtual SMetrics GetMetrics(bool resetPeriodicPeak, bool resetPeak) const = 0;
#else
  virtual SMetrics GetMetrics() const = 0;
#endif
};

#endif // _IALLOCATOR
