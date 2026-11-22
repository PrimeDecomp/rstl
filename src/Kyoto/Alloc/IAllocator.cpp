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
, mTotalHeapAllocations(totalHeapAllocations)
, mCurrentHeapAllocations(currentHeapAllocations)
, mRequestedHeapBytes(requestedHeapBytes)
, mAllocatedHeapBytes(allocatedHeapBytes)
, mFreeHeapBytes(freeHeapBytes)
, mPeakHeapAllocations(peakHeapAllocations)
, mPeakAllocatedHeapBytes(peakAllocatedHeapBytes)
#if RSTL_KYOTO_ALLOCATOR_METRICS_RESET
, mPeriodicPeakAllocatedHeapBytes(periodicPeakAllocatedHeapBytes)
#endif
, mMinAllocationSize(minAllocationSize)
, mMaxAllocationSize(maxAllocationSize)
, mAverageAllocationSize(averageAllocationSize)
, mSmallNumAllocs(smallNumAllocs)
, mSmallAllocatedBlocks(smallAllocatedBlocks)
, mSmallAvailableBlocks(smallAvailableBlocks)
, mMediumNumAllocs(mediumNumAllocs)
, mMediumAllocatedBlocks(mediumAllocatedBlocks)
, mMediumBlocksAvailable(mediumAvailableBlocks)
, mAllocationsSinceLastSnapshot(allocationsSinceLastSnapshot)
, mHeapPhysicalAddress(heapPhysicalAddress)
, mSmallPoolReservedBytes(smallPoolReservedBytes)
, mMediumPoolDataBytes(mediumPoolDataBytes)
, mFakeStatics(fakeStatics) {}

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
