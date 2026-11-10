#include "dolphin/types.h"
#include "rstl/RstlVersions.h"
#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
#include "dolphin/os/OSArena.h"
#if RSTL_KYOTO_CORRUPTION || RSTL_KYOTO_ALLOCATOR_EXTENDED_MEDIUM_POOL
#include "dolphin/dvd.h"
#endif
#endif

#include "Kyoto/Alloc/CGameAllocator.hpp"

#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Alloc/CCallStack.hpp"
#include "Kyoto/Alloc/CMediumAllocPool.hpp"
#include "Kyoto/Alloc/CSmallAllocPool.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Basics/COsContext.hpp"
#include "Kyoto/Basics/CStopwatch.hpp"

#include "rstl/algorithm.hpp"
#include "rstl/reserved_vector.hpp"

#include "stddef.h"
#include <stdint.h>
#include <string.h>

#if !RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
/* Here just to make sure the data section matches */
#if RSTL_KYOTO_DEBUG_BUILD
static const char* const string_NULL = "<NULL>";
#else
static const char* string_NULL = "<NULL>";
#endif
static const char* string_SOURCE_MODULE_UNLOADED = "<SOURCE MODULE UNLOADED>";
static const char* string_ = "";
static int gAllocatorTime = 0;
#if RSTL_KYOTO_DEBUG_BUILD
static const uchar kPostGuard[] = {
    0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef,
    0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef,
    0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef,
    0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef,
};
#endif

template < typename U1, typename U2 >
static inline U1 T_round_up(U2 val, int align) {
  return (val + (align - 1)) & ~(align - 1);
}

#endif

CGameAllocator::SGameMemInfo* CGameAllocator::GetMemInfoFromBlockPtr(const void* ptr) const {
  return reinterpret_cast< SGameMemInfo* >(reinterpret_cast< uintptr_t >(ptr) -
                                           sizeof(SGameMemInfo));
}

CGameAllocator::CGameAllocator()
: mInitialized(0)
#if !RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
, mHeapSize(0)
, mFirst(nullptr)
, mLast(nullptr)
, x54_(0)
#endif
, mOomCallback(nullptr)
, mOomTarget(nullptr)
, mSmallAllocPool(nullptr)
, mSmallAllocMainData(nullptr)
, mSmallAllocBookKeeping(nullptr)
, mSmallAllocFailed(false)
, mSmallAllocRetryDelay(0)
, mMediumPool(nullptr)
#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
, mMediumAllocFailed(false)
, mSmallHeapAllocations(0)
, mPeakSmallPoolEntries(0)
, mMem1HeapPhysicalAddress(0)
, x148_(0)
#else
, mTotalHeapAllocations(0)
, mCurrentHeapAllocations(0)
, mRequestedHeapBytes(0)
, mAllocatedHeapBytes(0)
, mFreeHeapBytes(0)
, mPeakHeapAllocations(0)
, mPeakAllocatedHeapBytes(0)
#if RSTL_KYOTO_ALLOCATOR_METRICS_RESET
, mPeriodicPeakAllocatedHeapBytes(0)
#endif
, mMinAllocationSize(0)
, mMaxAllocationSize(0)
, mAverageAllocationSize(0)
, mSmallHeapAllocations(0)
, mPeakSmallPoolEntries(0)
, mPreviousTotalHeapAllocations(0)
, mPhysicalAddr(nullptr)
, mFakeStatics(0)
#endif
, mSmallPoolReservedBytes(0)
#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
, mAllowMem2Fallback(false)
, mGrowingMediumPool(false)
#if RSTL_KYOTO_ALLOCATOR_EXTENDED_MEDIUM_POOL
, mAllocatingPuddle(false)
#endif
, mHandlingOutOfMemory(false)
#endif
{
#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
  mMediumPoolMem2 = nullptr;
  mMediumPoolMem1 = nullptr;
#else
  SGameMemInfo testSGameMemInfo(nullptr, nullptr, nullptr, 0, string_NULL, string_NULL);
  // clang-format off
  RS_VERIFY_THROW(172, (reinterpret_cast<uint>(&testSGameMemInfo.mGuardBlockStart) - reinterpret_cast<uint>(&testSGameMemInfo)) == 28, kXCP_InvalidSGameMemInfo, "SGameMemInfo must have mGuardBlockStart at end of structure.");
  // clang-format on
#endif
}

CGameAllocator::~CGameAllocator() {
  if (mMediumPool) {
    mMediumPool->ClearPuddles();
    FreeNormalAllocation(mMediumPool);
    mMediumPool = nullptr;
  }
}

#if !RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
bool CGameAllocator::Initialize(COsContext& ctx) {
  mHeapSize = ctx.GetBaseFreeRam() - 2 * sizeof(SGameMemInfo);
  mFirst = static_cast< SGameMemInfo* >(OSAllocFromArenaLo(mHeapSize, sizeof(SGameMemInfo)));
  mPhysicalAddr = reinterpret_cast< void* >(
      reinterpret_cast< intptr_t >(mFirst) -
      (reinterpret_cast< uintptr_t >(mFirst) & kAllocatorPointerTopNybbleMask));
  OSGetArenaLo();
#if !RSTL_KYOTO_PROTO_SOURCE_FORMS
  mLast = reinterpret_cast< SGameMemInfo* >(reinterpret_cast< uintptr_t >(mFirst) + mHeapSize -
                                            sizeof(SGameMemInfo));

  SGameMemInfo* headDest = mFirst;
  SGameMemInfo* headNext = mLast;
  const SGameMemInfo& head = SGameMemInfo(
      nullptr, headNext, headNext, mHeapSize - sizeof(SGameMemInfo) * 2, "MemHead", "MemHead");
  *headDest = head;
#else
  mLast = reinterpret_cast< SGameMemInfo* >(reinterpret_cast< char* >(mFirst - 1) + mHeapSize);

  const SGameMemInfo& head = SGameMemInfo(
      nullptr, mLast, mLast, mHeapSize - sizeof(SGameMemInfo) * 2, "MemHead", "MemHead");
  *mFirst = head;
#endif
  const SGameMemInfo& tail = SGameMemInfo(mFirst, nullptr, nullptr, 0, "MemTail", "MemTail");
  *mLast = tail;
  for (uint i = 0; i < 16; i++) {
    mBins[i] = nullptr;
  }

  AddFreeEntryToFreeList(mFirst);
  mTotalHeapAllocations = 0;
  mCurrentHeapAllocations = 0;
  mRequestedHeapBytes = 0;
  mAllocatedHeapBytes = 0;
  mFreeHeapBytes = mHeapSize;
  mPeakHeapAllocations = 0;
  mPeakAllocatedHeapBytes = 0;
#if RSTL_KYOTO_ALLOCATOR_METRICS_RESET && !RSTL_KYOTO_PROTO_SOURCE_FORMS
  mPeriodicPeakAllocatedHeapBytes = 0;
#endif
  mMinAllocationSize = 0;
  mMaxAllocationSize = 0;
  mAverageAllocationSize = 0;
  mSmallHeapAllocations = 0;
  mInitialized = 1;

  mSmallAllocMainData = Alloc(0xb0000, kHI_None, kSC_Unk1, kTP_Heap,
                              CCallStack(0xffffffff, "SmallAllocMainData   ", " - Ignore"));

  mSmallAllocBookKeeping = Alloc(0x16000, kHI_None, kSC_Unk1, kTP_Heap,
                                 CCallStack(0xffffffff, "SmallAllocBookKeeping", " - Ignore"));

  mSmallAllocPool = new (Alloc(0x20, kHI_None, kSC_Unk1, kTP_Heap,
                               CCallStack(0xffffffff, "SmallAllocClass      ", " - Ignore")))
      CSmallAllocPool(0x2c000, mSmallAllocMainData, mSmallAllocBookKeeping);

  mMediumPool =
      new (Alloc(0x1c, kHI_None, kSC_Unk1, kTP_Heap,
                 CCallStack(0xffffffff, "MediumAllocClass      ", " - Ignore"))) CMediumAllocPool();

#if !RSTL_KYOTO_PROTO_SOURCE_FORMS
  const char* line = "MediumAllocMainData   ";
  enum EMediumPoolSize { kMPS_MainDataSize = 0x20000 };
  EMediumPoolSize mediumSize = kMPS_MainDataSize;
  mInitialMediumPoolData =
      Alloc(mediumSize + CMediumAllocPool::GetBookKeepingMemoryRequired(0x1000), kHI_None, kSC_Unk1,
            kTP_Heap, CCallStack(0xffffffff, line, " - Ignore"));
  mCurrentHeapAllocations -= 4;
  mSmallPoolReservedBytes = 0xc6000;
#else
  uint mediumSize = CMediumAllocPool::GetAllocMemoryRequired(0x1000);
  mediumSize += CMediumAllocPool::GetBookKeepingMemoryRequired(0x1000);
  mInitialMediumPoolData = Alloc(mediumSize, kHI_None, kSC_Unk1, kTP_Heap,
                                 CCallStack(0xffffffff, "MediumAllocMainData   ", " - Ignore"));
  mSmallPoolReservedBytes = 0xc6000;
  mCurrentHeapAllocations -= 4;
#endif
  return true;
}

#endif

void CGameAllocator::Shutdown() {
  ReleaseAll();
  mInitialized = 0;
#if !RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
  x54_ = 0;
#endif
}

#if !RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
void* CGameAllocator::Alloc(size_t size, const EHint hint, const EScope scope, const EType type,
                            const CCallStack& callstack) {
  uint tmp = 0;
  void* buf = nullptr;
  const OSTick startTick = OSGetTick();

  if (hint & kHI_RoundUpLen) {
    size = T_round_up< size_t, size_t >(size, 32);
  }

  bool canUseSmallPool =
      size <= 56 && !(hint & (kHI_RoundUpLen | kHI_TopOfHeap)) && mSmallAllocPool;

  if (canUseSmallPool && mSmallAllocRetryDelay > 0) {
    canUseSmallPool = false;
    --mSmallAllocRetryDelay;
  }

  if (canUseSmallPool) {
    buf = mSmallAllocPool->Alloc(size);
    tmp = mSmallAllocPool->GetAllocatedSize();
    if (mPeakSmallPoolEntries < tmp) {
      mPeakSmallPoolEntries = tmp;
      static int sLastSmallAllocSize = 0;
      if (sLastSmallAllocSize + 128 < tmp) {
        sLastSmallAllocSize = tmp;
      }
    }

    if (buf != nullptr) {
      gAllocatorTime += (OSGetTick() - startTick);
      return buf;
    }
    mSmallAllocRetryDelay = 25;
    mSmallAllocFailed = true;
  }

  if (mMediumPool && size <= 0x400 && !(hint & kHI_TopOfHeap)) {
    if (!mMediumPool->HasPuddles()) {
#if !RSTL_KYOTO_PROTO_SOURCE_FORMS
      enum ENullPointer { kNP_NullPointer = 0 };
      ENullPointer cleared = kNP_NullPointer;
      mMediumPool->AddPuddle(0x1000, mInitialMediumPoolData, false);
      mInitialMediumPoolData = reinterpret_cast< void* >(cleared);
#else
      buf = nullptr;
      mMediumPool->AddPuddle(0x1000, mInitialMediumPoolData, false);
      mInitialMediumPoolData = buf;
#endif
    }

    buf = mMediumPool->Alloc(size);

    if (buf == nullptr) {
      void* puddlePtr = Alloc(CMediumAllocPool::GetAllocMemoryRequired(0x1000) +
                                  CMediumAllocPool::GetBookKeepingMemoryRequired(0x1000),
                              kHI_None, kSC_Unk1, kTP_Heap,
                              CCallStack(-1, "MediumAllocMainData   ", " - Ignore"));
      mMediumPool->AddPuddle(0x1000, puddlePtr, true);
      buf = mMediumPool->Alloc(size);
    }

    if (buf != nullptr) {
      gAllocatorTime += OSGetTick() - startTick;
      return buf;
    }

#if RSTL_KYOTO_ALLOCATOR_DEBUG_QUERIES
    if (!mMediumAllocFailed) {
      mMediumPool->GetTotalEntries();
      mMediumPool->GetNumAllocs();
      mMediumPool->GetNumBlocksAvailable();
    }
#endif
    mMediumAllocFailed = true;
  }

  const bool topOfHeap = (hint & kHI_TopOfHeap) != 0;
  uint roundedSize = T_round_up< size_t, size_t >(size, 32);
  SGameMemInfo* info = nullptr;

  if (topOfHeap) {
    info = FindFreeBlockFromTopOfHeap(roundedSize);
  } else {
    info = FindFreeBlock(roundedSize);
  }

  if (info == nullptr) {
    void* mediumBuf = nullptr;
    if (mOomCallback) {
      mOomCallback(mOomTarget, size);

      static bool bTriedCallback = false;
      if (!bTriedCallback) {
        bTriedCallback = true;
        mediumBuf = Alloc(size, hint, scope, type, callstack);
        bTriedCallback = false;
      } else {
        return nullptr;
      }
    }
    if (mediumBuf == nullptr) {
#if RSTL_KYOTO_ALLOCATOR_DEBUG_QUERIES
      (void)callstack.GetFileAndLineText();
      (void)callstack.GetTypeText();
#endif
      DumpAllocations();
      return nullptr;
    }
    return mediumBuf;
  }

  tmp = FixupAllocPtrs(info, size, roundedSize, hint, callstack);
  if (topOfHeap && !info->IsAllocated()) {
    info = info->GetNext();
  }

  UpdateAllocDebugStats(size, roundedSize, tmp);
  gAllocatorTime += OSGetTick() - startTick;
  return ++info;
}

CGameAllocator::SGameMemInfo* CGameAllocator::FindFreeBlock(uint len) {
  CGameAllocator::SGameMemInfo* ret = nullptr;
  uint binIndex = GetFreeBinEntryForSize(len);

  uint chosenBin = 0;
  SGameMemInfo* previous = nullptr;
  uint bestDelta = 0x10000000;

  for (; binIndex < 16 && !ret; ++binIndex) {
    SGameMemInfo* candidate = mBins[binIndex];
    SGameMemInfo* last = nullptr;
    for (; candidate; last = candidate, candidate = candidate->GetNextFree()) {
      if (!candidate->IsAllocated() && candidate->mSize >= len) {
        if (candidate->mSize - len < bestDelta && candidate->GetNext()) {
          ret = candidate;
          previous = last;
          bestDelta = candidate->mSize - len;
          chosenBin = binIndex;
          if (bestDelta < sizeof(SGameMemInfo)) {
            break;
          }
        }
      }
    }
  }

  if (ret) {
    if (previous == NULL) {
      mBins[chosenBin] = ret->GetNextFree();
    } else {
      previous->SetNextFree(ret->GetNextFree());
    }
  }
  return ret;
}

CGameAllocator::SGameMemInfo* CGameAllocator::FindFreeBlockFromTopOfHeap(uint size) {
  SGameMemInfo* iter = mLast;
  SGameMemInfo* ret = nullptr;

  while (iter != nullptr) {
    if (!iter->IsAllocated() && iter->GetLength() >= size) {
      ret = iter;
      break;
    }
    iter = iter->GetPrev();
  }

  RemoveFreeEntryFromFreeList(ret);
  return ret;
}

uint CGameAllocator::FixupAllocPtrs(SGameMemInfo* info, const uint len, uint roundedLen, EHint hint,
                                    const CCallStack& cs) {

  const bool topOfHeap = (hint & kHI_TopOfHeap) != 0;
  uint ret = 0;
#if !RSTL_KYOTO_PROTO_SOURCE_FORMS
  const SGameMemInfo* initialInfo = info;
  const size_t blockLength = initialInfo->GetLength();
#else
  const size_t blockLength = info->mSize;
#endif
  if (blockLength == roundedLen + sizeof(SGameMemInfo)) {
    ret = sizeof(SGameMemInfo);
    roundedLen += sizeof(SGameMemInfo);
  }

  SGameMemInfo* newPtr = info;
  if (blockLength != roundedLen) {
    SGameMemInfo* newInfo;

    SGameMemInfo* infoNext = info->GetNext();
    if (topOfHeap) {
      newInfo =
          reinterpret_cast< SGameMemInfo* >(reinterpret_cast< char* >(infoNext) - roundedLen) - 1;
      const SGameMemInfo& block = SGameMemInfo(info, infoNext, nullptr, len, "", "");
      *newInfo = block;
      info->mSize -= roundedLen + sizeof(SGameMemInfo);
      AddFreeEntryToFreeList(info);
      newPtr = newInfo;
    } else {
#if !RSTL_KYOTO_PROTO_SOURCE_FORMS
      newInfo = reinterpret_cast< SGameMemInfo* >(reinterpret_cast< uintptr_t >(info) + roundedLen +
                                                  sizeof(SGameMemInfo));
      SGameMemInfo* nextFree = info->mNextFree;
      nextFree = reinterpret_cast< SGameMemInfo* >(reinterpret_cast< uintptr_t >(nextFree) &
                                                   ~(kAllocatorPointerBits - 1));
      const SGameMemInfo& block = SGameMemInfo(
          info, infoNext, nextFree, info->mSize - roundedLen - sizeof(SGameMemInfo), "", "");
#else
      uint offset = roundedLen + sizeof(SGameMemInfo);
      newInfo = reinterpret_cast< SGameMemInfo* >(reinterpret_cast< char* >(info) + offset);
      const SGameMemInfo& block =
          SGameMemInfo(info, infoNext, info->GetNextFree(),
                       info->mSize - roundedLen - sizeof(SGameMemInfo), "", "");
#endif
      *newInfo = block;
      AddFreeEntryToFreeList(newInfo);
    }
    newPtr->mFileAndLine = cs.GetFileAndLineText();
    newPtr->mType = cs.GetTypeText();
    ret = sizeof(SGameMemInfo);

    infoNext->SetPrev(newInfo);
    info->SetNext(newInfo);
  } else {
    info->mFileAndLine = cs.GetFileAndLineText();
    info->mType = cs.GetTypeText();
  }

  newPtr->SetTopOfHeapAllocated(topOfHeap);
  newPtr->SetAllocated(true);
  newPtr->mSize = len;
  return ret;
}

void CGameAllocator::UpdateAllocDebugStats(uint len, uint roundedLen, uint offset) {
  ++mCurrentHeapAllocations;
  ++mTotalHeapAllocations;
  mRequestedHeapBytes += len;
  mAllocatedHeapBytes += roundedLen + offset;
  mFreeHeapBytes -= roundedLen + offset;

  if (mCurrentHeapAllocations > mPeakHeapAllocations) {
    mPeakHeapAllocations = mCurrentHeapAllocations;
  }

  if (mAllocatedHeapBytes > mPeakAllocatedHeapBytes) {
    mPeakAllocatedHeapBytes = mAllocatedHeapBytes;
  }

#if RSTL_KYOTO_ALLOCATOR_METRICS_RESET
  if (mAllocatedHeapBytes > mPeriodicPeakAllocatedHeapBytes) {
    mPeriodicPeakAllocatedHeapBytes = mAllocatedHeapBytes;
  }
#endif

  if (len < mMinAllocationSize) {
    mMinAllocationSize = len;
  }

  if (len > mMaxAllocationSize) {
    mMaxAllocationSize = len;
  }
  mAverageAllocationSize =
      (len + mAverageAllocationSize * (mTotalHeapAllocations - 1)) / mTotalHeapAllocations;
  if (len > 56) {
    return;
  }

  ++mSmallHeapAllocations;
}

#endif

bool CGameAllocator::Free(const void* ptr) {
  if (ptr == nullptr) {
    return true;
  }

  if (mSmallAllocPool && mSmallAllocPool->PtrWithinPool(ptr)) {
    return mSmallAllocPool->Free(ptr);
  }

  if (mMediumPool) {
    const int freed = mMediumPool->Free(ptr);
#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
    if (freed != 0) {
      return true;
    }
#else
    if (freed != 1) {
      return freed > 0;
    }
#endif
  }
#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
  FreeNormalAllocation(ptr);
  return true;
#else
  return FreeNormalAllocation(ptr);
#endif
}

#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
void CGameAllocator::FreeNormalAllocation(const void* p)
#else
bool CGameAllocator::FreeNormalAllocation(const void* p)
#endif
{
#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
  const int heapIndex = GetHeapIndex(p);
#endif
  SGameMemInfo* pMemInfo = GetMemInfoFromBlockPtr(p);
  size_t newLen = 0;
  const size_t infoLen = pMemInfo->mSize;
  SGameMemInfo* pNextMemInfo = pMemInfo->GetNext();
  size_t paddedSize = 0;
  if (pNextMemInfo) {
    paddedSize = reinterpret_cast< size_t >(pNextMemInfo) - reinterpret_cast< size_t >(pMemInfo) -
                 sizeof(SGameMemInfo);
    RS_VERIFY_THROW(
        857, pNextMemInfo->IsPostGuardBlockIntact(), kXCP_GuardBlockDamage_Post,
        CBasics::Stringize("Damage detected: overwrite detected after block %p of %d bytes",
                           pNextMemInfo, pNextMemInfo->mSize));
    RS_VERIFY_THROW(
        858, pNextMemInfo->IsPriorGuardBlockIntact(), kXCP_GuardBlockDamage_Prior,
        CBasics::Stringize("Damage detected: overwrite detected prior to block %p of %d bytes",
                           pNextMemInfo, pNextMemInfo->mSize));
  }
  RS_VERIFY_THROW(
      863, pMemInfo->IsPriorGuardBlockIntact(), kXCP_GuardBlockDamage_Prior,
      CBasics::Stringize("Damage detected: overwrite detected prior to block %p of %d bytes", p,
                         pMemInfo->mSize));
  RS_VERIFY_THROW(
      864, pMemInfo->IsPostGuardBlockIntact(), kXCP_GuardBlockDamage_Post,
      CBasics::Stringize("Damage detected: overwrite detected after block %p of %d bytes", p,
                         pMemInfo->mSize));
  // clang-format off
  RS_VERIFY_THROW(865, memcmp( kPostGuard, reinterpret_cast<uint8 const *>(p) + pMemInfo->mSize, paddedSize - pMemInfo->mSize ) == 0, kXCP_GuardBlockDamage_Post, CBasics::Stringize("Damage detected: overwrite detected after block %p of %d bytes", p, pMemInfo->mSize));
  // clang-format on
  pMemInfo->SetLength(paddedSize);
#if RSTL_KYOTO_DEBUG_BUILD
  memset(pMemInfo + 1, 0xde, pMemInfo->mSize);
#endif

  SGameMemInfo* prev = pMemInfo->GetPrev();
  SGameMemInfo* next = pMemInfo->GetNext();

  if (prev && !prev->IsAllocated()) {
#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
    RemoveFreeEntryFromFreeList(prev, heapIndex);
#else
    RemoveFreeEntryFromFreeList(prev);
#endif
    prev->SetNext(next);
    if (next) {
      next->SetPrev(prev);
    }
    newLen = sizeof(SGameMemInfo);
    prev->mSize += pMemInfo->mSize + sizeof(SGameMemInfo);
    pMemInfo = prev;
  }

  if (next && !next->IsAllocated() && next->GetNext()) {
#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
    RemoveFreeEntryFromFreeList(next, heapIndex);
#else
    RemoveFreeEntryFromFreeList(next);
#endif
    pMemInfo->SetNext(next->GetNext());
    if (pMemInfo->GetNext()) {
      pMemInfo->GetNext()->SetPrev(pMemInfo);
    }
    newLen += sizeof(SGameMemInfo);
    pMemInfo->mSize += next->mSize + sizeof(SGameMemInfo);
    pMemInfo->SetAllocated(false);
  } else {
    pMemInfo->SetAllocated(false);
  }
#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
  AddFreeEntryToFreeList(pMemInfo, heapIndex);
  SHeapMetrics& metrics = mHeaps[heapIndex].mMetrics;
  --metrics.mCurrentHeapAllocations;
  metrics.mRequestedHeapBytes -= infoLen;
  metrics.mAllocatedHeapBytes -= (paddedSize + newLen);
  metrics.mFreeHeapBytes += (paddedSize + newLen);
#else
  AddFreeEntryToFreeList(pMemInfo);
  mCurrentHeapAllocations -= 1;
  mRequestedHeapBytes -= infoLen;
  mAllocatedHeapBytes -= (paddedSize + newLen);
  mFreeHeapBytes += (paddedSize + newLen);
#endif
  if (infoLen <= 56) {
    mSmallHeapAllocations -= 1;
  }

#if !RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
  return true;
#endif
}

inline void CGameAllocator::ReleaseHeapAllocations(SGameMemInfo*& first, SGameMemInfo*& last) {
  SGameMemInfo* iter = first;
  while (iter != nullptr) {
    SGameMemInfo* next = iter->GetNext();
    if (iter->IsAllocated()) {
      FreeNormalAllocation(iter + 1);
    }
    iter = next;
  }

  first = nullptr;
  last = nullptr;
}

void CGameAllocator::ReleaseAll() {
  if (mMediumPool) {
    mMediumPool->ClearPuddles();
    FreeNormalAllocation(mMediumPool);
    mMediumPool = nullptr;
  }

#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
  for (int index = 0; index < kHeapCount; ++index) {
    SHeap& heap = mHeaps[index];
    ReleaseHeapAllocations(heap.mFirst, heap.mLast);
  }
#else
  ReleaseHeapAllocations(mFirst, mLast);
#endif
}

void* CGameAllocator::AllocSecondary(size_t size, EHint hint, EScope scope, EType type,
                                     const CCallStack& callstack) {
  return Alloc(size, hint, scope, type, callstack);
};

bool CGameAllocator::FreeSecondary(const void* ptr) { return Free(ptr); };

void CGameAllocator::ReleaseAllSecondary() {};

void CGameAllocator::SetOutOfMemoryCallback(FOutOfMemoryCb cb, const void* target) {
  mOomCallback = cb;
  mOomTarget = target;
};

IAllocator::SAllocInfo CGameAllocator::GetAllocInfo(const void* ptr) const {
  const SGameMemInfo* info = GetMemInfoFromBlockPtr(ptr);

  return SAllocInfo(info, info->GetLength(), info->IsAllocated(), false, info->mFileAndLine,
                    info->mType);
};

#if !RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
#if RSTL_KYOTO_ALLOCATOR_METRICS_RESET
IAllocator::SMetrics CGameAllocator::GetMetrics(bool resetPeriodicPeak, bool resetPeak) const
#else
IAllocator::SMetrics CGameAllocator::GetMetrics() const
#endif
{
  uint mediumAllocTotalAllocated = mMediumPool != nullptr ? mMediumPool->GetTotalEntries() * 32 : 0;
  uint mediumAllocBlocksAvailable =
      mMediumPool != nullptr ? mMediumPool->GetNumBlocksAvailable() : 0;
  uint mediumAllocAllocatedSize =
      mMediumPool != nullptr ? mMediumPool->GetTotalEntries() - mMediumPool->GetNumBlocksAvailable()
                             : 0;
  const uint mediumAllocNumAllocs = mMediumPool != nullptr ? mMediumPool->GetNumAllocs() : 0;
  SMetrics ret(mHeapSize, mTotalHeapAllocations, mCurrentHeapAllocations, mRequestedHeapBytes,
               mAllocatedHeapBytes, mFreeHeapBytes, mPeakHeapAllocations, mPeakAllocatedHeapBytes,
#if RSTL_KYOTO_ALLOCATOR_METRICS_RESET
               mPeriodicPeakAllocatedHeapBytes,
#endif
               mMinAllocationSize, mMaxAllocationSize, mAverageAllocationSize,
               mSmallAllocPool != nullptr ? mSmallAllocPool->GetNumAllocs() : 0,
               mSmallAllocPool != nullptr ? mSmallAllocPool->GetAllocatedSize() : 0,
               mSmallAllocPool != nullptr ? mSmallAllocPool->GetNumBlocksAvailable() : 0,
               mediumAllocNumAllocs, mediumAllocAllocatedSize, mediumAllocBlocksAvailable,
               mTotalHeapAllocations - mPreviousTotalHeapAllocations,
               reinterpret_cast< uintptr_t >(mPhysicalAddr), mSmallPoolReservedBytes,
               mediumAllocTotalAllocated, mFakeStatics);
  mPreviousTotalHeapAllocations = mTotalHeapAllocations;
#if RSTL_KYOTO_ALLOCATOR_METRICS_RESET
  if (resetPeriodicPeak) {
    mPeriodicPeakAllocatedHeapBytes = 0;
  }
  if (resetPeak) {
    mPeakAllocatedHeapBytes = 0;
  }
#endif
  return ret;
};

int CGameAllocator::EnumAllocations(FEnumAllocationsCb func, const void* ptr, bool b) const {

  int i = 0;
#if RSTL_KYOTO_DEBUG_BUILD
  const SGameMemInfo* lastIntact = nullptr;
#endif
  const SGameMemInfo* iter = mFirst;

  while (iter != nullptr) {
    if (!iter->IsPriorGuardBlockIntact()) {
#if RSTL_KYOTO_DEBUG_BUILD
      rs_debugger_printf("Invalid Prior Guard Block Detected@0x%08lx\nPrev Intact Block:0x%08lx.\n",
                         iter, lastIntact);
#endif
      return -1;
    }

    if (!iter->IsPostGuardBlockIntact()) {
#if RSTL_KYOTO_DEBUG_BUILD
      rs_debugger_printf("Invalid Post Guard Block Detected@0x%08lx\nPrev Intact Block:0x%08lx.\n",
                         iter, lastIntact);
#endif
      return -1;
    }

    const SGameMemInfo* next = iter->GetNext();
    SAllocInfo alloc(iter, iter->GetLength(), iter->IsAllocated(), false, iter->mFileAndLine,
                     iter->mType);
#if RSTL_KYOTO_DEBUG_BUILD
    lastIntact = iter;
#endif
    func(alloc, ptr);
    ++i;
    iter = next;
  }

  return i;
};

#endif

uint CGameAllocator::GetFreeBinEntryForSize(const uint size) {
  uint maxLen = 0x20;
  uint bin = 0;

  while (maxLen < 0x200000) {
    if (size < maxLen) {
      return bin;
    }

    maxLen <<= 1;
    ++bin;
  }

  return 0xf;
}

#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
void CGameAllocator::AddFreeEntryToFreeList(SGameMemInfo* info, int heapIndex)
#else
void CGameAllocator::AddFreeEntryToFreeList(SGameMemInfo* info)
#endif
{
  uint bin = GetFreeBinEntryForSize(info->GetLength());
#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
  SGameMemInfo** bins = mHeaps[heapIndex].mBins;
#else
  SGameMemInfo** bins = mBins;
#endif
  info->SetNextFree(bins[bin]);
  bins[bin] = info;
}

#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
void CGameAllocator::RemoveFreeEntryFromFreeList(SGameMemInfo* memInfo, int heapIndex)
#else
void CGameAllocator::RemoveFreeEntryFromFreeList(SGameMemInfo* memInfo)
#endif
{
  uint bin = GetFreeBinEntryForSize(memInfo->GetLength());
#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
  SGameMemInfo** bins = mHeaps[heapIndex].mBins;
#else
  SGameMemInfo** bins = mBins;
#endif
  SGameMemInfo* curBin = nullptr;
  SGameMemInfo* binIt = bins[bin];

  while (binIt != nullptr) {
    if (binIt == memInfo) {
      if (curBin == nullptr) {
        bins[bin] = binIt->GetNextFree();
      } else {
        curBin->SetNextFree(binIt->GetNextFree());
      }
      return;
    }

    curBin = binIt;
    binIt = binIt->GetNextFree();
  }
}

#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
CGameAllocator::SHeap::SHeap()
: mFirst(nullptr)
, mLast(nullptr)
, mArenaStart(nullptr)
, mArenaEnd(nullptr)
, mMetrics(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) {
#if RSTL_PRIME1 || defined(PRIME2)
  CBasics::ZeroMemory(mBins, sizeof(mBins));
#else
  memset(mBins, 0, sizeof(mBins));
#endif
}

int CGameAllocator::GetHeapCount() const { return kHeapCount; }

int CGameAllocator::GetHeapIndex(const void* ptr) const {
  if (ptr >= mHeaps[0].mArenaStart && ptr < mHeaps[0].mArenaEnd) {
    return 0;
  }
  if (ptr >= mHeaps[1].mArenaStart && ptr < mHeaps[1].mArenaEnd) {
    return 1;
  }
  return 0;
}
void CGameAllocator::InitializeHeap(int index, void* arena, uint size) {
  mHeaps[index] = SHeap();
  mHeaps[index].mFirst = static_cast< SGameMemInfo* >(arena);
  mHeaps[index].mArenaStart = arena;
  mHeaps[index].mArenaEnd = static_cast< char* >(arena) + size;
  mHeaps[index].mMetrics.mHeapSize = size;
  mHeaps[index].mMetrics.mFreeHeapBytes = size;
  mHeaps[index].mLast = reinterpret_cast< SGameMemInfo* >(static_cast< char* >(arena) + size) - 1;

  const SGameMemInfo& head = SGameMemInfo(nullptr, mHeaps[index].mLast, mHeaps[index].mLast,
                                          size - 2 * sizeof(SGameMemInfo), "MemHead", "MemHead");
  *mHeaps[index].mFirst = head;
  const SGameMemInfo& tail =
      SGameMemInfo(mHeaps[index].mFirst, nullptr, nullptr, 0, "MemTail", "MemTail");
  *mHeaps[index].mLast = tail;
  for (uint i = 0; i < 16; ++i) {
    mHeaps[index].mBins[i] = nullptr;
  }
  AddFreeEntryToFreeList(mHeaps[index].mFirst, index);
}

bool CGameAllocator::Initialize(COsContext& ctx) {
  uint mem1Reserve = 0x1000;
#if RSTL_KYOTO_ALLOCATOR_EXTENDED_MEDIUM_POOL
  mAllowMem2Fallback = DVDConvertPathToEntrynum("AllowMem1SpillOver.txt") != -1;
#else
  mAllowMem2Fallback = true;
#if RSTL_KYOTO_CORRUPTION
  mem1Reserve = DVDConvertPathToEntrynum("IncreaseRSOHeap.txt") != -1 ? 0x200000 : 0x180000;
#endif
#endif
  const uint mem1Size = ctx.GetMem1FreeRam() - mem1Reserve - 2 * sizeof(SGameMemInfo);
  const uint mem2Size = ctx.GetMem2FreeRam() - 2 * sizeof(SGameMemInfo);
  InitializeHeap(0, OSAllocFromArenaLo(mem1Size, sizeof(SGameMemInfo)), mem1Size);
  InitializeHeap(1, OSAllocFromMEM2ArenaLo(mem2Size, sizeof(SGameMemInfo)), mem2Size);
  const uintptr_t addr = reinterpret_cast< uintptr_t >(mHeaps[0].mFirst);
  mMem1HeapPhysicalAddress = addr - (addr & kAllocatorPointerTopNybbleMask);
  mSmallHeapAllocations = 0;
  mInitialized = true;
  mSmallAllocMainData = Alloc(0x100000, kHI_None, kSC_Unk1, kTP_Heap,
                              CCallStack(-1, "SmallAllocMainData   ", " - Ignore"));
  mSmallAllocBookKeeping = Alloc(0x20000, kHI_None, kSC_Unk1, kTP_Heap,
                                 CCallStack(-1, "SmallAllocBookKeeping", " - Ignore"));
  mSmallAllocPool = new (Alloc(sizeof(CSmallAllocPool), kHI_None, kSC_Unk1, kTP_Heap,
                               CCallStack(-1, "SmallAllocClass      ", " - Ignore")))
      CSmallAllocPool(0x40000, mSmallAllocMainData, mSmallAllocBookKeeping);
  mMediumPool =
      new (Alloc(sizeof(CMediumAllocPool), kHI_None, kSC_Unk1, kTP_Heap,
                 CCallStack(-1, "MediumAllocClass      ", " - Ignore"))) CMediumAllocPool();
  const uint mem1Bookkeeping = CMediumAllocPool::GetBookKeepingMemoryRequired(0x1000);
  mMediumPoolMem1 = Alloc(0x20000 + mem1Bookkeeping, kHI_None, kSC_Unk1, kTP_Heap,
                          CCallStack(-1, "MediumAllocMainData   ", " - Ignore"));
  const uint mem2Bookkeeping = CMediumAllocPool::GetBookKeepingMemoryRequired(0x2000);
  mMediumPoolMem2 = Alloc(0x40000 + mem2Bookkeeping, kHI_MEM2, kSC_Unk1, kTP_Heap,
                          CCallStack(-1, "MediumAllocMainData2  ", " - Ignore"));
  mSmallPoolReservedBytes = 0x120000;
  mHeaps[0].mMetrics.mCurrentHeapAllocations -= 4;
  return true;
}

static int gAllocatorTime = 0;
static const uint kMediumEntries[] = {0x1000, 0x2000};
static const uint kMaxMediumAlloc[] = {0x400, 0x4000};

void* CGameAllocator::Alloc(size_t size, EHint hint, EScope scope, EType type,
                            const CCallStack& callstack) {
  const OSTick start = OSGetTick();
  if (hint & kHI_RoundUpLen) {
    size = (size + 31) & ~31;
  }
  bool useSmall = size <= 56 && !(hint & 0xf) && mSmallAllocPool;
  if (useSmall && mSmallAllocRetryDelay > 0) {
    --mSmallAllocRetryDelay;
    useSmall = false;
  }
  if (useSmall) {
    void* result = mSmallAllocPool->Alloc(size);
    const uint blocks = mSmallAllocPool->GetAllocatedSize();
    if (mPeakSmallPoolEntries < blocks) {
      mPeakSmallPoolEntries = blocks;
      static uint lastReportedBlocks;
      if (lastReportedBlocks + 128 < blocks) {
        lastReportedBlocks = blocks;
      }
    }
    if (result) {
      gAllocatorTime += OSGetTick() - start;
      return result;
    }
    if (!mSmallAllocFailed) {
      OSReport("**************************************************\n");
      OSReport("************** Performance warning! **************\n");
      OSReport("*************** SmallAlloc Failed! ***************\n");
      OSReport("Num Entries:%d\tNumAllocs:%d\tNumFreeBlocks:%d\n",
               mSmallAllocPool->GetTotalEntries(), mSmallAllocPool->GetNumAllocs(),
               mSmallAllocPool->GetNumBlocksAvailable());
      OSReport("Try increasing 'kNumSmallAllocEntries' in CGameAllocator.cpp.\n");
      OSReport("**************************************************\n");
    }
    mSmallAllocRetryDelay = 25;
    mSmallAllocFailed = true;
  }

  int heapIndex;
  SGameMemInfo* info = nullptr;
  if (mMediumPool && size <= kMaxMediumAlloc[(hint & kHI_MEM2) != 0] &&
      !(hint & (kHI_TopOfHeap | kHI_NoMEM2Fallback)) && !mGrowingMediumPool) {
    mGrowingMediumPool = true;
    if (!mMediumPool->HasPuddles()) {
      mMediumPool->AddPuddle(kMediumEntries[0], mMediumPoolMem1, false, 0);
      mMediumPoolMem1 = nullptr;
      mMediumPool->AddPuddle(kMediumEntries[1], mMediumPoolMem2, false, 1);
      mMediumPoolMem2 = nullptr;
    }
    const bool mem2 = (hint & kHI_MEM2) != 0;
    const EHint puddleHint = mem2 ? kHI_MEM2 : kHI_None;
    void* result = mMediumPool->Alloc(size, mem2);
#if RSTL_KYOTO_ALLOCATOR_EXTENDED_MEDIUM_POOL
    if (!result) {
      const int mediumHeapIndex = mem2 ? 1 : 0;
      const SHeapMetrics& metrics = mHeaps[mediumHeapIndex].mMetrics;
      if (metrics.mFreeHeapBytes < metrics.mHeapSize / 4) {
        info = FindFreeBlockInSizeRange((size + 31) & ~31, (kMaxMediumAlloc[mem2] + 31) & ~31,
                                        mediumHeapIndex);
        if (info) {
          heapIndex = mediumHeapIndex;
        }
      }
    }
#endif
    if (!result && !info) {
      const uint entries = kMediumEntries[mem2];
      const uint bookkeeping = CMediumAllocPool::GetBookKeepingMemoryRequired(entries);
#if RSTL_KYOTO_ALLOCATOR_EXTENDED_MEDIUM_POOL
      mAllocatingPuddle = true;
#endif
      void* puddle =
          Alloc(CMediumAllocPool::GetAllocMemoryRequired(entries) + bookkeeping, puddleHint,
                kSC_Unk1, kTP_Heap, CCallStack(-1, "MediumAllocMainData   ", " - Ignore"));
#if RSTL_KYOTO_ALLOCATOR_EXTENDED_MEDIUM_POOL
      mAllocatingPuddle = false;
#endif
      if (puddle) {
        mMediumPool->AddPuddle(entries, puddle, true, mem2);
        result = mMediumPool->Alloc(size, mem2);
      }
    }
    if (!info) {
      mGrowingMediumPool = false;
      if (result) {
        gAllocatorTime += OSGetTick() - start;
        return result;
      }
      if (!mMediumAllocFailed) {
        OSReport("**************************************************\n");
        OSReport("************** Performance warning! **************\n");
        OSReport("*************** MediumAlloc Failed! ***************\n");
        OSReport("Num Entries:%d\tNumAllocs:%d\tNumFreeBlocks:%d\n", mMediumPool->GetTotalEntries(),
                 mMediumPool->GetNumAllocs(), mMediumPool->GetNumBlocksAvailable());
        OSReport("No medium alloc blocks available.\n");
        OSReport("**************************************************\n");
        mMediumAllocFailed = true;
      }
    }
  }

  const uint roundedSize = (size + 31) & ~31;
  const bool topOfHeap = (hint & kHI_TopOfHeap) != 0;
  if (!info) {
    info = FindFreeBlock(heapIndex, roundedSize, hint);
  }
  if (!info) {
    void* result = nullptr;
    if (mHandlingOutOfMemory) {
      return nullptr;
    }
    if (mOomCallback) {
      mHandlingOutOfMemory = true;
      while (mOomCallback(mOomTarget, size)) {
        result = Alloc(size, hint, scope, type, callstack);
        if (result) {
          break;
        }
      }
      mHandlingOutOfMemory = false;
    }
    if (!result) {
      DumpAllocations();
      return nullptr;
    }
    return result;
  }

  const uint offset = FixupAllocPtrs(info, heapIndex, size, roundedSize, hint, callstack);
  if (topOfHeap && !info->IsAllocated()) {
    info = info->GetNext();
  }
  UpdateAllocDebugStats(heapIndex, size, roundedSize, offset);
  gAllocatorTime += OSGetTick() - start;
  return info + 1;
}

CGameAllocator::SGameMemInfo* CGameAllocator::FindFreeBlock(int& heapIndex, uint size, EHint hint) {
  typedef SGameMemInfo* (CGameAllocator::*FFindBlock)(uint, int);
  FFindBlock find = (hint & kHI_TopOfHeap)
                        ? &CGameAllocator::FindFreeBlockFromTopOfHeap
                        : static_cast< FFindBlock >(&CGameAllocator::FindFreeBlock);
  if (hint & kHI_MEM2) {
    heapIndex = 1;
    SGameMemInfo* info = (this->*find)(size, 1);
    if (!mOomCallback && !info && mHeaps[0].mFirst) {
      heapIndex = 0;
      info = (this->*find)(size, 0);
    }
    return info;
  }
  heapIndex = 0;
  SGameMemInfo* info = (this->*find)(size, 0);
  if (mAllowMem2Fallback && !info && mHeaps[1].mFirst && !(hint & kHI_NoMEM2Fallback)) {
    heapIndex = 1;
    info = (this->*find)(size, 1);
  }
  return info;
}

inline CGameAllocator::SGameMemInfo* CGameAllocator::FindFreeBlockInBins(uint len, uint maxLen,
                                                                         int heapIndex) {
  SHeap& heap = mHeaps[heapIndex];
  CGameAllocator::SGameMemInfo* ret = nullptr;

  uint binIndex = GetFreeBinEntryForSize(len);
  const uint binEnd = maxLen == ~0u ? 16 : GetFreeBinEntryForSize(maxLen) + 1;

  uint chosenBin = 0;
  SGameMemInfo* previous = nullptr;
  uint bestDelta = 0x10000000;

  for (; binIndex < binEnd && !ret; ++binIndex) {
    SGameMemInfo* candidate = heap.mBins[binIndex];
    SGameMemInfo* last = nullptr;
    for (; candidate; last = candidate, candidate = candidate->GetNextFree()) {
      if (!candidate->IsAllocated() && candidate->mSize >= len && candidate->mSize <= maxLen) {
        if (candidate->mSize - len < bestDelta && candidate->GetNext()) {
          ret = candidate;
          previous = last;
          bestDelta = candidate->mSize - len;
          chosenBin = binIndex;
          if (bestDelta < sizeof(SGameMemInfo)) {
            break;
          }
        }
      }
    }
  }

  if (ret) {
    if (previous == nullptr) {
      const uintptr_t next = reinterpret_cast< uintptr_t >(ret->mNextFree);
      heap.mBins[chosenBin] =
          reinterpret_cast< SGameMemInfo* >(next & ~(kAllocatorPointerBits - 1));
    } else {
      previous->SetNextFree(ret->GetNextFree());
    }
  }
  return ret;
}

CGameAllocator::SGameMemInfo* CGameAllocator::FindFreeBlock(uint len, int heapIndex) {
  return FindFreeBlockInBins(len, ~0u, heapIndex);
}

#if RSTL_KYOTO_ALLOCATOR_EXTENDED_MEDIUM_POOL
CGameAllocator::SGameMemInfo* CGameAllocator::FindFreeBlockInSizeRange(uint len, uint maxLen,
                                                                       int heapIndex) {
  return FindFreeBlockInBins(len, maxLen, heapIndex);
}
#endif

CGameAllocator::SGameMemInfo* CGameAllocator::FindFreeBlockFromTopOfHeap(uint size, int heapIndex) {
  SGameMemInfo* iter = mHeaps[heapIndex].mLast;
  SGameMemInfo* ret = nullptr;

  while (iter != nullptr) {
    if (!iter->IsAllocated() && iter->GetLength() >= size) {
      ret = iter;
      break;
    }
    iter = iter->GetPrev();
  }

  RemoveFreeEntryFromFreeList(ret, heapIndex);
  return ret;
}

uint CGameAllocator::FixupAllocPtrs(SGameMemInfo* info, int heapIndex, const uint len,
                                    uint roundedLen, EHint hint, const CCallStack& cs) {

  const bool topOfHeap = (hint & kHI_TopOfHeap) != 0;
  uint ret = 0;
  const size_t blockLength = info->GetLength();
  if (blockLength == roundedLen + sizeof(SGameMemInfo)) {
    ret = sizeof(SGameMemInfo);
    roundedLen += sizeof(SGameMemInfo);
  }

  SGameMemInfo* newPtr = info;
  if (blockLength != roundedLen) {
    SGameMemInfo* newInfo;

    SGameMemInfo* infoNext = info->GetNext();
    if (topOfHeap) {
      newInfo =
          reinterpret_cast< SGameMemInfo* >(reinterpret_cast< char* >(infoNext) - roundedLen) - 1;
      const SGameMemInfo& block = SGameMemInfo(info, infoNext, nullptr, len, "", "");
      *newInfo = block;
      info->mSize -= roundedLen + sizeof(SGameMemInfo);
      AddFreeEntryToFreeList(info, heapIndex);
      newPtr = newInfo;
    } else {
      newInfo = reinterpret_cast< SGameMemInfo* >(reinterpret_cast< uintptr_t >(info) + roundedLen +
                                                  sizeof(SGameMemInfo));
      SGameMemInfo* nextFree = info->mNextFree;
      nextFree = reinterpret_cast< SGameMemInfo* >(reinterpret_cast< uintptr_t >(nextFree) &
                                                   ~(kAllocatorPointerBits - 1));
      const SGameMemInfo& block = SGameMemInfo(
          info, infoNext, nextFree, info->mSize - roundedLen - sizeof(SGameMemInfo), "", "");
      *newInfo = block;
      AddFreeEntryToFreeList(newInfo, heapIndex);
    }
    newPtr->mFileAndLine = cs.GetFileAndLineText();
    newPtr->mType = cs.GetTypeText();
    ret = sizeof(SGameMemInfo);

    infoNext->SetPrev(newInfo);
    info->SetNext(newInfo);
  } else {
    info->mFileAndLine = cs.GetFileAndLineText();
    info->mType = cs.GetTypeText();
  }

  newPtr->SetTopOfHeapAllocated(topOfHeap);
  newPtr->SetAllocated(true);
  newPtr->mSize = len;
  return ret;
}

void CGameAllocator::UpdateAllocDebugStats(int heapIndex, uint len, uint roundedLen, uint offset) {
  SHeapMetrics& metrics = mHeaps[heapIndex].mMetrics;
  ++metrics.mCurrentHeapAllocations;
  ++metrics.mTotalHeapAllocations;
  metrics.mRequestedHeapBytes += len;
  metrics.mAllocatedHeapBytes += roundedLen + offset;
  metrics.mFreeHeapBytes -= roundedLen + offset;

  if (metrics.mCurrentHeapAllocations > metrics.mPeakHeapAllocations) {
    metrics.mPeakHeapAllocations = metrics.mCurrentHeapAllocations;
  }

  if (metrics.mAllocatedHeapBytes > metrics.mPeakAllocatedHeapBytes) {
    metrics.mPeakAllocatedHeapBytes = metrics.mAllocatedHeapBytes;
  }

  if (metrics.mAllocatedHeapBytes > metrics.mPeriodicPeakAllocatedHeapBytes) {
    metrics.mPeriodicPeakAllocatedHeapBytes = metrics.mAllocatedHeapBytes;
  }

  if (len < metrics.mMinAllocationSize) {
    metrics.mMinAllocationSize = len;
  }

  if (len > metrics.mMaxAllocationSize) {
    metrics.mMaxAllocationSize = len;
  }
  metrics.mAverageAllocationSize =
      (len + metrics.mAverageAllocationSize * (metrics.mTotalHeapAllocations - 1)) /
      metrics.mTotalHeapAllocations;
  if (len > 56) {
    return;
  }

  ++mSmallHeapAllocations;
}

static inline uint GetMediumMemoryRequired(uint entries) {
  return CMediumAllocPool::GetAllocMemoryRequired(entries) +
         CMediumAllocPool::GetBookKeepingMemoryRequired(entries);
}

IAllocator::SAllocatorMetrics CGameAllocator::GetMetrics() const {
  const uint heapSize = mHeaps[0].mMetrics.mHeapSize + mHeaps[1].mMetrics.mHeapSize;
  const uint allocated =
      mHeaps[0].mMetrics.mAllocatedHeapBytes + mHeaps[1].mMetrics.mAllocatedHeapBytes;
  const uint mediumReserved =
      mMediumPool ? GetMediumMemoryRequired(mMediumPool->GetTotalEntries()) : 0;
  const uint mediumAvailable = mMediumPool ? mMediumPool->GetNumBlocksAvailable() : 0;
  const uint mediumAllocated =
      mMediumPool ? mMediumPool->GetTotalEntries() - mMediumPool->GetNumBlocksAvailable() : 0;
  const uint mediumNumAllocs = mMediumPool ? mMediumPool->GetNumAllocs() : 0;
  return SAllocatorMetrics(mSmallAllocPool ? mSmallAllocPool->GetNumAllocs() : 0,
                           mSmallAllocPool ? mSmallAllocPool->GetAllocatedSize() : 0,
                           mSmallAllocPool ? mSmallAllocPool->GetNumBlocksAvailable() : 0,
                           mediumNumAllocs, mediumAllocated, mediumAvailable,
                           mMem1HeapPhysicalAddress, mSmallPoolReservedBytes, mediumReserved,
                           heapSize, allocated);
}

IAllocator::SHeapMetrics CGameAllocator::GetHeapMetrics(int index, uint flags) const {
  SHeapMetrics snapshot = mHeaps[index].mMetrics;
  if (flags & 1) {
    mHeaps[index].mMetrics.mPeriodicPeakAllocatedHeapBytes = 0;
  }
  if (flags & 2) {
    mHeaps[index].mMetrics.mPeakAllocatedHeapBytes = 0;
  }
  mHeaps[index].mMetrics.x30_ = 0;
  return snapshot;
}

const char* CGameAllocator::GetHeapName(int index) const {
  switch (index) {
  case 0:
    return "Default";
  case 1:
    return "GPU+Audio";
  default:
    return "?????";
  }
}

int CGameAllocator::EnumAllocations(FEnumAllocationsCb func, const void* ptr, bool) const {
  typedef rstl::pair< const SGameMemInfo*, int > HeapStart;
  rstl::reserved_vector< HeapStart, kHeapCount > starts;
  starts.push_back(HeapStart(mHeaps[0].mFirst, 0));
  starts.push_back(HeapStart(mHeaps[1].mFirst, 1));
  rstl::sort_by_key(starts);

  int count = 0;
  for (int i = 0; i < kHeapCount; ++i) {
    const int heapIndex = starts[i].second;
    const SGameMemInfo* info = starts[i].first;
    while (info) {
      if (!info->IsPriorGuardBlockIntact()) {
        return -1;
      }
      if (!info->IsPostGuardBlockIntact()) {
        return -1;
      }
      const SGameMemInfo* next = info->GetNext();
      SAllocInfo allocInfo(info + 1, info->GetLength(), info->IsAllocated(), false,
                           info->mFileAndLine, info->mType);
      func(heapIndex, allocInfo, ptr);
      ++count;
      info = next;
    }
  }
  return count;
}

void CGameAllocator::DumpAllocations() const {
  int count = 0;
  for (int index = 0; index < kHeapCount; ++index) {
    for (const SGameMemInfo* info = mHeaps[index].mFirst; info; info = info->GetNext()) {
      if (++count % 4 == 0) {
        CStopwatch::Wait(0.005f);
      }
    }
  }
}

#else
static inline bool DoWait(int v) { return (v % 4) == 0; }

void CGameAllocator::DumpAllocations() const {
#if RSTL_KYOTO_ALLOCATOR_LARGEST_FREE_CHUNK
  GetLargestFreeChunk();
#endif
  uint i = 0;
  SGameMemInfo* iter = mFirst;

  while (iter != nullptr) {
    ++i;

    if (DoWait(i)) {
      CStopwatch::Wait(0.005f);
    }
    iter = iter->GetNext();
  }
}

#if RSTL_KYOTO_ALLOCATOR_LARGEST_FREE_CHUNK
size_t CGameAllocator::GetLargestFreeChunk() const {
  const SGameMemInfo* iter = mFirst;
  size_t ret = 0;
  while (iter != nullptr) {
    if (!iter->IsAllocated() && iter->GetLength() > ret) {
      ret = iter->GetLength();
    }
    iter = iter->GetNextFree();
  }

  return ret;
}
#endif

#endif

#if RSTL_KYOTO_ALLOCATOR_OFFSET_FAKE_STATICS
void CGameAllocator::OffsetFakeStatics(int offset) {
#if !RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
  mFakeStatics += offset;
#endif
}
#endif
