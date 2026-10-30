#ifndef _CFONTRENDERSTATE
#define _CFONTRENDERSTATE
#include "rstl/RstlVersions.h"

#include "rstl/list.hpp"

#include "Kyoto/Text/CBlockInstruction.hpp"
#include "Kyoto/Text/CDrawStringOptions.hpp"
#if RSTL_PRIME1
#include "Kyoto/Text/CRasterFont.hpp"
#endif

#include "Kyoto/Text/CSaveableState.hpp"
#include "Kyoto/Text/TextCommon.hpp"
#if RSTL_PRIME1
#include "rstl/vector.hpp"
#endif

class CBlockInstruction;
class CLineInstruction;
class CFontRenderState {
public:
  CFontRenderState();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void RefreshColor(EColorType col);
#else
  void RefreshColor(EColorType type);
#endif

  uint ConvertToTextureSpace(const CTextColor& color) const;
  void PushState();
  void PopState();
  void SetColor(EColorType type, const CTextColor& color);
  void RefreshPalette();
#if RSTL_PRIME1
  TToken< CRasterFont >& GetFont() { return *mState.GetFont(); }
#endif

  bool IsFinishedLoading() { return mState.IsFinishedLoading(); }
  CDrawStringOptions& GetOptions() { return mState.GetOptions(); }
  void SetFont(const TToken< CRasterFont >& font) { mState.SetFont(font); }
#if !RSTL_PRIME1
  TToken< CRasterFont >& GetFont() { return mState.GetFont(); }
#endif

  rstl::vector< CTextColor >& GetColors() { return mState.GetColors(); }
  rstl::vector< bool >& GetOverride() { return mState.GetOverride(); }
  float GetLineSpacing() const { return mState.GetLineSpacing(); }
  void SetLineSpacing(float spacing) { mState.SetLineSpacing(spacing); }
  int GetLineExtraSpacing() const { return mState.GetLineExtraSpacing(); }
  void SetExtraLineSpace(int spacing) { mState.SetLineExtraSpace(spacing); }
  const CBlockInstruction* GetBlock() const { return mCurBlock; }
  void SetBlock(const CBlockInstruction* block) {
    mCurBlock = const_cast< CBlockInstruction* >(block);
  }
  void SetX(int x) { mCurX = x; }
  int GetX() const { return mCurX; }
  void SetY(int y) { mCurY = y; }
  int GetY() const { return mCurY; }
#if !RSTL_PRIME1
  void AddY(const int y) { mCurY += y; }
#endif

  const CLineInstruction* GetLine() const { return mCurrentLineInst; }
#if !RSTL_PRIME1
  void SetLine(const CLineInstruction* line) { mCurrentLineInst = line; }
#endif

  bool IsFirstWordOnLine() const { return mLineInitialized; }
  void SetFirstWordOnLine(bool v) { mLineInitialized = v; }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void SetLine(const CLineInstruction* line) { mCurrentLineInst = line; }

  void SubX(const int x) { mCurX -= x; }

  void AddX(const int x) { mCurX += x; }

  void SubY(const int y) { mCurY -= y; }
  void AddY(const int y) { mCurY += y; }

  int GetSpacing(const int v) const {
#else
  int GetSpacing(const int value) const {
#endif

    if (GetBlock()->GetVerticalJustification() == kVerticalJustification_Full) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      return v;
#else
      return value;
#endif

    }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return (static_cast< int >(static_cast< float >(v) * GetLineSpacing()) + GetLineExtraSpacing());
#else
    return static_cast< int >(static_cast< float >(value) * GetLineSpacing()) +
           GetLineExtraSpacing();
#endif

  }

private:
  CSaveableState mState;
  CBlockInstruction* mCurBlock;
  CDrawStringOptions mDrawOpts;
  int mCurX;
  int mCurY;
  const CLineInstruction* mCurrentLineInst;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  uint xe0_;
  uint xe4_;
  rstl::vector< uint > xe8_;
  rstl::vector< uchar > xf8_;
#else
  uint xe8_;
  uint xec_;
  rstl::vector< CTextColor > xf0_;
  rstl::vector< bool > x100_;
#endif

  bool mLineInitialized;
  rstl::list< CSaveableState > mPushedStates;
};

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHECK_SIZEOF(CFontRenderState,
             (RSTL_VERSION >= RSTL_GM8P_00 ? 0x12c : 0x124))
#else
CHECK_SIZEOF(CFontRenderState, 0x12c)
#endif

#endif // _CFONTRENDERSTATE
