#ifndef _CRASTERFONT
#define _CRASTERFONT
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#include "rstl/types.h"

#include "Kyoto/TToken.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/vector.hpp"

#include "string.h"

class CTexture;
class CDrawStringOptions;
class CTextRenderBuffer;
class IObjectStore;

class CFontInfo {
public:
  CFontInfo(bool a, bool b, int c, int fontSize, const char* RSTL_PRIME1_CONST name)
  : x0_(a), x1_(b), x4_(c), mFontSize(fontSize) {
    strcpy(mName, name);
  }

private:
  bool x0_;
  bool x1_;
  int x4_;
  int mFontSize;
  char mName[64];
};

CHECK_SIZEOF(CFontInfo, 0x4c)

class CKernPair {
public:
  CKernPair(const wchar_t first, const wchar_t second, const int howMuch) : mHowMuch(howMuch) {
    mSecond = second;
    mFirst = first;
  }

#if RSTL_PRIME1
  CKernPair(const CKernPair& other)
  : mFirst(other.mFirst)
  , mSecond(other.mSecond)
  , mHowMuch(other.mHowMuch) {}
#endif

  wchar_t GetFirst() const { return mFirst; }
  wchar_t GetSecond() const { return mSecond; }
  int GetHowMuch() const { return mHowMuch; }

private:
  wchar_t mFirst;
  wchar_t mSecond;
  int mHowMuch;
};
// TODO: breaks clangd
// CHECK_SIZEOF(CKernPair, 0x8)

class CGlyph {
public:
  CGlyph(const int a, const int b, const int c, const float startU, const float startV,
         const float endU, const float endV,
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
         const int layer,
#endif
         const int cellWidth, const int cellHeight, const int baseline, const int kernStart
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
         , const uchar layer = 0
#endif
         )
  : mA(a)
  , mB(b)
  , mC(c)
  , mStartU(startU)
  , mStartV(startV)
  , mEndU(endU)
  , mEndV(endV)
#if RSTL_VERSION >= RSTL_GM8P_00
  , mLayer(layer)
#endif
  , mCellWidth(cellWidth)
  , mCellHeight(cellHeight)
  , mBaseline(baseline)
  , mKernStart(kernStart) {}

  short GetA() const { return mA; }
  short GetB() const { return mB; }
  short GetC() const { return mC; }
  float GetStartU() const { return mStartU; }
  float GetStartV() const { return mStartV; }
  float GetEndU() const { return mEndU; }
  float GetEndV() const { return mEndV; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int GetCellWidth() const { return mCellWidth; }
  int GetCellHeight() const { return mCellHeight; }
  int GetBaseLine() const { return mBaseline; }
#else
  uchar GetCellWidth() const { return mCellWidth; }
  uchar GetCellHeight() const { return mCellHeight; }
  uchar GetBaseline() const { return mBaseline; }
#endif

  int GetKernStart() const { return mKernStart; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int GetLayer() const {
#if RSTL_VERSION >= RSTL_GM8P_00
    return mLayer;
#else
    return 0;
#endif
  }
#else
  uchar GetLayer() const { return mLayer; }
#endif

private:
  short mA;
  short mB;
  short mC;
  float mStartU;
  float mStartV;
  float mEndU;
  float mEndV;
#if RSTL_VERSION >= RSTL_GM8P_00
  uchar mLayer;
  uchar mCellWidth;
  uchar mCellHeight;
  uchar mBaseline;
#else
  short mCellWidth;
  short mCellHeight;
  short mBaseline;
#endif
  short mKernStart;
};

CHECK_SIZEOF(CGlyph, 0x20)

enum EFontMode {
  kFM_None = -1,
  kFM_OneLayer,
  kFM_OneLayerOutline,
  kFM_FourLayers,
  kFM_TwoLayersOutline,
  kFM_TwoLayers,
};

class CRasterFont {
public:
#if RSTL_PRIME1
  friend class CFontInstruction;
#endif

  CRasterFont(CInputStream& in, IObjectStore* store);

  EFontMode GetMode() const;
  int GetMonoWidth() const;
#if RSTL_PRIME1
  int GetMonoHeight() const;
#endif
  int GetCarriageAdvance() RSTL_ECHOES_CONST;
  int GetBaseLine() const;

  void GetSize(const CDrawStringOptions&, int&, int&, const wchar_t*, int) const;
  void SetTexture(const TToken< CTexture >& texture);
#if RSTL_PRIME1
  inline TToken< CTexture > GetTexture() { return *mTexture; }
#else
  const TToken< CTexture >& GetTexture() const { return mTexture.data(); }
#endif
  bool IsFinishedLoading();
  const CGlyph* GetGlyph(wchar_t c) const;
  bool HasGlyph(wchar_t c) const { return GetGlyph(c) != nullptr; }

  void DrawString(const CDrawStringOptions& options, int x, int y, int& xOut, int& yOut,
                  CTextRenderBuffer* buffer, const wchar_t* str, int length) const;
  void DrawSpace(const CDrawStringOptions& options, int x, int y, int& xOut, int& yOut,
                 int length) const;

  void SinglePassDrawString(const CDrawStringOptions& options, int x, int y, int& xOut, int& yOut,
                            CTextRenderBuffer* buffer, const wchar_t* str, int length) const;

  void SetupRenderState();
#if RSTL_PRIME1
  int GetLineMargin();
#endif

private:
  bool mInitialized;
  int mMonoWidth;
  int mMonoHeight;
  rstl::vector< rstl::pair< wchar_t, CGlyph > > mGlyphs;
  rstl::vector< CKernPair > mKerning;
  EFontMode mMode;
  rstl::optional_object< CFontInfo > mFontInfo;
  rstl::optional_object< TToken< CTexture > > mTexture;
  int mBaseline;
  int mLineMargin;

  static int KernLookup(const rstl::vector< CKernPair >& kerning, int start,
                        RSTL_PRIME1_CONST int chr);
  const CGlyph* InternalGetGlyph(wchar_t chr) const;
};
CHECK_SIZEOF(CRasterFont, 0x94)

#if !RSTL_PRIME1 || RSTL_VERSION >= RSTL_GM8P_00
inline int CRasterFont::GetMonoWidth() const { return mMonoWidth; }
inline int CRasterFont::GetBaseLine() const { return mBaseline; }
inline const CGlyph* CRasterFont::GetGlyph(wchar_t c) const { return InternalGetGlyph(c); }
#endif
#if RSTL_PRIME1 && RSTL_VERSION >= RSTL_GM8P_00
inline int CRasterFont::GetMonoHeight() const { return mMonoHeight; }
inline int CRasterFont::GetLineMargin() { return mLineMargin; }
#endif

#endif // _CRASTERFONT
