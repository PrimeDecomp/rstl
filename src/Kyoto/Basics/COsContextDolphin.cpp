#include "Kyoto/Basics/COsContext.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "rstl/RstlVersions.h"

#include "dolphin/gx.h"
#include "dolphin/os.h"
#include "dolphin/vi.h"
#if !RSTL_KYOTO_OS_CONTEXT_ARENA_BLOCK
#include <string.h>
#endif

#if RSTL_KYOTO_WII_OS_CONTEXT
// Shared Wii entry points; the games' SDK header sets expose different subsets.
extern "C" {
void SCInit(void);
u32 SCCheckStatus(void);
u8 SCGetLanguage(void);
u8 SCGetAspectRatio(void);
u8 SCGetEuRgb60Mode(void);
void* OSGetMEM1ArenaLo(void);
void* OSGetMEM1ArenaHi(void);
void* OSGetMEM2ArenaLo(void);
void* OSGetMEM2ArenaHi(void);
void* OSAllocFromMEM2ArenaLo(u32 size, u32 align);
}

// Guessed names for the private system-configuration helpers.
static void EnsureSystemConfigReady() {
  static bool initialized = false;
  if (!initialized) {
    SCInit();
    initialized = true;
  }
  while (SCCheckStatus() == 1) {
  }
}

static int GetSystemLanguage() {
  EnsureSystemConfigReady();
  const uint language = SCGetLanguage();
  if (language < 1 || language > 6) {
    return 0;
  }
  return language - 1;
}

#if RSTL_KYOTO_OS_CONTEXT_ASPECT_RATIO
static COsContext::EAspectRatio GetSystemAspectRatio() {
  EnsureSystemConfigReady();
  return static_cast< COsContext::EAspectRatio >(SCGetAspectRatio() == 1);
}
#endif

bool COsContext::GetEuRgb60Mode() const {
  EnsureSystemConfigReady();
  return SCGetEuRgb60Mode() == 1;
}
#endif

COsContext::COsContext(bool, bool)
#if RSTL_KYOTO_OS_CONTEXT_ARENA_BLOCK
#if RSTL_KYOTO_WII_OS_CONTEXT
: mLanguage(RSTL_VERSION < RSTL_R3ME_00 ? kLang_Japanese : GetSystemLanguage())
#if RSTL_KYOTO_OS_CONTEXT_ASPECT_RATIO
, mAspectRatio(GetSystemAspectRatio())
#endif
, mMem1ArenaLo(nullptr)
, mMem1ArenaHi(nullptr)
, mMem1ArenaCurrent(nullptr)
#else
: mLanguage(OSGetLanguage())
#endif
, mArenaLo1(nullptr)
, mArenaHi(nullptr)
, mArenaLo2(nullptr)
#if !RSTL_KYOTO_WII_OS_CONTEXT
, mArenaBlock(nullptr)
, x28_(nullptr)
#endif
, mArenaBlockSize(0)
#if RSTL_KYOTO_WII_OS_CONTEXT
, mFifo(nullptr)
, mFifoSize(0)
, mGpuBuffer(nullptr)
, mGpuBufferSize(0)
, mSpareFrameBuffer(nullptr)
, mSpareFrameBufferSize(0)
, mSpareTexture(nullptr)
, mSpareTextureFormat(GX_TF_RGB565)
, mSpareTextureSize(0)
, mFrameBuffer1(nullptr)
, mFrameBuffer2(nullptr)
, mFrameBufferSize(0)
#endif
#endif
{
  CBasics::Init();

  switch (OSGetConsoleType()) {
  case OS_CONSOLE_RETAIL1:
#if RSTL_KYOTO_WII_OS_CONTEXT
    mConsoleType = 4; // GameCube retail classification in the Wii engine.
#else
    mConsoleType = kCT_Retail;
#endif
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
#if RSTL_KYOTO_WII_OS_CONTEXT
  // The Wii console classifications; original SDK constant names remain unresolved.
  case 0x10000010:
  case 0x10000011:
  case 0x10000012:
  case 0x10000020:
  case 0x10000021:
    mConsoleType = 3;
    break;
#endif
  }

#if RSTL_KYOTO_WII_OS_CONTEXT
  VIInit();
  VISetBlack(TRUE);
  int height;
  int fifoSize;
  int gpuBufferSize;
  if (VIGetTvFormat() == VI_PAL && !GetEuRgb60Mode()) {
    height = 528;
    fifoSize = 0x3d800;
    gpuBufferSize = RSTL_KYOTO_OS_CONTEXT_LARGE_GPU_BUFFER ? 0x64000 : 0x24000;
  } else {
    height = 448;
    fifoSize = 0x60000;
    gpuBufferSize = RSTL_KYOTO_OS_CONTEXT_LARGE_GPU_BUFFER ? 0x80000 : 0x40000;
  }
  mFifo = AllocFromMem1Arena(fifoSize);
  mFifoSize = fifoSize;
  mGpuBuffer = AllocFromMem1Arena(gpuBufferSize);
  mGpuBufferSize = gpuBufferSize;
  mSpareTextureSize = GXGetTexBufferSize(320, height / 2, mSpareTextureFormat, false, 0);
  mSpareTexture = AllocFromArena(mSpareTextureSize);
  mFrameBufferSize = height * 1280;
  mFrameBuffer1 = AllocFromArena(mFrameBufferSize);
  mFrameBuffer2 = AllocFromArena(mFrameBufferSize);
  mSpareFrameBufferSize = height * 1280;
  mSpareFrameBuffer = AllocFromArena(mSpareFrameBufferSize);
#elif RSTL_KYOTO_OS_CONTEXT_ARENA_BLOCK
  mArenaBlockSize = RSTL_VERSION == RSTL_GM8P_00 ? 0x23e000 : 0x1fe000;
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

#if defined(TARGET_PC)
  mFrameBuffer1 = reinterpret_cast< void* >(0xDEADBABEu);
  mFrameBuffer2 = reinterpret_cast< void* >(0xBABEDEADu);
#else
  mFrameBufferSize = (ushort)((mRenderMode.fbWidth + 15) & ~15) * mRenderMode.xfbHeight * 2;
  mFrameBuffer1 = OSAllocFromArenaLo(mFrameBufferSize, 32);
  mFrameBuffer2 = OSAllocFromArenaLo(mFrameBufferSize, 32);
  mArenaLo2 = OSGetArenaLo();
  mArenaLo1 = OSGetArenaLo();
  mArenaHi = OSGetArenaHi();
  mRenderMode.viWidth += 20;
  mRenderMode.viXOrigin -= 10;
#endif

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
#if defined(TARGET_PC)
  return operator new(sz);
#else
#if RSTL_KYOTO_WII_OS_CONTEXT
  void* ret = OSAllocFromMEM2ArenaLo(sz, 32);
  mArenaLo2 = OSGetMEM2ArenaLo();
  mArenaLo1 = OSGetMEM2ArenaLo();
  mArenaHi = OSGetMEM2ArenaHi();
#else
  void* ret = OSAllocFromArenaLo(sz, 32);
  mArenaLo2 = OSGetArenaLo();
  mArenaLo1 = OSGetArenaLo();
  mArenaHi = OSGetArenaHi();
#endif
  return ret;
#endif
}

#if RSTL_KYOTO_OS_CONTEXT_ARENA_BLOCK
int COsContext::GetLanguage() const { return mLanguage; }
#endif

#if RSTL_KYOTO_WII_OS_CONTEXT
void* COsContext::AllocFromMem1Arena(size_t sz) {
  void* result = OSAllocFromArenaLo(sz, 32);
  mMem1ArenaCurrent = OSGetMEM1ArenaLo();
  mMem1ArenaLo = OSGetMEM1ArenaLo();
  mMem1ArenaHi = OSGetMEM1ArenaHi();
  return result;
}
#endif
