#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Text/CFont.hpp"
#if !RSTL_CORRUPTION
#endif
#include "Kyoto/Graphics/CGraphics.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Graphics/CGX.hpp"
#include "dolphin/os.h"
#else
#include "Kyoto/Graphics/CTexture.hpp"
#endif

#if RSTL_CORRUPTION


extern "C" void* memcpy(void* destination, const void* source, unsigned long length);
extern const float kFontAdvance;
extern const float kFontCellSize;

// Guessed name. Native direct position, color and texture-coordinate descriptors.
static const GXVtxDescList skFontVertexDesc[] = {
    {GX_VA_POS, GX_DIRECT},
    {GX_VA_CLR0, GX_DIRECT},
    {GX_VA_TEX0, GX_DIRECT},
    {GX_VA_NULL, GX_NONE},
};

bool CFont::sFontInitialized = false;

unsigned char CFont::sSystemFont[65536] = {
#include "Kyoto/Text/SystemFont.inc"
};

// Guessed name. Set up the native unlit, textured two-dimensional font pass.
extern "C" CGraphics::SLightingState CFont_SetupDrawState() {
  Mtx44 projection;
  const CViewport& viewport = CGraphics::GetViewport();
  MTXOrtho(projection, viewport.mHeight / 2, -viewport.mHeight / 2,
           -viewport.mWidth / 2, viewport.mWidth / 2, -1.f, -10.f);
  GXSetProjection(projection, GX_ORTHOGRAPHIC);
  Mtx model;
  MTXIdentity(model);
  CGX::LoadPosMtxImm(model, GX_PNMTX0);
  CGraphics::SLightingState state = CGraphics::CaptureLightingState();
  CGraphics::SetLightingMode(3);
  CGX::SetVtxDescv(skFontVertexDesc);
  CGX::SetNumChans(1);
  CGX::SetNumTexGens(1);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0);
  CGX::SetNumIndStages(0);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,
                    GX_FALSE, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY,
                    GX_FALSE, GX_PTIDENTITY);
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  return state;
}

// Guessed name. Restore the lighting, projection and model matrix after drawing text.
extern "C" void CFont_RestoreDrawState(const CGraphics::SLightingState& state) {
  CGraphics::RestoreLightingState(state);
  CGraphics::SetProjectionState(CGraphics::GetProjectionState());
  CGraphics::SetModelMatrix(CGraphics::GetModelMatrix());
}

// Guessed name. Draw a glyph with half-texel coordinates in the current viewport.
extern "C" void CFont_DrawGlyphQuad(int textureWidth, int textureHeight, int x, int y,
                                  int sourceX, int sourceY, int sourceWidth, int sourceHeight,
                                  int width, int height, const CColor& color) {
  const float x0 = x - CGraphics::GetViewport().mWidth / 2;
  const float y0 = y - CGraphics::GetViewport().mHeight / 2;
  const float x1 = x0 + width;
  const float y1 = y0 + height;
  const float texWidth = textureWidth;
  const float texHeight = textureHeight;
  const float s0 = (0.5f + sourceX) / texWidth;
  const float s1 = (0.5f + (sourceX + sourceWidth)) / texWidth;
  const float t0 = (0.5f + sourceY) / texHeight;
  const float t1 = (0.5f + (sourceY + sourceHeight)) / texHeight;

  float z = 1.f;
  CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
  GXPosition3f32(x0, y0, z);
  GXColor1u32(color.GetColor_u32());
  GXTexCoord2f32(s0, t1);
  GXPosition3f32(x1, y0, z);
  GXColor1u32(color.GetColor_u32());
  GXTexCoord2f32(s1, t1);
  GXPosition3f32(x0, y1, z);
  GXColor1u32(color.GetColor_u32());
  GXTexCoord2f32(s0, t0);
  GXPosition3f32(x1, y1, z);
  GXColor1u32(color.GetColor_u32());
  GXTexCoord2f32(s1, t0);
  CGX::End();
}

void CFont::TileCopy8(unsigned char* destination, unsigned char* source) {
  for (unsigned int row = 0; row < 4; ++row) {
    for (unsigned int column = 0; column < 8; ++column) {
      destination[column] = source[column];
    }
    source += 256;
    destination += 8;
  }
}

void CFont::LinearToTile8(unsigned char* destination, unsigned char* source) {
  int offset;
  int rowOffset;
  short y;
  y = 0;
  rowOffset = 0;
  for (; y < 256; y += 4) {
    offset = rowOffset;
    for (short x = 0; x < 256; x += 8) {
      TileCopy8(destination, source + offset);
      destination += 32;
      offset += 8;
}
    rowOffset += 1024;
  }
}

void CFont::BindSystemFont() {
  CGraphics::LoadDolphinSpareTexture(256, 256, GX_TF_I8, sSystemFont, GX_TEXMAP0, GX_NEAR);
}

CFont::CFont(float scale) : mFontSize(static_cast< int >(kFontCellSize * scale)), mScale(scale) {
  if (!sFontInitialized) {
    sFontInitialized = true;
    unsigned char* tiled = rs_new_line_in("DolphinCFont.cpp", 193) unsigned char[65536];
    LinearToTile8(tiled, sSystemFont);
    memcpy(sSystemFont, tiled, sizeof(sSystemFont));
    CMemory::Free(tiled);
    DCFlushRange(sSystemFont, sizeof(sSystemFont));
  }
}

CFont::CFont(const CFont& other) : mFontSize(other.mFontSize), mScale(other.mScale) {}

CFont::~CFont() {}
#else
#if RSTL_PRIME1 && defined(ENABLE_DEBUG_TOOLS)
#include "Kyoto/Text/SystemFont.inc"

int CFont::sNumInstances = 0;
CTexture* CFont::mpTexture = nullptr;

CFont::CFont(float scale) : mFontSize(scale * 16.f), mScale(scale) {
  if (sNumInstances++ == 0) {
    mpTexture = rs_new CTexture(kTexelFormat_I8, 256, 256, 1);
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
#endif

#if RSTL_CORRUPTION
int CFont::CharWidth(char) const { return static_cast< int >(kFontAdvance * mScale); }
#else
int CFont::CharWidth(char c) const { return mScale * 15.f; }
#endif

#if RSTL_CORRUPTION
int CFont::StringWidth(const char* text) const {
  int width = 0;
  while (*text != 0) {
    ++text;
    width += static_cast< int >(kFontAdvance * mScale);
  }
  return width;
#else
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
#endif
}
#if RSTL_CORRUPTION

int CFont::CharsWidth(const char*, unsigned int length) const {
  return length * static_cast< int >(kFontAdvance * mScale);
}

void CFont::DrawString(const char* text, long x, long y, const CColor& color) const {
  CGraphics::SLightingState state = CFont_SetupDrawState();
  BindSystemFont();

  unsigned char character = *text;
  while (character != 0) {
    ++text;
    CFont_DrawGlyphQuad(256, 256, x, y, (character % 16) * 16, character & 0xf0, 16, 16,
                        static_cast< int >(kFontCellSize * mScale),
                        static_cast< int >(kFontCellSize * mScale), color);
    x += static_cast< int >(kFontAdvance * mScale);
    character = *text;
  }

  CFont_RestoreDrawState(state);
}
#endif
