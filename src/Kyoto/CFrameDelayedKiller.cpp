#include "Kyoto/CFrameDelayedKiller.hpp"

#include "Kyoto/Particles/CElementAllocationChunk.hpp"
#include "Kyoto/Particles/IElement.hpp"
#include "Kyoto/Platform.h"
#include "rstl/RstlVersions.h"
#include "rstl/list.hpp"

#include "dolphin/gx/GXManage.h"

#include "stdint.h"
#if defined(TARGET_PC)
#include <dolphin/gx/GXAurora.h>
#endif

static uint sCurList = 0;
#if !RSTL_PRIME1
static bool sInitialized = false;
#endif

static rstl::list< void* > sFrameDelayedList[2];

#if defined(__MWERKS__) && (RSTL_VERSION < RSTL_GM8P_00)
#pragma force_active on
CFrameDelayedKiller::Stats CFrameDelayedKiller::mUnusedStats = {0, 0, 0, 0, 0, 0};
#pragma force_active reset
#endif

void CFrameDelayedKiller::Initialize() {
#if !RSTL_PRIME1
  sInitialized = true;
#endif
  StallAndFlushAllAllocations();
}

void CFrameDelayedKiller::ShutDown() {
  StallAndFlushAllAllocations();
#if !RSTL_PRIME1
  sInitialized = false;
#endif
}

void CFrameDelayedKiller::FlushAllAllocations() {
  for (int i = 0; i < 2; ++i) {
    FlushAllocationsForFrame();
  }
}

void CFrameDelayedKiller::StallAndFlushAllAllocations() {
  if (!IS_TARGET_PC) {
    // At this point, we've already drained the GX FIFO
    GXDrawDone();
  }
  FlushAllAllocations();
}

void CFrameDelayedKiller::ScheduleDeletion(const EWhichFrame whichFrame, void* victim) {
#if !RSTL_PRIME1
  if (victim == nullptr) {
    return;
  }
  if (!sInitialized) {
    GXDrawDone();
    CMemory::Free(victim);
    return;
  }

#endif
#if RSTL_PRIME1
  uint index = whichFrame == true ? sCurList : sCurList ^ 1;
#else
  uint index = whichFrame == kWhichFrame_NextFrame ? sCurList : sCurList ^ 1;
#endif
  sFrameDelayedList[index].push_back(victim);
}

void CFrameDelayedKiller::FlushAllocationsForFrame() {
  sCurList ^= 1;
  rstl::list< void* >& list = sFrameDelayedList[sCurList];
  if (IS_TARGET_PC && !list.empty()) {
    IF_TARGET_PC(AuroraGXSync());
  }

  for (rstl::list< void* >::iterator t = list.begin(); t != list.end(); ++t) {
    CMemory::Free(*t);
  }

  list.clear();
}

CElementAllocationChunk::CElementAllocationChunk()
: mCapacity(256), mAllocatedWords(0), mAllocationCount(0) {}

bool CElementAllocationChunk::CanAllocate(uint size) const {
  return mCapacity > mAllocatedWords + (size + 3) / 4;
}

bool CElementAllocationChunk::Contains(const void* ptr) const {
#if RSTL_PRIME1 && NONMATCHING
  return reinterpret_cast< uintptr_t >(ptr) - reinterpret_cast< uintptr_t >(mData) < sizeof(mData);
#else
  int offset = static_cast< const char* >(ptr) - reinterpret_cast< const char* >(mData);
  int index = offset / 4;
  return mCapacity > index;
#endif
}

void* CElementAllocationChunk::Allocate(uint size) {
  void* ptr = &mData[mAllocatedWords];
  mAllocatedWords += (size + 3) / 4;
  ++mAllocationCount;
  return ptr;
}

void CElementAllocationChunk::Free(void*) { --mAllocationCount; }

void CElementAllocationChunk::Rewind(uint size) {
  uint words = (size + 3) / 4;
  if (words > mAllocatedWords) {
    mAllocatedWords = 0;
  } else {
    mAllocatedWords -= words;
  }
}

uint CElementAllocationChunk::GetAllocatedSize() const { return mAllocatedWords * 4; }

uint CElementAllocationChunk::GetAllocationCount() const { return mAllocationCount; }

void* IElement::operator new(size_t sz, const char* fileAndLine, const char* type) {
  return CElementAllocator::Alloc(sz, fileAndLine, type);
}

void IElement::operator delete(void* ptr, const size_t sz) { CElementAllocator::Free(ptr, sz); }
