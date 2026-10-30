#ifndef _CTEXTRENDERBUFFER
#define _CTEXTRENDERBUFFER
#include "rstl/RstlVersions.h"

#if !RSTL_PRIME1
#include "Kyoto/Graphics/CColor.hpp"
#endif

#include "Kyoto/Graphics/CGraphicsPalette.hpp"
#include "Kyoto/Math/CVector2i.hpp"
#include "Kyoto/TToken.hpp"
#include "Kyoto/Text/CFontImageDef.hpp"
#include "Kyoto/Text/CRasterFont.hpp"
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "Kyoto/Text/CTextColor.hpp"
#else
#include "rstl/auto_ptr.hpp"
#endif

#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

#if RSTL_PRIME1
class CColor;
class CRasterFont;
#endif

class CTextRenderBuffer {
public:
  enum ECmd {
    kC_CharacterRender,
    kC_ImageRender,
    kC_FontChange,
    kC_PaletteChange,
    kC_Invalid = -1,
  };
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  enum EMode {
    kM_AllocTally,
    kM_BufferFill,
  };
#else
  enum EMode { kM_AllocTally, kM_BufferFill };
#endif

  struct Primitive {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    Primitive(ECmd cmd, short x, short y, short chr, uint color, schar index)
#else
    Primitive(ECmd cmd, short x, short y, short chr, uint color, signed char index)
#endif

    : mColor(color), mCmd(cmd), mX(x), mY(y), mChar(chr), mIndex(index) {}

    uint mColor;
    ECmd mCmd;
    short mX;
    short mY;
    short mChar;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    schar mIndex;
#else
    signed char mIndex;
#endif

  };

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_GM8P_00
  struct SFontPalette {
    SFontPalette(EFontMode mode, const rstl::auto_ptr< CGraphicsPalette >& palette0,
                 const rstl::auto_ptr< CGraphicsPalette >& palette1,
                 const rstl::auto_ptr< CGraphicsPalette >& palette2,
                 const rstl::auto_ptr< CGraphicsPalette >& palette3)
    : mMode(mode)
    , mPalette0(palette0)
    , mPalette1(palette1)
    , mPalette2(palette2)
    , mPalette3(palette3) {}

    EFontMode mMode;
    uint mColors[2];
    rstl::auto_ptr< CGraphicsPalette > mPalette0;
    rstl::auto_ptr< CGraphicsPalette > mPalette1;
    rstl::auto_ptr< CGraphicsPalette > mPalette2;
    rstl::auto_ptr< CGraphicsPalette > mPalette3;
  };
#endif
#else
  struct SFontPalette {
    SFontPalette(EFontMode mode, const rstl::auto_ptr< CGraphicsPalette >& palette0,
                 const rstl::auto_ptr< CGraphicsPalette >& palette1,
                 const rstl::auto_ptr< CGraphicsPalette >& palette2,
                 const rstl::auto_ptr< CGraphicsPalette >& palette3)
    : mMode(mode)
    , mPalette0(palette0)
    , mPalette1(palette1)
    , mPalette2(palette2)
    , mPalette3(palette3) {}
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CTextRenderBuffer(EMode mode);
#else
    EFontMode mMode;
    ushort mColors[4];
    rstl::auto_ptr< CGraphicsPalette > mPalette0;
    rstl::auto_ptr< CGraphicsPalette > mPalette1;
    rstl::auto_ptr< CGraphicsPalette > mPalette2;
    rstl::auto_ptr< CGraphicsPalette > mPalette3;
  };
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_GM8P_00
  int GetNextAvailablePalette() const;
  int GetMatchingPaletteIndex(EFontMode mode, const CGraphicsPalette& palette) const;
#else
  CGraphicsPalette* GetNextAvailablePalette() const;
  int GetMatchingPaletteIndex(const CGraphicsPalette& palette) const;
#endif
#else
  explicit CTextRenderBuffer(EMode mode);
  void AddPaletteChange(const CGraphicsPalette& palette, EFontMode mode);
  void AddCharacter(const CVector2i& offset, short chr, uint color);
#endif

  void AddFontChange(const TToken< CRasterFont >& font);
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_GM8P_00
  void AddPaletteChange(const CGraphicsPalette& palette, EFontMode mode);
#else
  void AddPaletteChange(const CGraphicsPalette& palette);
#endif
  void AddCharacter(const CVector2i&, short chr, uint color);
#endif

  void AddImage(const CVector2i& offset, const CFontImageDef& image);

  void* GetOutStream();
  size_t GetCurLen();
  void SetMode(EMode mode);
  void Render(const CColor& color, float time) const;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int GetNumPrimitives() const { return mPrimOffsets.size(); }
#else
  int GetNumPrimitives() const { return mPrimitiveOffsets.size(); }
#endif

  Primitive GetPrimitive(int index) const;
  void SetPrimitive(const Primitive& prim, int index);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_GM8P_00
  const rstl::pair< CVector2i, CVector2i >& AccumulateTextBounds();
#else
  rstl::pair< CVector2i, CVector2i > AccumulateTextBounds();
#endif
  bool HasSpaceAvailable(const CVector2i& origin, const CVector2i& extent);
#else
  // Guessed name. Returns cached bounds, rebuilding them when dirty.
  const rstl::pair< CVector2i, CVector2i >& GetTextBounds();
#endif

private:
  void VerifyBuffer();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_GM8P_00
  void RecalculateTextBounds() const;
#endif
#else
  void AccumulateTextBounds();
  void GetNextAvailablePalette() const;
  int GetMatchingPaletteIndex(EFontMode mode, const CGraphicsPalette& palette) const;
  // Guessed name for the mode/layer palette expansion helper.
  static void GeneratePalette(EFontMode mode, int layer, CGraphicsPalette& dest,
                              const CGraphicsPalette& source);
#endif

  EMode mMode;
  rstl::vector< TToken< CRasterFont > > mFonts;
  rstl::vector< CFontImageDef > mImages;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::vector< int > mPrimOffsets;
#else
  rstl::vector< int > mPrimitiveOffsets;
#endif

  rstl::vector< signed char > mBytecode;
  uint mBlobSize;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  uint mCurBytecodeOffset;
#else
  uint mCurrentBytecodeOffset;
#endif

  mutable char mActiveFont;
  mutable char mActivePalette;
  mutable char mQueuedFont;
  mutable char mQueuedPalette;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_GM8P_00
  mutable rstl::reserved_vector< SFontPalette, 64 > mPalettes;
#else
  mutable rstl::reserved_vector< rstl::auto_ptr< CGraphicsPalette >, 64 > mPalettes;
#endif
#else
  mutable rstl::reserved_vector< SFontPalette, 64 > mPalettes;
#endif

  mutable int mNextPalette;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_GM8P_00
  mutable rstl::pair< CVector2i, CVector2i > mBounds;
  mutable bool mBoundsDirty;
#endif
#else
  rstl::pair< CVector2i, CVector2i > mTextBounds;
  bool mTextBoundsDirty;
#endif

};

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHECK_SIZEOF(CTextRenderBuffer, (RSTL_VERSION >= RSTL_GM8P_00 ? 0xb6c : 0x258))
#else
CHECK_SIZEOF(CTextRenderBuffer, 0xb6c)
#endif

NESTED_CHECK_SIZEOF(CTextRenderBuffer, Primitive, 0x10)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_GM8P_00
NESTED_CHECK_SIZEOF(CTextRenderBuffer, SFontPalette, 0x2c)
#endif
#else
NESTED_CHECK_SIZEOF(CTextRenderBuffer, SFontPalette, 0x2c)
#endif

#endif // _CTEXTRENDERBUFFER
