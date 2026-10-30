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
                   EVerticalJustification verticalJustification, const bool imageBaseline);
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int GetWordCount() const { return mWordCount; }
#else
  // CInstruction
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buffer) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buffer) const override;
  bool IsLineInstruction() const override { return true; }

  void InvokeTTB(CFontRenderState& state) const;
  void TestLargestFont(int width, int height, int baseline);
  void TestLargestImage(int width, int height, int baseline);
#endif

  int GetHeight() const;
  int GetBaseline() const;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void InvokeTTB(CFontRenderState& state) const;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
#if RSTL_VERSION >= VERSION_GM8P_00
  bool IsLineInstruction() const override { return true; }
#endif
#if RSTL_VERSION >= RSTL_R3ME_00
  ~CLineInstruction() {}
#endif

  void TestLargestFont(const int width, const int height, const int baseLine);
  void TestLargestImage(const int width, const int height, const int baseline);

  int GetX() const { return mCurX; }
  int GetWidth() const { return mCurX; }
  int GetY() const { return mCurY; }
#else
  int GetWordCount() const { return mWordCount; }
  int GetWidth() const { return mCurrentX; }
  int GetY() const { return mCurrentY; }
#endif

  void IncWords() { ++mWordCount; }
  void DecWords() { --mWordCount; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void AddWidth(int width) { mCurX += width; }
  void SubWidth(int width) { mCurX -= width; }
  void AddHeight(int height) { mCurY += height; }
  void SetHeight(int height) { mCurY = height; }

  EJustification GetJustification() const { return mJustification; }
  void SetJustification(const EJustification justification) { mJustification = justification; }
  EVerticalJustification GetVerticalJustification() const { return mVerticalJustification; }
  void SetVerticalJustification(const EVerticalJustification justification) {
#else
  void AddWidth(int width) { mCurrentX += width; }
  void SubWidth(int width) { mCurrentX -= width; }
  void AddHeight(int height) { mCurrentY += height; }
  void SetHeight(int height) { mCurrentY = height; }
  void SetJustification(EJustification justification) { mJustification = justification; }
  void SetVerticalJustification(EVerticalJustification justification) {
#endif

    mVerticalJustification = justification;
  }

private:
  int mWordCount;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int mCurX;
  int mCurY;
#else
  int mCurrentX;
  int mCurrentY;
#endif

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
