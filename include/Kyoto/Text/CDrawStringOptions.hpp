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
  void SetCharacterExtraSpace(int spacing) { mCharacterExtraSpace = spacing; }
  int GetCharacterExtraSpace() const { return mCharacterExtraSpace; }
#endif

private:
  ETextDirection mDirection;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_R3ME_00
  rstl::reserved_vector< uint, 16 > mColors;
#else
  rstl::reserved_vector< u32, 16 > mColors;
#endif
#if RSTL_VERSION >= RSTL_GM8P_00
  int mExtraCharacterSpacing;
#endif
#else
  rstl::reserved_vector< u32, 16 > mColors;
  int mCharacterExtraSpace; // Guessed name
#endif

};

#if !RSTL_PRIME1
CHECK_SIZEOF(CDrawStringOptions, 0x4c)
#endif

#endif // _CDRAWSTRINGOPTIONS
