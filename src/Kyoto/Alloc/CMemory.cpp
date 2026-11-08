#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Alloc/CMemorySys.hpp"

#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Alloc/CCallStack.hpp"
#include "Kyoto/Alloc/CGameAllocator.hpp"
#include "Kyoto/Basics/CInterruptGuard.hpp"
#include "Kyoto/Basics/RAssertDolphin.hpp"

#include "dolphin/os.h"

#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
void rs_reallyfatal_printf(const char* format, ...);
#endif

static CGameAllocator gGameAllocator;
IAllocator* CMemory::mpAllocator = &gGameAllocator;
bool CMemory::mInitialized;
uint gLeakCount = 0;
uint gLeakBytes = 0;

CMemorySys::CMemorySys(COsContext& ctx, IAllocator& allocator) {
  CMemory::Startup(ctx);
  CMemory::SetAllocator(ctx, allocator);
}

CMemorySys::~CMemorySys() { CMemory::Shutdown(); }

IAllocator& CMemorySys::GetGameAllocator() { return gGameAllocator; }

void CMemory::Startup(COsContext& ctx) { mInitialized = mpAllocator->Initialize(ctx); }

void CMemory::SetAllocator(COsContext& ctx, IAllocator& allocator) {
  if (mpAllocator != &allocator) {
    if (mpAllocator != nullptr) {
      mpAllocator->ReleaseAll();
    }
    mpAllocator = &allocator;
    mpAllocator->Initialize(ctx);
  }
}

#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
static bool cmemory_enum_alloc_cb(int, const IAllocator::SAllocInfo& info, const void*)
#else
static bool cmemory_enum_alloc_cb(const IAllocator::SAllocInfo& info, const void* ptr)
#endif
{
  if (info.mIsAllocated && info.x9_ == 0) {
    ++gLeakCount;
    gLeakBytes += info.mLen;
  }
  return true;
}

void CMemory::Shutdown() {
  CMemory::mInitialized = false;

#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
  uint liveAllocations = 0;
  for (int i = 0; i < mpAllocator->GetHeapCount(); ++i) {
    liveAllocations += mpAllocator->GetHeapMetrics(i, 0).mCurrentHeapAllocations;
  }
#elif RSTL_KYOTO_ALLOCATOR_METRICS_RESET
  const uint liveAllocations = mpAllocator->GetMetrics(0, false).x8_;
#else
  const uint liveAllocations = mpAllocator->GetMetrics().x8_;
#endif
  if (liveAllocations != 0) {
    gLeakCount = 0;
    gLeakBytes = 0;
    mpAllocator->EnumAllocations((IAllocator::FEnumAllocationsCb)cmemory_enum_alloc_cb, nullptr,
                                 false);
  }
  mpAllocator->Shutdown();
}

void* CMemory::Alloc(size_t len, IAllocator::EHint hint, IAllocator::EScope scope,
                     IAllocator::EType type, const CCallStack& callstack) {
#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
  CRestorableInterruptGuard interrupts;
#else
  volatile bool enabled = OSDisableInterrupts();
#endif
  void* ret = mpAllocator->Alloc(len, hint, scope, type, callstack);
  if (ret == nullptr) {
#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
    const IAllocator::SAllocatorMetrics metrics = mpAllocator->GetMetrics();
    uint freeBytes = 0;
    for (int i = 0; i < mpAllocator->GetHeapCount(); ++i) {
      freeBytes += mpAllocator->GetHeapMetrics(i, 0).mFreeHeapBytes;
    }
    rs_reallyfatal_printf("Alloc failed - Size: %d, Used: %d, Free: %d", len,
                          metrics.mAllocatedHeapBytes, freeBytes);
#elif RSTL_KYOTO_DEBUG_BUILD
    rs_debugger_printf("**************************************************\n");
    rs_debugger_printf("***************** Alloc Failed! ******************\n");
    rs_debugger_printf("Size:%d\n", len);
#elif RSTL_KYOTO_ALLOCATOR_METRICS_RESET
    const IAllocator::SMetrics metrics = mpAllocator->GetMetrics(0, false);
    rs_debugger_printf("Alloc failed - Size: %d, Used: %d, Free: %d", len, metrics.x10_,
                       metrics.mHeapSize2);
#else
    rs_debugger_printf("Alloc failed - Size: %d", len);
#endif
    RS_VERIFY_THROW(239, false, kXCP_BadAlloc, "kXCP_BadAlloc");
  }

#if !RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
  OSRestoreInterrupts(enabled);
#endif
  return ret;
}

void CMemory::Free(const void* ptr) {
#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
  CRestorableInterruptGuard interrupts;
#else
  volatile bool enabled = OSDisableInterrupts();
#endif
  if (ptr != nullptr && !mpAllocator->Free(ptr)) {
    RS_VERIFY_THROW(280, false, CMemory::kXCP_BadFree, "CMemory::kXCP_BadFree");
  }
#if !RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
  OSRestoreInterrupts(enabled);
#endif
}

void CMemory::SetOutOfMemoryCallback(IAllocator::FOutOfMemoryCb cb, const void* context) {
  mpAllocator->SetOutOfMemoryCallback(cb, context);
}

#if RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS
IAllocator::SAllocatorMetrics CMemory::GetMetrics() { return mpAllocator->GetMetrics(); }
#elif RSTL_KYOTO_ALLOCATOR_METRICS_RESET
IAllocator::SMetrics CMemory::GetMetrics(bool unk1, bool unk2) {
  return mpAllocator->GetMetrics(unk1, unk2);
}
#endif

#if RSTL_KYOTO_ALLOCATOR_OFFSET_FAKE_STATICS
void CMemory::OffsetFakeStatics(int offset) { mpAllocator->OffsetFakeStatics(offset); }
#endif

void* operator new(size_t sz, const char* fileAndLine, const char* type) {
  return CMemory::Alloc(sz, IAllocator::kHI_None, IAllocator::kSC_Unk1, IAllocator::kTP_Heap,
                        CCallStack(-1, fileAndLine, type));
}
void* operator new[](size_t sz, const char* fileAndLine, const char* type) {
  return CMemory::Alloc(sz, IAllocator::kHI_None, IAllocator::kSC_Unk1, IAllocator::kTP_Array,
                        CCallStack(-1, fileAndLine, type));
}
