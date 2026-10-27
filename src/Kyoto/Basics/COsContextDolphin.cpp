#include "Kyoto/Basics/COsContext.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "rstl/RstlVersions.h"

#include "dolphin/os.h"
#if !RSTL_KYOTO_OS_CONTEXT_ARENA_BLOCK
#include "dolphin/vi.h"
#include <string.h>
#endif

COsContext::COsContext(bool, bool)
#if RSTL_KYOTO_OS_CONTEXT_ARENA_BLOCK
: mLanguage(OSGetLanguage())
, mArenaLo1(nullptr)
, mArenaHi(nullptr)
, mArenaLo2(nullptr)
, mArenaBlock(nullptr)
, x28_(nullptr)
, mArenaBlockSize(0)
#endif
{
  CBasics::Init();

  switch (OSGetConsoleType()) {
  case OS_CONSOLE_RETAIL1:
    mConsoleType = kCT_Retail;
    break;
  case OS_CONSOLE_DEVHW1:
    mConsoleType = kCT_Development1;
    break;
  case OS_CONSOLE_DEVHW2:
  case OS_CONSOLE_DEVHW3:
    mConsoleType = kCT_Development2Or3;
    break;
  case OS_CONSOLE_EMULATOR:
    mConsoleType = kCT_Emulator;
    break;
  }

#if RSTL_KYOTO_OS_CONTEXT_ARENA_BLOCK
  mArenaBlockSize = 0x1fe000;
  mArenaBlock = AllocFromArena(mArenaBlockSize);
#endif
}

COsContext::~COsContext() {}

bool COsContext::Update() { return true; }

#if !RSTL_KYOTO_OS_CONTEXT_ARENA_BLOCK
COsKeyState COsContext::GetOsKeyState(int key) const {
  return COsKeyState(key, false, false, false, false);
}

int COsContext::OpenWindow(const char* title, int x, int y, int w, int h, bool fullscreen) {
  VIInit();
  GXRenderModeObj* rModeObj;
  switch (VIGetTvFormat()) {
  case VI_NTSC:
    rModeObj = &GXNtsc480IntDf;
    mFormat = 1;
    break;
  case VI_PAL:
    rModeObj = &GXPal528IntDf;
    mFormat = 2;
    break;
  case VI_MPAL:
    rModeObj = &GXMpal480IntDf;
    mFormat = 3;
    break;
  }

  if (w > 0) {
    mRenderMode.viWidth = w;
  }
  if (h > 0) {
    mRenderMode.viHeight = h;
  }

  GXAdjustForOverscan(rModeObj, &mRenderMode, 0, 16);

  mLeft = mRenderMode.viXOrigin;
  mTop = mRenderMode.viYOrigin;
  mRight = mRenderMode.viWidth;
  mBottom = mRenderMode.viHeight;

  mFrameBufferSize = (ushort)((mRenderMode.fbWidth + 15) & ~15) * mRenderMode.xfbHeight * 2;
  mFrameBuffer1 = OSAllocFromArenaLo(mFrameBufferSize, 32);
  mFrameBuffer2 = OSAllocFromArenaLo(mFrameBufferSize, 32);
  mArenaLo2 = OSGetArenaLo();
  mArenaLo1 = OSGetArenaLo();
  mArenaHi = OSGetArenaHi();
  mRenderMode.viWidth += 20;
  mRenderMode.viXOrigin -= 10;

  if (mProgressiveMode) {
    mRenderMode.viTVmode = VI_TVMODE_NTSC_PROG;
    mRenderMode.xFBmode = VI_XFBMODE_SF;
    uchar progressiveFilterPattern[7] = {4, 4, 16, 16, 16, 4, 4};
    memcpy(mRenderMode.vfilter, progressiveFilterPattern, 7);
  }
  VIConfigure(&mRenderMode);
  VIFlush();
  return -1;
}
#endif

void* COsContext::AllocFromArena(size_t sz) {
  void* ret = OSAllocFromArenaLo(sz, 32);

  mArenaLo2 = OSGetArenaLo();
  mArenaLo1 = OSGetArenaLo();
  mArenaHi = OSGetArenaHi();
  return ret;
}

#if RSTL_KYOTO_OS_CONTEXT_ARENA_BLOCK
int COsContext::GetLanguage() const { return mLanguage; }
#endif
