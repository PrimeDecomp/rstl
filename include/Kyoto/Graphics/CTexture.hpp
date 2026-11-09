#ifndef _CTEXTURE
#define _CTEXTURE
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/CARAMToken.hpp"
#include "Kyoto/SObjectTag.hpp"

#include "rstl/single_ptr.hpp"

#include "dolphin/gx/GXEnum.h"
#include "dolphin/gx/GXStruct.h"

class CInputStream;

class CResFactory;
class CDvdRequest;
class CGraphicsPalette;

enum ETexelFormat {
  kTexelFormat_Invalid = -1,
  kTexelFormat_I4 = 0,
  kTexelFormat_I8 = 1,
  kTexelFormat_IA4 = 2,
  kTexelFormat_IA8 = 3,
  kTexelFormat_C4 = 4,
  kTexelFormat_CI8 = 5,
  kTexelFormat_C14X2 = 6,
  kTexelFormat_RGB565 = 7,
  kTexelFormat_RGB5A3 = 8,
  kTexelFormat_RGBA8 = 9,
  kTexelFormat_CMPR = 10,
};

class CTexture {
public:
  class CDumpedBitmapDataReloader {
    int mState;
    CAssetId mTextureId;

    int mResourceSize;
    uint mBitmapSize;
    bool mShouldBeInARAM;
    rstl::single_ptr< CDvdRequest > mRequest;
    rstl::single_ptr< uchar > mData;

  public:
    CDumpedBitmapDataReloader(CAssetId textureId, uint bitmapSize, bool loadToARAM);

    void BeginReloadBitmapData(CResFactory& factory);
    void* TryBuildReloadedBitmapData(CResFactory& factory);
    int GetStatus() const { return mState; }
    CAssetId GetTextureId() const { return mTextureId; } // Guessed name
    const bool GetShouldBeInARAM() const { return mShouldBeInARAM; }

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
  // Guessed name
  enum EBitmapState {
    kBitmapState_Unloaded = 2,
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
  void* Lock();
  void MakeSwappable() const;
  void CountMemory() const;
  void UncountMemory() const;
  void SetFlag1(bool b) { mLocked = b; }
  void MangleMipmap(int mip);
  const char GetBitsPerPixel() const { return mBitsPerPixel; }
  void UnloadBitmapData(CAssetId textureId) const;

  bool TryReloadBitmapData(CResFactory& factory) const;
  int GetBitmapDataStatus() const; // Guessed name.
  EBitmapState GetBitmapState() const;

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
    return fmt == kTexelFormat_C4 ? true : fmt == kTexelFormat_CI8 ? true : fmt == kTexelFormat_C14X2 ? true : false;
  }

  static void SetMangleMipmaps(bool v) { sMangleMips = v; }
  static const bool GetMangleMipmaps() { return sMangleMips; }

  static int sCurrentFrameCount;
  static int sTotalAllocatedMemory;
  static bool sMangleMips;
  static uint sTotalAllocatedSecondaryMemory; // Guessed name
  static void SetBindCount(int count);        // Guessed name
  static int GetBindCount();                  // Guessed name
  static bool sReviewTextureSize;             // Guessed name
  static bool sCapTextureSize;                // Guessed name

private:
  void InvalidateTexmaps() const;         // Guessed name: clears cache entries for this texture.
  void VerifyBitmapDataAvailable() const; // Guessed name

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
#if RSTL_KYOTO_TEXTURE_SECONDARY_MEMORY
  uint mSecondaryMemoryAllocated; // Guessed name
#endif
  rstl::single_ptr< CGraphicsPalette > mGraphicsPalette;
  mutable rstl::single_ptr< CDumpedBitmapDataReloader > mBitmapReloader;
  GXTexFmt mNativeFormat;
  GXCITexFmt mNativeCIFormat;
  mutable GXTexObj mTexObj;
  mutable EClampMode mClampMode;
  mutable CARAMToken mARAMToken;
  mutable uint mFrameAllocated;
};
CHECK_SIZEOF(CTexture, RSTL_KYOTO_TEXTURE_SECONDARY_MEMORY ? 0x6c : 0x68)
NESTED_CHECK_SIZEOF(CTexture, CDumpedBitmapDataReloader, RSTL_KYOTO_ASSET_ID_CLASS ? 0x28 : 0x1c)

#if !defined(TARGET_PC)
inline void* CTexture::Lock() {
  mLocked = true;
  return GetBitMapData(0);
}
#endif

#endif // _CTEXTURE
