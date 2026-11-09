#include "Kyoto/Graphics/CGraphicsPalette.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/Graphics/DolphinGPUMemory.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/RstlVersions.h"

#include "dolphin/gx.h"
#include "dolphin/os.h"

uint CGraphicsPalette::sCurrentFrameCount = 0;

static inline GXTlutFmt format_to_format(EPaletteFormat format) {
  return static_cast< GXTlutFmt >(format);
}

#if RSTL_KYOTO_PALETTE_GPU_MEMORY
ushort* CGraphicsPalette::AllocateEntries(int numEntries, int useGPUMemory) {
  if (useGPUMemory == 1) {
    return static_cast< ushort* >(GPUMemory::EnsureAllocation(numEntries * sizeof(ushort)));
  }

  return static_cast< ushort* >(rs_aligned_new_line(numEntries * sizeof(ushort), 32));
}

CGraphicsPalette::CGraphicsPalette(EPaletteFormat format, int numEntries, int useGPUMemory)
#else
CGraphicsPalette::CGraphicsPalette(EPaletteFormat format, int numEntries)
#endif
: mFmt(format)
#if RSTL_PRIME1
#if NONMATCHING
, mFrameLoaded(0)
#endif
#endif

, mEntryCount(numEntries)
#if RSTL_PRIME1
, mEntries((ushort*)CMemory::Alloc(numEntries * sizeof(ushort), IAllocator::kHI_RoundUpLen))
#if NONMATCHING
, mTlutObj()
#endif
#elif RSTL_KYOTO_PALETTE_GPU_MEMORY
, mEntries(AllocateEntries(numEntries, useGPUMemory))
#else
, mEntries(static_cast< ushort* >(rs_aligned_new_line(numEntries * sizeof(ushort), 32)))
#endif

, mLocked(false)
#if RSTL_KYOTO_PALETTE_GPU_MEMORY
, mUsesGPUMemory(useGPUMemory == 1)
#endif
{
  GXInitTlutObj(&mTlutObj, mEntries.get(), format_to_format(mFmt), mEntryCount);
}

CGraphicsPalette::CGraphicsPalette(CInputStream& in)
#if RSTL_PRIME1
: mFmt(EPaletteFormat(in.ReadLong()))
#if NONMATCHING
, mFrameLoaded(0)
#endif
, mEntryCount(in.Get< short >() * in.Get< short >())
, mEntries((ushort*)CMemory::Alloc(mEntryCount * sizeof(ushort), IAllocator::kHI_RoundUpLen))
#if NONMATCHING
, mTlutObj()
#endif
#else
: mFmt(EPaletteFormat(in.ReadInt32()))
, mEntryCount(in.ReadInt16() * in.ReadInt16())
, mEntries(static_cast< ushort* >(rs_aligned_new_line(mEntryCount * sizeof(ushort), 57)))
#endif

, mLocked(false)
#if RSTL_KYOTO_PALETTE_GPU_MEMORY
, mUsesGPUMemory(false)
#endif
{
  in.Get(reinterpret_cast< uchar* >(mEntries.get()), mEntryCount * sizeof(ushort));
  GXInitTlutObj(&mTlutObj, mEntries.get(), format_to_format(mFmt), mEntryCount);
  DCFlushRange(mEntries.get(), mEntryCount * sizeof(ushort));
}

CGraphicsPalette::~CGraphicsPalette() {
#if RSTL_KYOTO_PALETTE_GPU_MEMORY
  ReleaseEntries();
#elif defined(TARGET_PC)
  GXDestroyTlutObj(&mTlutObj);
  CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_NextFrame,
                                        mEntries.release());
#else
  const uint frameDiff = sCurrentFrameCount - mFrameLoaded;
  if (frameDiff < 2) {
    CFrameDelayedKiller::ScheduleDeletion(frameDiff > 0
                                              ? CFrameDelayedKiller::kWhichFrame_ThisFrame
                                              : CFrameDelayedKiller::kWhichFrame_NextFrame,
                                          mEntries.release());
  }
#endif
}

#if RSTL_KYOTO_PALETTE_GPU_MEMORY
void CGraphicsPalette::ReleaseEntries() {
  if (mEntries.get() != nullptr) {
    if (mUsesGPUMemory) {
      GPUMemory::ReleaseAllocation();
      mEntries.release();
    } else {
      const uint frameDiff = sCurrentFrameCount - mFrameLoaded;
      if (frameDiff < 2) {
        CFrameDelayedKiller::ScheduleDeletion(frameDiff > 0
                                                  ? CFrameDelayedKiller::kWhichFrame_ThisFrame
                                                  : CFrameDelayedKiller::kWhichFrame_NextFrame,
                                              mEntries.release());
      } else {
        mEntries = nullptr;
      }
    }
  }
}
#endif

void CGraphicsPalette::Load() const {
  GXLoadTlut(const_cast< GXTlutObj* >(&mTlutObj), GX_TLUT0);
  mFrameLoaded = sCurrentFrameCount;
}

#if defined(TARGET_PC)
void* CGraphicsPalette::Lock() {
  // AuroraGXSync();
  mLocked = true;
  return mEntries.get();
}
#endif

void CGraphicsPalette::UnLock() {
#if RSTL_KYOTO_PALETTE_UNLOCK_STORE_RANGE
  DCStoreRange(mEntries.get(), mEntryCount * sizeof(ushort));
#endif
#if defined(TARGET_PC)
  GXInitTlutObjData(&mTlutObj, mEntries.get());
#else
  GXInitTlutObj(&mTlutObj, mEntries.get(), format_to_format(mFmt), mEntryCount);
#endif
  DCFlushRange(mEntries.get(), mEntryCount * sizeof(ushort));
  mLocked = false;
}

#if RSTL_KYOTO_PALETTE_GPU_MEMORY
ushort* CGraphicsPalette::GetPaletteData() { return mEntries.get(); }
#endif
