#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#include "rstl/macros.hpp"
#endif
#include "Kyoto/Text/CTextExecuteBuffer.hpp"

#if RSTL_CORRUPTION
#include "Kyoto/Alloc/Assert.hpp"
#else
#include "Kyoto/Math/CVector2i.hpp"

#endif
#include "Kyoto/Text/CBlockInstruction.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Text/CCharacterExtraSpaceInstruction.hpp"
#else
#if !RSTL_PRIME1
#include "Kyoto/Text/CCharacterExtraSpaceInstruction.hpp"
#endif

#endif
#include "Kyoto/Text/CColorInstruction.hpp"
#include "Kyoto/Text/CColorOverrideInstruction.hpp"
#if !RSTL_CORRUPTION
#include "Kyoto/Text/CFontImageDef.hpp"

#endif
#include "Kyoto/Text/CFontInstruction.hpp"
#include "Kyoto/Text/CFontRenderState.hpp"
#include "Kyoto/Text/CImageInstruction.hpp"
#include "Kyoto/Text/CLineExtraSpaceInstruction.hpp"
#include "Kyoto/Text/CLineInstruction.hpp"
#include "Kyoto/Text/CLineSpacingInstruction.hpp"
#include "Kyoto/Text/CPopStateInstruction.hpp"
#include "Kyoto/Text/CPushStateInstruction.hpp"
#include "Kyoto/Text/CRemoveColorOverrideInstruction.hpp"
#include "Kyoto/Text/CTextInstruction.hpp"
#include "Kyoto/Text/CTextRenderBuffer.hpp"
#if !RSTL_CORRUPTION
#include "Kyoto/Text/CWordBreakTables.hpp"
#endif
#include "Kyoto/Text/CWordInstruction.hpp"

#include "rstl/math.hpp"

CTextExecuteBuffer::CTextExecuteBuffer()
#if RSTL_CORRUPTION
: mCurrentBlock(nullptr)
, mCurLine(nullptr)
, mCurWordIt(mInstructions.end())
, mCurWordX(0)
, mCurWordY(0)
#else
: mCurBlock(nullptr)
, mCurLine(nullptr)
, mCurWordIt(mInstructions.end())
, mCurWordX(0)
, mCurWordY(0)
#endif
, mSpaceDistance(0)
, mImageBaseline(false) {}

void CTextExecuteBuffer::Clear() {
  mInstructions.clear();
  mState = CSaveableState();
#if RSTL_CORRUPTION
  mCurrentBlock = nullptr;
  mCurLine = nullptr;
  mCurWordIt = mInstructions.end();
  mCurWordX = 0;
  mCurWordY = 0;
#else
  mCurBlock = nullptr;
  mCurLine = nullptr;
  mCurWordIt = mInstructions.end();
  mCurWordX = 0;
  mCurWordY = 0;
#endif
  mSpaceDistance = 0;
}

void CTextExecuteBuffer::BeginBlock(int x, int y, int width, int height, bool imageBaseline,
#if RSTL_CORRUPTION
                                    ETextDirection dir, EJustification justification,
                                    EVerticalJustification verticalJustification) {
#else
                                    ETextDirection dir, EJustification just,
                                    EVerticalJustification vjust) {
#endif
  mImageBaseline = imageBaseline;
#if RSTL_CORRUPTION
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new_line(81)
      CBlockInstruction(x, y, width, height, dir, justification, verticalJustification);
  mCurrentBlock = static_cast< CBlockInstruction* >(instruction.GetPtr());
#else
  const rstl::ncrc_ptr< CInstruction > instruction =
      rs_new CBlockInstruction(x, y, width, height, dir, just, vjust);
  mCurBlock = static_cast< CBlockInstruction* >(instruction.GetPtr());
#endif
  if (mState.IsFinishedLoading()) {
#if RSTL_CORRUPTION
    mCurrentBlock->TestLargestFont(mState.GetFont()->GetMonoWidth(),
                               mState.GetFont()->GetCarriageAdvance(),
                               mState.GetFont()->GetBaseLine());
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    mCurBlock->TestLargestFont((*mState.GetFont())->GetMonoWidth(),
                               (*mState.GetFont())->GetCarriageAdvance(),
                               (*mState.GetFont())->GetBaseLine());
#else
    mCurBlock->TestLargestFont(mState.GetFont()->GetMonoWidth(),
                               mState.GetFont()->GetCarriageAdvance(),
                               mState.GetFont()->GetBaseLine());
#endif
#endif
  }
  Add(instruction);
#if RSTL_CORRUPTION
  mState.GetOptions().SetTextDirection(dir);
  mState.SetJustification(justification);
  mState.SetVerticalJustification(verticalJustification);
#else
  mState.GetOptions().SetTextDirection(dir);
  mState.SetJustification(just);
  mState.SetVerticalJustification(vjust);
#endif
}

void CTextExecuteBuffer::EndBlock() {
#if RSTL_CORRUPTION
  if (mCurLine) {
    TerminateLine(true);
#else
  if (mCurLine) {
    TerminateLine(
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
        true
#endif
    );
#endif
  }
#if RSTL_CORRUPTION
  mCurLine = nullptr;
  mCurrentBlock = nullptr;
#else
  mCurLine = nullptr;
  mCurBlock = nullptr;
#endif
}

void CTextExecuteBuffer::AddFont(const TToken< CRasterFont >& font) {
#if RSTL_CORRUPTION
  rstl::ncrc_ptr< CInstruction > instruction(rs_new_line(118) CFontInstruction(font));
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CFontInstruction(font);
#else
  rstl::ncrc_ptr< CInstruction > instruction(rs_new CFontInstruction(font));
#endif
#endif
  Add(instruction);
  mState.SetFont(font);
  if (font.IsLoaded()) {
#if RSTL_CORRUPTION
    if (mCurrentBlock) {
      mCurrentBlock->TestLargestFont(mState.GetFont()->GetMonoWidth(),
                                 mState.GetFont()->GetCarriageAdvance(),
                                 mState.GetFont()->GetBaseLine());
#else
    if (mCurBlock) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      mCurBlock->TestLargestFont((*mState.GetFont())->GetMonoWidth(),
                                 (*mState.GetFont())->GetCarriageAdvance(),
                                 (*mState.GetFont())->GetBaseLine());
#else
      mCurBlock->TestLargestFont(mState.GetFont()->GetMonoWidth(),
                                 mState.GetFont()->GetCarriageAdvance(),
                                 mState.GetFont()->GetBaseLine());
#endif
#endif
    }
#if RSTL_CORRUPTION
    if (mCurLine) {
      mCurLine->TestLargestFont(mState.GetFont()->GetMonoWidth(),
                                mState.GetFont()->GetCarriageAdvance(),
                                mState.GetFont()->GetBaseLine());
#else
    if (mCurLine) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      mCurLine->TestLargestFont((*mState.GetFont())->GetMonoWidth(),
                                (*mState.GetFont())->GetCarriageAdvance(),
                                (*mState.GetFont())->GetBaseLine());
#else
      mCurLine->TestLargestFont(mState.GetFont()->GetMonoWidth(),
                                mState.GetFont()->GetCarriageAdvance(),
                                mState.GetFont()->GetBaseLine());
#endif
#endif
    }
  }
}

#if RSTL_CORRUPTION
int CFontImageDef::GetWidth() const {
  TToken< CTexture > tex = mTextures[0];
  return tex->GetWidth() * mCropFactor.GetX();
}

int CFontImageDef::GetHeight() const {
  TToken< CTexture > tex = mTextures[0];
  return tex->GetHeight() * mCropFactor.GetY();
}
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_GM8P_00
int CFontImageDef::GetWidth() const { return GetMonoWidth(); }

int CFontImageDef::GetHeight() const { return GetMonoHeight(); }

inline bool CTextExecuteBuffer::ShouldWrapImage(const CFontImageDef& image) const {
  bool wrap = false;
  bool tooWide = false;
  if (mState.IsWordWrapping() &&
      mCurLine->GetWidth() + image.GetWidth() > mCurBlock->GetOutputWidth()) {
    tooWide = true;
  }
  if (tooWide && mCurLine->GetWordCount() > 0) {
    wrap = true;
  }
  return wrap;
}
#endif
#else
#if RSTL_VERSION < RSTL_R32J
int CFontImageDef::GetWidth() const {
  TToken< CTexture > tex = mTextures[0];
  return tex->GetWidth() * mCropFactor.GetX();
}

int CFontImageDef::GetHeight() const {
  TToken< CTexture > tex = mTextures[0];
  return tex->GetHeight() * mCropFactor.GetY();
}
#endif
#endif
#endif

void CTextExecuteBuffer::AddImage(const CFontImageDef& image) {
  if (!mCurLine) {
    StartNewLine();
  }
#if !RSTL_CORRUPTION
  if (mCurBlock && image.IsLoaded()) {
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_CORRUPTION
  if (mCurrentBlock && image.IsLoaded()) {
    const bool newLine =
        mState.IsWordWrapping() &&
        mCurLine->GetWidth() + image.GetWidth() > mCurrentBlock->GetOutputWidth() &&
        mCurLine->GetWordCount() > 0;
    if (newLine) {
#else
#if RSTL_PRIME1
#if RSTL_VERSION < RSTL_GM8P_00
    bool wrap = mState.mEnableWordWrap;
    if (wrap) {
      const int width = mCurLine->GetWidth() + image.GetWidth();
      wrap = width > mCurBlock->GetOutputWidth();
    }
    if (wrap) {
      wrap = mCurLine->GetWordCount() > 1;
    }
#else
    bool wrap = ShouldWrapImage(image);
#endif
#else
    const bool wrap = mState.IsWordWrapping() &&
                      mCurLine->GetWidth() + image.GetWidth() > mCurBlock->GetOutputWidth() &&
                      mCurLine->GetWordCount() > 0;
#endif
    if (wrap) {
#endif
      StartNewLine();
    }
#if RSTL_CORRUPTION
    mCurLine->TestLargestImage(image.GetMonoWidth(), image.GetHeight(),
                               image.CalculateBaseline());
    if (mCurrentBlock->GetTextDirection() == kTD_Horizontal) {
      mCurLine->AddWidth(image.GetWidth());
      if (mCurLine->GetWidth() > image.GetWidth()) {
        mCurrentBlock->SetWidth(mCurLine->GetWidth());
#else
#if RSTL_VERSION < RSTL_GM8P_00
    mCurLine->TestLargestImage(image.GetMonoWidth(), image.GetMonoHeight(),
                               image.CalculateBaseline());
#else
    mCurLine->TestLargestImage(image.GetMonoWidth(), image.GetHeight(), image.CalculateBaseline());
#endif
    if (mCurBlock->GetTextDirection() == kTD_Horizontal) {
      mCurLine->AddWidth(image.GetWidth());
      if (mCurLine->GetWidth() > image.GetWidth()) {
        mCurBlock->SetWidth(mCurLine->GetWidth());
#endif
      }
    }
  }

  const rstl::ncrc_ptr< CInstruction > instruction = rs_new_line(177) CImageInstruction(image);
  Add(instruction);
}

void CTextExecuteBuffer::AddColor(EColorType type, const CTextColor& color) {
#if RSTL_CORRUPTION
  Add(rs_new_line(189) CColorInstruction(type, color));
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CColorInstruction(type, color);
  Add(instruction);
#else
  Add(rs_new CColorInstruction(type, color));
#endif
#endif
}

#if RSTL_CORRUPTION
void CTextExecuteBuffer::AddColorOverride(int index, const CTextColor& color) {
  Add(rs_new_line(200) CColorOverrideInstruction(index, color));
#else
void CTextExecuteBuffer::AddColorOverride(int idx, const CTextColor& color) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CColorOverrideInstruction(idx, color);
  Add(instruction);
#else
  Add(rs_new CColorOverrideInstruction(idx, color));
#endif
#endif
}

#if RSTL_CORRUPTION
void CTextExecuteBuffer::AddRemoveColorOverride(int index) {
  Add(rs_new_line(211) CRemoveColorOverrideInstruction(index));
#else
void CTextExecuteBuffer::AddRemoveColorOverride(int idx) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CRemoveColorOverrideInstruction(idx);
  Add(instruction);
#else
  Add(rs_new CRemoveColorOverrideInstruction(idx));
#endif
#endif
}

void CTextExecuteBuffer::AddLineSpacing(float spacing) {
#if RSTL_CORRUPTION
  const rstl::ncrc_ptr< CInstruction > instruction =
      rs_new_line(220) CLineSpacingInstruction(spacing);
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CLineSpacingInstruction(spacing);
#else
  rstl::ncrc_ptr< CInstruction > instruction(rs_new CLineSpacingInstruction(spacing));
#endif
#endif
  Add(instruction);
  mState.SetLineSpacing(spacing);
}

#if RSTL_CORRUPTION
void CTextExecuteBuffer::AddLineExtraSpace(int spacing) {
  const rstl::ncrc_ptr< CInstruction > instruction =
      rs_new_line(232) CLineExtraSpaceInstruction(spacing);
#else
void CTextExecuteBuffer::AddLineExtraSpace(int space) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CLineExtraSpaceInstruction(space);
#else
  rstl::ncrc_ptr< CInstruction > instruction(rs_new CLineExtraSpaceInstruction(space));
#endif
#endif
  Add(instruction);
#if RSTL_CORRUPTION
  mState.SetLineExtraSpace(spacing);
#else
  mState.SetLineExtraSpace(space);
#endif
}

#if RSTL_CORRUPTION
void CTextExecuteBuffer::AddCharacterExtraSpace(int spacing) {
  const rstl::ncrc_ptr< CInstruction > instruction =
      rs_new_line(244) CCharacterExtraSpaceInstruction(spacing);
  Add(instruction);
  mState.GetOptions().SetCharacterExtraSpace(spacing);
}
#else
#if !RSTL_PRIME1
void CTextExecuteBuffer::AddCharacterExtraSpace(int spacing) {
  rstl::ncrc_ptr< CInstruction > inst(rs_new CCharacterExtraSpaceInstruction(spacing));
  Add(inst);
  mState.GetOptions().SetCharacterExtraSpace(spacing);
}
#endif
#endif

#if RSTL_CORRUPTION
void CTextExecuteBuffer::AddJustification(EJustification justification) {
  mState.SetJustification(justification);
  if (mCurLine && mCurLine->GetWidth() == 0) {
    mCurLine->SetJustification(justification);
#else
void CTextExecuteBuffer::AddJustification(EJustification just) {
  mState.SetJustification(just);
  if (mCurLine && mCurLine->GetWidth() == 0) {
    mCurLine->SetJustification(just);
#endif
  }
}

#if RSTL_CORRUPTION
void CTextExecuteBuffer::AddVerticalJustification(EVerticalJustification justification) {
  mState.SetVerticalJustification(justification);
  if (mCurLine && mCurLine->GetWidth() == 0) {
    mCurLine->SetVerticalJustification(justification);
#else
void CTextExecuteBuffer::AddVerticalJustification(EVerticalJustification just) {
  mState.SetVerticalJustification(just);
  if (mCurLine && mCurLine->GetWidth() == 0) {
    mCurLine->SetVerticalJustification(just);
#endif
  }
}

void CTextExecuteBuffer::AddPushState() {
#if RSTL_CORRUPTION
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new_line(286) CPushStateInstruction();
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CPushStateInstruction();
#else
  rstl::ncrc_ptr< CInstruction > instruction(rs_new CPushStateInstruction());
#endif
#endif
  Add(instruction);
  mStateStack.push_front(mState);
}

void CTextExecuteBuffer::AddPopState() {
#if RSTL_CORRUPTION
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new_line(298) CPopStateInstruction();
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CPopStateInstruction();
#else
  rstl::ncrc_ptr< CInstruction > instruction(rs_new CPopStateInstruction());
#endif
#endif
  Add(instruction);
#if RSTL_CORRUPTION
  RS_VERIFY_THROW_DIRECT(302, !mStateStack.empty(), false, "popping a non-existant state!");
#endif
  mState = mStateStack.front();
  mStateStack.pop_front();
  if (mCurLine->GetWidth() == 0) {
    mCurLine->SetJustification(mState.GetJustification());
    mCurLine->SetVerticalJustification(mState.GetVerticalJustification());
  }
}

#if RSTL_CORRUPTION
void CTextExecuteBuffer::TerminateLineLTR(bool lastLine) {
  if (mCurLine->GetY() == 0 && mState.IsFinishedLoading()) {
    mCurLine->SetHeight(
        rstl::max_val(mState.GetFont()->GetCarriageAdvance(), mCurLine->GetHeight()));
#else
void CTextExecuteBuffer::TerminateLineLTR(
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
    bool lastLine
#endif
) {
  if (mCurLine->GetY() == 0 && mState.IsFinishedLoading()) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    mCurLine->SetHeight(
        rstl::max_val((*mState.GetFont())->GetCarriageAdvance(), mCurLine->GetHeight()));
#else
    mCurLine->SetHeight(
        rstl::max_val(mState.GetFont()->GetCarriageAdvance(), mCurLine->GetHeight()));
#endif
#endif
  }
#if RSTL_CORRUPTION
  mCurrentBlock->AddHeight(
      mCurrentBlock->GetVerticalJustification() == kVerticalJustification_Full || lastLine
          ? mCurLine->GetY()
#else
  mCurBlock->AddHeight(mCurBlock->GetVerticalJustification() == kVerticalJustification_Full
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
                               || lastLine
#endif
                           ? mCurLine->GetY()
#endif
                           : mState.GetLineExtraSpacing() +
                                 static_cast< int >(mCurLine->GetY() * mState.GetLineSpacing()));
}

#if RSTL_CORRUPTION
void CTextExecuteBuffer::TerminateLine(bool lastLine) {
  if (mCurrentBlock->GetTextDirection() == kTD_Horizontal) {
    TerminateLineLTR(lastLine);
#else
void CTextExecuteBuffer::TerminateLine(
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
    bool lastLine
#endif
) {
  if (mCurBlock->GetTextDirection() == kTD_Horizontal) {
    TerminateLineLTR(
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
        lastLine
#endif
    );
#endif
  }
}

void CTextExecuteBuffer::StartNewWord() {
#if RSTL_CORRUPTION
  rstl::ncrc_ptr< CInstruction > instruction(rs_new_line(381) CWordInstruction());
  mCurWordIt = Add(instruction);
  mCurrentX = 0;
  mCurrentY = 0;
  mCurWordX = mCurLine->GetWidth();
  mCurWordY = mCurLine->GetY();
  mCurLine->IncWords();
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CWordInstruction();
#else
  rstl::ncrc_ptr< CInstruction > instruction(rs_new CWordInstruction());
#endif
  mCurWordIt = Add(instruction);
  mCurX = 0;
  mCurY = 0;
  mCurWordX = mCurLine->GetWidth();
  mCurWordY = mCurLine->GetY();
  mCurLine->IncWords();
#endif
}

void CTextExecuteBuffer::StartNewLine() {
#if RSTL_CORRUPTION
  if (mCurLine) {
    TerminateLine(false);
#else
  if (mCurLine) {
    TerminateLine(
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
        false
#endif
    );
#endif
  }
#if RSTL_CORRUPTION
  const rstl::ncrc_ptr< CInstruction > instruction =
      rstl::ncrc_ptr< CInstruction >(rs_new_line(404) CLineInstruction(
          0, 0, 0, mState.GetJustification(), mState.GetVerticalJustification(), mImageBaseline));
  mCurWordIt = Add(instruction);
  mCurLine = static_cast< CLineInstruction* >(instruction.GetPtr());
#else
  const rstl::ncrc_ptr< CInstruction > instruction = rstl::ncrc_ptr< CInstruction >(
      rs_new CLineInstruction(0, 0, 0
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                              , mImageBaseline
#endif
                              , mState.GetJustification(), mState.GetVerticalJustification()
#if !RSTL_PRIME1
                              , mImageBaseline
#endif
                              ));
  mCurWordIt = Add(instruction);
  mCurLine = static_cast< CLineInstruction* >(instruction.GetPtr());
#endif
  mSpaceDistance = 0;
  StartNewWord();
#if RSTL_CORRUPTION
  mCurrentBlock->IncLines();
#else
  mCurBlock->IncLines();
#endif
}

#if RSTL_CORRUPTION
CLineInstruction::CLineInstruction(int words, int width, int height, EJustification justification,
                                   EVerticalJustification verticalJustification,
                                   bool imageBaseline)
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
#if !RSTL_PRIME1
CLineInstruction::CLineInstruction(int words, int width, int height, EJustification justification,
                                   EVerticalJustification verticalJustification,
                                   const bool imageBaseline)
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
#endif
#endif

void CTextExecuteBuffer::MoveWordLTR() {
#if RSTL_CORRUPTION
  mCurLine->SubWidth(mCurrentX + mSpaceDistance);
  if (mCurLine->GetY() > mCurWordY) {
    mCurLine->SetHeight(mCurWordY);
#else
  mCurLine->SubWidth(mCurX + mSpaceDistance);
  if (mCurLine->GetY() > mCurWordY) {
    mCurLine->SetHeight(mCurWordY);
#endif
  }
  mSpaceDistance = 0;
#if RSTL_CORRUPTION
  mCurLine->DecWords();
  TerminateLineLTR(false);
#else
  mCurLine->DecWords();
  TerminateLineLTR(
#endif
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_CORRUPTION
#if !RSTL_PRIME1
      false
#endif
  );
#endif
  const rstl::ncrc_ptr< CInstruction > instruction =
#if RSTL_CORRUPTION
      rs_new_line(434) CLineInstruction(1, mCurrentX, mCurrentY, mState.GetJustification(),
                                        mState.GetVerticalJustification(), mImageBaseline);
  mCurLine = static_cast< CLineInstruction* >(instruction.GetPtr());
  mInstructions.insert(mCurWordIt, instruction);
  mInstructions.insert(mCurWordIt, rs_new_line(437) CWordInstruction());
  mCurrentBlock->IncLines();
#else
      rs_new CLineInstruction(1, mCurX, mCurY
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                              , mImageBaseline
#endif
                              , mState.GetJustification(), mState.GetVerticalJustification()
#if !RSTL_PRIME1
                              , mImageBaseline
#endif
                              );
  mCurLine = static_cast< CLineInstruction* >(instruction.GetPtr());
  mInstructions.insert(mCurWordIt, instruction);
  mInstructions.insert(mCurWordIt, rs_new CWordInstruction());
  mCurBlock->IncLines();
#endif
}

int CTextExecuteBuffer::WrapOneLTR(const wchar_t* str, int len) {
  int rem = len;
  if (mState.IsFinishedLoading()) {
    int width, height;
#if RSTL_CORRUPTION
    mState.GetFont()->GetSize(mState.GetOptions(), width, height, str, len);
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    (*mState.GetFont())->GetSize(mState.GetOptions(), width, height, str, len);
#else
    mState.GetFont()->GetSize(mState.GetOptions(), width, height, str, len);
#endif

#endif
    if (mState.IsWordWrapping()) {
#if RSTL_CORRUPTION
      if (width + mCurLine->GetWidth() > mCurrentBlock->GetOutputWidth() &&
          mCurLine->GetWordCount() > 1 && mCurrentX + width < mCurrentBlock->GetOutputWidth()) {
#else
      if (width + mCurLine->GetWidth() > mCurBlock->GetOutputWidth() &&
          mCurLine->GetWordCount() > 1 && mCurX + width < mCurBlock->GetOutputWidth()) {
#endif
        MoveWordLTR();
      }
#if RSTL_CORRUPTION
      if (width + mCurLine->GetWidth() > mCurrentBlock->GetOutputWidth() && len > 1) {
        const int lineWidth = mCurLine->GetWidth();
        const int outputWidth = mCurrentBlock->GetOutputWidth();
        rem = rstl::max_val(1, rstl::min_val(len, 2 * ((outputWidth - lineWidth) /
                                                       mState.GetFont()->GetMonoWidth())));
#else
      if (width + mCurLine->GetWidth() > mCurBlock->GetOutputWidth() && len > 1) {
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
        const int lineWidth = mCurLine->GetWidth();
        const int outputWidth = mCurBlock->GetOutputWidth();
        rem = rstl::max_val(1, rstl::min_val(len, 2 * ((outputWidth - lineWidth) /
                                                       mState.GetFont()->GetMonoWidth())));
#endif
        int rank = 5;
#endif
        do {
          --rem;
#if RSTL_CORRUPTION
            mState.GetFont()->GetSize(mState.GetOptions(), width, height, str, rem);
        } while (width + mCurLine->GetWidth() > mCurrentBlock->GetOutputWidth() && rem > 1);
#else
          int endRank = rem > 1 ? CWordBreakTables::GetEndRank(str[rem - 1]) : 4;
          int beginRank = CWordBreakTables::GetBeginRank(str[rem]);
          if (endRank < rank && endRank <= beginRank) {
            rank = endRank;
          } else if (beginRank < rank && beginRank <= endRank) {
            rank = endRank;
          } else {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
            (*mState.GetFont())->GetSize(mState.GetOptions(), width, height, str, rem);
#else
            mState.GetFont()->GetSize(mState.GetOptions(), width, height, str, rem);
#endif
          }
        } while (width + mCurLine->GetWidth() > mCurBlock->GetOutputWidth() && rem > 1);
#endif
      }
    }
#if RSTL_CORRUPTION
    if (mState.GetFont()->GetCarriageAdvance() > mCurrentY) {
      mCurrentY = mState.GetFont()->GetCarriageAdvance();
    }
    mCurLine->TestLargestFont(mState.GetFont()->GetMonoWidth(),
                              mState.GetFont()->GetCarriageAdvance(),
                              mState.GetFont()->GetBaseLine());
    mCurLine->AddWidth(width);
    if (mCurLine->GetWidth() > mCurrentBlock->GetLineX()) {
      mCurrentBlock->SetWidth(mCurLine->GetWidth());
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    if ((*mState.GetFont())->GetCarriageAdvance() > mCurY) {
      mCurY = (*mState.GetFont())->GetCarriageAdvance();
    }
    mCurLine->TestLargestFont((*mState.GetFont())->GetMonoWidth(),
                              (*mState.GetFont())->GetCarriageAdvance(),
                              (*mState.GetFont())->GetBaseLine());
#else
    if (mState.GetFont()->GetCarriageAdvance() > mCurY) {
      mCurY = mState.GetFont()->GetCarriageAdvance();
    }
    mCurLine->TestLargestFont(mState.GetFont()->GetMonoWidth(),
                              mState.GetFont()->GetCarriageAdvance(),
                              mState.GetFont()->GetBaseLine());
#endif
    mCurLine->AddWidth(width);
    if (mCurLine->GetWidth() > mCurBlock->GetLineX()) {
      mCurBlock->SetWidth(mCurLine->GetWidth());
#endif
    }
#if RSTL_CORRUPTION
    mCurrentX += width;
#else
    mCurX += width;

#endif
    const rstl::ncrc_ptr< CInstruction > instruction = CTextInstruction::Create(str, rem);
    Add(instruction);
    if (rem != len) {
      StartNewLine();
    }
  }
  return rem;
}

void CTextExecuteBuffer::AddStringFragment(const wchar_t* str, int len) {
  int consumed = 0;
#if RSTL_CORRUPTION
  if (mCurrentBlock->GetTextDirection() == kTD_Horizontal) {
#else
  if (mCurBlock->GetTextDirection() == kTD_Horizontal) {
#endif
    while (consumed != len) {
      consumed += WrapOneLTR(str + consumed, len - consumed);
    }
  }
}

void CTextExecuteBuffer::AddString(const wchar_t* str, int len) {
  if (!mCurLine) {
    StartNewLine();
  }
  int wordStart = 0;
  int i = 0;
  for (; str[i] && (i < len || len == -1); ++i) {
    if (str[i] == L'\n' || str[i] == L' ') {
      AddStringFragment(str + wordStart, i - wordStart);
      wordStart = i + 1;
      if (str[i] == L'\n') {
        StartNewLine();
      } else {
        StartNewWord();
        int width = 0;
        int height = 0;
        if (mState.IsFinishedLoading()) {
          wchar_t space = L' ';
#if RSTL_CORRUPTION
          mState.GetFont()->GetSize(mState.GetOptions(), width, height, &space, 1);
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
          (*mState.GetFont())->GetSize(mState.GetOptions(), width, height, &space, 1);
#else
          mState.GetFont()->GetSize(mState.GetOptions(), width, height, &space, 1);
#endif
#endif
        }
#if RSTL_CORRUPTION
        if (mCurrentBlock->GetTextDirection() == kTD_Horizontal) {
          mCurLine->AddWidth(width);
#else
        if (mCurBlock->GetTextDirection() == kTD_Horizontal) {
          mCurLine->AddWidth(width);
#endif
          mSpaceDistance = width;
        } else {
          mCurLine->AddHeight(height);
          mSpaceDistance = height;
        }
      }
    }
  }
  if (i > wordStart) {
    AddStringFragment(str + wordStart, i - wordStart);
  }
}

rstl::vector< CToken > CTextExecuteBuffer::GetAssets() const {
  int count = 0;
#if RSTL_CORRUPTION
  for (InstList::const_iterator it = mInstructions.begin(); it != mInstructions.end(); ++it) {
#else
  for (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
       AUTO(it, mInstructions.begin());
#else
       InstList::const_iterator it = mInstructions.begin();
#endif
       it != mInstructions.end(); ++it) {
#endif
    count += (*it)->GetAssetCount();
  }
  rstl::vector< CToken > assets;
  if (count > 0) {
    assets.reserve(count);
#if RSTL_CORRUPTION
    for (InstList::const_iterator it = mInstructions.begin(); it != mInstructions.end(); ++it) {
#else
    for (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
         AUTO(it, mInstructions.begin());
#else
         InstList::const_iterator it = mInstructions.begin();
#endif
         it != mInstructions.end(); ++it) {
#endif
      (*it)->GetAssets(assets);
    }
  }
  return assets;
}

CTextRenderBuffer CTextExecuteBuffer::BuildRenderBuffer() const {
  CTextRenderBuffer buffer(CTextRenderBuffer::kM_AllocTally);
  {
    CFontRenderState state;
#if RSTL_CORRUPTION
    for (InstList::const_iterator it = mInstructions.begin(); it != mInstructions.end(); ++it) {
#else
    for (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
         AUTO(it, mInstructions.begin());
#else
         InstList::const_iterator it = mInstructions.begin();
#endif
         it != mInstructions.end(); ++it) {
#endif
      (*it)->Invoke(state, &buffer);
    }
  }
  buffer.SetMode(CTextRenderBuffer::kM_BufferFill);
  {
    CFontRenderState state;
#if RSTL_CORRUPTION
    for (InstList::const_iterator it = mInstructions.begin(); it != mInstructions.end(); ++it) {
#else
    for (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
         AUTO(it, mInstructions.begin());
#else
         InstList::const_iterator it = mInstructions.begin();
#endif
         it != mInstructions.end(); ++it) {
#endif
      (*it)->Invoke(state, &buffer);
    }
  }
  return buffer;
}

CTextRenderBuffer CTextExecuteBuffer::BuildRenderBufferPage(InstList::const_iterator start,
                                                            InstList::const_iterator pageStart,
                                                            InstList::const_iterator pageEnd) {
  CTextRenderBuffer buffer(CTextRenderBuffer::kM_AllocTally);
  {
    CFontRenderState state;
    for (InstList::const_iterator it = start; it != pageStart; ++it) {
      (*it)->PageInvoke(state, &buffer);
    }
    for (InstList::const_iterator it = pageStart; it != pageEnd; ++it) {
      (*it)->Invoke(state, &buffer);
    }
  }
  buffer.SetMode(CTextRenderBuffer::kM_BufferFill);
  {
    CFontRenderState state;
    for (InstList::const_iterator it = start; it != pageStart; ++it) {
      (*it)->PageInvoke(state, &buffer);
    }
    for (InstList::const_iterator it = pageStart; it != pageEnd; ++it) {
      (*it)->Invoke(state, &buffer);
    }
  }
  return buffer;
}

rstl::list< CTextRenderBuffer >
CTextExecuteBuffer::BuildRenderBufferPages(const CVector2i& extent) const {
  rstl::list< CTextRenderBuffer > pages;
  InstList::const_iterator it = mInstructions.begin();
  while (it != mInstructions.end()) {
    CTextRenderBuffer buffer(CTextRenderBuffer::kM_AllocTally);
    {
      CFontRenderState state;
#if RSTL_CORRUPTION
      for (InstList::const_iterator it2 = mInstructions.begin(); it2 != mInstructions.end();
           ++it2) {
#else
      for (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
           AUTO(it2, mInstructions.begin());
#else
           InstList::const_iterator it2 = mInstructions.begin();
#endif
           it2 != mInstructions.end(); ++it2) {
#endif
        (*it2)->Invoke(state, &buffer);
      }
    }
    buffer.SetMode(CTextRenderBuffer::kM_BufferFill);
    CFontRenderState state;
    InstList::const_iterator pageEnd = it;
    bool seeking = true;
#if RSTL_CORRUPTION
    for (InstList::const_iterator it2 = mInstructions.begin(); it2 != mInstructions.end(); ++it2) {
#else
    for (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
         AUTO(it2, mInstructions.begin());
#else
         InstList::const_iterator it2 = mInstructions.begin();
#endif
         it2 != mInstructions.end(); ++it2) {
#endif
      if (it2 == it) {
        seeking = false;
      }
      if (seeking) {
        (*it2)->PageInvoke(state, &buffer);
      } else {
        (*it2)->Invoke(state, &buffer);
#if RSTL_CORRUPTION
        if ((*it2)->IsLineInstruction() && state.GetY() > extent.GetY()) {
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        if (!buffer.HasSpaceAvailable(CVector2i(0, 0), extent)) {
#else
        if ((*it2)->IsLineInstruction() && state.GetY() > extent.GetY()) {
#endif
#endif
          break;
        }
        ++pageEnd;
      }
    }
    pages.push_back(BuildRenderBufferPage(mInstructions.begin(), it, pageEnd));
    it = pageEnd;
  }
  return pages;
}
