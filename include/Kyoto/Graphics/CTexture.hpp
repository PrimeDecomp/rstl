#ifndef _CTEXTURE
#define _CTEXTURE
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/CARAMToken.hpp"
#if !RSTL_PRIME1
#include "Kyoto/SObjectTag.hpp"
#endif

#include "rstl/single_ptr.hpp"

#include "dolphin/gx/GXEnum.h"
#include "dolphin/gx/GXStruct.h"

class CInputStream;

class CResFactory;
class CDvdRequest;
class CGraphicsPalette;

enum ETexelFormat {
  kTF_Invalid = -1,
  kTF_I4 = 0,
  kTF_I8 = 1,
  kTF_IA4 = 2,
  kTF_IA8 = 3,
  kTF_C4 = 4,
  kTF_C8 = 5,
  kTF_C14X2 = 6,
  kTF_RGB565 = 7,
  kTF_RGB5A3 = 8,
  kTF_RGBA8 = 9,
  kTF_CMPR = 10,
};

class CTexture {
public:
  class CDumpedBitmapDataReloader {
    int mState;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    uint mTextureId;
#else
    CAssetId mTextureId;
#endif

    int mResourceSize;
    uint mBitmapSize;
    bool mShouldBeInARAM;
    rstl::single_ptr< CDvdRequest > mRequest;
    rstl::single_ptr< uchar > mData;

  public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CDumpedBitmapDataReloader(uint unk1, uint unk2, bool unk3);
#else
    CDumpedBitmapDataReloader(CAssetId textureId, uint bitmapSize, bool loadToARAM);
#endif

    void BeginReloadBitmapData(CResFactory& factory);
    void* TryBuildReloadedBitmapData(CResFactory& factory);
    int GetStatus() const { return mState; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const bool GetShouldBeInARAM() const { return mShouldBeInARAM; }
#else
    const bool GetShouldBeInARAM() const { return mShouldBeInARAM; } // Guessed name.
#endif

  };

  enum EClampMode {
    kCM_Clamp,
    kCM_Repeat,
    kCM_Mirror,
  };
  enum EAutoMipmap {
    kAM_Zero,
  };
  enum EBlackKey {
    kBK_Zero,
  };

  CTexture(ETexelFormat fmt, short w, short h, int mips);
  CTexture(CInputStream& stream, EAutoMipmap mip, EBlackKey bk);
  ~CTexture();

  // Used in certain destructors
  void ScheduleDeletion();
  void LoadMipLevel(int, GXTexMapID tex, EClampMode) const;
  void Load(GXTexMapID texMapId, EClampMode clampMode) const;
  bool HasPalette() const { return IsCITextureFormat(mTexelFormat); }
  const void* GetConstBitMapData(const int mip) const;
  void* GetBitMapData(int);
  ETexelFormat GetTexelFormat() const { return mTexelFormat; }
  const short GetWidth() const { return mWidth; }
  const short GetHeight() const { return mHeight; }
  int GetNumberOfMipMaps() const { return mNumMips; }
  uint GetMemoryAllocated() const { return mMemoryAllocated; }
  bool GetNoSwap() const { return mNoSwap; }
  void* Lock() {
    mLocked = true;
    return GetBitMapData(0);
  }

  void MakeSwappable() const;
  void CountMemory() const;
  void UncountMemory() const;
  void SetFlag1(bool b) { mLocked = b; }
  void MangleMipmap(int mip);
  const char GetBitsPerPixel() const { return mBitsPerPixel; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void UnloadBitmapData(uint unk) const;
#else
  void UnloadBitmapData(CAssetId textureId) const;
#endif

  bool TryReloadBitmapData(CResFactory& factory) const;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int fn_8030F088() const;
#else
  int GetBitmapDataStatus() const; // Guessed name.
#endif

  bool LoadToMRAM() const;
  bool LoadToARAM() const;
  bool IsARAMTransferInProgress() const;
  static int TexelFormatBitsPerPixel(ETexelFormat fmt);
  void InitBitmapBuffers(const ETexelFormat fmt, const short w, const short h, const int mips);
  void InitTextureObjects();
  void UnLock();
  CGraphicsPalette* GetPalette() { return mGraphicsPalette.get(); }
  const CGraphicsPalette* GetPalette() const { return mGraphicsPalette.get(); }

  static void InvalidateTexmap(GXTexMapID id);
  static bool IsCITextureFormat(ETexelFormat fmt) {
    return fmt == kTF_C4 ? true : fmt == kTF_C8 ? true : fmt == kTF_C14X2 ? true : false;
  }

#if RSTL_PRIME1
  static void SetMangleMipmaps(bool v) { sMangleMips = v; }
  static const bool GetMangleMipmaps() { return sMangleMips; }
#endif

  static int sCurrentFrameCount;
  static int sTotalAllocatedMemory;
  static bool sMangleMips;

private:
#if !RSTL_PRIME1
  void InvalidateTexmaps() const; // Guessed name: clears cache entries for this texture.
#endif

  ETexelFormat mTexelFormat;
  short mWidth;
  short mHeight;
  char mNumMips;
  char mBitsPerPixel;
  bool mLocked : 1;
#if RSTL_PRIME1
  mutable bool mCanLoadPalette : 1;
#endif

  bool mIsPowerOfTwo : 1;
  mutable bool mNoSwap : 1;
  mutable bool mCounted : 1;
  mutable bool mCanLoadObj : 1;
  uint mMemoryAllocated;
  rstl::single_ptr< CGraphicsPalette > mGraphicsPalette;
  mutable rstl::single_ptr< CDumpedBitmapDataReloader > mBitmapReloader;
  GXTexFmt mNativeFormat;
  GXCITexFmt mNativeCIFormat;
  mutable GXTexObj mTexObj;
  mutable EClampMode mClampMode;
  mutable CARAMToken mARAMToken;
  mutable uint mFrameAllocated;
};
CHECK_SIZEOF(CTexture, 0x68)
#if !RSTL_PRIME1
NESTED_CHECK_SIZEOF(CTexture, CDumpedBitmapDataReloader, 0x1c)
#endif

#endif // _CTEXTURE
