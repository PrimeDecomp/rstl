#include "Kyoto/Graphics/CMoviePlayer.hpp"
#include "rstl/macros.hpp"

#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Alloc/LockedCache.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "Kyoto/Audio/CAIInterruptManager.hpp"
#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Audio/DolphinCRSFAudio.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Basics/CInterruptGuard.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

#include "rstl/RstlVersions.h"
#include "rstl/math.hpp"

#include "dolphin/ai.h"
#include "dolphin/base/PPCArch.h"
#include "dolphin/gx/GXVert.h"
#if defined(TARGET_PC)
#include "Metaforce/Audio.hpp"
#include "Metaforce/Display.hpp"
#include "dolphin/gx.h"
#include "dolphin/thp.h"
#include <algorithm>

#else
#include "dolphin/gx/GXGeometry.h"
#include "dolphin/gx/GXTev.h"
#include "dolphin/gx/GXTexture.h"
#include "dolphin/thp/THPAudio.h"
#include "dolphin/thp/THPPlayer.h"
#include "dolphin/thp/THPVideoDecode.h"
#endif

#include "math.h"
#include "string.h"

static const uint skTHPVersion = 0x11000;
static int sNumReferences = 0;
static CMoviePlayer* sAudioPlayer;
#if !defined(TARGET_PC)
static const short* curAudioBuffer;
static int soundBufferIndex;
ATTRIBUTE_ALIGN_DECL(32, static short soundBuffer[2][320]);
#endif
static bool sAudioEnabled = true;
static uchar sSfxVolume = 127;

struct CMoviePlayer::SIndexLoad {
  rstl::single_ptr< CDvdRequest > mHeaderRequest;
  rstl::single_ptr< CDvdRequest > mVideoRequest;
  rstl::single_ptr< CDvdRequest > mAudioRequest;
  rstl::single_ptr< uchar > mBuffer;
  int mState;

#if !RSTL_PRIME1
  ~SIndexLoad() {}
#endif

  SIndexLoad() : mBuffer(static_cast< uchar* >(rs_aligned_new_line(64, 277))), mState(0) {}
};
ATTRIBUTE_ALIGN_DECL(32, const unsigned char skInterlacePattern[32]) = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static void MyTHPGXRestore() {
  CGX::SetZMode(TRUE, GX_ALWAYS, FALSE);
  CGX::SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_SET);
  CGX::SetNumTexGens(1);
  CGX::SetNumChans(0);
  CGX::SetNumTevStages(1);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
}

static void MyTHPGXYuv2RgbSetup(bool field, bool deinterlace) {
  GXVtxDescList attr[3] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetZMode(TRUE, GX_ALWAYS, FALSE);
  CGX::SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
  CGX::SetNumChans(0);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, FALSE, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, FALSE, GX_PTIDENTITY);

  if (deinterlace) {
    CGX::SetNumTexGens(2);
    CGX::SetNumTevStages(4);
  } else {
    CGX::SetNumTexGens(3);
    CGX::SetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX0, FALSE, GX_PTIDENTITY);

    float n = field ? 0.25f : 0.f;
    float mtx[8] = {0.125f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.25f, n};
    GXLoadTexMtxImm(reinterpret_cast< MtxPtr >(mtx), GX_TEXMTX0, GX_MTX2x4);
    GXTexObj obj;
    GXInitTexObj(&obj, skInterlacePattern, 8, 4, GX_TF_I8, GX_REPEAT, GX_REPEAT, FALSE);
    GXInitTexObjLOD(&obj, GX_NEAR, GX_NEAR, 0.f, 0.f, 0.f, FALSE, FALSE, GX_ANISO_1);
    GXLoadTexObj(&obj, GX_TEXMAP3);
#if defined(TARGET_PC)
    GXDestroyTexObj(&obj);
#endif
    CTexture::InvalidateTexmap(GX_TEXMAP3);
    CGX::SetTevOrder(GX_TEVSTAGE4, GX_TEXCOORD2, GX_TEXMAP3, GX_COLOR_NULL);
    CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE4);
    CGX::SetTevColorIn(GX_TEVSTAGE4, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
    CGX::SetTevAlphaIn(GX_TEVSTAGE4, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    CGX::SetAlphaCompare(GX_LESS, 128, GX_AOP_AND, GX_ALWAYS, 0);
    CGX::SetNumTevStages(5);
  }

  CGX::SetVtxDescv(attr);
  GXSetColorUpdate(TRUE);
  GXSetAlphaUpdate(FALSE);
  GXInvalidateTexAll();
  GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_POS, GX_CLR_RGBA, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_TEX0, GX_CLR_RGBA, GX_RGBX8, 0);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_C0);
  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, false, GX_TEVPREV);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_A0);
  CGX::SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, false, GX_TEVPREV);
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR_NULL);
  CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
  CGX::SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, false, GX_TEVPREV);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_APREV);
  CGX::SetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, false, GX_TEVPREV);
  CGX::SetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K1);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K1_A);
  CGX::SetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_CPREV);
  CGX::SetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevAlphaIn(GX_TEVSTAGE2, GX_CA_TEXA, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
  CGX::SetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  CGX::SetTevColorIn(GX_TEVSTAGE3, GX_CC_APREV, GX_CC_CPREV, GX_CC_KONST, GX_CC_ZERO);
  CGX::SetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevAlphaIn(GX_TEVSTAGE3, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
  CGX::SetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevKColorSel(GX_TEVSTAGE3, GX_TEV_KCSEL_K2);
  const GXColorS10 color = {-90, 0, -114, 135};
  GXSetTevColorS10(GX_TEVREG0, color);
  const GXColor kColor0 = {0, 0, 226, 88};
  CGX::SetTevKColor(GX_KCOLOR0, kColor0);
  const GXColor kColor1 = {179, 0, 0, 182};
  CGX::SetTevKColor(GX_KCOLOR1, kColor1);
  const GXColor kColor2 = {255, 0, 255, 128};
  CGX::SetTevKColor(GX_KCOLOR2, kColor2);
}

static void MyTHPYuv2RgbTextureSetup(void* y, void* u, void* v, ushort width, ushort height) {
  GXTexObj yTex;
  GXInitTexObj(&yTex, y, width, height, GX_TF_I8, GX_CLAMP, GX_CLAMP, FALSE);
  GXInitTexObjLOD(&yTex, GX_NEAR, GX_NEAR, 0.f, 0.f, 0.f, FALSE, FALSE, GX_ANISO_1);
  GXLoadTexObj(&yTex, GX_TEXMAP0);

  GXTexObj uTex;
  GXInitTexObj(&uTex, u, width / 2, height / 2, GX_TF_I8, GX_CLAMP, GX_CLAMP, FALSE);
  GXInitTexObjLOD(&uTex, GX_NEAR, GX_NEAR, 0.f, 0.f, 0.f, FALSE, FALSE, GX_ANISO_1);
  GXLoadTexObj(&uTex, GX_TEXMAP1);

  GXTexObj vTex;
  GXInitTexObj(&vTex, v, static_cast< s16 >(width / 2), height / 2, GX_TF_I8, GX_CLAMP, GX_CLAMP,
               FALSE);
  GXInitTexObjLOD(&vTex, GX_NEAR, GX_NEAR, 0.f, 0.f, 0.f, FALSE, FALSE, GX_ANISO_1);
  GXLoadTexObj(&vTex, GX_TEXMAP2);

#if defined(TARGET_PC)
  GXDestroyTexObj(&yTex);
  GXDestroyTexObj(&uTex);
  GXDestroyTexObj(&vTex);
#endif

  CTexture::InvalidateTexmap(GX_TEXMAP0);
  CTexture::InvalidateTexmap(GX_TEXMAP1);
  CTexture::InvalidateTexmap(GX_TEXMAP2);
}

#if !RSTL_PRIME1
static rstl::string SelectMoviePath(const char* path) {
  rstl::string name(path);
  if (CGraphics::Is50Hz()) {
    rstl::string palName = name + "_pal";
    if (CDvdFile::FileExists(palName.data())) {
      return palName;
    }
    RS_WARNING_DEBUGGER_PRINT("WARNING: No PAL version of %s\n", path);
  }
  return name;
}

static bool ShouldEnableLockedCache() { return LCGetBase() != GetLockedCacheAllocationBase(); }
#endif

CMoviePlayer::CMoviePlayer(const char* path, const float preLoadSeconds, const bool loop,
                           const bool deinterlace)
#if RSTL_PRIME1
: mDvdFile(path)
, mHeader()
, mThpComponents()
, mVideoInfo()
, mAudioInfo()
, mIndexLoad(rs_new_line(367) SIndexLoad)
#else
: mDvdFile(SelectMoviePath(path).data())
, mIndexLoad(rs_new_line(367) SIndexLoad)
#endif

, mNextReadSize(0)
, mNextReadOff(0)
, mReadSizeWrapped(0)
, mReadOffWrapped(0)
, mCurLoadFrame(0)
, mRequestFrameWrapped(0)
, mCurFrame(0)
, mDecodedTexSlot(0)
, mDrawTexSlot(-1)
, mAudioSlot(-1)
, mDecodedTexCount(0)
, mFrameRem(0.f)
#if !RSTL_PRIME1
, mPrefetchFrame(0)
, mPrefetchOff(0)
, mPrefetchSize(0)
#endif

, mPlayMode(kPM_Playing)
, mTotalSeconds(0.f)
, mCurSeconds(0.f)
, mPreLoadSeconds(preLoadSeconds)
, mPreLoadFrames(0)
, mLoop(loop)
, mDeinterlace(deinterlace)
#if !RSTL_PRIME1
, mIs60Hz(false)
#endif

, mHasAudio(false)
, mFieldFlip(false)
, mCachedBytes(0)
, mFieldIndex(0)
#if !RSTL_PRIME1
, mVolume(127)
#endif
{
#if !RSTL_PRIME1
  if (sNumReferences == 0 && ShouldEnableLockedCache()) {
    LCEnable();
  }
#endif

  static bool sThpInitialized = false;
  if (!sThpInitialized) {
    sThpInitialized = true;
    THPInit();
  }
  {
    IF_TARGET_PC(const metaforce::AudioLockGuard lock);
    ++sNumReferences;
    VerifyCallbackStatus();
  }
  mIndexLoad->mHeaderRequest = mDvdFile.SyncRead(mIndexLoad->mBuffer.get(), 64);
}

#if !RSTL_PRIME1
CMoviePlayer::~CMoviePlayer() {
  IF_TARGET_PC(const metaforce::AudioLockGuard lock);

  CancelReadRequests();
  --sNumReferences;
  VerifyCallbackStatus();
  if (sNumReferences == 0 && ShouldEnableLockedCache()) {
    LCDisable();
  }
  if (sAudioPlayer == this) {
    sAudioPlayer = nullptr;
  }
}

#endif
bool CMoviePlayer::ContinueLoading() {
  if (mIndexLoad.null()) {
    return false;
  }
  uchar* const buffer = mIndexLoad->mBuffer.get();

  switch (mIndexLoad->mState) {
  case 0:
    if (mIndexLoad->mHeaderRequest->IsComplete()) {
      memcpy(&mHeader, buffer, sizeof(THPHeader));
      BE_SWAP_INPLACE(mHeader.version);
      BE_SWAP_INPLACE(mHeader.bufSize);
      BE_SWAP_INPLACE(mHeader.audioMaxSamples);
      BE_SWAP_INPLACE(mHeader.frameRate);
      BE_SWAP_INPLACE(mHeader.numFrames);
      BE_SWAP_INPLACE(mHeader.firstFrameSize);
      BE_SWAP_INPLACE(mHeader.movieDataSize);
      BE_SWAP_INPLACE(mHeader.compInfoDataOffsets);
      BE_SWAP_INPLACE(mHeader.offsetDataOffsets);
      BE_SWAP_INPLACE(mHeader.movieDataOffsets);
      BE_SWAP_INPLACE(mHeader.finalFrameDataOffsets);
#if !RSTL_PRIME1
      if (close_enough(mHeader.frameRate, 29.97f)) {
        mHeader.frameRate = 30.f;
      }
      if (close_enough(mHeader.frameRate, 59.94f)) {
        mHeader.frameRate = 60.f;
      }
      if (close_enough(mHeader.frameRate, 60.f)) {
        mIs60Hz = true;
      }
#endif
      // clang-format off
      RS_VERIFY_THROW(449, !strcmp ( mHeader.magic,"THP" ), false, "Invalid THP header!");
      // clang-format on
      RS_VERIFY_THROW(450, mHeader.version == skTHPVersion, false,
                      CBasics::Stringize("Invalid movie version!  Expected %x, got %x",
                                         skTHPVersion, mHeader.version));
      mIndexLoad->mHeaderRequest =
          mDvdFile.AsyncSeekRead(buffer, 32, kSO_Begin, mHeader.compInfoDataOffsets);
      ++mIndexLoad->mState;
    } else {
      return true;
    }
  case 1: {
    if (mIndexLoad->mHeaderRequest->IsComplete()) {
      memcpy(&mThpComponents, buffer, sizeof(THPFrameCompInfo));
      mThpComponents.numComponents =
          CBasics::SwapBytes(static_cast< uint >(mThpComponents.numComponents));
      mIndexLoad->mHeaderRequest = nullptr;
      uchar* audioBuffer = buffer + 32;
      int offset = mHeader.compInfoDataOffsets + sizeof(THPFrameCompInfo);

      for (uint i = 0; i < mThpComponents.numComponents; ++i) {
        switch (mThpComponents.frameComp[i]) {
        case 0:
          mIndexLoad->mVideoRequest = mDvdFile.AsyncSeekRead(buffer, 32, kSO_Begin, offset);
          offset += sizeof(IF_RSTL_PRIME1_ELSE(THPVideoInfoOld, THPVideoInfo));

          break;
        case 1:
          mIndexLoad->mAudioRequest = mDvdFile.AsyncSeekRead(audioBuffer, 32, kSO_Begin, offset);
          offset += sizeof(IF_RSTL_PRIME1_ELSE(THPAudioInfoOld, THPAudioInfo));

          mHasAudio = true;
          break;

        default:
          RS_VERIFY_THROW(484, false, false, "Unknown component type!");
          break;
        }
      }
      ++mIndexLoad->mState;
    } else {
      return true;
    }
  }
  case 2: {
    bool complete = true;
    bool hasVideo = mIndexLoad->mVideoRequest;
    bool hasAudio = mIndexLoad->mAudioRequest;

    if (hasVideo && !mIndexLoad->mVideoRequest->IsComplete()) {
      complete = false;
    }
    if (hasAudio && !mIndexLoad->mAudioRequest->IsComplete()) {
      complete = false;
    }
    if (!complete) {
      return true;
    }
    if (hasVideo) {
      memcpy(&mVideoInfo, buffer, sizeof(IF_RSTL_PRIME1_ELSE(THPVideoInfoOld, THPVideoInfo)));

      mVideoInfo.xSize = CBasics::SwapBytes(static_cast< uint >(mVideoInfo.xSize));
      mVideoInfo.ySize = CBasics::SwapBytes(static_cast< uint >(mVideoInfo.ySize));
    }
    if (hasAudio) {
      memcpy(&mAudioInfo, buffer + 32, sizeof(IF_RSTL_PRIME1_ELSE(THPAudioInfoOld, THPAudioInfo)));

      mAudioInfo.sndChannels = CBasics::SwapBytes(static_cast< uint >(mAudioInfo.sndChannels));
      mAudioInfo.sndFrequency = CBasics::SwapBytes(static_cast< uint >(mAudioInfo.sndFrequency));
      mAudioInfo.sndNumSamples =
          CBasics::SwapBytes(static_cast< uint >(mAudioInfo.sndNumSamples));
      RS_VERIFY_THROW(527, mAudioInfo.sndNumTracks <= 1, false,
                      CBasics::Stringize("Can't support multiple audio tracks in movie %s",
                                         mDvdFile.GetFilename().data()));
    }
  }
  default:;
  }

  mIndexLoad = nullptr;
  mTextures.reserve(3);
  mNextReadOff = mHeader.movieDataOffsets;
  mNextReadSize = mHeader.firstFrameSize;
  mReadSizeWrapped = mHeader.firstFrameSize;
  mReadOffWrapped = mHeader.movieDataOffsets;
  mTotalSeconds = mHeader.numFrames / mHeader.frameRate;

  if (mPreLoadSeconds < 0.f) {
    mPreLoadSeconds = mTotalSeconds;
    mPreLoadFrames = mHeader.numFrames;

  } else if (mPreLoadSeconds > 0.f) {
    mPreLoadFrames = rstl::min_val(static_cast< uint >(mHeader.numFrames),
                                   static_cast< uint >(mPreLoadSeconds * mHeader.frameRate));

    mPreLoadSeconds = rstl::min_val(mPreLoadSeconds, mTotalSeconds);
  }
  if (mPreLoadFrames > 0) {
    mRequestQueue.reserve(mPreLoadFrames);
  }
  PostDVDReadRequestIfNeeded();
  return false;
}

#if RSTL_PRIME1
CMoviePlayer::~CMoviePlayer() {
  IF_TARGET_PC(const metaforce::AudioLockGuard lock);
  --sNumReferences;
  VerifyCallbackStatus();
  if (sAudioPlayer == this) {
    sAudioPlayer = nullptr;
  }
}
#endif

void CMoviePlayer::InitializeTextures() {
  IF_TARGET_PC(const metaforce::AudioLockGuard lock);

  const uint ySize = OSRoundUp32B(mVideoInfo.xSize * mVideoInfo.ySize);
  const uint uvSize = OSRoundUp32B(mVideoInfo.xSize * mVideoInfo.ySize / 4);
  const uint audioSize = mHeader.audioMaxSamples * 4;
  for (int i = 0; i < mTextures.capacity(); ++i) {
    void* y = rs_aligned_new_line(ySize, 592);
    void* u = rs_aligned_new_line(uvSize, 593);
    void* v = rs_aligned_new_line(uvSize, 594);
    void* audio = rs_aligned_new_line(audioSize, 595);
    DCFlushRangeNoSync(y, ySize);
    DCFlushRangeNoSync(u, uvSize);
    DCFlushRangeNoSync(v, uvSize);
    DCFlushRangeNoSync(audio, audioSize);
#if RSTL_KYOTO_MOVIE_TEXTURE_PUSH_UNSAFE
    mTextures.push_back_unsafe(CTHPTextureSet(y, u, v, audio));
#else
    mTextures.push_back(CTHPTextureSet(y, u, v, audio));
#endif
  }

  PPCSync();
  mDecodedTexSlot = 0;
  mDrawTexSlot = -1;
  mAudioSlot = -1;
}

void CMoviePlayer::PostDVDReadRequestIfNeeded() {
#if !RSTL_PRIME1
  CInterruptGuard interrupts;
#endif

  if (mCurLoadFrame < mHeader.numFrames) {
#if RSTL_PRIME1
    mRequestBuffer = rstl::auto_ptr< uchar >(
        static_cast< uchar* >(CMemory::Alloc(mNextReadSize, IAllocator::kHI_RoundUpLen)));
    mRequest = mDvdFile.AsyncSeekRead(mRequestBuffer.get(), mNextReadSize, kSO_Begin,
                                      static_cast< int >(mNextReadOff));
#else
    bool usedPrefetch = false;
    if (!mPrefetchRequest.null()) {
      if (mPrefetchOff == mNextReadOff && mNextReadSize == mPrefetchSize &&
          mPrefetchFrame == mCurLoadFrame) {
        mRequestBuffer = rstl::auto_ptr< uchar >(mPrefetchBuffer.release());
        mRequest = mPrefetchRequest;
        if (mRequest->IsComplete()) {
          PrefetchNextFrame();
        }
        usedPrefetch = true;
      } else {
        RS_WARNING_PRINT("Movie warning: Mispredicted read!\n");
        mPrefetchRequest = nullptr;
        mPrefetchBuffer = nullptr;
        mPrefetchFrame = mHeader.numFrames;
      }
    }
    if (!usedPrefetch) {
      mRequestBuffer =
          rstl::auto_ptr< uchar >(static_cast< uchar* >(rs_aligned_new_line(mNextReadSize, 653)));
      rstl::single_ptr< CRealDvdRequest > request(rs_new_line(655) CRealDvdRequest);
      DVDOpen(const_cast< char* >(mDvdFile.GetFilename().data()), &request->FileInfo());
      request->FileInfo().cb.userData = this;
      DVDReadAsyncPrio(&request->FileInfo(), mRequestBuffer.get(), mNextReadSize, mNextReadOff,
                       DVDCallback, 2);
      mRequest = request;
    }
#endif
  }
}

#if !RSTL_PRIME1
void CMoviePlayer::DVDCallback(s32 result, DVDFileInfo* info) {
  if (result == 10) {
    return;
  }
  DCInvalidateRange(info->cb.addr, info->cb.length);
  gDvdActivity = true;
  static_cast< CMoviePlayer* >(info->cb.userData)->HandleDVDInterrupt(info);
}

void CMoviePlayer::HandleDVDInterrupt(DVDFileInfo* info) {
  if (!mPrefetchRequest.null() || mRequest.null() || &mRequest->FileInfo() != info) {
    return;
  }
  PrefetchNextFrame();
}

void CMoviePlayer::PrefetchNextFrame() {
  mPrefetchFrame = mCurLoadFrame + 1;
  if (mPrefetchFrame != mHeader.numFrames) {
    mPrefetchSize = CBasics::SwapBytes(*reinterpret_cast< const uint* >(mRequestBuffer.get()));
    mPrefetchOff = mNextReadOff + mNextReadSize;
    mPrefetchBuffer = static_cast< uchar* >(rs_aligned_new_line(mPrefetchSize, 721));
    rstl::single_ptr< CRealDvdRequest > request(rs_new_line(722) CRealDvdRequest);
    DVDOpen(const_cast< char* >(mDvdFile.GetFilename().data()), &request->FileInfo());
    request->FileInfo().cb.userData = this;
    DVDReadAsyncPrio(&request->FileInfo(), mPrefetchBuffer.get(), mPrefetchSize, mPrefetchOff,
                     DVDCallback, 2);
    mPrefetchRequest = request;
  }
}
#endif

void CMoviePlayer::ReadCompleted() {
#if !RSTL_PRIME1
  CInterruptGuard interrupts;
#endif

  mRequest = nullptr;
  if (mCurLoadFrame == mRequestQueue.size() && mPreLoadFrames > mCurLoadFrame) {
#if RSTL_PRIME1
    mRequestQueue.push_back(mRequestBuffer);
#else
    mRequestQueue.push_back_unsafe(mRequestBuffer);
#endif

    mCachedBytes += mNextReadSize;
  }
  mNextReadOff += mNextReadSize;
  mNextReadSize = CBasics::SwapBytes(*reinterpret_cast< const uint* >(mRequestBuffer.get()));
  ++mCurLoadFrame;
  if (mCurLoadFrame == mPreLoadFrames) {
    if (mCurLoadFrame == mHeader.numFrames) {
      mReadSizeWrapped = mHeader.firstFrameSize;
      mReadOffWrapped = mHeader.movieDataOffsets;
    } else {
      mReadSizeWrapped = mNextReadSize;
      mReadOffWrapped = mNextReadOff;
    }
  }
  if (mCurLoadFrame >= mHeader.numFrames && mLoop) {
    mNextReadOff = mReadOffWrapped;
    mNextReadSize = mReadSizeWrapped;
    mCurLoadFrame = mPreLoadFrames;
  }
}

template < typename T >
// ReSharper disable once CppRedundantInlineSpecifier
static inline void KeepDebugArgument(const volatile T& value) {
  (void)value;
}

// Release-side argument-retention hypothesis; original macro name is unknown.
#define MOVIE_DEBUG_WARN(condition, format, value)                                                 \
  do {                                                                                             \
    (void)(condition);                                                                             \
    KeepDebugArgument(value);                                                                      \
  } while (0)

void CMoviePlayer::DecodeFromRead(const void* ptr) {
  uchar work[4096 + 32];
  void* alignedWork = reinterpret_cast< void* >((reinterpret_cast< uintptr_t >(work) + 31) & ~31);
  if (mTextures.empty()) {
    InitializeTextures();
  }
  CTHPTextureSet& texture = mTextures[mDecodedTexSlot];
  const uint* sizes = static_cast< const uint* >(ptr) + 2;
  const uchar* dataStart = static_cast< const uchar* >(ptr) + 8 + mThpComponents.numComponents * 4;
  int offset = 0;
  if (!IS_TARGET_PC) {
    texture.SetAudioSamplesConsumed(0);
    texture.SetAudioSamples(0);
  }
  for (uint i = 0; i < mThpComponents.numComponents; ++i) {
    const uchar* data = dataStart;
    if (mThpComponents.frameComp[i] == 0) {
      const int result = THPVideoDecode(const_cast< uchar* >(data), texture.Y(), texture.U(),
                                        texture.V(), alignedWork);
      if (result != 0) {
        RS_WARNING_PRINT("Error decoding video: %d\n", result);
      }
    } else if (mThpComponents.frameComp[i] == 1) {
      // Publish PCM and its counters together; the mixer may be reading this
      // slot while the game thread decodes the next video frame.
      IF_TARGET_PC(const metaforce::AudioLockGuard lock);

      const uint samples =
          THPAudioDecode(static_cast< short* >(texture.Audio()), const_cast< uchar* >(data), 0);
      const BOOL interrupts = OSDisableInterrupts();
      texture.SetAudioSamples(samples);
      texture.SetAudioSamplesConsumed(0);
      OSRestoreInterrupts(interrupts);
    }
    uint size = CBasics::SwapBytes(*sizes);

    offset += size;
    dataStart += size;
    ++sizes;
  }

#if RSTL_KYOTO_MOVIE_FRAME_SIZE_WARNINGS
  if (mIs60Hz && offset > 33 * 1024) {
    RS_WARNING_PRINT("BUG: 60fps Movie frame size > 33k (%d bytes)!\n", offset);
  } else if (offset > 60 * 1024) {
    RS_WARNING_PRINT("BUG: Movie frame size > 60k (%d bytes)!\n", offset);
  }
#else
  MOVIE_DEBUG_WARN(offset > 55 * 1024, "WARNING: Movie frame size > 55k! %d\n", offset);
#endif
  RSTL_PRIME1_CONST int next = mDecodedTexSlot + 1;

  mDecodedTexSlot = next;
  if (next == mTextures.size()) {
    mDecodedTexSlot = 0;
  }
}

#undef MOVIE_DEBUG_WARN

void CMoviePlayer::Update(RSTL_PRIME1_CONST float dt) {
  RS_VERIFY_THROW(870, !NeedsContinueLoading(), false,
                  "Movie is being updated before it's fully loaded");
  if (mCurLoadFrame < mPreLoadFrames) {
    if (!mRequest.null() && mRequest->IsComplete()) {
      ReadCompleted();
      if (mCurLoadFrame >= mRequestQueue.size() && mCurLoadFrame < mPreLoadFrames &&
          mRequestQueue.size() < mHeader.numFrames) {
        PostDVDReadRequestIfNeeded();
      }
    }
  } else if (!mRequest.null()) {
#if RSTL_PRIME1
    bool canDecode = false;
    if (mRequestFrameWrapped >= mRequestQueue.size() && mCurLoadFrame >= mRequestQueue.size()) {
      canDecode = true;
    }
    if (mRequest->IsComplete() && mDecodedTexCount < 2 && canDecode) {
      DecodeFromRead(mRequestBuffer.get());
      ReadCompleted();

      PostDVDReadRequestIfNeeded();

      ++mDecodedTexCount;
      ++mRequestFrameWrapped;
      if (mRequestFrameWrapped >= mHeader.numFrames && mLoop) {
        mRequestFrameWrapped = 0;
      }
    }
#else
    const bool canDecode =
        mRequestFrameWrapped >= mRequestQueue.size() && mCurLoadFrame >= mRequestQueue.size();
    if (mDecodedTexCount < 2 && canDecode && mRequest->IsComplete()) {
      ReadCompleted();
      rstl::auto_ptr< uchar > buffer;
      if (mCurLoadFrame < mHeader.numFrames) {
        buffer = mRequestBuffer;
      } else {
        buffer = rstl::auto_ptr< uchar >(mRequestBuffer.get());
        buffer.release();
      }
      PostDVDReadRequestIfNeeded();
      DecodeFromRead(buffer.get());

      ++mDecodedTexCount;
      ++mRequestFrameWrapped;
      if (mRequestFrameWrapped >= mHeader.numFrames && mLoop) {
        mRequestFrameWrapped = 0;
      }
    }
#endif
  }
  if (mRequest.null() && mPlayMode == kPM_Playing && mRequestQueue.size() < mHeader.numFrames) {
    PostDVDReadRequestIfNeeded();
  }
  if (mDecodedTexCount < 2 && mPlayMode == kPM_Playing && mRequestFrameWrapped < mPreLoadFrames) {
    const int frame = rstl::min_val(mRequestFrameWrapped, mRequestQueue.size() - 1);
    if (frame == -1) {
      return;
    }
    DecodeFromRead(mRequestQueue[frame].get());
    ++mDecodedTexCount;
    ++mRequestFrameWrapped;
    if (mRequestFrameWrapped >= mHeader.numFrames && mLoop) {
      mRequestFrameWrapped = 0;
    }
  }
  if (mDecodedTexCount > 0 && mPlayMode == kPM_Playing) {
    mCurSeconds += dt;
    if (mLoop) {
      mCurSeconds = CMath::ModF(mCurSeconds, mTotalSeconds);
    } else {
      mCurSeconds = rstl::min_val(mCurSeconds, mTotalSeconds);
    }
    float remainder = mFrameRem - dt;
    const float frameDt = 1.f / mHeader.frameRate;
    if (remainder <= 0.f) {
      if (!mFieldFlip) {
        if (++mDrawTexSlot >= mTextures.size()) {
          mDrawTexSlot = 0;
        }
        const BOOL interrupts = OSDisableInterrupts();
        if (mAudioSlot == -1) {
          mAudioSlot = 0;
        }
        OSRestoreInterrupts(interrupts);
        --mDecodedTexCount;
        ++mCurFrame;
        if (mCurFrame == mHeader.numFrames && mLoop) {
          mCurFrame = 0;
        }
        remainder += frameDt;
        mFieldIndex = 0;
      } else {
        remainder += dt;
        mFieldFlip = false;
      }
    }
    mFrameRem = remainder;
  }
}

#if !RSTL_PRIME1
void CMoviePlayer::DrawFrame(int left, int right, int bottom, int top) {
  const float l = left;
  const float r = right;
  const float b = bottom;
  const float t = top;
  CVector3f v1(l, 0.f, t);
  CVector3f v2(r, 0.f, t);
  CVector3f v3(l, 0.f, b);
  CVector3f v4(r, 0.f, b);
  DrawFrame(v1, v2, v3, v4);
}
#endif

void CMoviePlayer::DrawFrame(const CVector3f& v1, const CVector3f& v2, const CVector3f& v3,
                             const CVector3f& v4) {
  if (mDrawTexSlot == -1) {
    return;
  }
#if IS_TARGET_PC
  const CGraphics::CProjectionState projection = CGraphics::GetProjectionState();
  metaforce::AdjustUiProjection();
#endif
  CGraphics::SetUseVideoFilter(mDeinterlace);
  const BOOL interrupts = OSDisableInterrupts();
  sAudioPlayer = this;
  OSRestoreInterrupts(interrupts);
  CTHPTextureSet& texture = mTextures[mDrawTexSlot];
  const bool field = CGraphics::GetDolphinLastFrameAbove();
#if RSTL_PRIME1
  const bool deinterlace = mDeinterlace;
#endif
  MyTHPGXYuv2RgbSetup(field, IF_RSTL_PRIME1_ELSE(deinterlace, mDeinterlace || mIs60Hz));

  MyTHPYuv2RgbTextureSetup(texture.Y(), texture.U(), texture.V(), mVideoInfo.xSize,
                           mVideoInfo.ySize);
  CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT7, 4);
  GXPosition3f32(v1.GetX(), v1.GetY(), v1.GetZ());
  GXTexCoord2u16(0, 0);
  GXPosition3f32(v3.GetX(), v3.GetY(), v3.GetZ());
  GXTexCoord2u16(0, 1);
  GXPosition3f32(v4.GetX(), v4.GetY(), v4.GetZ());
  GXTexCoord2u16(1, 1);
  GXPosition3f32(v2.GetX(), v2.GetY(), v2.GetZ());
  GXTexCoord2u16(1, 0);
  CGX::End();
  MyTHPGXRestore();
#if IS_TARGET_PC
  CGraphics::SetProjectionState(projection);
#endif
  if (mFieldIndex == 0 && IF_RSTL_PRIME1_ELSE(field, !field && !mIs60Hz)) {
    mFieldFlip = true;
  }
  ++mFieldIndex;
}

void CMoviePlayer::SetPlayMode(const EPlayMode mode) {
  IF_TARGET_PC(const metaforce::AudioLockGuard lock);
  mPlayMode = mode;
}

#if !RSTL_PRIME1
CMoviePlayer::EPlayMode CMoviePlayer::GetPlayMode() const { return mPlayMode; }
#endif

float CMoviePlayer::GetTotalSeconds() const {
  RS_VERIFY_THROW(1133, !NeedsContinueLoading(), false,
                  "::GetTotalSeconds - Movie hasn't finished loading");
  return mTotalSeconds;
}

float CMoviePlayer::GetPlayedSeconds() const {
  RS_VERIFY_THROW(1140, !NeedsContinueLoading(), false,
                  "::GeTPlayedSeconds - Movie hasn't finished loading");
  return mCurSeconds + mFrameRem;
}

float CMoviePlayer::GetCachedSeconds() const {
  RS_VERIFY_THROW(1147, !NeedsContinueLoading(), false,
                  "::GetCachedSeconds - Movie hasn't finished loading");
  return mRequestQueue.size() / mHeader.frameRate;
}

bool CMoviePlayer::GetIsFullyCached() const {
  RS_VERIFY_THROW(1154, !NeedsContinueLoading(), false,
                  "::GetIsFullyCached - Movie hasn't finished loading");
  return mRequestQueue.size() >= mPreLoadFrames;
}

bool CMoviePlayer::GetIsMovieFinishedPlaying() const {
  RS_VERIFY_THROW(1161, !NeedsContinueLoading(), false,
                  "::GetIsMovieFinishedPlaying - Movie hasn't finished loading");
  return !mLoop && mCurFrame == mHeader.numFrames;
}

#if !RSTL_PRIME1
void CMoviePlayer::CancelReadRequests() {
  rstl::single_ptr< CRealDvdRequest > prefetch;
  rstl::single_ptr< CRealDvdRequest > request;
  {
    CInterruptGuard interrupts;
    prefetch = mPrefetchRequest;
    request = mRequest;
  }
  if (!prefetch.null()) {
    prefetch->PostCancelRequest();
    prefetch = nullptr;
  }
  if (!request.null()) {
    request->PostCancelRequest();
    request = nullptr;
  }
}
#endif

void CMoviePlayer::Rewind() {
  RS_VERIFY_THROW(1200, !NeedsContinueLoading(), false, "::Rewind - Movie hasn't finished loading");
#if RSTL_PRIME1
  if (!mRequest.null()) {
    mRequest->PostCancelRequest();
    mRequest = nullptr;
  }
#else
  CancelReadRequests();
#endif
  
  IF_TARGET_PC(const metaforce::AudioLockGuard lock);
#if IS_TARGET_PC
  mAudioPhase = 0;
  mAudioPrimed = false;
#endif

  mRequestBuffer = rstl::auto_ptr< uchar >(nullptr);
#if !RSTL_PRIME1
  mPrefetchBuffer = nullptr;
#endif

  mNextReadSize = mHeader.firstFrameSize;
  mNextReadOff = mHeader.movieDataOffsets;
  mReadSizeWrapped = mHeader.firstFrameSize;
  mReadOffWrapped = mHeader.movieDataOffsets;
  mCurLoadFrame = 0;
  mRequestFrameWrapped = 0;
  mCurFrame = 0;
  mDecodedTexSlot = 0;
  mDrawTexSlot = -1;
  mAudioSlot = -1;
  mDecodedTexCount = 0;
  mFrameRem = 0.f;
  mCurSeconds = 0.f;
  mTextures.clear();
}

void CMoviePlayer::StaticMyAudioCallback(IF_TARGET_PC(short* output, size_t frames, u32 rate, u32 channels)) {
#if IS_TARGET_PC
  CMoviePlayer* player = sAudioPlayer;
  if (!player || player->mPlayMode != kPM_Playing ||
      player->mAudioSlot < 0 || player->mTextures.empty()) {
    return;
  }
  const u32 sourceRate = player->mAudioInfo.sndFrequency;
  if (!sourceRate || sourceRate > 96000) {
    return;
  }

  for (size_t frame = 0; frame < frames; ++frame, output += channels) {
    short audio[2];
    if (sourceRate == rate) {
      player->MixAudio(audio, nullptr, 1);
    } else {
      if (!player->mAudioPrimed) {
        player->MixAudio(player->mAudioHistory[0], nullptr, 1);
        player->MixAudio(player->mAudioHistory[1], nullptr, 1);
        player->mAudioPrimed = true;
      }
      for (u32 channel = 0; channel < 2; ++channel) {
        const s32 a = player->mAudioHistory[0][channel];
        const s32 b = player->mAudioHistory[1][channel];
        audio[channel] = a + (s64(b - a) * player->mAudioPhase) / rate;
      }
      player->mAudioPhase += sourceRate;
      while (player->mAudioPhase >= rate) {
        player->mAudioPhase -= rate;
        for (u32 channel = 0; channel < 2; ++channel) {
          player->mAudioHistory[0][channel] = player->mAudioHistory[1][channel];
        }
        player->MixAudio(player->mAudioHistory[1], nullptr, 1);
      }
    }
    // THPAudioDecode follows the console's R,L ordering. Native output is L,R;
    // movies remain a stereo source in every speaker layout.
    if (sAudioEnabled && sSfxVolume != 0) {
      output[0] = std::clamp(s32(output[0]) + audio[1], -32768, 32767);
      output[1] = std::clamp(s32(output[1]) + audio[0], -32768, 32767);
    }
  }
#else
  if (sAudioPlayer != nullptr && sAudioPlayer->mHasAudio) {
    curAudioBuffer = static_cast< const short* >(OSPhysicalToCached(AIGetDMAStartAddr()));
    soundBufferIndex ^= 1;
    short* buffer = soundBuffer[soundBufferIndex];
    AIInitDMA(reinterpret_cast< uintptr_t >(buffer), sizeof(soundBuffer[0]));
    const BOOL interrupts = OSEnableInterrupts();
    if (curAudioBuffer != nullptr) {
      DCInvalidateRange(const_cast< short* >(curAudioBuffer), sizeof(soundBuffer[0]));
    }
    sAudioPlayer->MixAudio(buffer, curAudioBuffer, 160);
    DCFlushRange(buffer, sizeof(soundBuffer[0]));
    OSRestoreInterrupts(interrupts);
  }
#endif
}

void CMoviePlayer::MixAudio(short* out, const short* in, unsigned long samples) {
  const short* input = in;
  if (mAudioSlot == -1) {
    if (in != nullptr) {
      memcpy(out, in, samples * 4);
    } else {
      memset(out, 0, samples * 4);
    }
    return;
  }
#if RSTL_PRIME1
  const uchar volume = rstl::min_val(127, sSfxVolume * 100 >> 7);
#else
  const uchar volume = rstl::min_val(127, mVolume * sSfxVolume * 100 >> 14);
#endif

  const ushort attenuation =
      sAudioEnabled ? static_cast< ushort >(CAudioSys::GetScaledVolume(volume)) : 0;
  for (int frame = 0; samples != 0 && frame < 3; ++frame) {
    CTHPTextureSet& texture = mTextures[mAudioSlot];
    uint count = texture.GetAudioSamples() - texture.GetAudioSamplesConsumed();
    if (count > samples) {
      count = samples;
    } else {
      if (++mAudioSlot == mTextures.size()) {
        mAudioSlot = 0;
      }
    }
    const uint consumed = texture.GetAudioSamplesConsumed();
    const short* audio = static_cast< short* >(texture.Audio()) + consumed * 2;
    texture.SetAudioSamplesConsumed(count + consumed);
    if (in != nullptr) {
      for (uint i = 0; i < count * 2; ++i) {
        int sample = *input + ((attenuation * *audio) >> 15);
        if (sample < -32768) {
          sample = -32768;
        } else if (sample > 32767) {
          sample = 32767;
        }
        *out = sample;

        ++out;
        ++input;
        ++audio;
      }
    } else {
      for (uint i = 0; i < count * 2; ++i) {
        int sample = (attenuation * *audio) >> 15;
        if (sample < -32768) {
          sample = -32768;
        } else if (sample > 32767) {
          sample = 32767;
        }
        *out = sample;

        ++out;
        ++audio;
      }
    }
    samples -= count;
  }
  if (samples != 0) {
    if (in != nullptr) {
      memcpy(out, input, samples * 4);
    } else {
      memset(out, 0, samples * 4);
    }
  }
}

void CMoviePlayer::VerifyCallbackStatus() {
  if (sNumReferences > 0) {
    CAIInterruptManager::RunDMACallback(StaticMyAudioCallback);

  } else {
    CAIInterruptManager::CancelDMACallback(StaticMyAudioCallback);
  }
}

uint CMoviePlayer::GetWidth() const {
  RS_VERIFY_THROW(1379, !NeedsContinueLoading(), false,
                  "::GetWidth - Movie hasn't finished loading");
  return mVideoInfo.xSize;
}

uint CMoviePlayer::GetHeight() const {
  RS_VERIFY_THROW(1388, !NeedsContinueLoading(), false,
                  "::GetWidth - Movie hasn't finished loading");
  return mVideoInfo.ySize;
}

void CMoviePlayer::SetAudioEnabled(RSTL_PRIME1_CONST bool enabled) {
  IF_TARGET_PC(const metaforce::AudioLockGuard lock);
  sAudioEnabled = enabled;
}

bool CMoviePlayer::GetAudioEnabled() {
  IF_TARGET_PC(const metaforce::AudioLockGuard lock);
  return sAudioEnabled;
}

void CMoviePlayer::SetGlobalMovieVolume(uchar volume) {
  IF_TARGET_PC(const metaforce::AudioLockGuard lock);
  sSfxVolume = rstl::min_val(static_cast< uchar >(127), volume);
}
