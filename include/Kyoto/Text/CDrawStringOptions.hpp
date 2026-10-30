#ifndef _CDRAWSTRINGOPTIONS
#define _CDRAWSTRINGOPTIONS
#include "rstl/RstlVersions.h"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Text/TextCommon.hpp"
#include "rstl/reserved_vector.hpp"

class CDrawStringOptions {
public:
  CDrawStringOptions();

  void SetTextDirection(ETextDirection dir) { mDirection = dir; }
  ETextDirection GetTextDirection() const { return mDirection; }
#if !RSTL_PRIME1
  uint GetPaletteEntry(int idx) const { return mColors[idx]; }
#endif

  void SetPaletteEntry(int idx, uint color) { mColors[idx] = color; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  uint GetPaletteEntry(int idx) const { return mColors[idx]; }
  int GetExtraCharacterSpacing() const {
#if RSTL_VERSION >= RSTL_GM8P_00
    return mExtraCharacterSpacing;
#else
    return 0;
#endif
  }
#else
  // Guessed names
  void SetCharacterExtraSpace(int spacing) { mExtraCharacterSpacing = spacing; }
  int GetCharacterExtraSpace() const { return mExtraCharacterSpacing; }
#endif

private:
  ETextDirection mDirection;
  rstl::reserved_vector<
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1 && RSTL_VERSION >= RSTL_R3ME_00
      uint,
#else
      u32,
#endif
      16 >
      mColors;
#if RSTL_VERSION >= RSTL_GM8P_00
  int mExtraCharacterSpacing;
#endif
};

#if !RSTL_PRIME1
CHECK_SIZEOF(CDrawStringOptions, 0x4c)
#endif

#endif // _CDRAWSTRINGOPTIONS
