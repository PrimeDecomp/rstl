#ifndef _CLINEINSTRUCTION
#define _CLINEINSTRUCTION
#include "rstl/RstlVersions.h"

#include "Kyoto/Text/CInstruction.hpp"
#include "Kyoto/Text/TextCommon.hpp"

class CLineInstruction : public CInstruction {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CLineInstruction(int words, int width, int height, const bool imageBaseline,
                   EJustification justification, EVerticalJustification verticalJustification)
  : mWordCount(words)
  , mCurX(width)
  , mCurY(height)
  , mLargestFontHeight(0)
  , mLargestFontWidth(0)
  , mLargestFontBaseline(0)
  , mLargestImageHeight(0)
  , mLargestImageWidth(0)
  , mLargestImageBaseline(0)
  , mJustification(justification)
  , mVerticalJustification(verticalJustification)
  , mImageBaseline(imageBaseline) {}
#else
  CLineInstruction(int words, int width, int height, EJustification justification,
                   EVerticalJustification verticalJustification, RSTL_KYOTO_SCALAR_PARAM_CONST bool imageBaseline);
#endif

  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
#if !RSTL_PRIME1 || RSTL_VERSION >= RSTL_GM8P_00
  bool IsLineInstruction() const override { return true; }
#endif
#if RSTL_KYOTO_LINE_INSTRUCTION_INLINE_DTOR
  ~CLineInstruction() {}
#endif

  void InvokeTTB(CFontRenderState& state) const;
  void TestLargestFont(int width, int height, int baseline);
  void TestLargestImage(int width, int height, int baseline);

  int GetHeight() const;
  int GetBaseline() const;
  int GetWordCount() const { return mWordCount; }
#if RSTL_PRIME1
  int GetX() const { return mCurX; }
#endif
  int GetWidth() const { return mCurX; }
  int GetY() const { return mCurY; }

  void IncWords() { ++mWordCount; }
  void DecWords() { --mWordCount; }
  void AddWidth(int width) { mCurX += width; }
  void SubWidth(int width) { mCurX -= width; }
  void AddHeight(int height) { mCurY += height; }
  void SetHeight(int height) { mCurY = height; }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  EJustification GetJustification() const { return mJustification; }
#endif
  void SetJustification(RSTL_PRIME1_CONST EJustification justification) {
    mJustification = justification;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  EVerticalJustification GetVerticalJustification() const { return mVerticalJustification; }
#endif
  void SetVerticalJustification(RSTL_PRIME1_CONST EVerticalJustification justification) {
    mVerticalJustification = justification;
  }

private:
  int mWordCount;
  int mCurX;
  int mCurY;
  int mLargestFontHeight;
  int mLargestFontWidth;
  int mLargestFontBaseline;
  int mLargestImageHeight;
  int mLargestImageWidth;
  int mLargestImageBaseline;
  EJustification mJustification;
  EVerticalJustification mVerticalJustification;
  bool mImageBaseline;
};

CHECK_SIZEOF(CLineInstruction, 0x34)

#endif // _CLINEINSTRUCTION
