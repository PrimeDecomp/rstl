#ifndef _CFONT
#define _CFONT
#include "rstl/RstlVersions.h"

#include "Kyoto/Graphics/CColor.hpp"

#include "types.h"

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
#if !RSTL_PRIME1
  int GetFontSize() const { return mFontSize; }
#endif

  void DrawString(const char* str, long x, long y, const CColor& col) const;
#if RSTL_PRIME1
  int GetFontSize() const { return mFontSize; }
#endif

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
