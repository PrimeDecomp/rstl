#ifndef _CFONT
#define _CFONT
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#include "Kyoto/Graphics/CColor.hpp"

#include "rstl/types.h"

class CTexture;

class CFont {
public:
  CFont(float scale);
  ~CFont();
#if RSTL_PRIME1
#ifdef ENABLE_DEBUG_TOOLS
  CFont(const CFont& other);
  int StringWidth(const char* str) const;
#endif
#endif

  int CharWidth(char) const;
  int GetFontSize() const { return mFontSize; }

  void DrawString(const char* str, long x, long y, const CColor& col) const;

private:
#if RSTL_PRIME1
#ifdef ENABLE_DEBUG_TOOLS
  static void TileCopy8(uchar* dst, const uchar* src);
  static void LinearToTile8(uchar* dst, const uchar* src);
  static const uchar sSystemFont[65536];
  static int sNumInstances;
  static CTexture* mpTexture;
#endif
#endif

  int mFontSize;
  float mScale;
};
CHECK_SIZEOF(CFont, 0x8)

#endif // _CFONT
