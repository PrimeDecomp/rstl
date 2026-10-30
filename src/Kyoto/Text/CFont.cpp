#include "rstl/RstlVersions.h"
#include "Kyoto/Text/CFont.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CTexture.hpp"

#if RSTL_PRIME1 && defined(ENABLE_DEBUG_TOOLS)
#include "Kyoto/Text/SystemFont.inc"

int CFont::sNumInstances = 0;
CTexture* CFont::mpTexture = nullptr;

CFont::CFont(float scale) : mFontSize(scale * 16.f), mScale(scale) {
  if (sNumInstances++ == 0) {
    mpTexture = rs_new CTexture(kTF_I8, 256, 256, 1);
    LinearToTile8(static_cast< uchar* >(mpTexture->Lock()), sSystemFont);
    mpTexture->UnLock();
  }
}

CFont::CFont(const CFont& other) : mFontSize(other.mFontSize), mScale(other.mScale) {
  ++sNumInstances;
}

CFont::~CFont() {
  if (--sNumInstances == 0) {
    delete mpTexture;
    mpTexture = nullptr;
  }
}

int CFont::StringWidth(const char* str) const {
  int width = 0;
  for (; *str; ++str)
    width += CharWidth(*str);
  return width;
}

void CFont::TileCopy8(uchar* dst, const uchar* src) {
  for (int row = 0; row < 4; ++row) {
    for (int col = 0; col < 8; ++col) {
      *dst++ = src[col];
    }
    src += 256;
  }
}

void CFont::LinearToTile8(uchar* dst, const uchar* src) {
  for (int y = 0; y < 256; y += 4) {
    for (int x = 0; x < 256; x += 8) {
      TileCopy8(dst, src + y * 256 + x);
      dst += 32;
    }
  }
}
#else
CFont::CFont(float scale) : mFontSize(scale * 16.f), mScale(scale) {}

CFont::~CFont() {}
#endif

int CFont::CharWidth(char c) const { return mScale * 15.f; }

void CFont::DrawString(const char* str, long x, long y, const CColor& col) const {
#if RSTL_PRIME1 && defined(ENABLE_DEBUG_TOOLS)
  const bool lightsWereOn = CGraphics::BeginRender2D(*mpTexture);
  for (const uchar* chr = reinterpret_cast< const uchar* >(str); *chr; ++chr) {
    CGraphics::DoRender2D(*mpTexture, x, y, (*chr & 15) * 16, *chr & 0xf0,
                          16, 16, mFontSize, mFontSize, col);
    x += CharWidth(*chr);
  }
  CGraphics::EndRender2D(lightsWereOn);
#endif
}

