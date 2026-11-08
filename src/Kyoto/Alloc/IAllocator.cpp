#include "Kyoto/Alloc/IAllocator.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/COsContext.hpp"

#if RSTL_KYOTO_ALLOCATOR_METRICS_RESET
IAllocator::SMetrics::SMetrics(uint heapSize, uint unk1, uint unk2, uint unk3, uint unk4,
                               uint heapSize2, uint unk5, uint unk6, uint unk7, uint unk8,
                               uint unk9, uint unk10, uint smallAllocNumAllocs,
                               uint smallAllocAllocatedSize, uint smallAllocRemainingSize,
                               uint mediumAllocNumAllocs, uint mediumAllocAllocatedSize,
                               uint mediumAllocBlocksAvailable, uint unk11, uint unk12, uint unk13,
                               uint mediumAllocTotalAllocated, uint fakeStatics)
#else
IAllocator::SMetrics::SMetrics(uint heapSize, uint unk1, uint unk2, uint unk3, uint unk4,
                               uint heapSize2, uint unk5, uint unk6, uint unk7, uint unk8,
                               uint unk9, uint smallAllocNumAllocs, uint smallAllocAllocatedSize,
                               uint smallAllocRemainingSize, uint mediumAllocNumAllocs,
                               uint mediumAllocAllocatedSize, uint mediumAllocBlocksAvailable,
                               uint unk10, uint unk11, uint unk12, uint mediumAllocTotalAllocated,
                               uint fakeStatics)
#endif
: mHeapSize(heapSize)
, x4_(unk1)
, x8_(unk2)
, xc_(unk3)
, x10_(unk4)
, mHeapSize2(heapSize2)
, x18_(unk5)
, x1c_(unk6)
, x20_(unk7)
, x24_(unk8)
, x28_(unk9)
#if RSTL_KYOTO_ALLOCATOR_METRICS_RESET
, x2c_(unk10)
#endif
, mSmallNumAllocs(smallAllocNumAllocs)
, mSmallAllocatedSize(smallAllocAllocatedSize)
, mSmallRemainingSize(smallAllocRemainingSize)
, mMediumNumAllocs(mediumAllocNumAllocs)
, mMediumAllocatedSize(mediumAllocAllocatedSize)
, mMediumBlocksAvailable(mediumAllocBlocksAvailable)
#if RSTL_KYOTO_ALLOCATOR_METRICS_RESET
, x48_(unk11)
, x4c_(unk12)
, x50_(unk13)
#else
, x44_(unk10)
, x48_(unk11)
, x4c_(unk12)
#endif
, mMediumTotalAllocated(mediumAllocTotalAllocated)
, mFakeStatics(fakeStatics) {
}

IAllocator::SAllocatorMetrics::SAllocatorMetrics(
    uint smallNumAllocs, uint smallAllocatedBlocks, uint smallAvailableBlocks, uint mediumNumAllocs,
    uint mediumAllocatedBlocks, uint mediumAvailableBlocks, uint mem1HeapPhysicalAddress,
    uint smallPoolReservedBytes, uint mediumPoolReservedBytes, uint heapSize,
    uint allocatedHeapBytes)
: mSmallNumAllocs(smallNumAllocs)
, mSmallAllocatedBlocks(smallAllocatedBlocks)
, mSmallAvailableBlocks(smallAvailableBlocks)
, mMediumNumAllocs(mediumNumAllocs)
, mMediumAllocatedBlocks(mediumAllocatedBlocks)
, mMediumAvailableBlocks(mediumAvailableBlocks)
, mMem1HeapPhysicalAddress(mem1HeapPhysicalAddress)
, mSmallPoolReservedBytes(smallPoolReservedBytes)
, mMediumPoolReservedBytes(mediumPoolReservedBytes)
, mHeapSize(heapSize)
, mAllocatedHeapBytes(allocatedHeapBytes) {}

IAllocator::SHeapMetrics::SHeapMetrics(uint heapSize, uint totalHeapAllocations,
                                       uint currentHeapAllocations, uint requestedHeapBytes,
                                       uint allocatedHeapBytes, uint freeHeapBytes,
                                       uint peakHeapAllocations, uint peakAllocatedHeapBytes,
                                       uint periodicPeakAllocatedHeapBytes, uint minAllocationSize,
                                       uint maxAllocationSize, uint averageAllocationSize,
                                       uint unk30)
: mHeapSize(heapSize)
, mTotalHeapAllocations(totalHeapAllocations)
, mCurrentHeapAllocations(currentHeapAllocations)
, mRequestedHeapBytes(requestedHeapBytes)
, mAllocatedHeapBytes(allocatedHeapBytes)
, mFreeHeapBytes(freeHeapBytes)
, mPeakHeapAllocations(peakHeapAllocations)
, mPeakAllocatedHeapBytes(peakAllocatedHeapBytes)
, mPeriodicPeakAllocatedHeapBytes(periodicPeakAllocatedHeapBytes)
, mMinAllocationSize(minAllocationSize)
, mMaxAllocationSize(maxAllocationSize)
, mAverageAllocationSize(averageAllocationSize)
, x30_(unk30) {}

IAllocator::~IAllocator() {}
