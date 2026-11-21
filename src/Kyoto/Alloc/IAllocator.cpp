#include "Kyoto/Alloc/IAllocator.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/COsContext.hpp"

IAllocator::SMetrics::SMetrics(
    uint heapSize, uint totalHeapAllocations, uint currentHeapAllocations, uint requestedHeapBytes,
    uint allocatedHeapBytes, uint freeHeapBytes, uint peakHeapAllocations, uint peakAllocatedHeapBytes,
#if RSTL_KYOTO_ALLOCATOR_METRICS_RESET
    uint periodicPeakAllocatedHeapBytes,
#endif
    uint minAllocationSize, uint maxAllocationSize, uint averageAllocationSize, uint smallNumAllocs,
    uint smallAllocatedBlocks, uint smallAvailableBlocks, uint mediumNumAllocs,
    uint mediumAllocatedBlocks, uint mediumAvailableBlocks, uint allocationsSinceLastSnapshot,
    uint heapPhysicalAddress, uint smallPoolReservedBytes, uint mediumPoolDataBytes, uint fakeStatics)
: mHeapSize(heapSize)
, x4_(totalHeapAllocations)
, x8_(currentHeapAllocations)
, xc_(requestedHeapBytes)
, x10_(allocatedHeapBytes)
, mHeapSize2(freeHeapBytes)
, x18_(peakHeapAllocations)
, x1c_(peakAllocatedHeapBytes)
#if RSTL_KYOTO_ALLOCATOR_METRICS_RESET
, x20_(periodicPeakAllocatedHeapBytes)
, x24_(minAllocationSize)
, x28_(maxAllocationSize)
, x2c_(averageAllocationSize)
#else
, x20_(minAllocationSize)
, x24_(maxAllocationSize)
, x28_(averageAllocationSize)
#endif
, mSmallNumAllocs(smallNumAllocs)
, mSmallAllocatedSize(smallAllocatedBlocks)
, mSmallRemainingSize(smallAvailableBlocks)
, mMediumNumAllocs(mediumNumAllocs)
, mMediumAllocatedSize(mediumAllocatedBlocks)
, mMediumBlocksAvailable(mediumAvailableBlocks)
#if RSTL_KYOTO_ALLOCATOR_METRICS_RESET
, x48_(allocationsSinceLastSnapshot)
, x4c_(heapPhysicalAddress)
, x50_(smallPoolReservedBytes)
#else
, x44_(allocationsSinceLastSnapshot)
, x48_(heapPhysicalAddress)
, x4c_(smallPoolReservedBytes)
#endif
, mMediumTotalAllocated(mediumPoolDataBytes)
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
