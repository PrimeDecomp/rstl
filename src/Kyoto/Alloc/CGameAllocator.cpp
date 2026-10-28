// #include "dolphin/os/OSArena.h"
#include "dolphin/types.h"

#include "Kyoto/Alloc/CGameAllocator.hpp"

#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Alloc/CCallStack.hpp"
#include "Kyoto/Alloc/CMediumAllocPool.hpp"
#include "Kyoto/Alloc/CSmallAllocPool.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Basics/COsContext.hpp"
#include "Kyoto/Basics/CStopwatch.hpp"

#include "stddef.h"
#include <stdint.h>
#if RSTL_KYOTO_DEBUG_BUILD
#include <string.h>
#endif

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

CGameAllocator::SGameMemInfo* CGameAllocator::GetMemInfoFromBlockPtr(const void* ptr) const {
  return reinterpret_cast< SGameMemInfo* >(reinterpret_cast< uintptr_t >(ptr) -
                                           sizeof(SGameMemInfo));
}

CGameAllocator::CGameAllocator()
: mInitialized(0)
, mHeapSize(0)
, mFirst(nullptr)
, mLast(nullptr)
, x54_(0)
, mOomCallback(nullptr)
, mOomTarget(nullptr)
, mSmallAllocPool(nullptr)
, mSmallAllocMainData(nullptr)
, mSmallAllocBookKeeping(nullptr)
, mSmallAllocFailed(false)
, mSmallAllocRetryDelay(0)
, mMediumPool(nullptr)
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
, mSmallPoolReservedBytes(0) {
  SGameMemInfo testSGameMemInfo(nullptr, nullptr, nullptr, 0, string_NULL, string_NULL);
  // clang-format off
  RS_VERIFY_THROW(172, (reinterpret_cast<uint>(&testSGameMemInfo.mGuardBlockStart) - reinterpret_cast<uint>(&testSGameMemInfo)) == 28, kXCP_InvalidSGameMemInfo, "SGameMemInfo must have mGuardBlockStart at end of structure.");
  // clang-format on
}

CGameAllocator::~CGameAllocator() {
  if (mMediumPool) {
    mMediumPool->ClearPuddles();
    FreeNormalAllocation(mMediumPool);
    mMediumPool = nullptr;
  }
}

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

void CGameAllocator::Shutdown() {
  ReleaseAll();
  mInitialized = 0;
  x54_ = 0;
}

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

bool CGameAllocator::Free(const void* ptr) {
  if (ptr == nullptr) {
    return true;
  }

  if (mSmallAllocPool && mSmallAllocPool->PtrWithinPool(ptr)) {
    return mSmallAllocPool->Free(ptr);
  }

  if (mMediumPool) {
    int tmp = mMediumPool->Free(ptr);
    if (tmp != 1) {
      return tmp > 0;
    }
  }
  return FreeNormalAllocation(ptr);
}

bool CGameAllocator::FreeNormalAllocation(const void* p) {
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
    RemoveFreeEntryFromFreeList(prev);
    prev->SetNext(next);
    if (next) {
      next->SetPrev(prev);
    }
    newLen = sizeof(SGameMemInfo);
    prev->mSize += pMemInfo->mSize + sizeof(SGameMemInfo);
    pMemInfo = prev;
  }

  if (next && !next->IsAllocated() && next->GetNext()) {
    RemoveFreeEntryFromFreeList(next);
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
  AddFreeEntryToFreeList(pMemInfo);

  mCurrentHeapAllocations -= 1;
  mRequestedHeapBytes -= infoLen;
  mAllocatedHeapBytes -= (paddedSize + newLen);
  mFreeHeapBytes += (paddedSize + newLen);
  if (infoLen <= 56) {
    mSmallHeapAllocations -= 1;
  }

  return true;
};

void CGameAllocator::ReleaseAll() {
  if (mMediumPool) {
    mMediumPool->ClearPuddles();
    FreeNormalAllocation(mMediumPool);
    mMediumPool = nullptr;
  }

  SGameMemInfo* iter = mFirst;
  while (iter != nullptr) {
    SGameMemInfo* next = iter->GetNext();
    if (iter->IsAllocated()) {
      FreeNormalAllocation(((uchar*)iter) + sizeof(SGameMemInfo));
    }
    iter = next;
  }

  mFirst = nullptr;
  mLast = nullptr;
};

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

#if RSTL_KYOTO_ALLOCATOR_METRICS_RESET
IAllocator::SMetrics CGameAllocator::GetMetrics(bool resetPeriodicPeak, bool resetPeak) const {
#else
IAllocator::SMetrics CGameAllocator::GetMetrics() const {
#endif
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

void CGameAllocator::AddFreeEntryToFreeList(SGameMemInfo* info) {
  uint bin = GetFreeBinEntryForSize(info->GetLength());
  info->SetNextFree(mBins[bin]);
  mBins[bin] = info;
}

void CGameAllocator::RemoveFreeEntryFromFreeList(SGameMemInfo* memInfo) {
  uint bin = GetFreeBinEntryForSize(memInfo->GetLength());
  SGameMemInfo* curBin = nullptr;
  SGameMemInfo* binIt = mBins[bin];

  while (binIt != nullptr) {
    if (binIt == memInfo) {
      if (curBin == nullptr) {
        mBins[bin] = binIt->GetNextFree();
      } else {
        curBin->SetNextFree(binIt->GetNextFree());
      }
      return;
    }

    curBin = binIt;
    binIt = binIt->GetNextFree();
  }
}

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

void CGameAllocator::OffsetFakeStatics(const int offset) { mFakeStatics += offset; }
