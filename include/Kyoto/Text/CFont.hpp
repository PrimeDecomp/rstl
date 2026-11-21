#ifndef _CFONT
#define _CFONT
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#endif

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#endif
#include "Kyoto/Graphics/CColor.hpp"

#include "rstl/types.h"

#if !RSTL_CORRUPTION
class CTexture;
#endif

class CFont {
public:
#if RSTL_CORRUPTION
  explicit CFont(float scale);
  CFont(const CFont& other);
  ~CFont();
#else
  CFont(float scale);
  ~CFont();
#if RSTL_PRIME1
#ifdef ENABLE_DEBUG_TOOLS
  CFont(const CFont& other);
  int StringWidth(const char* str) const;
#endif
#endif

#endif
  int CharWidth(char) const;
#if RSTL_CORRUPTION
  int CharsWidth(const char* text, unsigned int length) const;
  int StringWidth(const char* text) const;
#endif
  int GetFontSize() const { return mFontSize; }

  void DrawString(const char* str, long x, long y, const CColor& col) const;

private:
#if RSTL_CORRUPTION
  static void BindSystemFont();
  static void LinearToTile8(unsigned char* destination, unsigned char* source);
  static void TileCopy8(unsigned char* destination, unsigned char* source);

  static unsigned char sSystemFont[65536];
  static bool sFontInitialized;
#else
#if RSTL_PRIME1
#ifdef ENABLE_DEBUG_TOOLS
  static void TileCopy8(uchar* dst, const uchar* src);
  static void LinearToTile8(uchar* dst, const uchar* src);
  static const uchar sSystemFont[65536];
  static int sNumInstances;
  static CTexture* mpTexture;
#endif
#endif
#endif

  int mFontSize;
  float mScale;
};
CHECK_SIZEOF(CFont, 0x8)

#endif // _CFONT
