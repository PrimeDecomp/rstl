#include "rstl/RstlVersions.h"
// G2ME01 scaffold remaining identity and code-generation questions are recorded in research.
#include "Kyoto/Graphics/CGraphics.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/COsContext.hpp"
#include "Kyoto/Basics/CStopwatch.hpp"

#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphicsSys.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Animation/CSkinnedModel.hpp"
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
#include "Kyoto/Graphics/DolphinGPUMemory.hpp"
#endif

#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Alloc/LockedCache.hpp"
#include "Kyoto/Basics/RAssertDolphin.hpp"

#if RSTL_PRIME1
#include "GameVersions.h"
#endif

#include "dolphin/types.h"
#include "rstl/math.hpp"

#include "dolphin/gx.h"
#include "dolphin/vi.h"

#include "string.h"

#if !RSTL_PRIME1
static bool sGXAborted;
#endif

bool CGraphicsSys::mGraphicsInitialized;
static CStopwatch sFPSTimer;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
bool sIs50Hz = RSTL_VERSION == RSTL_GM8P_00;
#if RSTL_VERSION == RSTL_GM8P_00
static uchar* sGraphicsArena;
static int sGraphicsArenaSize;
static uint sGraphicsArenaOffset;
static void* sSpareFrameBuffer;
static int sSpareFrameBufferSize;

// Guessed name. Native PAL allocates the video/FIFO/skinning buffers from one arena.
static void* AllocateGraphicsArena(uint size) {
  void* ret = sGraphicsArena + sGraphicsArenaOffset;
  sGraphicsArenaOffset += size;
  return ret;
}
#else
ATTRIBUTE_ALIGN_DECL(32, static uchar sSpareFrameBuffer[640 * 448]);
#endif
#else
static int sSpareAllocationSize;
static void* sSpareAllocation;
#endif

// clang-format off
CTevCombiners::CTevPass CGraphics::kEnvModulateConstColor(
  CTevCombiners::ColorPass(
    CTevCombiners::kCS_Zero,
    CTevCombiners::kCS_RasterColor,
    CTevCombiners::kCS_RegisterC0,
    CTevCombiners::kCS_Zero
  ),
  CTevCombiners::AlphaPass(
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_RasterAlpha,
    CTevCombiners::kAS_RegisterA0,
    CTevCombiners::kAS_Zero
  )
);
CTevCombiners::CTevPass CGraphics::kEnvConstColor(
  CTevCombiners::ColorPass(
    CTevCombiners::kCS_Zero,
    CTevCombiners::kCS_Zero,
    CTevCombiners::kCS_Zero,
    CTevCombiners::kCS_RegisterC0
  ),
  CTevCombiners::AlphaPass(
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_RegisterA0
  )
);
CTevCombiners::CTevPass CGraphics::kEnvModulate(
  CTevCombiners::ColorPass(
    CTevCombiners::kCS_Zero,
    CTevCombiners::kCS_RasterColor,
    CTevCombiners::kCS_TextureColor,
    CTevCombiners::kCS_Zero
  ),
  CTevCombiners::AlphaPass(
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_RasterAlpha,
    CTevCombiners::kAS_TextureAlpha,
    CTevCombiners::kAS_Zero
  )
);
CTevCombiners::CTevPass CGraphics::kEnvDecal(
  CTevCombiners::ColorPass(
    CTevCombiners::kCS_RasterColor,
    CTevCombiners::kCS_TextureColor,
    CTevCombiners::kCS_TextureAlpha,
    CTevCombiners::kCS_Zero
  ),
  CTevCombiners::AlphaPass(
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_RasterAlpha
  )
);
CTevCombiners::CTevPass CGraphics::kEnvBlend(
  CTevCombiners::ColorPass(
    CTevCombiners::kCS_RasterColor,
    CTevCombiners::kCS_One,
    CTevCombiners::kCS_TextureColor,
    CTevCombiners::kCS_Zero
  ),
  CTevCombiners::AlphaPass(
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_TextureAlpha,
    CTevCombiners::kAS_RasterAlpha,
    CTevCombiners::kAS_Zero
  )
);
CTevCombiners::CTevPass CGraphics::kEnvReplace(
  CTevCombiners::ColorPass(
    CTevCombiners::kCS_Zero,
    CTevCombiners::kCS_Zero,
    CTevCombiners::kCS_Zero,
    CTevCombiners::kCS_TextureColor
  ),
  CTevCombiners::AlphaPass(
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_TextureAlpha
  )
);
static CTevCombiners::CTevPass kEnvBlendCTandCConCF(
  CTevCombiners::ColorPass(
    CTevCombiners::kCS_RegisterC0,
    CTevCombiners::kCS_TextureColor,
    CTevCombiners::kCS_RasterColor,
    CTevCombiners::kCS_Zero
  ),
  CTevCombiners::AlphaPass(
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_RasterAlpha
  )
);
CTevCombiners::CTevPass CGraphics::kEnvModulateAlpha(
  CTevCombiners::ColorPass(
    CTevCombiners::kCS_Zero,
    CTevCombiners::kCS_Zero,
    CTevCombiners::kCS_Zero,
    CTevCombiners::kCS_RasterColor
  ),
  CTevCombiners::AlphaPass(
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_TextureAlpha,
    CTevCombiners::kAS_RasterAlpha,
    CTevCombiners::kAS_Zero
  )
);
CTevCombiners::CTevPass CGraphics::kEnvModulateColor(
  CTevCombiners::ColorPass(
    CTevCombiners::kCS_Zero,
    CTevCombiners::kCS_TextureColor,
    CTevCombiners::kCS_RasterColor,
    CTevCombiners::kCS_Zero
  ),
  CTevCombiners::AlphaPass(
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_Konst,
    CTevCombiners::kAS_RasterAlpha,
    CTevCombiners::kAS_Zero
  )
);
CTevCombiners::CTevPass CGraphics::kEnvModulateColorByAlpha(
  CTevCombiners::ColorPass(
    CTevCombiners::kCS_Zero,
    CTevCombiners::kCS_PreviousColor,
    CTevCombiners::kCS_PreviousAlpha,
    CTevCombiners::kCS_Zero
  ),
  CTevCombiners::AlphaPass(
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_Zero,
    CTevCombiners::kAS_PreviousAlpha
  )
);
// clang-format on

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CGraphics::CRenderState CGraphics::sRenderState;
VecPtr CGraphics::vtxBuffer;
VecPtr CGraphics::nrmBuffer;
Vec2Ptr CGraphics::txtBuffer0;
Vec2Ptr CGraphics::txtBuffer1;
uint* CGraphics::clrBuffer;
#else
CGraphics::CRenderState CGraphics::mRenderState;
VecPtr CGraphics::mVertexBuffer;
VecPtr CGraphics::mNormalBuffer;
Vec2Ptr CGraphics::mTexCoordBuffer0;
Vec2Ptr CGraphics::mTexCoordBuffer1;
uint* CGraphics::mColorBuffer;
#endif

bool CGraphics::mJustReset;
ERglCullMode CGraphics::mCullMode;
int CGraphics::mNumLightsActive;
float CGraphics::mDepthNear;
VecPtr CGraphics::mpVtxBuffer;
VecPtr CGraphics::mpNrmBuffer;
Vec2Ptr CGraphics::mpTxtBuffer0;
Vec2Ptr CGraphics::mpTxtBuffer1;
uint* CGraphics::mpClrBuffer;

// Guessed name inherited from Prime; these unused fields are not yet identified.
struct CGXLightParams {
  int x0_;
  int x4_;
  int x8_;
  int xc_;
  int x10_;

  CGXLightParams() : x0_(4), x4_(0), x8_(0), xc_(2), x10_(2) {}
};
CGXLightParams mLightParams[8];

struct {
  Vec mPosition;
  Vec mNormal;
  Vec2 mTexCoord0;
  Vec2 mTexCoord1;
  uint mColor;
  ushort mTextureUsed;
  uchar mStreamFlags;

} vtxDescr;

CVector3f CGraphics::kDefaultPositionVector(0.f, 0.f, 0.f);
CVector3f CGraphics::kDefaultDirectionVector(0.f, 1.f, 0.f);
CGraphics::CProjectionState CGraphics::mProj(true, -1.f, 1.f, 1.f, -1.f, 1.f, 100.f);
CTransform4f CGraphics::mViewMatrix = CTransform4f::Identity();
CTransform4f CGraphics::mModelMatrix = CTransform4f::Identity();
CColor CGraphics::mClearColor = CColor::Black();
#if RSTL_PRIME1
CVector3f CGraphics::mViewPoint(0.f, 0.f, 0.f);
#endif

GXLightObj CGraphics::mLightObj[8];
GXTexRegion CGraphics::mTexRegions[GX_MAX_TEXMAP];
GXTexRegion CGraphics::mTexRegionsCI[GX_MAX_TEXMAP / 2];
GXRenderModeObj CGraphics::mRenderModeObj;
Mtx CGraphics::mGXViewPointMatrix;
#if RSTL_PRIME1
Mtx CGraphics::mGXModelMatrix;
#endif

Mtx CGraphics::mGxModelView;
#if !RSTL_PRIME1
Mtx CGraphics::mGxModelViewInvXpose;
#endif

Mtx CGraphics::mCameraMtx;

int CGraphics::mNumPrimitives;
int CGraphics::mFrameCounter;
float CGraphics::mFramesPerSecond;
float CGraphics::mLastFramesPerSecond;
#if !RSTL_PRIME1
static float sFrameWaitFraction;
static float sPreviousFrameWaitFraction;
#endif

int CGraphics::mNumBreakpointsWaiting;
int CGraphics::mFlippingState;
bool CGraphics::mLastFrameUsedAbove;
bool CGraphics::mInterruptLastFrameUsedAbove;
uchar CGraphics::mLightActive;
uchar CGraphics::mLightsWereOn;
void* CGraphics::mpFrameBuf1;
void* CGraphics::mpFrameBuf2;
void* CGraphics::mpCurrenFrameBuf;
int CGraphics::mSpareBufferSize;
void* CGraphics::mpSpareBuffer;
int CGraphics::mSpareBufferTexCacheSize;
GXTexRegionCallback CGraphics::mGXDefaultTexRegionCallback;
void* CGraphics::mpFifo;
#if !RSTL_PRIME1
uint CGraphics::mFifoSize = 0;
#if RSTL_VERSION != RSTL_G2MP
bool CGraphics::sIs50Hz;
#endif
#endif

GXFifoObj* CGraphics::mpFifoObj;
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_GM8J_00
uint CGraphics::mFifoSize = 0;
#endif
#endif

uint CGraphics::mRenderTimings;
float CGraphics::mSecondsMod900;
CTimeProvider* CGraphics::mpExternalTimeProvider;
int CGraphics::mScreenStretch;
int CGraphics::mScreenPositionX;
int CGraphics::mScreenPositionY;
#if !RSTL_PRIME1
static bool sUseStreamVertexDelay;
static int sTotalSize;
static int sAlloced;
static uchar* sGraphicsArena;
#endif

CViewport CGraphics::mViewport = {0, 0, 640, 480, 320.f, 240.f};
ELightType CGraphics::mLightTypes[8] = {
    kLT_Directional, kLT_Directional, kLT_Directional, kLT_Directional,
    kLT_Directional, kLT_Directional, kLT_Directional, kLT_Directional,
};

const CTevCombiners::CTevPass& CGraphics::kEnvPassthru = CTevCombiners::kEnvPassthru;
bool CGraphics::mIsBeginSceneClearFb = true;
ERglEnum CGraphics::mDepthFunc = kE_LEqual;
ERglPrimitive CGraphics::mCurrentPrimitive = kP_Points;
float CGraphics::mDepthFar = 1.f;
u32 CGraphics::mClearDepthValue = GX_MAX_Z24;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_GM8E_02
float CGraphics::mPixelAspectRatio = 1.f;
#endif
#else
#if RSTL_VERSION == RSTL_G2MP
bool CGraphics::sIs50Hz = true;
#endif
float CGraphics::mPixelAspectRatio = 1.f;
bool CGraphics::mUseNormalMatrix = true;
#endif

bool CGraphics::mIsGXModelMatrixIdentity = true;
bool CGraphics::mFirstFrame = true;
bool CGraphics::mUseVideoFilter = true;
float CGraphics::mBrightness = 1.f;

const GXTexMapID CGraphics::kSpareBufferTexMapID = GX_TEXMAP7;
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_GM8J_00
void CGraphics::InitGraphicsFifo(GXFifoObj* obj, void* fifo, uint fifoSize) {
  GXFifoObj fifoObj;
  GXInitFifoBase(&fifoObj, fifo, fifoSize);
  GXSetCPUFifo(&fifoObj);
  GXSetGPFifo(&fifoObj);
  GXInitFifoLimits(obj, fifoSize - 0x4000, fifoSize - 0x10000);
  GXSetCPUFifo(obj);
  GXSetGPFifo(obj);
}
#endif
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION == RSTL_GM8P_00
void CGraphics::ConfigureVideoMode(bool initial, bool rgb60) {
  if (!initial) {
    CFrameDelayedKiller::StallAndFlushAllAllocations();
  }
  bool is60Hz = true;
  GXRenderModeObj* mode = nullptr;
  switch (VIGetTvFormat()) {
  case VI_NTSC:
  case VI_MPAL:
    *static_cast< int* >(nullptr) = 0;
    break;
  case VI_PAL:
  case VI_EURGB60:
    if (rgb60) {
      mode = &GXEurgb60Hz480IntDf;
    } else {
      mode = &GXPal528IntDf;
      is60Hz = false;
    }
    break;
  }
  GXAdjustForOverscan(mode, &mRenderModeObj, 0, is60Hz ? 16 : 0);
  mPixelAspectRatio = is60Hz ? 1.f : 1.1785715f;
  sIs50Hz = !is60Hz;
  mRenderModeObj.viWidth += 20;
  mRenderModeObj.viXOrigin -= 10;

  mpFrameBuf1 = nullptr;
  mpFrameBuf2 = nullptr;
  mpFifo = nullptr;
  sSpareFrameBuffer = nullptr;
  CSkinnedModel::SetSkinningBuffer(nullptr, 0);
  sGraphicsArenaOffset = 0;

  uint frameBufferSize =
      static_cast< ushort >((mRenderModeObj.fbWidth + 15) & ~15) * mRenderModeObj.xfbHeight * 2;
  mpFrameBuf1 = AllocateGraphicsArena(frameBufferSize);
  mpFrameBuf2 = AllocateGraphicsArena(frameBufferSize);
  sSpareFrameBufferSize = is60Hz ? 0x46000 : 0x52800;
  uint fifoSize = is60Hz ? 0x60000 : 0x3d800;
  int skinningSize = is60Hz ? 0x80000 : 0x64000;
  mpFifo = AllocateGraphicsArena(fifoSize);
  sSpareFrameBuffer = AllocateGraphicsArena(sSpareFrameBufferSize);
  CSkinnedModel::SetSkinningBuffer(AllocateGraphicsArena(skinningSize), skinningSize);
  mSpareBufferSize = sSpareFrameBufferSize;
  mpSpareBuffer = sSpareFrameBuffer;

  if (!initial) {
    mRenderModeObj.viWidth += mScreenStretch * 2;
    mRenderModeObj.viXOrigin += mScreenPositionX - mScreenStretch;
    mRenderModeObj.viYOrigin += mScreenPositionY;
    VIWaitForRetrace();
    VIWaitForRetrace();
  }
  VIConfigure(&mRenderModeObj);
  VIFlush();
  if (initial) {
    mpFifoObj = GXInit(mpFifo, fifoSize);
  }
  GXFifoObj fifoObj;
  GXInitFifoBase(&fifoObj, mpFifo, fifoSize);
  GXSetCPUFifo(&fifoObj);
  GXSetGPFifo(&fifoObj);
  GXInitFifoBase(mpFifoObj, mpFifo, fifoSize);
  GXInitFifoLimits(mpFifoObj, fifoSize - 0x4000, fifoSize - 0x10000);
  GXSetCPUFifo(mpFifoObj);
  GXSetGPFifo(mpFifoObj);
  GXSetCopyFilter(mRenderModeObj.aa, mRenderModeObj.sample_pattern, GX_ENABLE,
                 mRenderModeObj.vfilter);
}

bool CGraphics::Startup(const COsContext& osContext, bool rgb60) {
  sGraphicsArena = static_cast< uchar* >(osContext.GetArenaBlock());
  sGraphicsArenaSize = osContext.GetArenaBlockSize();
  sGraphicsArenaOffset = 0;
  VIInit();
  VISetBlack(TRUE);
  ConfigureVideoMode(true, rgb60);
  GXSetMisc(GX_MT_XF_FLUSH, 8);
  GXSetDither(GX_FALSE);
#else
bool CGraphics::Startup(const COsContext& osContext, uint fifoSize, void* fifoBase) {
  mpFifo = fifoBase;
  mpFifoObj = GXInit(fifoBase, fifoSize);
#if RSTL_VERSION >= RSTL_GM8J_00
  mFifoSize = fifoSize;
  InitGraphicsFifo(mpFifoObj, mpFifo, fifoSize);
#else
  GXFifoObj fifoObj;
  GXInitFifoBase(&fifoObj, mpFifo, fifoSize);
  GXSetCPUFifo(&fifoObj);
  GXSetGPFifo(&fifoObj);
  GXInitFifoLimits(mpFifoObj, fifoSize - 0x4000, fifoSize - 0x10000);
  GXSetCPUFifo(mpFifoObj);
  GXSetGPFifo(mpFifoObj);
  GXSetMisc(GX_MT_XF_FLUSH, 8);
  GXSetDither(GX_FALSE);
#endif

#endif
#else
#if RSTL_VERSION < RSTL_R32J
void CGraphics::InitGraphicsFifo(GXFifoObj* obj, void* fifo, uint fifoSize) {
  GXFifoObj fifoObj;
  GXInitFifoBase(&fifoObj, fifo, fifoSize);
  GXSetCPUFifo(&fifoObj);
  GXSetGPFifo(&fifoObj);
#if RSTL_VERSION == RSTL_G2MP
  GXInitFifoBase(obj, fifo, fifoSize);
#endif
  GXInitFifoLimits(obj, fifoSize - 0x4000, fifoSize - 0x10000);
  GXSetCPUFifo(obj);
  GXSetGPFifo(obj);
}
#endif

// Guessed names for the graphics arena helpers.
static void InitializeGraphicsArena(void* base, int size) {
  sGraphicsArena = static_cast< uchar* >(base);
  sTotalSize = size;
  sAlloced = 0;
}

static void* AllocateGraphicsArena(int size) {
  void* result = sGraphicsArena + sAlloced;
  sAlloced += size;
  return result;
}

static void ResetGraphicsArena() { sAlloced = 0; }

bool CGraphics::Startup(const COsContext& osContext, bool progressive) {
  InitializeGraphicsArena(osContext.GetArenaBlock(), osContext.GetArenaBlockSize());
  VIInit();
  VISetBlack(TRUE);
  ConfigureVideo(true, progressive);
#endif

  CGX::ResetGXStates();
  InitGraphicsVariables();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION == RSTL_GM8P_00
  ConfigureFrameBuffer();
#else
  ConfigureFrameBuffer(osContext);
#endif
#else
  ConfigureFrameBuffer();
#endif
  for (int i = 0; i < ARRAY_SIZE(mTexRegions); ++i) {
    GXInitTexCacheRegion(&mTexRegions[i], false, 0x8000 * i, GX_TEXCACHE_32K, 0x80000 + 0x8000 * i,
                         GX_TEXCACHE_32K);
  }
  for (int i = 0; i < ARRAY_SIZE(mTexRegionsCI); ++i) {
    GXInitTexCacheRegion(&mTexRegionsCI[i], false, (8 + 2 * i) << 15, GX_TEXCACHE_32K,
                         (9 + 2 * i) << 15, GX_TEXCACHE_32K);
  }
  mGXDefaultTexRegionCallback = GXSetTexRegionCallback(TexRegionCallback);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION == RSTL_GM8P_00
  mSpareBufferSize = sSpareFrameBufferSize;
#else
  mSpareBufferSize = ARRAY_SIZE(sSpareFrameBuffer);
#endif
  mpSpareBuffer = sSpareFrameBuffer;
#else
  mSpareBufferSize = sSpareAllocationSize;
  mpSpareBuffer = sSpareAllocation;
#endif

  mSpareBufferTexCacheSize = 0x10000;
  return true;
}

#if !RSTL_PRIME1
#if RSTL_VERSION != RSTL_G2MP
// Guessed name.
static void SetProgressiveFilter(GXRenderModeObj& mode) {
  const u8 filter[7] = {4, 4, 16, 16, 16, 4, 4};
  memcpy(mode.vfilter, filter, sizeof(filter));
}
#endif

void CGraphics::ConfigureVideo(bool initial, uchar progressive) {
  if (!initial) {
    CFrameDelayedKiller::StallAndFlushAllAllocations();
  }
#if RSTL_VERSION == RSTL_G2MP
  // On PAL, `progressive` selects PAL60 (EURGB60) over 50 Hz PAL.
  bool is60Hz = true;
  GXRenderModeObj* mode = nullptr;
  switch (VIGetTvFormat()) {
  case VI_MPAL:
    rs_debugger_printf("MPAL TV Format not compatible with PAL build");
    break;
  case VI_NTSC:
    rs_debugger_printf("NTSC TV Format not compatible with PAL build");
    break;
  case VI_PAL:
  case VI_EURGB60:
    if (!progressive) {
      is60Hz = false;
      mode = &GXPal528IntDf;
    } else {
      mode = &GXEurgb60Hz480IntDf;
    }
    break;
  }
  GXAdjustForOverscan(mode, &mRenderModeObj, 0, is60Hz ? 16 : 0);
  mPixelAspectRatio = is60Hz ? 1.f : 1.1785715f;
  sIs50Hz = !is60Hz;
  mRenderModeObj.viWidth += 20;
  mRenderModeObj.viXOrigin -= 10;
#else
  GXRenderModeObj* mode = nullptr;
  switch (VIGetTvFormat()) {
  case VI_MPAL:
    if (!progressive) {
      mode = &GXMpal480IntDf;
    } else {
      mode = &GXNtsc480Prog;
    }
    break;
  case VI_NTSC:
    if (!progressive) {
      mode = &GXNtsc480IntDf;
    } else {
      mode = &GXNtsc480Prog;
    }
    break;
  case VI_PAL:
  case VI_EURGB60:
    rs_debugger_printf("PAL TV Format not compatible with NTSC build");
    break;
  }
  GXAdjustForOverscan(mode, &mRenderModeObj, 0, 16);
  mPixelAspectRatio = 1.f;
  sIs50Hz = false;
  mRenderModeObj.viWidth += 20;
  mRenderModeObj.viXOrigin -= 10;
  if (progressive) {
    SetProgressiveFilter(mRenderModeObj);
  }
#endif
  mpFrameBuf1 = nullptr;
  mpFrameBuf2 = nullptr;
  mpFifo = nullptr;
  sSpareAllocation = nullptr;
  GPUMemory::SetBuffer(nullptr, 0);
  ResetGraphicsArena();
  int frameBufferSize = ((mRenderModeObj.fbWidth + 15) & 0xfff0) * mRenderModeObj.xfbHeight * 2;
  mpFrameBuf1 = AllocateGraphicsArena(frameBufferSize);
  mpFrameBuf2 = AllocateGraphicsArena(frameBufferSize);
#if RSTL_VERSION == RSTL_G2MP
  sSpareAllocationSize = is60Hz ? 0x46000 : 0x52800;
  mFifoSize = is60Hz ? 0x60000 : 0x3D800;
  const int gpuBufferSize = is60Hz ? 0x40000 : 0x24000;
  mpFifo = AllocateGraphicsArena(mFifoSize);
  sSpareAllocation = AllocateGraphicsArena(sSpareAllocationSize);
  GPUMemory::SetBuffer(AllocateGraphicsArena(gpuBufferSize), gpuBufferSize);
#else
  sSpareAllocationSize = 0x46000;
  mFifoSize = 0x60000;
  mpFifo = AllocateGraphicsArena(mFifoSize);
  sSpareAllocation = AllocateGraphicsArena(sSpareAllocationSize);
  GPUMemory::SetBuffer(AllocateGraphicsArena(0x40000), 0x40000);
#endif
  mSpareBufferSize = sSpareAllocationSize;
  mpSpareBuffer = sSpareAllocation;
  if (!initial) {
    mRenderModeObj.viWidth += mScreenStretch * 2;
    mRenderModeObj.viXOrigin += mScreenPositionX - mScreenStretch;
    mRenderModeObj.viYOrigin += mScreenPositionY;
  }
  if (!initial) {
    VIWaitForRetrace();
    VIWaitForRetrace();
  }
  VIConfigure(&mRenderModeObj);
  VIFlush();
  if (initial) {
    mpFifoObj = GXInit(mpFifo, mFifoSize);
  }
  InitGraphicsFifo(mpFifoObj, mpFifo, mFifoSize);
  GXSetCopyFilter(mRenderModeObj.aa, mRenderModeObj.sample_pattern, GX_TRUE,
                  mRenderModeObj.vfilter);
}
#endif

GXTexRegion* CGraphics::TexRegionCallback(const GXTexObj* obj, GXTexMapID id) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static int nextTexRgn = 0;
  static int nextTexRgnCI = 0;
#else
  static signed char nextTexRgn = 0;
  static signed char nextTexRgnCI = 0;
  const GXTexFmt fmt = GXGetTexObjFmt(obj);
#endif

  if (id == GX_TEXMAP7) {
#if !RSTL_PRIME1
    if (fmt != GX_TF_C4 && fmt != GX_TF_C8 && fmt != GX_TF_C14X2) {
#endif

    return &mTexRegions[0];
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  } else {
    GXTexFmt fmt = GXGetTexObjFmt(obj);
#else
    }
    return &mTexRegionsCI[0];
  }
#endif

    if (fmt != GX_TF_C4 && fmt != GX_TF_C8 && fmt != GX_TF_C14X2) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      if (nextTexRgn == 0) {
        ++nextTexRgn;
#else
    do {
      nextTexRgn = (nextTexRgn + 1) & 7;
    } while (nextTexRgn == 0);
    return &mTexRegions[nextTexRgn];
#endif

      }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      return &mTexRegions[nextTexRgn++ & 7];
    } else {
      return &mTexRegionsCI[nextTexRgnCI++ & 3];
    }
  }
#else
  do {
    nextTexRgnCI = (nextTexRgnCI + 1) & 3;
  } while (nextTexRgnCI == 0);
  return &mTexRegionsCI[nextTexRgnCI];
#endif

}

void CGraphics::InitGraphicsVariables() {
  for (int i = 0; i < ARRAY_SIZE(mLightTypes); ++i) {
    mLightTypes[i] = kLT_Directional;
  }
  mLightActive = 0;
  SetDepthWriteMode(false, mDepthFunc, false);
  SetCullMode(kCM_None);
  SetAmbientColor(CColor(0.2f, 0.2f, 0.2f, 1.f));
  mIsGXModelMatrixIdentity = false;
  SetIdentityViewPointMatrix();
  SetIdentityModelMatrix();
  SetViewport(0, 0, mViewport.mWidth, mViewport.mHeight);
  SetPerspective(60.f,
                 static_cast< float >(mViewport.mWidth) / static_cast< float >(mViewport.mHeight),
                 mProj.GetNear(), mProj.GetFar());
  SetCopyClear(mClearColor, 1.f);
  const GXColor white = {0xFF, 0xFF, 0xFF, 0xFF};
  CGX::SetChanMatColor(CGX::Channel0, white);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  sRenderState.ResetFlushAll();
#else
  mRenderState.ResetFlushAll();
#endif

}

void CGraphics::Shutdown() {
  GXSetTexRegionCallback(mGXDefaultTexRegionCallback);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION == RSTL_GM8P_00
  CFrameDelayedKiller::StallAndFlushAllAllocations();
  mpFrameBuf1 = nullptr;
  mpFrameBuf2 = nullptr;
  mpFifo = nullptr;
  sSpareFrameBuffer = nullptr;
  CSkinnedModel::SetSkinningBuffer(nullptr, 0);
  sGraphicsArenaOffset = 0;
#endif
#else
  CFrameDelayedKiller::StallAndFlushAllAllocations();
  mpFrameBuf1 = nullptr;
  mpFrameBuf2 = nullptr;
  mpFifo = nullptr;
  sSpareAllocation = nullptr;
  GPUMemory::SetBuffer(nullptr, 0);
  ResetGraphicsArena();
#endif

}

void CGraphics::InitGraphicsDefaults() {
  SetDepthRange(0.f, 1.f);
  mIsGXModelMatrixIdentity = false;
  SetModelMatrix(mModelMatrix);
  SetViewPointMatrix(mViewMatrix);
  SetDepthWriteMode(false, mDepthFunc, false);
  SetCullMode(mCullMode);
  SetViewport(mViewport.mLeft, mViewport.mTop, mViewport.mWidth, mViewport.mHeight);
  FlushProjection();
  CTevCombiners::Init();
  DisableAllLights();
  SetDefaultVtxAttrFmt();
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION == RSTL_GM8P_00
void CGraphics::ConfigureFrameBuffer() {
#else
void CGraphics::ConfigureFrameBuffer(const COsContext& osContext) {
  mRenderModeObj = osContext.GetRenderModeObj();
  mpFrameBuf1 = osContext.GetFramebuf1();
  mpFrameBuf2 = osContext.GetFramebuf2();
#endif
#else
void CGraphics::ConfigureFrameBuffer() {
#endif

  VIConfigure(&mRenderModeObj);
  VISetNextFrameBuffer(mpFrameBuf1);
  mpCurrenFrameBuf = mpFrameBuf2;
  GXSetViewport(0.f, 0.f, static_cast< float >(mRenderModeObj.fbWidth),
                static_cast< float >(mRenderModeObj.efbHeight), 0.f, 1.f);
  GXSetScissor(0, 0, mRenderModeObj.fbWidth, mRenderModeObj.efbHeight);
  GXSetDispCopySrc(0, 0, mRenderModeObj.fbWidth, mRenderModeObj.efbHeight);
  GXSetDispCopyDst(mRenderModeObj.fbWidth, mRenderModeObj.efbHeight);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  GXSetDispCopyYScale(static_cast< float >(mRenderModeObj.xfbHeight) /
                      static_cast< float >(mRenderModeObj.efbHeight));
#else
  GXSetDispCopyYScale(static_cast< float >(mRenderModeObj.xfbHeight) / mRenderModeObj.efbHeight);
#endif

  GXSetCopyFilter(mRenderModeObj.aa, mRenderModeObj.sample_pattern, GX_ENABLE,
                  mRenderModeObj.vfilter);
  if (mRenderModeObj.aa) {
    GXSetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
  } else {
    GXSetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
  }
  GXSetDispCopyGamma(GX_GM_1_0);
  GXCopyDisp(mpCurrenFrameBuf, true);
  VIFlush();
  VIWaitForRetrace();
  VIWaitForRetrace();
  mViewport.mWidth = mRenderModeObj.fbWidth;
  mViewport.mHeight = mRenderModeObj.efbHeight;
  InitGraphicsDefaults();
}

void CGraphics::EnableLight(ERglLight light) {
  CGX::SetNumChans(1);
  int lightsWereOn = mLightActive;
  GXLightID lightId = static_cast< GXLightID >(1 << light);
  if ((lightsWereOn & lightId) == GX_LIGHT_NULL) {
    mLightActive |= lightId;
    CGX::SetChanCtrl(CGX::Channel0, true, GX_SRC_REG, GX_SRC_REG,
                     static_cast< GXLightID >((lightsWereOn | lightId) & (GX_MAX_LIGHT - 1)),
                     GX_DF_CLAMP, GX_AF_SPOT);
    ++mNumLightsActive;
  }
  mLightsWereOn = mLightActive;
}

static inline GXLightID get_hw_light_index(ERglLight light) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if NONMATCHING
  // one instruction, no branches
  return static_cast< GXLightID >((1u << light) & (GX_MAX_LIGHT - 1));
#else
  if (light == kLight0) {
    return GX_LIGHT0;
  } else if (light == kLight1) {
    return GX_LIGHT1;
  } else if (light == kLight2) {
    return GX_LIGHT2;
  } else if (light == kLight3) {
    return GX_LIGHT3;
  } else if (light == kLight4) {
    return GX_LIGHT4;
  } else if (light == kLight5) {
    return GX_LIGHT5;
  } else if (light == kLight6) {
    return GX_LIGHT6;
  }
  // wtf?
  return static_cast< GXLightID >(light == kLight7 ? GX_LIGHT7 : 0);
#endif
#else
  return static_cast< GXLightID >(1u << light);
#endif

}

void CGraphics::LoadLight(ERglLight light, const CLight& info) {
  GXLightID lightId = get_hw_light_index(light);
#if RSTL_PRIME1
  ELightType type = info.GetType();
#endif

  CVector3f pos = info.GetPosition();
  CVector3f dir = info.GetDirection();

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  switch (type) {
#else
  switch (info.GetType()) {
#endif

  case kLT_Spot: {
    MTXMultVec(mCameraMtx, reinterpret_cast< VecPtr >(&pos), reinterpret_cast< VecPtr >(&pos));
    GXLightObj* obj = &mLightObj[light];
    GXInitLightPos(obj, pos.GetX(), pos.GetY(), pos.GetZ());
    MTXMultVecSR(mCameraMtx, reinterpret_cast< VecPtr >(&dir), reinterpret_cast< VecPtr >(&dir));
    GXInitLightDir(obj, dir.GetX(), dir.GetY(), dir.GetZ());
    GXInitLightAttn(obj, 1.f, 0.f, 0.f, info.GetAttenuationConstant(), info.GetAttenuationLinear(),
                    info.GetAttenuationQuadratic());
    GXInitLightSpot(obj, info.GetSpotCutoff(), GX_SP_COS2);
    break;
  }
  case kLT_Point:
  case kLT_LocalAmbient: {
    MTXMultVec(mCameraMtx, reinterpret_cast< VecPtr >(&pos), reinterpret_cast< VecPtr >(&pos));
    GXInitLightPos(&mLightObj[light], pos.GetX(), pos.GetY(), pos.GetZ());
    GXInitLightAttn(&mLightObj[light], 1.f, 0.f, 0.f, info.GetAttenuationConstant(),
                    info.GetAttenuationLinear(), info.GetAttenuationQuadratic());
    break;
  }
  case kLT_Directional: {
    MTXMultVecSR(mCameraMtx, reinterpret_cast< VecPtr >(&dir), reinterpret_cast< VecPtr >(&dir));
    dir = -dir;
    GXInitLightPos(&mLightObj[light], dir.GetX() * 1048576.f, dir.GetY() * 1048576.f,
                   dir.GetZ() * 1048576.f);
    GXInitLightAttn(&mLightObj[light], 1.f, 0.f, 0.f, 1.f, 0.f, 0.f);
    break;
  }
  case kLT_Custom:
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  case kLT_Hard:
#endif
  {
    MTXMultVec(mCameraMtx, reinterpret_cast< VecPtr >(&pos), reinterpret_cast< VecPtr >(&pos));
    GXLightObj* obj = &mLightObj[light];
    GXInitLightPos(obj, pos.GetX(), pos.GetY(), pos.GetZ());
    MTXMultVecSR(mCameraMtx, reinterpret_cast< VecPtr >(&dir), reinterpret_cast< VecPtr >(&dir));
    GXInitLightDir(obj, dir.GetX(), dir.GetY(), dir.GetZ());
    GXInitLightAttn(obj, info.GetAngleAttenuationConstant(), info.GetAngleAttenuationLinear(),
                    info.GetAngleAttenuationQuadratic(), info.GetAttenuationConstant(),
                    info.GetAttenuationLinear(), info.GetAttenuationQuadratic());
    break;
  }
  default:
    break;
  }

  GXInitLightColor(&mLightObj[light], info.GetColor().GetGXColor());
  GXLoadLightObjImm(&mLightObj[light], lightId);
  mLightTypes[light] = info.GetType();
}

void CGraphics::DisableAllLights() {
  mNumLightsActive = 0;
  mLightActive = 0;
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
#if !RSTL_PRIME1
  CGX::SetChanCtrl(CGX::Channel1, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
#endif

}

// https://en.wikipedia.org/wiki/Hamming_weight
static inline uint popcount8(uint b) {
  b = (b & 0x55) + ((b & 0xAA) >> 1);
  b = (b & 0x33) + ((b & 0xCC) >> 2);
  return (static_cast< uchar >(b) & 0xF) + (static_cast< uchar >(b) >> 4);
}

void CGraphics::SetLightState(uchar lights) {
  GXAttnFn attnFn = GX_AF_NONE;
  if (lights != 0) {
    attnFn = GX_AF_SPOT;
  }
  GXDiffuseFn diffFn = GX_DF_NONE;
  if (lights != 0) {
    diffFn = GX_DF_CLAMP;
  }
  CGX::SetChanCtrl(CGX::Channel0, lights != 0 ? GX_ENABLE : GX_DISABLE, GX_SRC_REG,
                   (vtxDescr.mStreamFlags & 2) != 0 ? GX_SRC_VTX : GX_SRC_REG,

                   static_cast< GXLightID >(lights), diffFn, attnFn);
  mLightActive = lights;
  mNumLightsActive = popcount8(lights);
}

void CGraphics::SetViewMatrix() {
#if RSTL_PRIME1
  Mtx mtx;
  MTXTrans(mtx, -mViewPoint.GetX(), -mViewPoint.GetY(), -mViewPoint.GetZ());
  MTXConcat(mGXViewPointMatrix, mtx, mCameraMtx);
#endif

  if (mIsGXModelMatrixIdentity) {
    MTXCopy(mCameraMtx, mGxModelView);
  } else {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    MTXConcat(mCameraMtx, mGXModelMatrix, mGxModelView);
#else
    MTXConcat(mCameraMtx, mModelMatrix.GetCStyleMatrix(), mGxModelView);
#endif

  }
  GXLoadPosMtxImm(mGxModelView, GX_PNMTX0);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  Mtx nrmMtx;
  MTXInvXpose(mGxModelView, nrmMtx);
  GXLoadNrmMtxImm(nrmMtx, GX_PNMTX0);
#else
  if (mUseNormalMatrix) {
    MTXInvXpose(mGxModelView, mGxModelViewInvXpose);
    GXLoadNrmMtxImm(mGxModelViewInvXpose, GX_PNMTX0);
  }
#endif

}

void CGraphics::SetViewPointMatrix(const CTransform4f& xf) {
  mViewMatrix = xf;
  mGXViewPointMatrix[0][0] = xf.Get00();
  mGXViewPointMatrix[0][1] = xf.Get10();
  mGXViewPointMatrix[0][2] = xf.Get20();
  mGXViewPointMatrix[0][3] = 0.f;
  mGXViewPointMatrix[1][0] = xf.Get02();
  mGXViewPointMatrix[1][1] = xf.Get12();
  mGXViewPointMatrix[1][2] = xf.Get22();
  mGXViewPointMatrix[1][3] = 0.f;
  mGXViewPointMatrix[2][0] = -xf.Get01();
  mGXViewPointMatrix[2][1] = -xf.Get11();
  mGXViewPointMatrix[2][2] = -xf.Get21();
  mGXViewPointMatrix[2][3] = 0.f;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mViewPoint = xf.GetTranslation();
#else
  Mtx translation;
  const CVector3f viewPoint = xf.GetTranslation();
  MTXTrans(translation, -viewPoint.GetX(), -viewPoint.GetY(), -viewPoint.GetZ());
  MTXConcat(mGXViewPointMatrix, translation, mCameraMtx);
#endif

  SetViewMatrix();
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CGraphics::SetIdentityViewPointMatrix() {
  mViewMatrix = CTransform4f::Identity();
  MTXIdentity(mGXViewPointMatrix);
  mGXViewPointMatrix[2][2] = 0.f;
  mGXViewPointMatrix[1][1] = 0.f;
  mGXViewPointMatrix[1][2] = 1.f;
  mGXViewPointMatrix[2][1] = -1.f;
  mViewPoint = CVector3f::Zero();
  SetViewMatrix();
}
#else
void CGraphics::SetIdentityViewPointMatrix() { SetViewPointMatrix(CTransform4f::Identity()); }
#endif

void CGraphics::SetModelMatrix(const CTransform4f& xf) {
  if (&xf == &CTransform4f::Identity()) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    if (!mIsGXModelMatrixIdentity) {
      mModelMatrix = xf;
      mIsGXModelMatrixIdentity = true;
      SetViewMatrix();
    }
#else
    if (mIsGXModelMatrixIdentity) {
#endif

    return;
  }
#if !RSTL_PRIME1
    mIsGXModelMatrixIdentity = true;
  } else {
    mIsGXModelMatrixIdentity = false;
  }
#endif

  mModelMatrix = xf;
#if RSTL_PRIME1
  mIsGXModelMatrixIdentity = false;
  mGXModelMatrix[0][0] = xf.Get00();
  mGXModelMatrix[0][1] = xf.Get01();
  mGXModelMatrix[0][2] = xf.Get02();
  mGXModelMatrix[0][3] = xf.Get03();
  mGXModelMatrix[1][0] = xf.Get10();
  mGXModelMatrix[1][1] = xf.Get11();
  mGXModelMatrix[1][2] = xf.Get12();
  mGXModelMatrix[1][3] = xf.Get13();
  mGXModelMatrix[2][0] = xf.Get20();
  mGXModelMatrix[2][1] = xf.Get21();
  mGXModelMatrix[2][2] = xf.Get22();
  mGXModelMatrix[2][3] = xf.Get23();
#endif

  SetViewMatrix();
}

void CGraphics::SetIdentityModelMatrix() {
  if (!mIsGXModelMatrixIdentity) {
    mModelMatrix = CTransform4f::Identity();
    mIsGXModelMatrixIdentity = true;
    SetViewMatrix();
  }
}

void CGraphics::SetOrtho(float left, float right, float top, float bottom, float znear,
                         float zfar) {
  mProj = CProjectionState(false, left, right, top, bottom, znear, zfar);
  FlushProjection();
}

void CGraphics::SetPerspective(float fovy, float aspect, float znear, float zfar) {
  float t = tan(CRelAngle::FromDegrees(fovy).AsRadians() / 2.f);
  mProj = CProjectionState(true,                               // Is Projection
                           -(aspect * 2.f * znear * t * 0.5f), // Left
                           (aspect * 2.f * znear * t * 0.5f),  // Right
                           (znear * 2.f * t * 0.5f),           // Top
                           -(znear * 2.f * t * 0.5f),          // Bottom
                           znear, zfar);
  FlushProjection();
}

CMatrix4f CGraphics::GetPerspectiveProjectionMatrix() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if NONMATCHING
  // construct in place
  return CMatrix4f(
#else
  CMatrix4f mtx(
#endif
#else
  // construct in place
  return CMatrix4f(
#endif

      // clang-format off
    (mProj.GetNear() * 2.f) / (mProj.GetRight() - mProj.GetLeft()),
    -(mProj.GetRight() + mProj.GetLeft()) / (mProj.GetRight() - mProj.GetLeft()),
    0.f,
    0.f,
    0.f,
    -(mProj.GetTop() + mProj.GetBottom()) / (mProj.GetTop() - mProj.GetBottom()),
    (mProj.GetNear() * 2.f) / (mProj.GetTop() - mProj.GetBottom()),
    0.f,
    0.f,
    (mProj.GetFar() + mProj.GetNear()) / (mProj.GetFar() - mProj.GetNear()),
    0.f,
    -(mProj.GetFar() * 2.f * mProj.GetNear()) / (mProj.GetFar() - mProj.GetNear()),
    0.f,
    1.f,
    0.f,
    0.f
      // clang-format on
  );
#if RSTL_PRIME1
#if !NONMATCHING
  return mtx;
#endif
#endif

}

CMatrix4f CGraphics::CalculatePerspectiveMatrix(float fovy, float aspect, float znear, float zfar) {
  float t = tan(CRelAngle::FromDegrees(fovy).AsRadians() / 2.f);
#if !RSTL_PRIME1
  float depth = zfar - znear;
  float twoNear = 2.f * znear;
#endif

  float right = aspect * 2.f * znear * t * 0.5f;
#if !RSTL_PRIME1
  float top = twoNear * t * 0.5f;
#endif

  float left = -right;
#if RSTL_PRIME1
  float top = znear * 2.f * t * 0.5f;
#endif

  float bottom = -top;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if NONMATCHING
  // construct in place
  return CMatrix4f(
#else
  CMatrix4f mtx(
#endif
#else
  // construct in place
  return CMatrix4f(
#endif

      // clang-format off
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    (2.f * znear) / (right - left),
#else
    twoNear / (right - left),
#endif

    -(right + left) / (right - left),
    0.f,
    0.f,
    0.f,
    -(top + bottom) / (top - bottom),
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    (2.f * znear) / (top - bottom),
#else
    twoNear / (top - bottom),
#endif

    0.f,
    0.f,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    (zfar + znear) / (zfar - znear),
#else
    (zfar + znear) / depth,
#endif

    0.f,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    -(2.f * zfar * znear) / (zfar - znear),
#else
    -(2.f * zfar * znear) / depth,
#endif

    0.f,
    1.f,
    0.f,
    0.f
      // clang-format on
  );
#if RSTL_PRIME1
#if !NONMATCHING
  return mtx;
#endif
#endif

}

void CGraphics::SetViewport(int left, int bottom, int width, int height) {
  mViewport.mLeft = left;
  mViewport.mTop = mRenderModeObj.efbHeight - (bottom + height);
  mViewport.mWidth = width;
  mViewport.mHeight = height;
  mViewport.mHalfWidth = static_cast< float >(width / 2);
  mViewport.mHalfHeight = static_cast< float >(height / 2);
  GXSetViewport(static_cast< float >(mViewport.mLeft), static_cast< float >(mViewport.mTop),
                static_cast< float >(mViewport.mWidth), static_cast< float >(mViewport.mHeight),
                mDepthNear, mDepthFar);
#if !RSTL_PRIME1
}

int CGraphics::GetViewportTop(int bottom) {
  return mRenderModeObj.efbHeight - (bottom + mViewport.mHeight);
#endif

}

void CGraphics::SetScissor(int left, int bottom, int width, int height) {
  GXSetScissor(left, mRenderModeObj.efbHeight - (bottom + height), width, height);
}

void CGraphics::SetAmbientColor(const CColor& color) {
  CGX::SetChanAmbColor(CGX::Channel0, color.GetGXColor());
  CGX::SetChanAmbColor(CGX::Channel1, color.GetGXColor());
}

void CGraphics::SetCopyClear(const CColor& color, float depth) {
  mClearColor = color;
  mClearDepthValue = static_cast< u32 >(depth * GX_MAX_Z24);
  GXSetCopyClear(color.GetGXColor(), mClearDepthValue);
}

void CGraphics::SetClearColor(const CColor& color) {
  mClearColor = color;
  GXSetCopyClear(mClearColor.GetGXColor(), mClearDepthValue);
}

#if !RSTL_PRIME1
CColor CGraphics::GetClearColor() { return mClearColor; }
#endif

void CGraphics::ClearBackAndDepthBuffers() {
  GXInvalidateTexAll();
  if (mRenderModeObj.field_rendering) {
    GXSetViewportJitter(0.f, 0.f, static_cast< float >(mRenderModeObj.fbWidth),
                        static_cast< float >(mRenderModeObj.xfbHeight), 0.f, 1.f, VIGetNextField());
  } else {
    GXSetViewport(0.f, 0.f, static_cast< float >(mRenderModeObj.fbWidth),
                  static_cast< float >(mRenderModeObj.xfbHeight), 0.f, 1.f);
  }
  GXInvalidateVtxCache();
}

#if !RSTL_PRIME1
static const GXVtxDescList skPosColorTexDirect[] = {
    {GX_VA_POS, GX_DIRECT},
    {GX_VA_CLR0, GX_DIRECT},
    {GX_VA_TEX0, GX_DIRECT},
    {GX_VA_NULL, GX_NONE},
};
#endif

void CGraphics::BeginScene() { ClearBackAndDepthBuffers(); }

void CGraphics::SwapBuffers() {
  GXDisableBreakPt();
  mFlippingState = 1;
}

void CGraphics::VideoPreCallback(u32 retraceCount) {
  if (mNumBreakpointsWaiting != 0 && mFlippingState == 1) {
    if (mFirstFrame) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      VISetBlack(GX_FALSE);
#else
      VISetBlack(FALSE);
#endif

      mFirstFrame = false;
    }
    VISetNextFrameBuffer(mpCurrenFrameBuf);
    VIFlush();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    void* frameBuf = mpCurrenFrameBuf == mpFrameBuf1 ? mpFrameBuf2 : mpFrameBuf1;
#else
    mpCurrenFrameBuf = mpCurrenFrameBuf == mpFrameBuf1 ? mpFrameBuf2 : mpFrameBuf1;
#endif

    mFlippingState = 2;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    mpCurrenFrameBuf = frameBuf;
#else
  }

  static int stalledRetraces = 0;
  static u32 lastOverflowCount = 0;
  GXBool overHighWaterMark;
  GXBool ignored;
  GXGetGPStatus(&overHighWaterMark, &ignored, &ignored, &ignored, &ignored);
  const u32 overflowCount = GXGetOverflowCount();
  if (overHighWaterMark && lastOverflowCount == overflowCount) {
    if (++stalledRetraces >= 10) {
      GXAbortFrame();
      OSResumeThread(GXGetCurrentGXThread());
      GXFifoObj fifo;
      GXInitFifoBase(&fifo, mpFifo, mFifoSize);
      GXSetCPUFifo(&fifo);
      GXSetGPFifo(&fifo);
      GXSetCPUFifo(mpFifoObj);
      GXSetGPFifo(mpFifoObj);
      sGXAborted = true;
      stalledRetraces = 0;
    }
  } else {
    stalledRetraces = 0;
    lastOverflowCount = overflowCount;
#endif

  }
}

void CGraphics::VideoPostCallback(u32 retraceCount) {
  if (mNumBreakpointsWaiting != 0) {
    if (mFlippingState == 2) {
      --mNumBreakpointsWaiting;
      mFlippingState = 0;
      CStopwatch& timer = sFPSTimer;
      float elapsed = timer.GetElapsedTime();
      mLastFramesPerSecond = mFramesPerSecond;
      mFramesPerSecond = 1.f / elapsed;
      timer.Reset();
      mInterruptLastFrameUsedAbove = VIGetNextField() == 1;
    }
  }
}

void CGraphics::EndScene() {
#if RSTL_PRIME1
  CGX::SetZMode(true, GX_LEQUAL, true);
#endif

  volatile int& numBreakPt = const_cast< volatile int& >(mNumBreakpointsWaiting);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  while (numBreakPt > 0) {
#else
  volatile bool& aborted = const_cast< volatile bool& >(sGXAborted);
  const OSTime start = OSGetTime();
  if (sGXAborted) {
    GXAbortFrame();
  } else {
    CStopwatch waitTimer;
    while (numBreakPt > 0 && !aborted) {
#endif

    OSYieldThread();
#if !RSTL_PRIME1
      if (OSTicksToMilliseconds(OSGetTime() - start) > 250) {
        const bool interrupts = OSDisableInterrupts();
        GXAbortFrame();
        aborted = true;
        OSRestoreInterrupts(interrupts);
      }
    }
    const float elapsedMs = static_cast< float >(waitTimer.GetElapsedMicros() / 1000);
    const float framePeriod = sIs50Hz ? 20.f : (50.f / 3.f);
    sPreviousFrameWaitFraction = sFrameWaitFraction;
    sFrameWaitFraction = (framePeriod - elapsedMs) / framePeriod;
  }
  if (aborted) {
    mNumBreakpointsWaiting = 0;
    aborted = false;
    mpFifoObj = GXInit(mpFifo, mFifoSize);
    InitGraphicsFifo(mpFifoObj, mpFifo, mFifoSize);
    SetDefaultVtxAttrFmt();
    CGX::ResetGXStatesFull();
    InitGraphicsVariables();
#endif

  }
  ++mNumBreakpointsWaiting;
#if !RSTL_PRIME1
  // The original declares a 3x4 matrix here; MTXOrtho writes all four rows past its end.
  Mtx projection;
  MTXOrtho(projection, mViewport.mHeight / 2, -mViewport.mHeight / 2, -mViewport.mWidth / 2,
           mViewport.mWidth / 2, -1.f, -10.f);
  GXSetProjection(projection, GX_ORTHOGRAPHIC);
  Mtx model;
  MTXIdentity(model);
  GXLoadPosMtxImm(model, GX_PNMTX0);
  CGX::SetZMode(true, GX_LEQUAL, false);
  CGX::SetVtxDescv(skPosColorTexDirect);
  CGX::Begin(GX_TRIANGLES, GX_VTXFMT0, 288);
  for (int i = 96; i != 0; --i) {
    GXPosition3f32(0.f, 1.f, 1.f);
    GXColor1u32(0);
    GXTexCoord2f32(0.f, 0.f);
    GXPosition3f32(0.f, 0.f, 1.f);
    GXColor1u32(0);
    GXTexCoord2f32(0.f, 0.f);
    GXPosition3f32(1.f, 0.f, 1.f);
    GXColor1u32(0);
    GXTexCoord2f32(0.f, 0.f);
  }
  CGX::End();
  CGX::SetZMode(true, GX_LEQUAL, true);
#endif

  void*& frameBuf = mpCurrenFrameBuf;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  float brightness = CMath::Clamp(0.f, mBrightness, 2.f);
  static const u8 copyFilter[7] = {0x00, 0x00, 0x15, 0x16, 0x15, 0x00, 0x00};
  const u8* inFilter = mUseVideoFilter ? mRenderModeObj.vfilter : copyFilter;
  u8 vfilter[7];
  for (int i = 0; i < 7; i++) {
    vfilter[i] = static_cast< u8 >(static_cast< float >(inFilter[i]) * brightness);
#else
  const float brightness = CMath::Clamp(0.f, mBrightness, 2.f);
  static const u8 unfiltered[7] = {0, 0, 21, 22, 21, 0, 0};
  const u8* filter = mUseVideoFilter ? mRenderModeObj.vfilter : unfiltered;
  u8 adjustedFilter[7];
  for (int i = 0; i < 7; ++i) {
    adjustedFilter[i] = static_cast< u8 >(brightness * filter[i]);
#endif

  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  GXSetCopyFilter(mRenderModeObj.aa, mRenderModeObj.sample_pattern, true, vfilter);
#else
  GXSetCopyFilter(mRenderModeObj.aa, mRenderModeObj.sample_pattern, GX_TRUE, adjustedFilter);
#endif

  GXCopyDisp(frameBuf, mIsBeginSceneClearFb ? GX_TRUE : GX_FALSE);
  GXSetCopyFilter(mRenderModeObj.aa, mRenderModeObj.sample_pattern,
                  mUseVideoFilter ? GX_ENABLE : GX_DISABLE, mRenderModeObj.vfilter);
  GXSetBreakPtCallback(SwapBuffers);
  VISetPreRetraceCallback(VideoPreCallback);
  VISetPostRetraceCallback(VideoPostCallback);
  GXFlush();
#if RSTL_PRIME1
  GXFifoObj* fifo = GXGetGPFifo();
#endif

  void* readPtr;
  void* writePtr;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  GXGetFifoPtrs(fifo, &readPtr, &writePtr);
#else
#if RSTL_VERSION >= RSTL_R32J
  GXFifoObj gpFifo;
  GXGetGPFifo(&gpFifo);
  GXGetFifoPtrs(&gpFifo, &readPtr, &writePtr);
#else
  GXGetFifoPtrs(GXGetGPFifo(), &readPtr, &writePtr);
#endif
#endif

  GXEnableBreakPt(writePtr);
  mLastFrameUsedAbove = mInterruptLastFrameUsedAbove;
  ++mFrameCounter;
  CFrameDelayedKiller::FlushAllocationsForFrame();
}

void CGraphics::SetDepthWriteMode(const bool test, ERglEnum comp, const bool write) {
  mDepthFunc = comp;
  CGX::SetZMode(test, static_cast< GXCompare >(comp), write);
}

void CGraphics::SetCullMode(ERglCullMode cullMode) {
  mCullMode = cullMode;
  GXSetCullMode(static_cast< GXCullMode >(cullMode));
}

void CGraphics::SetBlendMode(ERglBlendMode mode, ERglBlendFactor src, ERglBlendFactor dst,
                             ERglLogicOp op) {
  CGX::SetBlendMode(static_cast< GXBlendMode >(mode), static_cast< GXBlendFactor >(src),
                    static_cast< GXBlendFactor >(dst), static_cast< GXLogicOp >(op));
}

void CGraphics::SetAlphaCompare(ERglAlphaFunc comp0, uchar ref0, ERglAlphaOp op,
                                ERglAlphaFunc comp1, uchar ref1) {
  CGX::SetAlphaCompare(static_cast< GXCompare >(comp0), static_cast< uchar >(ref0),
                       static_cast< GXAlphaOp >(op), static_cast< GXCompare >(comp1),
                       static_cast< uchar >(ref1));
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
static const GXVtxDescList skPosColorTexDirect[] = {
    {GX_VA_POS, GX_DIRECT},
    {GX_VA_CLR0, GX_DIRECT},
    {GX_VA_TEX0, GX_DIRECT},
    {GX_VA_NULL, GX_NONE},
};
#else
;
#endif

void CGraphics::Render2D(const CTexture& tex, int x, int y, int w, int h, const CColor& col) {
#if !RSTL_PRIME1
  Render2D(&tex, x, y, w, h, col);
}

void CGraphics::Render2D(const CTexture* tex, int x, int y, int w, int h, const CColor& col) {
#endif

  Mtx44 proj;
  MTXOrtho(proj, mViewport.mHeight / 2, -mViewport.mHeight / 2, -mViewport.mWidth / 2,
           mViewport.mWidth / 2, -1.f, -10.f);
  GXSetProjection(proj, GX_ORTHOGRAPHIC);
  uint c = col.GetColor_u32();

  Mtx mtx;
  MTXIdentity(mtx);
  GXLoadPosMtxImm(mtx, GX_PNMTX0);

  float x2, y2, x1, y1;
  x1 = x - mViewport.mWidth / 2;
  y1 = y - mViewport.mHeight / 2;
  x2 = x1 + w;
  y2 = y1 + h;

  // Save state + setup
  CGX::SetVtxDescv(skPosColorTexDirect);
#if RSTL_PRIME1
  SetTevStates(6);
#endif

  mLightsWereOn = mLightActive;
  if (mLightActive != 0) {
    DisableAllLights();
  }
  ERglCullMode cullMode = mCullMode;
  SetCullMode(kCM_None);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  tex.Load(GX_TEXMAP0, CTexture::kCM_Repeat);
#else
  if (tex != nullptr) {
    SetTevStates(6);
    tex->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  } else {
    SetTevStates(2);
    SetTevOp(kTS_Stage0, kEnvPassthru);
  }
#endif

  // Draw
  CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
  GXPosition3f32(x1, y1, 1.f);
  GXColor1u32(c);
  GXTexCoord2f32(0.f, 0.f);
  GXPosition3f32(x2, y1, 1.f);
  GXColor1u32(c);
  GXTexCoord2f32(1.f, 0.f);
  GXPosition3f32(x1, y2, 1.f);
  GXColor1u32(c);
  GXTexCoord2f32(0.f, 1.f);
  GXPosition3f32(x2, y2, 1.f);
  GXColor1u32(c);
  GXTexCoord2f32(1.f, 1.f);
  CGX::End();

  // Restore state
  if (mLightsWereOn != 0) {
    SetLightState(mLightsWereOn);
  }
  FlushProjection();
  mIsGXModelMatrixIdentity = false;
  SetModelMatrix(mModelMatrix);
  SetCullMode(cullMode);
}

bool CGraphics::BeginRender2D(const CTexture& tex) {
  Mtx44 proj;
  MTXOrtho(proj, mViewport.mHeight / 2, -mViewport.mHeight / 2, -mViewport.mWidth / 2,
           mViewport.mWidth / 2, -1.f, -10.f);
  GXSetProjection(proj, GX_ORTHOGRAPHIC);
  Mtx mtx;
  MTXIdentity(mtx);
  GXLoadPosMtxImm(mtx, GX_PNMTX0);
  mLightsWereOn = mLightActive;
  DisableAllLights();
  CGX::SetVtxDescv(skPosColorTexDirect);
  SetTevStates(6);
  SetTevOp(kTS_Stage0, kEnvModulate);
  tex.Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  return mLightsWereOn != 0;
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CGraphics::DoRender2D(const CTexture& tex, int x, int y, int srcX, int srcY,
                           int srcW, int srcH, int w, int h, const CColor& col) {
  const float u0 = float(srcX) / tex.GetWidth();
  const float u1 = float(srcX + srcW) / tex.GetWidth();
  const float v0 = float(srcY) / tex.GetHeight();
  const float v1 = float(srcY + srcH) / tex.GetHeight();
  const float x0 = x - mViewport.mWidth / 2;
  const float y0 = y - mViewport.mHeight / 2;
  const ERglCullMode cullMode = mCullMode;
  SetCullMode(kCM_None);
  CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
  GXPosition3f32(x0, y0, 1.f);
  GXColor1u32(col.GetColor_u32());
  GXTexCoord2f32(u0, v1);
  GXPosition3f32(x0 + w, y0, 1.f);
  GXColor1u32(col.GetColor_u32());
  GXTexCoord2f32(u1, v1);
  GXPosition3f32(x0, y0 + h, 1.f);
  GXColor1u32(col.GetColor_u32());
  GXTexCoord2f32(u0, v0);
  GXPosition3f32(x0 + w, y0 + h, 1.f);
  GXColor1u32(col.GetColor_u32());
  GXTexCoord2f32(u1, v0);
  CGX::End();
  SetCullMode(cullMode);
}

void CGraphics::EndRender2D(bool lightsWereOn) {
  if (lightsWereOn) {
#else
void CGraphics::EndRender2D(bool restoreLights) {
  if (restoreLights) {
#endif

    SetLightState(mLightsWereOn);
  }
  FlushProjection();
  mIsGXModelMatrixIdentity = false;
  SetModelMatrix(mModelMatrix);
#if !RSTL_PRIME1
}

void CGraphics::DoRender2D(const CTexture& tex, int x, int y, int texX, int texY, int texW,
                           int texH, int w, int h, const CColor& col) {
  const float width = tex.GetWidth();
  const float height = tex.GetHeight();
  const float s0 = texX / width;
  const float s1 = (texX + texW) / width;
  const float t0 = texY / height;
  const float t1 = (texY + texH) / height;
  const float x1 = x - mViewport.mWidth / 2;
  const float y1 = y - mViewport.mHeight / 2;
  const float x2 = x1 + w;
  const float y2 = y1 + h;

  CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
  GXPosition3f32(x1, y1, 1.f);
  GXColor1u32(col.GetColor_u32());
  GXTexCoord2f32(s0, t1);
  GXPosition3f32(x2, y1, 1.f);
  GXColor1u32(col.GetColor_u32());
  GXTexCoord2f32(s1, t1);
  GXPosition3f32(x1, y2, 1.f);
  GXColor1u32(col.GetColor_u32());
  GXTexCoord2f32(s0, t0);
  GXPosition3f32(x2, y2, 1.f);
  GXColor1u32(col.GetColor_u32());
  GXTexCoord2f32(s1, t0);
  CGX::End();
#endif

}

void CGraphics::DrawPrimitive(ERglPrimitive primitive, const float* pos, const CVector3f& normal,
                              const CColor& col, int numVerts) {
  StreamBegin(primitive);
  StreamNormal(reinterpret_cast< const float* >(&normal));
  StreamColor(col);
  for (int i = 0; i < numVerts; ++i) {
    StreamVertex(pos + i * 3);
  }
  StreamEnd();
}

#define STREAM_PRIM_BUFFER_SIZE 240

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#define VTX_BUFFER_ADDR static_cast< uchar* >(LCGetBase())
#if NONMATCHING
// Bug fix: these should be 3 times larger to avoid overflow
// Likely the result of bad pointer arithmetic
#define NRM_BUFFER_ADDR (VTX_BUFFER_ADDR + ((STREAM_PRIM_BUFFER_SIZE + 1) * sizeof(Vec)))
#define TXT0_BUFFER_ADDR (NRM_BUFFER_ADDR + ((STREAM_PRIM_BUFFER_SIZE + 1) * sizeof(Vec)))
#else
#define NRM_BUFFER_ADDR (VTX_BUFFER_ADDR + ((STREAM_PRIM_BUFFER_SIZE + 1) * sizeof(float)))
#define TXT0_BUFFER_ADDR (NRM_BUFFER_ADDR + ((STREAM_PRIM_BUFFER_SIZE + 1) * sizeof(float)))
#endif
#define TXT1_BUFFER_ADDR (TXT0_BUFFER_ADDR + ((STREAM_PRIM_BUFFER_SIZE + 1) * sizeof(Vec2)))
#define CLR_BUFFER_ADDR (TXT1_BUFFER_ADDR + ((STREAM_PRIM_BUFFER_SIZE + 1) * sizeof(Vec2)))
#else
#define NRM_BUFFER_ADDR(base) ((base) + ((STREAM_PRIM_BUFFER_SIZE + 1) * sizeof(float)))
#define TXT0_BUFFER_ADDR(base)                                                                     \
  (NRM_BUFFER_ADDR(base) + ((STREAM_PRIM_BUFFER_SIZE + 1) * sizeof(float)))
#define TXT1_BUFFER_ADDR(base)                                                                     \
  (TXT0_BUFFER_ADDR(base) + ((STREAM_PRIM_BUFFER_SIZE + 1) * sizeof(Vec2)))
#define CLR_BUFFER_ADDR(base)                                                                      \
  (TXT1_BUFFER_ADDR(base) + ((STREAM_PRIM_BUFFER_SIZE + 1) * sizeof(Vec2)))
#endif

static const uchar kHasNormals = 1;
static const uchar kHasColor = 2;
static const uchar kHasTexture = 4;

void CGraphics::StreamBegin(ERglPrimitive primitive) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  vtxBuffer = reinterpret_cast< VecPtr >(VTX_BUFFER_ADDR);
  nrmBuffer = reinterpret_cast< VecPtr >(NRM_BUFFER_ADDR);
  txtBuffer0 = reinterpret_cast< Vec2Ptr >(TXT0_BUFFER_ADDR);
  txtBuffer1 = reinterpret_cast< Vec2Ptr >(TXT1_BUFFER_ADDR);
  clrBuffer = reinterpret_cast< uint* >(CLR_BUFFER_ADDR);
#else
  uchar* base = static_cast< uchar* >(GetLockedCacheAllocationBase());
  mVertexBuffer = reinterpret_cast< VecPtr >(base);
  mNormalBuffer = reinterpret_cast< VecPtr >(NRM_BUFFER_ADDR(base));
  mTexCoordBuffer0 = reinterpret_cast< Vec2Ptr >(TXT0_BUFFER_ADDR(base));
  mTexCoordBuffer1 = reinterpret_cast< Vec2Ptr >(TXT1_BUFFER_ADDR(base));
  mColorBuffer = reinterpret_cast< uint* >(CLR_BUFFER_ADDR(base));
#endif

  ResetVertexDataStream(true);
  mCurrentPrimitive = primitive;
  vtxDescr.mStreamFlags = kHasColor;

}

void CGraphics::StreamVertex(float x, float y, float z) {
  vtxDescr.mPosition.x = x;
  vtxDescr.mPosition.y = y;
  vtxDescr.mPosition.z = z;

  UpdateVertexDataStream();
}

void CGraphics::StreamVertex(const float* vtx) {
  vtxDescr.mPosition.x = vtx[0];
  vtxDescr.mPosition.y = vtx[1];
  vtxDescr.mPosition.z = vtx[2];

  UpdateVertexDataStream();
}

void CGraphics::StreamVertex(const CVector3f& vtx) {
  vtxDescr.mPosition.x = vtx.GetX();
  vtxDescr.mPosition.y = vtx.GetY();
  vtxDescr.mPosition.z = vtx.GetZ();

  UpdateVertexDataStream();
}

void CGraphics::StreamNormal(const float* nrm) {
  vtxDescr.mNormal.x = nrm[0];
  vtxDescr.mNormal.y = nrm[1];
  vtxDescr.mNormal.z = nrm[2];
  vtxDescr.mStreamFlags |= kHasNormals;

}

void CGraphics::StreamColor(uint color) {
  vtxDescr.mColor = color;
  vtxDescr.mStreamFlags |= kHasColor;

}

void CGraphics::StreamColor(const CColor& color) {
  vtxDescr.mColor = color.GetColor_u32();
  vtxDescr.mStreamFlags |= kHasColor;

}

void CGraphics::StreamColor(float r, float g, float b, float a) {
  // clang-format off
  vtxDescr.mColor = (static_cast< uchar >(r * 255.f) << 24) |

                   (static_cast< uchar >(g * 255.f) << 16) |
                   (static_cast< uchar >(b * 255.f) << 8) |
                   static_cast< uchar >(a * 255.f);
  // clang-format on
  vtxDescr.mStreamFlags |= kHasColor;

}

void CGraphics::StreamTexcoord(const CVector2f& uv) {
  vtxDescr.mTexCoord0.x = uv.GetX();
  vtxDescr.mTexCoord0.y = uv.GetY();
  vtxDescr.mStreamFlags |= kHasTexture;
  vtxDescr.mTextureUsed |= 1;

}

void CGraphics::StreamTexcoord(float u, float v) {
  vtxDescr.mTexCoord0.x = u;
  vtxDescr.mTexCoord0.y = v;
  vtxDescr.mStreamFlags |= kHasTexture;
  vtxDescr.mTextureUsed |= 1;

}

void CGraphics::StreamEnd() {
  if (mNumPrimitives != 0) {
    FlushStream();
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  vtxBuffer = nullptr;
  vtxDescr.mStreamFlags = 0;
  vtxDescr.mTextureUsed = 0;
  nrmBuffer = nullptr;
  txtBuffer0 = nullptr;
  txtBuffer1 = nullptr;
  clrBuffer = nullptr;
#else
  mVertexBuffer = nullptr;
  vtxDescr.mStreamFlags = 0;
  vtxDescr.mTextureUsed = 0;
  mNormalBuffer = nullptr;
  mTexCoordBuffer0 = nullptr;
  mTexCoordBuffer1 = nullptr;
  mColorBuffer = nullptr;
#endif

}

void CGraphics::SetLineWidth(float w, ERglTexOffset offs) {
  CGX::SetLineWidth(static_cast< uchar >(w * 6.f), static_cast< GXTexOffset >(offs));
}

void CGraphics::UpdateVertexDataStream() {
  ++mNumPrimitives;
  mpVtxBuffer->x = vtxDescr.mPosition.x;
  mpVtxBuffer->y = vtxDescr.mPosition.y;
  mpVtxBuffer->z = vtxDescr.mPosition.z;

  ++mpVtxBuffer;
  if ((vtxDescr.mStreamFlags & kHasNormals) != 0) {
    mpNrmBuffer->x = vtxDescr.mNormal.x;
    mpNrmBuffer->y = vtxDescr.mNormal.y;
    mpNrmBuffer->z = vtxDescr.mNormal.z;

    ++mpNrmBuffer;
  }
  if ((vtxDescr.mStreamFlags & kHasTexture) != 0) {
    mpTxtBuffer0->x = vtxDescr.mTexCoord0.x;
    mpTxtBuffer0->y = vtxDescr.mTexCoord0.y;

    ++mpTxtBuffer0;

    mpTxtBuffer1->x = vtxDescr.mTexCoord1.x;
    mpTxtBuffer1->y = vtxDescr.mTexCoord1.y;

    ++mpTxtBuffer1;
  }
  if ((vtxDescr.mStreamFlags & kHasColor) != 0) {
    *mpClrBuffer = vtxDescr.mColor;

    ++mpClrBuffer;
  }
  mJustReset = 0;
  if (mNumPrimitives == STREAM_PRIM_BUFFER_SIZE) {
    FlushStream();
    ResetVertexDataStream(false);
  }
}

void CGraphics::ResetVertexDataStream(bool initial) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mpVtxBuffer = vtxBuffer;
  mpNrmBuffer = nrmBuffer;
  mpTxtBuffer0 = txtBuffer0;
  mpTxtBuffer1 = txtBuffer1;
  mpClrBuffer = clrBuffer;
#else
  mpVtxBuffer = mVertexBuffer;
  mpNrmBuffer = mNormalBuffer;
  mpTxtBuffer0 = mTexCoordBuffer0;
  mpTxtBuffer1 = mTexCoordBuffer1;
  mpClrBuffer = mColorBuffer;
#endif

  mNumPrimitives = 0;

  if (initial) {
    return;
  }

  switch (mCurrentPrimitive) {
  case kP_TriangleFan:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    mpVtxBuffer = vtxBuffer + 1;
#else
    mpVtxBuffer = mVertexBuffer + 1;
#endif
    memcpy(mpVtxBuffer, &vtxDescr.mPosition, sizeof(Vec));

    ++mpVtxBuffer;

    if ((vtxDescr.mStreamFlags & kHasNormals) != 0) {

      ++mpNrmBuffer;
      memcpy(mpNrmBuffer, &vtxDescr.mNormal, sizeof(Vec));

      ++mpNrmBuffer;
    }

    if ((vtxDescr.mStreamFlags & kHasTexture) != 0) {

      ++mpTxtBuffer0;
      memcpy(mpTxtBuffer0, &vtxDescr.mTexCoord0, sizeof(Vec2));

      ++mpTxtBuffer0;

      ++mpTxtBuffer1;
      memcpy(mpTxtBuffer1, &vtxDescr.mTexCoord1, sizeof(Vec2));

      ++mpTxtBuffer1;
    }

    if ((vtxDescr.mStreamFlags & kHasColor) != 0) {

      ++mpClrBuffer;
      *mpClrBuffer = vtxDescr.mColor;

      ++mpClrBuffer;
    }

    mNumPrimitives += 2;
    break;

  default:
    break;
  }

  mJustReset = 1;
}

void CGraphics::FlushStream() {
  GXVtxDescList descriptors[10];
  GXVtxDescList* next = descriptors;
  const GXVtxDescList position = {GX_VA_POS, GX_DIRECT};
  *next++ = position;
  if ((vtxDescr.mStreamFlags & kHasNormals) != 0) {
    const GXVtxDescList normal = {GX_VA_NRM, GX_DIRECT};
    *next++ = normal;

  }
  if ((vtxDescr.mStreamFlags & kHasColor) != 0) {
    const GXVtxDescList color = {GX_VA_CLR0, GX_DIRECT};
    *next++ = color;

  }
  if ((vtxDescr.mStreamFlags & kHasTexture) != 0) {
    const GXVtxDescList texture = {GX_VA_TEX0, GX_DIRECT};
    *next++ = texture;

  }
  const GXVtxDescList end = {GX_VA_NULL, GX_NONE};
  *next = end;
  CGX::SetVtxDescv(descriptors);
  SetTevStates(vtxDescr.mStreamFlags);
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  if (sUseStreamVertexDelay) {
    FullRenderWithVertexDelay();
  } else {
#endif

  FullRender();
#if !RSTL_PRIME1
  }
#endif

}

void CGraphics::SetTevStates(uchar flags) {
  switch (flags) {
  case 0:
  case kHasNormals:
  case kHasColor:
  case kHasNormals | kHasColor:
    CGX::SetNumChans(1);
    CGX::SetNumTexGens(0);
    CGX::SetNumTevStages(1);
    CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    break;
  case kHasTexture:
  case kHasNormals | kHasTexture:
  case kHasColor | kHasTexture:
  case kHasNormals | kHasColor | kHasTexture:
    CGX::SetNumChans(1);
    if ((vtxDescr.mTextureUsed & 3) != 0) {

      CGX::SetNumTexGens(2);
    } else {
      CGX::SetNumTexGens(1);
    }
    CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0);
    break;
  }
  CGX::SetNumIndStages(0);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY, false, GX_PTIDENTITY);

  uint light = mLightActive;
  GXAttnFn attnFn = GX_AF_NONE;
  if (light != 0) {
    attnFn = GX_AF_SPOT;
  }
  GXDiffuseFn diffFn = GX_DF_NONE;
  if (light != 0) {
    diffFn = GX_DF_CLAMP;
  }
  CGX::SetChanCtrl(CGX::Channel0, light ? GX_ENABLE : GX_DISABLE, GX_SRC_REG,
                   (flags & kHasColor) ? GX_SRC_VTX : GX_SRC_REG, static_cast< GXLightID >(light),
                   diffFn, attnFn);
}

#if !RSTL_PRIME1
void CGraphics::FullRenderWithVertexDelay() {
  CGX::Begin(static_cast< GXPrimitive >(mCurrentPrimitive), GX_VTXFMT0, mNumPrimitives);
  switch (vtxDescr.mStreamFlags) {
  case 0:
    for (int i = 0; i < mNumPrimitives; i++) {
      GXPosition3f32(mVertexBuffer[i].x, mVertexBuffer[i].y, mVertexBuffer[i].z);
      DelayStreamVertex();
    }
    break;
  case kHasNormals:
    for (int i = 0; i < mNumPrimitives; i++) {
      GXPosition3f32(mVertexBuffer[i].x, mVertexBuffer[i].y, mVertexBuffer[i].z);
      GXNormal3f32(mNormalBuffer[i].x, mNormalBuffer[i].y, mNormalBuffer[i].z);
      DelayStreamVertex();
    }
    break;
  case kHasColor:
    for (int i = 0; i < mNumPrimitives; i++) {
      GXPosition3f32(mVertexBuffer[i].x, mVertexBuffer[i].y, mVertexBuffer[i].z);
      GXColor1u32(mColorBuffer[i]);
      DelayStreamVertex();
    }
    break;
  case kHasTexture:
    for (int i = 0; i < mNumPrimitives; i++) {
      GXPosition3f32(mVertexBuffer[i].x, mVertexBuffer[i].y, mVertexBuffer[i].z);
      GXTexCoord2f32(mTexCoordBuffer0[i].x, mTexCoordBuffer0[i].y);
      DelayStreamVertex();
    }
    break;
  case kHasNormals | kHasTexture:
    for (int i = 0; i < mNumPrimitives; i++) {
      GXPosition3f32(mVertexBuffer[i].x, mVertexBuffer[i].y, mVertexBuffer[i].z);
      GXNormal3f32(mNormalBuffer[i].x, mNormalBuffer[i].y, mNormalBuffer[i].z);
      GXTexCoord2f32(mTexCoordBuffer0[i].x, mTexCoordBuffer0[i].y);
      DelayStreamVertex();
    }
    break;
  case kHasNormals | kHasColor:
    for (int i = 0; i < mNumPrimitives; i++) {
      GXPosition3f32(mVertexBuffer[i].x, mVertexBuffer[i].y, mVertexBuffer[i].z);
      GXNormal3f32(mNormalBuffer[i].x, mNormalBuffer[i].y, mNormalBuffer[i].z);
      GXColor1u32(mColorBuffer[i]);
      DelayStreamVertex();
    }
    break;
  case kHasColor | kHasTexture:
    for (int i = 0; i < mNumPrimitives; i++) {
      GXPosition3f32(mVertexBuffer[i].x, mVertexBuffer[i].y, mVertexBuffer[i].z);
      GXColor1u32(mColorBuffer[i]);
      GXTexCoord2f32(mTexCoordBuffer0[i].x, mTexCoordBuffer0[i].y);
      DelayStreamVertex();
    }
    break;
  case kHasNormals | kHasColor | kHasTexture:
    for (int i = 0; i < mNumPrimitives; i++) {
      GXPosition3f32(mVertexBuffer[i].x, mVertexBuffer[i].y, mVertexBuffer[i].z);
      GXNormal3f32(mNormalBuffer[i].x, mNormalBuffer[i].y, mNormalBuffer[i].z);
      GXColor1u32(mColorBuffer[i]);
      GXTexCoord2f32(mTexCoordBuffer0[i].x, mTexCoordBuffer0[i].y);
      DelayStreamVertex();
    }
    break;
  }
  CGX::End();
}
#endif

void CGraphics::FullRender() {
  CGX::Begin(static_cast< GXPrimitive >(mCurrentPrimitive), GX_VTXFMT0, mNumPrimitives);
  switch (vtxDescr.mStreamFlags) {

  case 0:
    for (int i = 0; i < mNumPrimitives; i++) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      const Vec& vtx = vtxBuffer[i];
#else
      const Vec& vtx = mVertexBuffer[i];
#endif

      GXPosition3f32(vtx.x, vtx.y, vtx.z);
    }
    break;
  case kHasNormals:
    for (int i = 0; i < mNumPrimitives; i++) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      const Vec& vtx = vtxBuffer[i];
#else
      const Vec& vtx = mVertexBuffer[i];
#endif

      GXPosition3f32(vtx.x, vtx.y, vtx.z);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      const Vec& nrm = nrmBuffer[i];
#else
      const Vec& nrm = mNormalBuffer[i];
#endif

      GXNormal3f32(nrm.x, nrm.y, nrm.z);
    }
    break;
  case kHasColor:
    for (int i = 0; i < mNumPrimitives; i++) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      const Vec& vtx = vtxBuffer[i];
#else
      const Vec& vtx = mVertexBuffer[i];
#endif

      GXPosition3f32(vtx.x, vtx.y, vtx.z);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      GXColor1u32(clrBuffer[i]);
#else
      GXColor1u32(mColorBuffer[i]);
#endif

    }
    break;
  case kHasTexture:
    for (int i = 0; i < mNumPrimitives; i++) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      const Vec& vtx = vtxBuffer[i];
#else
      const Vec& vtx = mVertexBuffer[i];
#endif

      GXPosition3f32(vtx.x, vtx.y, vtx.z);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      const Vec2& uv = txtBuffer0[i];
#else
      const Vec2& uv = mTexCoordBuffer0[i];
#endif

      GXTexCoord2f32(uv.x, uv.y);
    }
    break;
  case kHasNormals | kHasTexture:
    for (int i = 0; i < mNumPrimitives; i++) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      const Vec& vtx = vtxBuffer[i];
#else
      const Vec& vtx = mVertexBuffer[i];
#endif

      GXPosition3f32(vtx.x, vtx.y, vtx.z);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      const Vec& nrm = nrmBuffer[i];
#else
      const Vec& nrm = mNormalBuffer[i];
#endif

      GXNormal3f32(nrm.x, nrm.y, nrm.z);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      const Vec2& uv = txtBuffer0[i];
#else
      const Vec2& uv = mTexCoordBuffer0[i];
#endif

      GXTexCoord2f32(uv.x, uv.y);
    }
    break;
  case kHasNormals | kHasColor:
    for (int i = 0; i < mNumPrimitives; i++) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      const Vec& vtx = vtxBuffer[i];
#else
      const Vec& vtx = mVertexBuffer[i];
#endif

      GXPosition3f32(vtx.x, vtx.y, vtx.z);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      const Vec& nrm = nrmBuffer[i];
#else
      const Vec& nrm = mNormalBuffer[i];
#endif

      GXNormal3f32(nrm.x, nrm.y, nrm.z);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      GXColor1u32(clrBuffer[i]);
#else
      GXColor1u32(mColorBuffer[i]);
#endif

    }
    break;
  case kHasColor | kHasTexture:
    for (int i = 0; i < mNumPrimitives; i++) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      const Vec& vtx = vtxBuffer[i];
#else
      const Vec& vtx = mVertexBuffer[i];
#endif

      GXPosition3f32(vtx.x, vtx.y, vtx.z);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      GXColor1u32(clrBuffer[i]);
      const Vec2& uv = txtBuffer0[i];
#else
      GXColor1u32(mColorBuffer[i]);
      const Vec2& uv = mTexCoordBuffer0[i];
#endif

      GXTexCoord2f32(uv.x, uv.y);
    }
    break;
  case kHasNormals | kHasColor | kHasTexture:
    for (int i = 0; i < mNumPrimitives; i++) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      const Vec& vtx = vtxBuffer[i];
#else
      const Vec& vtx = mVertexBuffer[i];
#endif

      GXPosition3f32(vtx.x, vtx.y, vtx.z);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      const Vec& nrm = nrmBuffer[i];
#else
      const Vec& nrm = mNormalBuffer[i];
#endif

      GXNormal3f32(nrm.x, nrm.y, nrm.z);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      GXColor1u32(clrBuffer[i]);
      const Vec2& uv = txtBuffer0[i];
#else
      GXColor1u32(mColorBuffer[i]);
      const Vec2& uv = mTexCoordBuffer0[i];
#endif

      GXTexCoord2f32(uv.x, uv.y);
    }
    break;
  }
  CGX::End();
}

void CGraphics::SetDepthRange(float near, float far) {
  mDepthNear = near;
  mDepthFar = far;
  GXSetViewport(static_cast< float >(mViewport.mLeft), static_cast< float >(mViewport.mTop),
                static_cast< float >(mViewport.mWidth), static_cast< float >(mViewport.mHeight),
                mDepthNear, mDepthFar);
}

static inline GXTevStageID get_texture_unit(const ERglTevStage stage) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_GM8P_00
  return static_cast< GXTevStageID >(stage);
#else
  if (stage == kTS_Stage0) {
    return GX_TEVSTAGE0;
  } else if (stage == kTS_Stage1) {
    return GX_TEVSTAGE1;
  } else if (stage == kTS_Stage2) {
    return GX_TEVSTAGE2;
  } else if (stage == kTS_Stage3) {
    return GX_TEVSTAGE3;
  } else if (stage == kTS_Stage4) {
    return GX_TEVSTAGE4;
  } else if (stage == kTS_Stage5) {
    return GX_TEVSTAGE5;
  } else if (stage == kTS_Stage6) {
    return GX_TEVSTAGE6;
  } else if (stage == kTS_Stage7) {
    return GX_TEVSTAGE7;
  } else if (stage == kTS_Stage8) {
    return GX_TEVSTAGE8;
  } else if (stage == kTS_Stage9) {
    return GX_TEVSTAGE9;
  } else if (stage == kTS_Stage10) {
    return GX_TEVSTAGE10;
  } else if (stage == kTS_Stage11) {
    return GX_TEVSTAGE11;
  } else if (stage == kTS_Stage12) {
    return GX_TEVSTAGE12;
  } else if (stage == kTS_Stage13) {
    return GX_TEVSTAGE13;
  } else if (stage == kTS_Stage14) {
    return GX_TEVSTAGE14;
  }
  // wtf?
  return static_cast< GXTevStageID >(stage == kTS_Stage15 ? GX_TEVSTAGE15 : 0);
#endif
#else
  return static_cast< GXTevStageID >(stage);
#endif

}

void CGraphics::SetTevOp(ERglTevStage stage, const CTevCombiners::CTevPass& pass) {
  CTevCombiners::SetupPass(get_texture_unit(stage), pass);
#if !RSTL_PRIME1
}

void CGraphics::SetTevRegisterColor(int index, const CColor& color) {
  CTevCombiners::SetTevRegisterColor(index, color);
#endif

}

void CGraphics::SetFog(ERglFogMode mode, float startz, float endz, const CColor& color) {
  CGX::SetFog(static_cast< GXFogType >(mode), startz, endz, mProj.GetNear(), mProj.GetFar(),
              color.GetGXColor());
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CGraphics::ResetGfxStates() { sRenderState.Set(0); }
#else
void CGraphics::ResetGfxStates() { mRenderState.Set(0); }
#endif

void CGraphics::SetDefaultVtxAttrFmt() {
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT2, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_NRM, GX_NRM_XYZ, GX_S16, 14);
  GXSetVtxAttrFmt(GX_VTXFMT2, GX_VA_NRM, GX_NRM_XYZ, GX_S16, 14);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
  GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
  GXSetVtxAttrFmt(GX_VTXFMT2, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT2, GX_VA_TEX0, GX_TEX_ST, GX_U16, 15);
  for (int i = 1; i <= 7; ++i) {
    GXAttr attr = static_cast< GXAttr >(GX_VA_TEX0 + i);
    GXSetVtxAttrFmt(GX_VTXFMT0, attr, GX_TEX_ST, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT1, attr, GX_TEX_ST, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT2, attr, GX_TEX_ST, GX_F32, 0);
  }
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CGraphics::LoadDolphinSpareTexture(int width, int height, GXTexFmt fmt, void* data,
#else
void CGraphics::LoadDolphinSpareTexture(int width, int height, GXTexFmt fmt, const void* data,
#endif

                                        GXTexMapID texId) {
  GXTexObj texObj;
  GXInitTexObj(&texObj, data != nullptr ? data : mpSpareBuffer, width, height, fmt, GX_CLAMP,
               GX_CLAMP, GX_DISABLE);
  GXInitTexObjLOD(&texObj, GX_NEAR, GX_NEAR, 0.f, 0.f, 0.f, GX_DISABLE, GX_DISABLE, GX_ANISO_1);
  GXLoadTexObj(&texObj, texId);
  CTexture::InvalidateTexmap(texId);
  if (texId == GX_TEXMAP7) {
    GXInvalidateTexRegion(&mTexRegions[0]);
  }
}

void CGraphics::LoadDolphinSpareTexture(int width, int height, GXCITexFmt fmt, GXTlut tlut,
                                        void* data, GXTexMapID texId) {
  GXTexObj texObj;
  GXInitTexObjCI(&texObj, data != nullptr ? data : mpSpareBuffer, width, height, fmt, GX_CLAMP,
                 GX_CLAMP, GX_DISABLE, tlut);
  GXInitTexObjLOD(&texObj, GX_NEAR, GX_NEAR, 0.f, 0.f, 0.f, GX_DISABLE, GX_DISABLE, GX_ANISO_1);
  GXLoadTexObj(&texObj, texId);
  CTexture::InvalidateTexmap(texId);
  if (texId == GX_TEXMAP7) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    GXInvalidateTexRegion(&mTexRegions[0]);
#else
    GXInvalidateTexRegion(&mTexRegionsCI[0]);
#endif

  }
}

void CGraphics::TickRenderTimings() {
  mRenderTimings = (mRenderTimings + 1) % (900 * 60);
  mSecondsMod900 = static_cast< float >(mRenderTimings) / 60.f;
}

float CGraphics::GetSecondsMod900() {
  if (mpExternalTimeProvider != nullptr) {
    return mpExternalTimeProvider->GetSecondsMod900();
  }
  return mSecondsMod900;
}

void CGraphics::SetExternalTimeProvider(CTimeProvider* timeProvider) {
  mpExternalTimeProvider = timeProvider;
}

void CGraphics::FlushProjection() {
  float right = mProj.GetRight();
  float left = mProj.GetLeft();
  float top = mProj.GetTop();
  float bottom = mProj.GetBottom();
  float near = mProj.GetNear();
  float far = mProj.GetFar();
  if (mProj.IsPerspective()) {
    Mtx44 mtx;
    MTXFrustum(mtx, top, bottom, left, right, near, far);
    GXSetProjection(mtx, GX_PERSPECTIVE);
  } else {
    Mtx44 mtx;
    MTXOrtho(mtx, top, bottom, left, right, near, far);
    GXSetProjection(mtx, GX_ORTHOGRAPHIC);
  }
}

const CGraphics::CProjectionState& CGraphics::GetProjectionState() { return mProj; }

void CGraphics::SetProjectionState(const CProjectionState& proj) {
  mProj = proj;
  FlushProjection();
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CGraphics::CClippedScreenRect
CGraphics::ClipScreenRectFromVS(const CVector3f& p1, const CVector3f& p2, ETexelFormat fmt) {
  if (!p1.IsNonZero() || !p2.IsNonZero()) {
    return CClippedScreenRect();
#else
CGraphics::CClippedScreenQuad
CGraphics::ClipScreenQuadFromVS(const CVector3f& p1, const CVector3f& p2, const CVector3f& p3,
                                const CVector3f& p4, ETexelFormat fmt) {
  if (p1.GetY() < GetProjectionState().GetNear() || p2.GetY() < GetProjectionState().GetNear() ||
      p3.GetY() < GetProjectionState().GetNear() || p4.GetY() < GetProjectionState().GetNear()) {
    return CClippedScreenQuad();
#endif

  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (p1.GetY() < GetProjectionState().GetNear() || p2.GetY() < GetProjectionState().GetNear()) {
    return CClippedScreenRect();
  }
  if (p1.GetY() > GetProjectionState().GetFar() || p2.GetY() > GetProjectionState().GetFar()) {
    return CClippedScreenRect();
#else
  if (p1.GetY() > GetProjectionState().GetFar() || p2.GetY() > GetProjectionState().GetFar() ||
      p3.GetY() > GetProjectionState().GetFar() || p4.GetY() > GetProjectionState().GetFar()) {
    return CClippedScreenQuad();
#endif

  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CVector2i p1p = ProjectPoint(p1);
  CVector2i p2p = ProjectPoint(p2);

  int minX = rstl::min_val(p1p.GetX(), p2p.GetX());
  int minY = rstl::min_val(p1p.GetY(), p2p.GetY());

  int maxX = p1p.GetX() - p2p.GetX();
  int maxY = p1p.GetY() - p2p.GetY();
  if (maxX < 0) {
    maxX = -maxX;
  }
  if (maxY < 0) {
    maxY = -maxY;
  }
#else
  rstl::reserved_vector< CVector2f, 4 > texCoords;
  rstl::reserved_vector< CVector2i, 4 > points;
  points.push_back(ProjectPoint(p1));
  points.push_back(ProjectPoint(p2));
  points.push_back(ProjectPoint(p3));
  points.push_back(ProjectPoint(p4));
  int minX = rstl::min_val(rstl::min_val(points[0].GetX(), points[1].GetX()),
                           rstl::min_val(points[2].GetX(), points[3].GetX()));
  int minY = rstl::min_val(rstl::min_val(points[0].GetY(), points[1].GetY()),
                           rstl::min_val(points[2].GetY(), points[3].GetY()));
  int maxX = rstl::max_val(rstl::max_val(points[0].GetX(), points[1].GetX()),
                           rstl::max_val(points[2].GetX(), points[3].GetX()));
  int maxY = rstl::max_val(rstl::max_val(points[0].GetY(), points[1].GetY()),
                           rstl::max_val(points[2].GetY(), points[3].GetY()));
  const CViewport& vp = mViewport;
  const int width = maxX - minX;
  const int height2 = maxY - minY;
  const int vpLeft = vp.mLeft;
  const int vpWidth = vp.mWidth;
  const int vpTop = vp.mTop;
  const int vpHeight = vp.mHeight;
#endif

  int left = minX & ~1;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (left >= mViewport.mLeft + mViewport.mWidth) {
    return CClippedScreenRect();
#else
  if (left >= vpLeft + vpWidth) {
    return CClippedScreenQuad();
#endif

  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int right = minX + maxX + 2 & ~1;
  if (right <= mViewport.mLeft) {
    return CClippedScreenRect();
#else
  int right = (minX + width + 2) & ~1;
  if (right <= vpLeft) {
    return CClippedScreenQuad();
#endif

  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  left = rstl::max_val(left, mViewport.mLeft) & ~1;
  right = rstl::min_val(right, mViewport.mLeft + mViewport.mWidth) + 1 & ~1;
#else
  left = rstl::max_val(left, vpLeft) & ~1;
  right = (rstl::min_val(right, vpLeft + vpWidth) + 1) & ~1;
#endif

  int top = minY & ~1;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (top >= mViewport.mTop + mViewport.mHeight) {
    return CClippedScreenRect();
#else
  if (top >= vpTop + vpHeight) {
    return CClippedScreenQuad();
#endif

  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int bottom = minY + maxY + 2 & ~1;
  if (bottom <= mViewport.mTop) {
    return CClippedScreenRect();
#else
  int bottom = (minY + height2 + 2) & ~1;
  if (bottom <= vpTop) {
    return CClippedScreenQuad();
#endif

  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  top = rstl::max_val(top, mViewport.mTop) & ~1;
  bottom = rstl::min_val(bottom, mViewport.mTop + mViewport.mHeight) + 1 & ~1;

  float minV = CCast::LtoF(minY - top) / CCast::LtoF(bottom - top);
  float maxV = CCast::LtoF(maxY + (minY - top) + 1) / CCast::LtoF(bottom - top);
#else
  top = rstl::max_val(top, vpTop) & ~1;
  bottom = (rstl::min_val(bottom, vpTop + vpHeight) + 1) & ~1;
#endif

  int texAlign = 4;
  switch (fmt) {
  case kTF_I8:
    texAlign = 8;
    break;
  case kTF_IA8:
  case kTF_RGB565:
  case kTF_RGB5A3:
    texAlign = 4;
    break;
  case kTF_RGBA8:
    texAlign = 2;
    break;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int texWidth = texAlign + (right - left) - 1 & ~(texAlign - 1);
  float minU = CCast::LtoF(minX - left) / CCast::LtoF(texWidth);
  float maxU = CCast::LtoF(maxX + (minX - left) + 1) / CCast::LtoF(texWidth);
  return CClippedScreenRect(left, top, right - left, bottom - top, texWidth, minU, maxU, minV,
                            maxV);
#else
  const int texWidth = (right - left + texAlign - 1) & ~(texAlign - 1);
  if (left + texWidth > mViewport.mWidth) {
    left = mViewport.mWidth - texWidth;
  }
  for (int i = 0; i < 4; ++i) {
    texCoords.push_back(CVector2f(float(points[i].GetX() - left) / float(texWidth - 1),
                                  float(points[i].GetY() - top) / float(bottom - top - 1)));
  }
  return CClippedScreenQuad(left, top, right - left, texWidth, bottom - top, texCoords);
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CGraphics::CClippedScreenRect
CGraphics::ClipScreenRectFromMS(const CVector3f& p1, const CVector3f& p2, ETexelFormat fmt) {
  return ClipScreenRectFromVS(mViewMatrix.TransposeMultiply(mModelMatrix * p1),
                              mViewMatrix.TransposeMultiply(mModelMatrix * p2), fmt);
#else
CGraphics::CClippedScreenQuad
CGraphics::ClipScreenQuadFromMS(const CVector3f& p1, const CVector3f& p2, const CVector3f& p3,
                                const CVector3f& p4, ETexelFormat fmt) {
  return ClipScreenQuadFromVS(mViewMatrix.TransposeMultiply(mModelMatrix * p1),
                              mViewMatrix.TransposeMultiply(mModelMatrix * p2),
                              mViewMatrix.TransposeMultiply(mModelMatrix * p3),
                              mViewMatrix.TransposeMultiply(mModelMatrix * p4), fmt);
#endif

}

float CGraphics::GetFPS() {
  BOOL level = OSDisableInterrupts();
  float value = rstl::min_val(mFramesPerSecond, mLastFramesPerSecond);
  OSRestoreInterrupts(level);
  return value;
}

void CGraphics::SetUseVideoFilter(const bool b) {
  mUseVideoFilter = b;
  GXSetCopyFilter(mRenderModeObj.aa, mRenderModeObj.sample_pattern, b ? GX_ENABLE : GX_DISABLE,
                  mRenderModeObj.vfilter);
}

bool CGraphics::GetUseVideoFilter() { return mUseVideoFilter; }

int CGraphics::GetFrameCounter() { return mFrameCounter; }

CVector2i CGraphics::ProjectPoint(const CVector3f& point) {
  CVector3f vec = GetPerspectiveProjectionMatrix().MultiplyOneOverW(point);
  vec.SetX(vec.GetX() * GetViewport().mHalfWidth + GetViewport().mHalfWidth);
  vec.SetY(-vec.GetY() * GetViewport().mHalfHeight + GetViewport().mHalfHeight);
  return CVector2i(CCast::FtoL(vec.GetX()), CCast::FtoL(vec.GetY()));
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION == RSTL_GM8P_00
void CGraphics::SetProgressiveMode(bool rgb60) {
  bool was60Hz = GetProgressiveMode();
  OSSetEuRgb60Mode(rgb60);
  if (rgb60 != was60Hz) {
    VISetBlack(TRUE);
    VIFlush();
    for (int i = 0; i < 10; ++i) {
      VIWaitForRetrace();
    }
    ConfigureVideoMode(false, rgb60);
    ConfigureFrameBuffer();
    for (int i = 0; i < 10; ++i) {
      VIWaitForRetrace();
    }
    VISetBlack(FALSE);
    VIFlush();
    VIWaitForRetrace();
  }
}

bool CGraphics::GetProgressiveMode() { return VIGetTvFormat() == VI_EURGB60; }

bool CGraphics::CanSetProgressiveMode() { return true; }

bool CGraphics::GetProgressiveDefault() { return OSGetEuRgb60Mode() == 1; }
#else
void CGraphics::SetProgressiveMode(bool b) {
  bool isProgressive = GetProgressiveMode();
  OSSetProgressiveMode(b ? OS_PROGRESSIVE_MODE_ON : OS_PROGRESSIVE_MODE_OFF);
  if (b == isProgressive) {
    return;
  }

  VISetBlack(TRUE);
  VIFlush();
  VIWaitForRetrace();
  for (int i = 0; i < 10; ++i) {
    VIWaitForRetrace();
  }

  if (b) {
    mRenderModeObj.viTVmode = VI_TVMODE_NTSC_PROG;
    mRenderModeObj.xFBmode = VI_XFBMODE_SF;
    const u8 vfilter[7] = {0x04, 0x04, 0x10, 0x10, 0x10, 0x04, 0x04};
    memcpy(mRenderModeObj.vfilter, vfilter, 7);
  } else {
    mRenderModeObj.viTVmode = VI_TVMODE_NTSC_INT;
    mRenderModeObj.xFBmode = VI_XFBMODE_DF;
    memcpy(mRenderModeObj.vfilter, GXNtsc480IntDf.vfilter, 7);
  }
  GXSetCopyFilter(mRenderModeObj.aa, mRenderModeObj.sample_pattern, GX_ENABLE,
                  mRenderModeObj.vfilter);

  VIConfigure(&mRenderModeObj);
  VISetBlack(TRUE);
  VIFlush();
  for (int i = 0; i < 100; ++i) {
    VIWaitForRetrace();
  }
  VISetBlack(FALSE);
  VIFlush();
  for (int i = 0; i < 2; ++i) {
    VIWaitForRetrace();
  }
}

bool CGraphics::GetProgressiveMode() { return mRenderModeObj.viTVmode == VI_TVMODE_NTSC_PROG; }

bool CGraphics::CanSetProgressiveMode() { return VIGetDTVStatus() != 0; }

bool CGraphics::GetProgressiveDefault() { return OSGetProgressiveMode() == OS_PROGRESSIVE_MODE_ON; }
#endif
#else
#if RSTL_VERSION == RSTL_G2MP
void CGraphics::SetPAL60(bool enabled) {
  const bool isPAL60 = GetProgressiveMode();
  OSSetEuRgb60Mode(enabled);
  if (enabled == isPAL60) {
    return;
  }

  VISetBlack(TRUE);
  VIFlush();
  for (int i = 0; i < 10; ++i) {
    VIWaitForRetrace();
  }
  ConfigureVideo(false, enabled);
  ConfigureFrameBuffer();
  for (int i = 0; i < 10; ++i) {
    VIWaitForRetrace();
  }
  VISetBlack(FALSE);
  VIFlush();
  VIWaitForRetrace();
}

// On PAL the progressive-mode getter reports PAL60 (EURGB60).
bool CGraphics::GetProgressiveMode() { return VIGetTvFormat() == VI_EURGB60; }
#else
void CGraphics::SetProgressiveMode(bool b) {
  bool isProgressive = GetProgressiveMode();
  OSSetProgressiveMode(b ? OS_PROGRESSIVE_MODE_ON : OS_PROGRESSIVE_MODE_OFF);
  if (b == isProgressive) {
    return;
  }

  VISetBlack(TRUE);
  VIFlush();
  VIWaitForRetrace();
  for (int i = 0; i < 10; ++i) {
    VIWaitForRetrace();
  }

  if (b) {
    mRenderModeObj.viTVmode = VI_TVMODE_NTSC_PROG;
    mRenderModeObj.xFBmode = VI_XFBMODE_SF;
    SetProgressiveFilter(mRenderModeObj);
  } else {
    mRenderModeObj.viTVmode = VI_TVMODE_NTSC_INT;
    mRenderModeObj.xFBmode = VI_XFBMODE_DF;
    memcpy(mRenderModeObj.vfilter, GXNtsc480IntDf.vfilter, 7);
  }
  GXSetCopyFilter(mRenderModeObj.aa, mRenderModeObj.sample_pattern, GX_ENABLE,
                  mRenderModeObj.vfilter);

  VIConfigure(&mRenderModeObj);
  VISetBlack(TRUE);
  VIFlush();
  for (int i = 0; i < 100; ++i) {
    VIWaitForRetrace();
  }
  VISetBlack(FALSE);
  VIFlush();
  for (int i = 0; i < 2; ++i) {
    VIWaitForRetrace();
  }
}

bool CGraphics::GetProgressiveMode() { return mRenderModeObj.viTVmode == VI_TVMODE_NTSC_PROG; }

bool CGraphics::CanSetProgressiveMode() { return VIGetDTVStatus() != 0; }

bool CGraphics::GetProgressiveDefault() { return OSGetProgressiveMode() == OS_PROGRESSIVE_MODE_ON; }
#endif
#endif

void CGraphics::GetScreenPosition(int* stretch, int* xOffset, int* yOffset) {
  if (stretch != nullptr) {
    *stretch = mScreenStretch;
  }
  if (xOffset != nullptr) {
    *xOffset = mScreenPositionX;
  }
  if (yOffset != nullptr) {
    *yOffset = mScreenPositionY;
  }
}

void CGraphics::SetScreenPosition(int stretch, int xOffset, int yOffset) {
  int stretchChange = stretch - mScreenStretch;
  int xChange = xOffset - mScreenPositionX;
  int yChange = yOffset - mScreenPositionY;
  if (stretchChange == 0 && xChange == 0 && yChange == 0) {
    return;
  }
  mRenderModeObj.viWidth += stretchChange * 2;
  mRenderModeObj.viXOrigin += xChange - stretchChange;
  mRenderModeObj.viYOrigin += yChange;
  VIConfigure(&mRenderModeObj);
  VIFlush();
  mScreenStretch = stretch;
  mScreenPositionX = xOffset;
  mScreenPositionY = yOffset;
}

void CGraphics::SetIsBeginSceneClearFb(bool b) { mIsBeginSceneClearFb = b; }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION == RSTL_GM8P_00
CGraphicsSys::CGraphicsSys(const COsContext& osContext, const CMemorySys& memorySys, bool rgb60) {
  if (mGraphicsInitialized != true) {
    mGraphicsInitialized = CGraphics::Startup(osContext, rgb60);
  }
}
#else
CGraphicsSys::CGraphicsSys(const COsContext& osContext, const CMemorySys& memorySys, uint fifoSize,
                           void* fifoBase) {
  if (mGraphicsInitialized != true) {
    mGraphicsInitialized = CGraphics::Startup(osContext, fifoSize, fifoBase);
  }
}

#endif
#else
void CGraphics::SetUseNormalMatrix(bool enabled) {
  if (mUseNormalMatrix != enabled) {
    mUseNormalMatrix = enabled;
    if (enabled) {
      SetViewMatrix();
    }
  }
}

void CGraphics::SetUseStreamVertexDelay(bool enabled) { sUseStreamVertexDelay = enabled; }

CGraphicsSys::CGraphicsSys(const COsContext& osContext, const CMemorySys& memorySys,
                           bool progressive) {
  if (mGraphicsInitialized != true) {
    mGraphicsInitialized = CGraphics::Startup(osContext, progressive);
  }
}
#endif

CGraphicsSys::~CGraphicsSys() {
  if (mGraphicsInitialized == true) {
    CGraphics::Shutdown();
    mGraphicsInitialized = false;
  }
}

CGraphics::CRenderState::CRenderState() {
  x0_ = 0;
  x4_ = 0;
}

void CGraphics::CRenderState::Flush() {}

int CGraphics::CRenderState::SetVtxState(const float* pos, const float* nrm, const uint* clr) {
  CGX::SetArray(GX_VA_POS, pos, 12);
  CGX::SetArray(GX_VA_NRM, nrm, 12);
  CGX::SetArray(GX_VA_CLR0, clr, 4);
  int result = 1;
  if (nrm != nullptr) {
    result |= 2;
  }
  if (clr != nullptr) {
    result |= 16;
  }
  return result;
}

void CGraphics::CRenderState::ResetFlushAll() {
  x0_ = 0;
  SetVtxState(nullptr, nullptr, nullptr);
  for (int i = 0; i < 8; i++) {
    CGX::SetArray(static_cast< GXAttr >(GX_VA_TEX0 + i), nullptr, 8);
  }
  Flush();
}

#if RSTL_PRIME1
// Preserve the signed conversion bias ordering left by stripped code.
static float StrippedIntToFloat(int n) { return static_cast< float >(n); }
#endif

