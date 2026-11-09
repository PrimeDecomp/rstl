#include "Kyoto/Graphics/CTexture.hpp"

#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Alloc/IAllocator.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CARAMToken.hpp"
#include "Kyoto/CDvdRequest.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#if RSTL_KYOTO_CORRUPTION
#include "Corruption/Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CGraphicsPalette.hpp"
#else
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CGraphicsPalette.hpp"
#endif
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Platform.h"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#if RSTL_KYOTO_BUFFER_FACTORIES
#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#endif
#include "rstl/RstlVersions.h"
#include "rstl/single_ptr.hpp"

#include "dolphin/base/PPCArch.h"
#include "dolphin/gx/GXEnum.h"
#include "dolphin/gx/GXStruct.h"
#include "dolphin/gx/GXTexture.h"
#include "dolphin/os.h"
#include "dolphin/os/OSCache.h"
#include "stdint.h"
#include "types.h"

#if RSTL_PRIME1
#include "Prime/MetroidPrime/CGameDebug.hpp"
#endif
#ifdef ENABLE_BBA_TOOLS
#include "Kyoto/Network/CBBASupport.hpp"
#endif

int CTexture::sCurrentFrameCount = 0;
int CTexture::sTotalAllocatedMemory = 0;
#if RSTL_KYOTO_TEXTURE_SECONDARY_MEMORY
uint CTexture::sTotalAllocatedSecondaryMemory = 0;
#endif
bool CTexture::sMangleMips = false;
#if !RSTL_PRIME1
static int sTextureLoadCount;
#endif
#if RSTL_KYOTO_TEXTURE_SECONDARY_MEMORY
bool CTexture::sCapTextureSize = false;
#endif

#define ROUND_UP_4(v) (((v) + 3) & ~3)

#if RSTL_PRIME1
static const CTexture* sLoadedTextures[GX_MAX_TEXMAP];
#else
static uintptr_t sLoadedTextureStorage[GX_MAX_TEXMAP];
static uintptr_t* sLoadedTextures = sLoadedTextureStorage;
#endif

CTexture::CTexture(ETexelFormat fmt, const short w, const short h, int mips)
: mTexelFormat(fmt)
, mWidth(w)
, mHeight(h)
, mNumMips(mips)
, mBitsPerPixel(TexelFormatBitsPerPixel(fmt))
, mLocked(false)
#if RSTL_PRIME1
, mCanLoadPalette(false)
#endif

, mIsPowerOfTwo(false)
, mNoSwap(true)
, mCounted(false)
#if RSTL_PRIME1
, mCanLoadObj(false)
#else
, mCanLoadObj(true)
#endif

, mMemoryAllocated(0)
#if RSTL_PRIME1
, mFrameAllocated(sCurrentFrameCount)
#if NONMATCHING
, mTexObj()
#endif
#endif
#if RSTL_KYOTO_TEXTURE_SECONDARY_MEMORY
, mSecondaryMemoryAllocated(0)
#endif

, mNativeFormat(GX_TF_RGB565)
, mNativeCIFormat(GX_TF_C8)
, mClampMode(kCM_Repeat)
#if !RSTL_PRIME1
, mFrameAllocated(sCurrentFrameCount)
#endif
{

  InitBitmapBuffers(fmt, w, h, mips);
  InitTextureObjects();
}

CTexture::CTexture(CInputStream& in, EAutoMipmap automip, EBlackKey blackKey)
: mTexelFormat(kTexelFormat_Invalid)
, mWidth(0)
, mHeight(0)
, mNumMips(0)
, mBitsPerPixel(0)
, mLocked(false)
#if RSTL_PRIME1
, mCanLoadPalette(false)
#endif

, mIsPowerOfTwo(false)
, mNoSwap(true)
, mCounted(false)
#if RSTL_PRIME1
, mCanLoadObj(false)
#else
, mCanLoadObj(true)
#endif

, mMemoryAllocated(0)
#if RSTL_PRIME1
, mFrameAllocated(sCurrentFrameCount)
#if NONMATCHING
, mTexObj()
#endif
#endif
#if RSTL_KYOTO_TEXTURE_SECONDARY_MEMORY
, mSecondaryMemoryAllocated(0)
#endif

, mNativeFormat(GX_TF_RGB565)
, mNativeCIFormat(GX_TF_C8)
, mClampMode(kCM_Repeat)
#if !RSTL_PRIME1
, mFrameAllocated(sCurrentFrameCount)
#endif
{

  mTexelFormat = ETexelFormat(in.Get< uint >());
  mWidth = in.ReadUint16();
  mHeight = in.ReadUint16();
  mNumMips = in.ReadInt32();

  if (IsCITextureFormat(mTexelFormat)) {
    mGraphicsPalette = rs_new_line(131) CGraphicsPalette(in);
#if RSTL_PRIME1
    mCanLoadPalette = true;
#endif
  }
  mBitsPerPixel = TexelFormatBitsPerPixel(mTexelFormat);
#if RSTL_PRIME1
  DEBUG_TOOLS_CALL(PrepareTextureLoad(in, mWidth, mHeight, mNumMips, mBitsPerPixel));
#endif

#if RSTL_KYOTO_TEXTURE_SECONDARY_MEMORY
  const int origWidth = mWidth;
  const int origHeight = mHeight;
  int skippedBytes;
  const int bitsPerPixel = mBitsPerPixel;
  if (sCapTextureSize) {
    skippedBytes = 0;
    while (mNumMips > 1 && mWidth * mHeight > 0x10000) {
      skippedBytes += (bitsPerPixel * (mWidth * mHeight)) >> 3;
      mWidth >>= 1;
      mHeight >>= 1;
      --mNumMips;
    }
  } else {
    skippedBytes = 0;
  }

  mSecondaryMemoryAllocated = skippedBytes;
  if (mSecondaryMemoryAllocated != 0) {
    RS_WARNING_DEBUGGER_PRINT("Dynamically resizing texture from %dx%d to %dx%d\n", origWidth,
                              origHeight, mWidth, mHeight);
    in.ReadBytes(nullptr, mSecondaryMemoryAllocated);
  }

  InitBitmapBuffers(GetTexelFormat(), GetWidth(), GetHeight(), GetNumberOfMipMaps());
#else
  InitBitmapBuffers(mTexelFormat, mWidth, mHeight, mNumMips);
#endif
  int bufLen = 0;
  for (int i = 0; i < mNumMips;) {
    int width = ((GetWidth() >> i) + 3) & ~3;
    int height = ((GetHeight() >> i) + 3) & ~3;
    int page = width * height;
    i++;
    bufLen += (GetBitsPerPixel() * page) >> 3;
  }

  uchar* buf = static_cast< uchar* >(mARAMToken.GetMRAMSafe());

  for (int off = 0, len = 0; off < bufLen; off += len) {
    len = bufLen - off;
    if (len > 256) {
      len = 256;
    }

    in.Get(buf + off, len);
    DCFlushRangeNoSync(buf + off, OSRoundUp32B(len));
  }

  if (sMangleMips != false) {
    for (int i = 1; i < mNumMips; ++i) {
      MangleMipmap(i);
    }
#if RSTL_PRIME1
    if (DEBUG_TOOLS_GET_BOOL(kDO_ReviewTextureSize, false))
      DCFlushRangeNoSync(buf, OSRoundUp32B(bufLen));
#endif
  }

  InitTextureObjects();
  PPCSync();
}

CTexture::~CTexture() {
  if (IS_TARGET_PC) {
    ScheduleDeletion();
  }
  UncountMemory();
}

void CTexture::InitTextureObjects() {
#if IS_TARGET_PC
  GXDestroyTexObj(&mTexObj);
#endif

  mIsPowerOfTwo =
      CMath::FloorPowerOfTwo(mWidth) == mWidth && CMath::FloorPowerOfTwo(mHeight) == mHeight;

  if (!mIsPowerOfTwo) {
    mClampMode = kCM_Clamp;
  }
  bool hasMips = mNumMips > 1;
  GXTexWrapMode wrap = static_cast< GXTexWrapMode >(mClampMode);

  short width = mWidth;
  short height = mHeight;
  void* buf = mARAMToken.GetMRAMSafe();
  CountMemory();

  if (IsCITextureFormat(mTexelFormat)) {
    GXInitTexObjCI(&mTexObj, buf, width, height, mNativeCIFormat, wrap, wrap, hasMips, 0);
  } else {
    GXInitTexObj(&mTexObj, buf, width, height, mNativeFormat, wrap, wrap, hasMips);
    GXInitTexObjLOD(&mTexObj, mNumMips > 1 ? GX_LIN_MIP_LIN : GX_LINEAR, GX_LINEAR, 0.f,
                    mNumMips - 1.f, 0.f, false, false, mNumMips > 1 ? GX_ANISO_4 : GX_ANISO_1);
  }

  mCanLoadObj = true;
#if !RSTL_PRIME1
  InvalidateTexmaps();
#endif
}

void CTexture::Load(GXTexMapID tex, EClampMode clamp) const {
  // Prime 2: The low address bits distinguish the three clamp modes in the cache.
  if (IS_TARGET_PC ||
      IF_RSTL_PRIME1_ELSE((sLoadedTextures[tex] != this || mCanLoadObj),
                          (reinterpret_cast< uintptr_t >(this) + clamp != sLoadedTextures[tex]))) {

    void* ptr;
    if (IS_TARGET_PC || RSTL_PRIME1 || mCanLoadObj) {
#if RSTL_KYOTO_DEBUG_BUILD
      if (mARAMToken.GetStatus() == CARAMToken::kStatus_Unowned) {
        VerifyBitmapDataAvailable();
      }
#endif
      IF_NOT_TARGET_PC(ptr = mARAMToken.GetMRAMSafe());
      CountMemory();

      if (!mGraphicsPalette.null()) {
        mGraphicsPalette->Load();
        IF_RSTL_PRIME1(mCanLoadPalette = false);
      }
#if !RSTL_PRIME1
      else {
        mCanLoadObj = false;
      }
      IF_NOT_TARGET_PC(GXInitTexObjData(&mTexObj, ptr));
#endif
    }
    IF_RSTL_PRIME1(mCanLoadObj = false);

#if RSTL_PRIME1
    if (mClampMode != clamp) {
      if (!mIsPowerOfTwo) {
        mClampMode = kCM_Clamp;
      } else {
        mClampMode = clamp;
      }
      GXInitTexObjWrapMode(&mTexObj, GXTexWrapMode(mClampMode), GXTexWrapMode(mClampMode));
    }
    IF_NOT_TARGET_PC(GXInitTexObjData(&mTexObj, ptr));
    GXLoadTexObj(&mTexObj, tex);
    sLoadedTextures[tex] = this;
    mFrameAllocated = sCurrentFrameCount;
#else
    if (mClampMode != clamp && mIsPowerOfTwo) {
      mClampMode = clamp;
      GXInitTexObjWrapMode(&mTexObj, GXTexWrapMode(mClampMode), GXTexWrapMode(mClampMode));
    }
    GXLoadTexObj(&mTexObj, tex);
    sLoadedTextures[tex] = reinterpret_cast< uintptr_t >(this) + clamp;
    mFrameAllocated = sCurrentFrameCount;
    ++sTextureLoadCount;
#endif
  }
}

void CTexture::LoadMipLevel(int mip, GXTexMapID tex, EClampMode clamp) const {
  RS_VERIFY_THROW(313, mARAMToken.GetStatus() != CARAMToken::kStatus_Unowned, false,
                  "Cannot load a texture whose bitmap data has been dumped from memory!  "
                  "Please bug this!");
  char* ptr = static_cast< char* >(mARAMToken.GetMRAMSafe());

  GXTexObj obj = mTexObj;
  int width = mWidth;
  int height = mHeight;

  int offset = 0;
  GXTexWrapMode wrap = static_cast< GXTexWrapMode >(clamp);

  for (int i = 0; i < mip; i++) {
    int w = ROUND_UP_4(width);
    int h = ROUND_UP_4(height);
    offset += OSRoundUp32B(((mBitsPerPixel * (w * h)) / 8));
    width /= 2;
    height /= 2;
  }

  GXInitTexObj(&obj, ptr + offset, width, height, mNativeFormat, wrap, wrap, false);
  GXInitTexObjLOD(&obj, GX_LINEAR, GX_LINEAR, 0.f, 0.f, 0.f, false, false, GX_ANISO_1);
  if (!mGraphicsPalette.null()) {
    mGraphicsPalette->Load();
#if RSTL_PRIME1
    mCanLoadPalette = false;
#endif
  }

  GXLoadTexObj(&obj, tex);
  sLoadedTextures[tex] = 0;

  mFrameAllocated = sCurrentFrameCount;
}

void CTexture::UnloadBitmapData(CAssetId textureId) const {
  if (IS_TARGET_PC) {
    return;
  }
#ifdef ENABLE_BBA_TOOLS
  if (kInvalidAssetId != CBBASupport::FindNetworkAsset(textureId).mTag.GetId()) {
    RS_DEBUGGER_PRINT("Warning: Texture Asset id %s not unloaded because it is a network asset.\n",
                      textureId.ToHexString().c_str());
    return;
  }
#endif

  if (!mBitmapReloader.null()) {
    bool loadToARAM = mBitmapReloader->GetShouldBeInARAM();
    mBitmapReloader =
        rs_new_line(376) CDumpedBitmapDataReloader(textureId, mMemoryAllocated, loadToARAM);

  } else {
    bool complete = mARAMToken.GetStatus() == CARAMToken::kS_Zero ||
                    mARAMToken.GetStatus() == CARAMToken::kS_Two ||
                    mARAMToken.GetStatus() == CARAMToken::kS_Five;

    mARAMToken = CARAMToken();
    mBitmapReloader =
        rs_new_line(385) CDumpedBitmapDataReloader(textureId, mMemoryAllocated, complete);
#if !RSTL_PRIME1
    mCanLoadObj = true;
#endif
  }
}

bool CTexture::TryReloadBitmapData(CResFactory& factory) const {
  if (mBitmapReloader.null()) {
    return true;
  }

#if !RSTL_PRIME1
  mCanLoadObj = true;
#endif

  mBitmapReloader->BeginReloadBitmapData(factory);
  uchar* ptr = static_cast< uchar* >(mBitmapReloader->TryBuildReloadedBitmapData(factory));

  if (ptr != nullptr) {
    bool loadToARAM = mBitmapReloader->GetShouldBeInARAM();

    mBitmapReloader = nullptr;

    mARAMToken.PostConstruct(ptr, mMemoryAllocated, 1);
    const_cast< CTexture& >(*this).InitTextureObjects();

    if (loadToARAM) {

      LoadToARAM();
    }

    return true;
  }
  return false;
}

CTexture::EBitmapState CTexture::GetBitmapState() const {
  if (!mBitmapReloader.null()) {
    if (mBitmapReloader->GetStatus() == 0) {
      return kBitmapState_Unloaded;
    }

    return static_cast< EBitmapState >(5);
  }

  switch (mARAMToken.GetStatus()) {
  case CARAMToken::kS_Zero:
    return static_cast< EBitmapState >(1);
  case CARAMToken::kS_One:
    return static_cast< EBitmapState >(0);
  case CARAMToken::kS_Five:
  case CARAMToken::kS_Two:
    return static_cast< EBitmapState >(3);
  case CARAMToken::kS_Three:
  case CARAMToken::kS_Four:
    return static_cast< EBitmapState >(4);
  default:
    return static_cast< EBitmapState >(-1);
  }
}

CTexture::CDumpedBitmapDataReloader::CDumpedBitmapDataReloader(CAssetId textureId, uint bitmapSize,
                                                               bool loadToARAM)
: mState(0)
, mTextureId(textureId)
, mResourceSize(0)
, mBitmapSize(bitmapSize)
, mShouldBeInARAM(loadToARAM) {}

void CTexture::CDumpedBitmapDataReloader::BeginReloadBitmapData(CResFactory& factory) {
  if (mState != 0) {
    return;
  }
  SObjectTag tag('TXTR', mTextureId);
  mResourceSize = factory.ResourceSize(tag);
  mData = static_cast< uchar* >(rs_aligned_new_line(mResourceSize, 488));
  mRequest = factory.GetResLoader().LoadResourceAsync(tag, reinterpret_cast< char* >(mData.get()));

  mState = 1;
}

void* CTexture::CDumpedBitmapDataReloader::TryBuildReloadedBitmapData(CResFactory& factory) {
  if (mRequest->IsComplete()) {
    mState = 2;
    mRequest = nullptr;

    SObjectTag tag('TXTR', mTextureId);
    rstl::single_ptr< CInputStream > buf =
        factory.GetResLoader().LoadResourceFromMemorySync(tag, mData.get());

    CInputStream& in = *buf;
    ETexelFormat format = ETexelFormat(in.ReadInt32());
#if RSTL_PRIME1
    int w = in.ReadInt16();
    int h = in.ReadInt16();
    int numMips = in.ReadInt32();
#elif RSTL_KYOTO_TEXTURE_SECONDARY_MEMORY
    short w = in.ReadUint16();
    short h = in.ReadUint16();
    char numMips = in.Get< uint >();
#else
    const short w = in.Get< short >();
    const short h = in.ReadInt16();
    const int numMips = in.Get< int >();
#endif

    const int bitsPerPixel = TexelFormatBitsPerPixel(format);
#if RSTL_KYOTO_TEXTURE_SECONDARY_MEMORY
    if (sCapTextureSize) {
      while (numMips > 1 && w * h > 0x10000) {
        w >>= 1;
        --numMips;
        h >>= 1;
      }
    }
#endif

    if (IsCITextureFormat(format)) {
      CGraphicsPalette tmp(in);
    }

#if RSTL_PRIME1
    DEBUG_TOOLS_CALL(PrepareTextureReload(in, w, h, numMips, bitsPerPixel, mBitmapSize));
#endif

    int bufLen = 0;
    for (int i = 0; i < numMips;) {
      int width = ((w >> i) + 3) & ~3;
      int height = ((h >> i) + 3) & ~3;
      int page = (width * height);
      i++;
      bufLen += (page * bitsPerPixel) >> 3;
    }

    void* ptr = rs_aligned_new_line(mBitmapSize, 540);

    for (int off = 0, len = 0; off < bufLen; off += len) {
      len = bufLen - off;
      if (len > 256) {
        len = 256;
      }

      in.Get(static_cast< char* >(ptr) + off, len);
      DCFlushRangeNoSync(static_cast< char* >(ptr) + off, OSRoundUp32B(len));
    }
    PPCSync();
    mData = nullptr;
    return ptr;
  }
  return nullptr;
}

bool CTexture::LoadToMRAM() const {
#if !RSTL_PRIME1
  if (mARAMToken.GetStatus() == CARAMToken::kS_One) {
    return true;
  }

  mCanLoadObj = true;
#endif

  if (mARAMToken.GetStatus() == CARAMToken::kStatus_Unowned) {
    return false;
  }

  mFrameAllocated = sCurrentFrameCount;
  CountMemory();
  return mARAMToken.LoadToMRAM();
}

bool CTexture::LoadToARAM() const {
  if (IS_TARGET_PC) {
    return mARAMToken.LoadToARAM();
  }

#if !RSTL_PRIME1
  mCanLoadObj = true;
#endif

  if (mARAMToken.GetStatus() == CARAMToken::kStatus_Unowned) {
    return false;
  }

  if (mNoSwap) {
    return false;
  }

  if (mFrameAllocated < sCurrentFrameCount - 1) {
    bool ret = mARAMToken.LoadToARAM();

    if (mARAMToken.GetStatus() != CARAMToken::kS_One) {
      UncountMemory();
#if RSTL_PRIME1
      mCanLoadObj = true;
#else
      InvalidateTexmaps();
#endif
    }
    return ret;
  }
  return false;
}

bool CTexture::IsARAMTransferInProgress() const {
  if (IS_TARGET_PC) {
    return false;
  }

  if (mNoSwap) {
    return false;
  }
  return mARAMToken.GetStatus() >= CARAMToken::kS_Two &&
         mARAMToken.GetStatus() <= CARAMToken::kS_Five;
}

int CTexture::TexelFormatBitsPerPixel(ETexelFormat fmt) {
  switch (fmt) {
  case kTexelFormat_I4:
  case kTexelFormat_C4:
  case kTexelFormat_CMPR:
    return 4;
  case kTexelFormat_I8:
  case kTexelFormat_IA4:
  case kTexelFormat_CI8:
    return 8;
  case kTexelFormat_IA8:
  case kTexelFormat_C14X2:
  case kTexelFormat_RGB565:
  case kTexelFormat_RGB5A3:
    return 16;
  case kTexelFormat_RGBA8:
    return 32;
  default:
    return 0;
  }
}

void CTexture::InitBitmapBuffers(ETexelFormat fmt, short width, short height, int mips) {
  switch (fmt) {
  case kTexelFormat_C4:
    mNativeCIFormat = GX_TF_C4;
    break;
  case kTexelFormat_CI8:
    mNativeCIFormat = GX_TF_C8;
    break;
  case kTexelFormat_C14X2:
    mNativeCIFormat = GX_TF_C14X2;
    break;
  case kTexelFormat_I4:
    mNativeFormat = GX_TF_I4;
    break;
  case kTexelFormat_I8:
    mNativeFormat = GX_TF_I8;
    break;
  case kTexelFormat_IA4:
    mNativeFormat = GX_TF_IA4;
    break;
  case kTexelFormat_IA8:
    mNativeFormat = GX_TF_IA8;
    break;
  case kTexelFormat_RGB565:
    mNativeFormat = GX_TF_RGB565;
    break;
  case kTexelFormat_RGB5A3:
    mNativeFormat = GX_TF_RGB5A3;
    break;
  case kTexelFormat_RGBA8:
    mNativeFormat = GX_TF_RGBA8;
    break;
  case kTexelFormat_CMPR:
    mNativeFormat = GX_TF_CMPR;
    break;
  default:
    break;
  }

  bool hasMips = mips > 1;
  mMemoryAllocated = GXGetTexBufferSize(
      width, height, HasPalette() ? mNativeCIFormat : mNativeFormat, hasMips, hasMips ? 11 : 0);
#if !RSTL_PRIME1
  mCanLoadObj = true;
#endif

  mARAMToken.PostConstruct(rs_aligned_new_line(mMemoryAllocated, 732), mMemoryAllocated, 1);
  CountMemory();
}

#if defined(TARGET_PC)
void* CTexture::Lock() {
  // AuroraGXSync();
  void* data = GetBitMapData(0);
  GXDestroyCopyTex(data);
  mLocked = true;
  return data;
}
#endif

void CTexture::UnLock() {
  mLocked = false;
#if !RSTL_PRIME1
  mCanLoadObj = true;
#endif
  CountMemory();
#if defined(TARGET_PC)
  GXInitTexObjData(&mTexObj, mARAMToken.GetMRAMSafe());
  mCanLoadObj = true;
#endif
  DCFlushRange(mARAMToken.GetMRAMSafe(), OSRoundUp32B(mMemoryAllocated));
}

#if RSTL_KYOTO_BUFFER_FACTORIES
CFactoryFnReturn FTextureFactory(const SObjectTag& tag, const rstl::auto_ptr< uchar >& data,
                                 int length, const CVParamTransfer& xfer) {
  CMemoryInStream in(data.get(), length);
  return rs_new_line(750) CTexture(in, CTexture::kAM_Zero, CTexture::kBK_Zero);
}
#else
RSTL_PRIME1_CONST CFactoryFnReturn FTextureFactory(const SObjectTag& tag, CInputStream& in,
                                                   const CVParamTransfer& xfer) {
  return rs_new CTexture(in, CTexture::kAM_Zero, CTexture::kBK_Zero);
}
#endif

const void* CTexture::GetConstBitMapData(const int mip) const {
  int offset = 0;
  for (int i = 0; i < mip; i++) {
    offset += (GetBitsPerPixel() >> 3) * ((GetWidth() >> i) * (GetHeight() >> i));
  }

  return static_cast< const uchar* >(mARAMToken.GetMRAMSafe()) + offset;
}

void* CTexture::GetBitMapData(int mip) { return const_cast< void* >(GetConstBitMapData(mip)); }

void CTexture::MangleMipmap(int mip) {
  if (mip >= mNumMips) {
    return;
  }

  const uint colors[4] = {
      0x000000FF,
      0x0000FF00,
      0x00FF0000,
      0x0000FFFF,
  };
  const uint color = colors[(mip - 1) & 3];
  ushort rgb565Color = ((color >> 3) & 0x001F) | // B
                       ((color >> 5) & 0x07E0) | // G
                       ((color >> 8) & 0xF800);  // R
  ushort rgb555Color = ((color >> 3) & 0x001F) | // B
                       ((color >> 6) & 0x03E0) | // G
                       ((color >> 9) & 0x7C00);  // R
  ushort rgb4Color = ((color >> 4) & 0x000F) |   // B
                     ((color >> 8) & 0x00F0) |   // G
                     ((color >> 12) & 0x0F00);   // R

  int width = mWidth;
  int height = mHeight;

  int offset = 0;
  for (int i = 0; i < mip; i++) {
    offset += width * height;
    width /= 2;
    height /= 2;
  }

  switch (GetTexelFormat()) {
  case kTexelFormat_RGB565: {
    ushort* ptr = static_cast< ushort* >(mARAMToken.GetMRAMSafe());
    for (int i = 0; i < width * height; ++i) {
      ptr[i + offset] = rgb565Color;
    }
    break;
  }
  case kTexelFormat_CMPR: {
    ushort* ptr = static_cast< ushort* >(mARAMToken.GetMRAMSafe()) + offset / 4;
    for (int i = 0; i < width * height / 16; ++i, ptr += 4) {
      ptr[0] = rgb565Color;
      ptr[1] = rgb565Color;
      ptr[2] = 0;
      ptr[3] = 0;
    }
    break;
  }
  case kTexelFormat_RGB5A3: {
    ushort* ptr = static_cast< ushort* >(mARAMToken.GetMRAMSafe());
    for (int i = 0; i < width * height; ++i) {
      ushort& val = ptr[i + offset];
      if (val & 0x8000) {
        val = rgb555Color | 0x8000;
      } else {
        val = (val & 0xF000) | rgb4Color;
      }
    }
    break;
  }
  }
}

void CTexture::MakeSwappable() const {
  if (!mNoSwap) {
    return;
  }

  mNoSwap = false;
}

void CTexture::CountMemory() const {
  if (mCounted) {
    return;
  }

  mCounted = true;
  sTotalAllocatedMemory += mMemoryAllocated;
#if RSTL_KYOTO_TEXTURE_SECONDARY_MEMORY
  sTotalAllocatedSecondaryMemory += mSecondaryMemoryAllocated;
#endif
}

void CTexture::UncountMemory() const {
  if (!mCounted) {
    return;
  }

  mCounted = false;
#if !RSTL_PRIME1
  mCanLoadObj = true;
#endif

  sTotalAllocatedMemory -= mMemoryAllocated;
#if RSTL_KYOTO_TEXTURE_SECONDARY_MEMORY
  sTotalAllocatedSecondaryMemory -= mSecondaryMemoryAllocated;
#endif
}

void CTexture::InvalidateTexmap(GXTexMapID texmap) { sLoadedTextures[texmap] = 0; }

void CTexture::ScheduleDeletion() {
#if IS_TARGET_PC
  GXDestroyTexObj(&mTexObj);
  for (auto& tex : sLoadedTextures) {
    if (tex == this) {
      tex = nullptr;
    }
  }
#endif

  if (mARAMToken.GetStatus() != CARAMToken::kStatus_Unowned) {
    IF_TARGET_PC(GXDestroyCopyTex(mARAMToken.GetMRAMSafe()));
    CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_NextFrame,
                                          mARAMToken.ForceSyncMRAM());
  }
}

#if !RSTL_PRIME1
void CTexture::InvalidateTexmaps() const {
  for (int i = 0; i < GX_MAX_TEXMAP; ++i) {
    if (sLoadedTextures[i] == reinterpret_cast< uintptr_t >(this) + mClampMode) {
      sLoadedTextures[i] = 0;
    }
  }
}

#if RSTL_KYOTO_DEBUG_BUILD
void CTexture::VerifyBitmapDataAvailable() const {
  if (mBitmapReloader.null()) {
    RS_VERIFY_THROW(941, mARAMToken.GetStatus() != CARAMToken::kStatus_Unowned, false,
                    "Cannot load a texture whose bitmap data has been dumped from memory!  "
                    "Please bug this!");
  } else {
    CAssetId textureId = mBitmapReloader->GetTextureId();
    RS_VERIFY_THROW(947, mARAMToken.GetStatus() != CARAMToken::kStatus_Unowned, false,
                    CBasics::Stringize("Tried to load texture (id %s) whose bitmap data has "
                                       "been dumped from memory!  Please bug this!",
                                       textureId.ToHexString().c_str()));
  }
}
#endif

int CTexture::GetBindCount() { return sTextureLoadCount; }

void CTexture::SetBindCount(int count) { sTextureLoadCount = count; }
#endif
