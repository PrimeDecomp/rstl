#include "Kyoto/Graphics/CGraphicsPalette.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/RstlVersions.h"

#include "dolphin/gx.h"
#include "dolphin/os.h"

uint CGraphicsPalette::sCurrentFrameCount = 0;

static inline GXTlutFmt format_to_format(EPaletteFormat format) {
  return static_cast< GXTlutFmt >(format);
}

CGraphicsPalette::CGraphicsPalette(EPaletteFormat format, int numEntries)
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
#else
, mEntries(static_cast< ushort* >(
      CMemory::Alloc(numEntries * sizeof(ushort), IAllocator::kHI_RoundUpLen)))
#endif

, mLocked(false) {
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
, mEntries(static_cast< ushort* >(
      CMemory::Alloc(mEntryCount * sizeof(ushort), IAllocator::kHI_RoundUpLen)))
#endif

, mLocked(false) {
  in.Get(reinterpret_cast< uchar* >(mEntries.get()), mEntryCount * sizeof(ushort));
  GXInitTlutObj(&mTlutObj, mEntries.get(), format_to_format(mFmt), mEntryCount);
  DCFlushRange(mEntries.get(), mEntryCount * sizeof(ushort));
}

CGraphicsPalette::~CGraphicsPalette() {
#if defined(TARGET_PC)
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

void CGraphicsPalette::Load() const {
  GXLoadTlut(&mTlutObj, GX_TLUT0);
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
  DCStoreRange(mEntries.get(), mEntryCount * sizeof(ushort));
#if defined(TARGET_PC)
  GXInitTlutObjData(&mTlutObj, mEntries.get());
#else
  GXInitTlutObj(&mTlutObj, mEntries.get(), format_to_format(mFmt), mEntryCount);
#endif
  DCFlushRange(mEntries.get(), mEntryCount * sizeof(ushort));
  mLocked = false;
}
