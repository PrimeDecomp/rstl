#ifndef _CGRAPHICSPALETTE
#define _CGRAPHICSPALETTE
#include "rstl/RstlVersions.h"

#include "types.h"

#if !RSTL_PRIME1
#include "dolphin/gx.h"
#endif

#include "rstl/single_ptr.hpp"
#if RSTL_PRIME1
#include "dolphin/gx/GXEnum.h"
#include "dolphin/gx/GXStruct.h"
#endif

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

#if RSTL_PRIME1
  inline GXTlutFmt GetTlutFmt() const { return static_cast< GXTlutFmt >(mFmt); }
#endif

  ushort* GetPaletteData() { return mEntries.get(); }
  const ushort* GetPaletteData() const { return mEntries.get(); }
#if !RSTL_PRIME1
  EPaletteFormat GetFormat() const { return mFmt; }
  uint GetEntryCount() const { return mEntryCount; }
#endif

  void Load() const;
  void* Lock() {
    mLocked = true;
    return mEntries.get();
  }
  void UnLock();

#if RSTL_PRIME1
public:
#endif

  static uint sCurrentFrameCount;

private:
  EPaletteFormat mFmt;
  mutable uint mFrameLoaded;
  uint mEntryCount;
  rstl::single_ptr< ushort > mEntries;
  GXTlutObj mTlutObj;
  bool mLocked;
};
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
static inline GXTlutFmt format_to_format(EPaletteFormat fmt) {
  return static_cast< GXTlutFmt >(fmt);
}
#else
CHECK_SIZEOF(CGraphicsPalette, 0x20)
#endif

#endif // _CGRAPHICSPALETTE
