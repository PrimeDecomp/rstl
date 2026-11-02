#ifndef _CGRAPHICSPALETTE
#define _CGRAPHICSPALETTE
#include "rstl/RstlVersions.h"

#include "types.h"

#include "dolphin/gx.h"

#include "rstl/single_ptr.hpp"

class CInputStream;

enum EPaletteFormat {
  kPF_IA8 = GX_TL_IA8,
  kPF_RGB565 = GX_TL_RGB565,
  kPF_RGB5A3 = GX_TL_RGB5A3,
};

class CGraphicsPalette {
public:
  CGraphicsPalette(EPaletteFormat format, int numEntries);
  CGraphicsPalette(CInputStream& in);
  ~CGraphicsPalette();

  GXTlutFmt GetTlutFmt() const { return static_cast< GXTlutFmt >(mFmt); }
  ushort* GetPaletteData() { return mEntries.get(); }
  const ushort* GetPaletteData() const { return mEntries.get(); }
  EPaletteFormat GetFormat() const { return mFmt; }
  uint GetEntryCount() const { return mEntryCount; }

  void Load() const;
  void* Lock() {
    mLocked = true;
    return mEntries.get();
  }
  void UnLock();

  static uint sCurrentFrameCount;

private:
  EPaletteFormat mFmt;
  mutable uint mFrameLoaded;
  uint mEntryCount;
  rstl::single_ptr< ushort > mEntries;
  GXTlutObj mTlutObj;
  bool mLocked;
};
CHECK_SIZEOF(CGraphicsPalette, 0x20)

#endif // _CGRAPHICSPALETTE
