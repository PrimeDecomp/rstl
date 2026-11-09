#ifndef _CGRAPHICSPALETTE
#define _CGRAPHICSPALETTE
#include "rstl/RstlVersions.h"

#include "types.h"

#include "dolphin/gx.h"

#include "rstl/single_ptr.hpp"

class CInputStream;

enum EPaletteFormat {
  kPaletteFormat_IA8 = GX_TL_IA8,
  kPaletteFormat_RGB565 = GX_TL_RGB565,
  kPaletteFormat_RGB5A3 = GX_TL_RGB5A3,
};

class CGraphicsPalette {
public:
#if RSTL_KYOTO_PALETTE_GPU_MEMORY
  CGraphicsPalette(EPaletteFormat format, int numEntries, int useGPUMemory = 0);
#else
  CGraphicsPalette(EPaletteFormat format, int numEntries);
#endif
  CGraphicsPalette(CInputStream& in);
  ~CGraphicsPalette();

  GXTlutFmt GetTlutFmt() const { return static_cast< GXTlutFmt >(mFmt); }
  ushort* GetPaletteData();
  const ushort* GetPaletteData() const { return mEntries.get(); }
  EPaletteFormat GetFormat() const { return mFmt; }
  uint GetEntryCount() const { return mEntryCount; }

  void Load() const;
  void* Lock();
  void UnLock();

  static uint sCurrentFrameCount;

private:
#if RSTL_KYOTO_PALETTE_GPU_MEMORY
  static ushort* AllocateEntries(int numEntries, int useGPUMemory); // Guessed name
  void ReleaseEntries();                                            // Guessed name
#endif

  EPaletteFormat mFmt;
  mutable uint mFrameLoaded;
  uint mEntryCount;
  rstl::single_ptr< ushort > mEntries;
  GXTlutObj mTlutObj;
  bool mLocked;
#if RSTL_KYOTO_PALETTE_GPU_MEMORY
  bool mUsesGPUMemory; // Guessed name
#endif
};
CHECK_SIZEOF(CGraphicsPalette, 0x20)

#if !RSTL_KYOTO_PALETTE_GPU_MEMORY
inline ushort* CGraphicsPalette::GetPaletteData() { return mEntries.get(); }
#endif

#if !defined(TARGET_PC)
inline void* CGraphicsPalette::Lock() {
  mLocked = true;
  return GetPaletteData();
}
#endif // !defined(TARGET_PC)

#endif // _CGRAPHICSPALETTE
