#ifndef _CFONTRENDERSTATE
#define _CFONTRENDERSTATE
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#endif

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#else
#include "rstl/list.hpp"

#endif
#include "Kyoto/Text/CBlockInstruction.hpp"
#include "Kyoto/Text/CDrawStringOptions.hpp"
#if !RSTL_CORRUPTION
#include "Kyoto/Text/CRasterFont.hpp"

#endif
#include "Kyoto/Text/CSaveableState.hpp"
#include "Kyoto/Text/TextCommon.hpp"
#if RSTL_CORRUPTION
#include "rstl/list.hpp"


#else
#include "rstl/vector.hpp"
#endif

class CBlockInstruction;
class CLineInstruction;
class CFontRenderState {
public:
  CFontRenderState();
  void RefreshColor(EColorType type);

  uint ConvertToTextureSpace(const CTextColor& color) const;
  void PushState();
  void PopState();
  void SetColor(EColorType type, const CTextColor& color);
  void RefreshPalette();
#if !RSTL_CORRUPTION
  TToken< CRasterFont >& GetFont() {
#if RSTL_PRIME1
    return *mState.GetFont();
#else
    return mState.GetFont();
#endif
  }

#endif
  bool IsFinishedLoading() { return mState.IsFinishedLoading(); }
  CDrawStringOptions& GetOptions() { return mState.GetOptions(); }
  void SetFont(const TToken< CRasterFont >& font) { mState.SetFont(font); }
#if RSTL_CORRUPTION
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
  void AddY(const int y) { mCurY += y; }

  const CLineInstruction* GetLine() const { return mCurrentLineInst; }
  void SetLine(const CLineInstruction* line) { mCurrentLineInst = line; }

  bool IsFirstWordOnLine() const { return mLineInitialized; }
  void SetFirstWordOnLine(bool v) { mLineInitialized = v; }
#if !RSTL_CORRUPTION

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1

  void SubX(const int x) { mCurX -= x; }

  void AddX(const int x) { mCurX += x; }

  void SubY(const int y) { mCurY -= y; }
#endif
#endif

  int GetSpacing(const int value) const {
    if (GetBlock()->GetVerticalJustification() == kVerticalJustification_Full) {
      return value;
    }

    return static_cast< int >(static_cast< float >(value) * GetLineSpacing()) +
           GetLineExtraSpacing();
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

#if RSTL_CORRUPTION
CHECK_SIZEOF(CFontRenderState, 0x12c)
#else
CHECK_SIZEOF(CFontRenderState, (RSTL_VERSION >= RSTL_GM8P_00 ? 0x12c : 0x124))
#endif

#endif // _CFONTRENDERSTATE
