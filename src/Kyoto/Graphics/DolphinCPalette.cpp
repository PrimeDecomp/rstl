#include "rstl/RstlVersions.h"
#include "Kyoto/Graphics/CGraphicsPalette.hpp"

#include "Kyoto/Alloc/CMemorySys.hpp"

#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/Alloc/CMemory.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

#if RSTL_PRIME1
#include "dolphin/gx/GXEnum.h"
#endif

#include "dolphin/gx/GXTexture.h"
#include "dolphin/os/OSCache.h"

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
// TODO: verify that this Prime/Echoes difference is real
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
// TODO: verify that this Prime/Echoes difference is real
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
  const uint frameDiff = sCurrentFrameCount - mFrameLoaded;

  if (frameDiff < 2) {
    CFrameDelayedKiller::ScheduleDeletion(frameDiff > 0
                                              ? CFrameDelayedKiller::kWhichFrame_ThisFrame
                                              : CFrameDelayedKiller::kWhichFrame_NextFrame,
                                          mEntries.release());

  }
}

void CGraphicsPalette::Load() const {
  GXLoadTlut(&mTlutObj, GX_TLUT0);
  mFrameLoaded = sCurrentFrameCount;
}

void CGraphicsPalette::UnLock() {
  DCStoreRange(mEntries.get(), mEntryCount * sizeof(ushort));
  GXInitTlutObj(&mTlutObj, mEntries.get(), format_to_format(mFmt), mEntryCount);
  DCFlushRange(mEntries.get(), mEntryCount * sizeof(ushort));
  mLocked = false;
}

