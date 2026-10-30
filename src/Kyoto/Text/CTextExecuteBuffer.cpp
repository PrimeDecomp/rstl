#include "rstl/RstlVersions.h"
#include "Kyoto/Text/CTextExecuteBuffer.hpp"

#if RSTL_PRIME1
#include "Kyoto/Math/CVector2i.hpp"
#endif

#include "Kyoto/Text/CBlockInstruction.hpp"
#if !RSTL_PRIME1
#include "Kyoto/Text/CCharacterExtraSpaceInstruction.hpp"
#endif

#include "Kyoto/Text/CColorInstruction.hpp"
#include "Kyoto/Text/CColorOverrideInstruction.hpp"
#if RSTL_PRIME1
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
#include "Kyoto/Text/CWordBreakTables.hpp"
#include "Kyoto/Text/CWordInstruction.hpp"

#include "rstl/math.hpp"

CTextExecuteBuffer::CTextExecuteBuffer()
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
: mCurBlock(nullptr)
, mCurLine(nullptr)
, mCurWordIt(mInstructions.end())
, mCurWordX(0)
, mCurWordY(0)
#else
: mCurrentBlock(nullptr)
, mCurrentLine(nullptr)
, mCurrentWord(mInstructions.end())
, mCurrentWordX(0)
, mCurrentWordY(0)
#endif

, mSpaceDistance(0)
, mImageBaseline(false) {}

void CTextExecuteBuffer::Clear() {
  mInstructions.clear();
  mState = CSaveableState();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mCurBlock = nullptr;
  mCurLine = nullptr;
  mCurWordIt = mInstructions.end();
  mCurWordX = 0;
  mCurWordY = 0;
#else
  mCurrentBlock = nullptr;
  mCurrentLine = nullptr;
  mCurrentWord = mInstructions.end();
  mCurrentWordX = 0;
  mCurrentWordY = 0;
#endif

  mSpaceDistance = 0;
}

void CTextExecuteBuffer::BeginBlock(int x, int y, int width, int height, bool imageBaseline,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                    ETextDirection dir, EJustification just,
                                    EVerticalJustification vjust) {
#else
                                    ETextDirection direction, EJustification justification,
                                    EVerticalJustification verticalJustification) {
#endif

  mImageBaseline = imageBaseline;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::ncrc_ptr< CInstruction > instruction =
      rs_new CBlockInstruction(x, y, width, height, dir, just, vjust);
  mCurBlock = static_cast< CBlockInstruction* >(instruction.GetPtr());
#else
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CBlockInstruction(
      x, y, width, height, direction, justification, verticalJustification);
  mCurrentBlock = static_cast< CBlockInstruction* >(instruction.GetPtr());
#endif

  if (mState.IsFinishedLoading()) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    mCurBlock->TestLargestFont((*mState.GetFont())->GetMonoWidth(),
                                  (*mState.GetFont())->GetCarriageAdvance(),
                                  (*mState.GetFont())->GetBaseLine());
#else
    mCurrentBlock->TestLargestFont(mState.GetFont()->GetMonoWidth(),
                                   mState.GetFont()->GetCarriageAdvance(),
                                   mState.GetFont()->GetBaseLine());
#endif

  }
  Add(instruction);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mState.GetOptions().SetTextDirection(dir);
  mState.SetJustification(just);
  mState.SetVerticalJustification(vjust);
#else
  mState.GetOptions().SetTextDirection(direction);
  mState.SetJustification(justification);
  mState.SetVerticalJustification(verticalJustification);
#endif

}

void CTextExecuteBuffer::EndBlock() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (mCurLine) {
    TerminateLine();
#else
  if (mCurrentLine) {
    TerminateLine(true);
#endif

  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mCurLine = nullptr;
  mCurBlock = nullptr;
#else
  mCurrentLine = nullptr;
  mCurrentBlock = nullptr;
#endif

}

void CTextExecuteBuffer::AddFont(const TToken< CRasterFont >& font) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CFontInstruction(font);
  Add(instruction);
#else
  rstl::ncrc_ptr< CInstruction > inst(rs_new CFontInstruction(font));
  Add(inst);
#endif

  mState.SetFont(font);
  if (font.IsLoaded()) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    if (mCurBlock) {
      mCurBlock->TestLargestFont((*mState.GetFont())->GetMonoWidth(),
                                    (*mState.GetFont())->GetCarriageAdvance(),
                                    (*mState.GetFont())->GetBaseLine());
#else
    if (mCurrentBlock) {
      mCurrentBlock->TestLargestFont(mState.GetFont()->GetMonoWidth(),
                                     mState.GetFont()->GetCarriageAdvance(),
                                     mState.GetFont()->GetBaseLine());
#endif

    }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    if (mCurLine) {
      mCurLine->TestLargestFont((*mState.GetFont())->GetMonoWidth(),
                                   (*mState.GetFont())->GetCarriageAdvance(),
                                   (*mState.GetFont())->GetBaseLine());
#else
    if (mCurrentLine) {
      mCurrentLine->TestLargestFont(mState.GetFont()->GetMonoWidth(),
                                    mState.GetFont()->GetCarriageAdvance(),
                                    mState.GetFont()->GetBaseLine());
#endif

    }
  }
}

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

void CTextExecuteBuffer::AddImage(const CFontImageDef& image) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (!mCurLine) {
#else
  if (!mCurrentLine) {
#endif

    StartNewLine();
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (mCurBlock && image.IsLoaded()) {
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
    if (wrap) {
#else
  if (mCurrentBlock && image.IsLoaded()) {
    const bool newLine =
        mState.IsWordWrapping() &&
        mCurrentLine->GetWidth() + image.GetWidth() > mCurrentBlock->GetOutputWidth() &&
        mCurrentLine->GetWordCount() > 0;
    if (newLine) {
#endif

      StartNewLine();
    }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION < RSTL_GM8P_00
    mCurLine->TestLargestImage(image.GetMonoWidth(), image.GetMonoHeight(),
                                  image.CalculateBaseline());
#else
    mCurLine->TestLargestImage(image.GetMonoWidth(), image.GetHeight(),
                                  image.CalculateBaseline());
#endif
    if (mCurBlock->GetTextDirection() == kTD_Horizontal) {
      mCurLine->AddWidth(image.GetWidth());
      if (mCurLine->GetWidth() > image.GetWidth()) {
        mCurBlock->SetWidth(mCurLine->GetWidth());
#else
    mCurrentLine->TestLargestImage(image.GetMonoWidth(), image.GetHeight(),
                                   image.CalculateBaseline());
    if (mCurrentBlock->GetTextDirection() == kTD_Horizontal) {
      mCurrentLine->AddWidth(image.GetWidth());
      if (mCurrentLine->GetWidth() > image.GetWidth()) {
        mCurrentBlock->SetWidth(mCurrentLine->GetWidth());
#endif

      }
    }
  }
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CImageInstruction(image);
  Add(instruction);
}

void CTextExecuteBuffer::AddColor(EColorType type, const CTextColor& color) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CColorInstruction(type, color);
  Add(instruction);
#else
  Add(rs_new CColorInstruction(type, color));
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CTextExecuteBuffer::AddColorOverride(int idx, const CTextColor& color) {
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CColorOverrideInstruction(idx, color);
  Add(instruction);
#else
void CTextExecuteBuffer::AddColorOverride(int index, const CTextColor& color) {
  Add(rs_new CColorOverrideInstruction(index, color));
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CTextExecuteBuffer::AddRemoveColorOverride(int idx) {
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CRemoveColorOverrideInstruction(idx);
  Add(instruction);
#else
void CTextExecuteBuffer::AddRemoveColorOverride(int index) {
  Add(rs_new CRemoveColorOverrideInstruction(index));
#endif

}

void CTextExecuteBuffer::AddLineSpacing(float spacing) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CLineSpacingInstruction(spacing);
  Add(instruction);
#else
  rstl::ncrc_ptr< CInstruction > inst(rs_new CLineSpacingInstruction(spacing));
  Add(inst);
#endif

  mState.SetLineSpacing(spacing);
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CTextExecuteBuffer::AddLineExtraSpace(int space) {
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CLineExtraSpaceInstruction(space);
  Add(instruction);
  mState.SetLineExtraSpace(space);
#else
void CTextExecuteBuffer::AddLineExtraSpace(int spacing) {
  rstl::ncrc_ptr< CInstruction > inst(rs_new CLineExtraSpaceInstruction(spacing));
  Add(inst);
  mState.SetLineExtraSpace(spacing);
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CTextExecuteBuffer::AddJustification(EJustification just) {
  mState.SetJustification(just);
  if (mCurLine && mCurLine->GetWidth() == 0) {
    mCurLine->SetJustification(just);
#else
void CTextExecuteBuffer::AddCharacterExtraSpace(int spacing) {
  rstl::ncrc_ptr< CInstruction > inst(rs_new CCharacterExtraSpaceInstruction(spacing));
  Add(inst);
  mState.GetOptions().SetCharacterExtraSpace(spacing);
}

void CTextExecuteBuffer::AddJustification(EJustification justification) {
  mState.SetJustification(justification);
  if (mCurrentLine && mCurrentLine->GetWidth() == 0) {
    mCurrentLine->SetJustification(justification);
#endif

  }
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CTextExecuteBuffer::AddVerticalJustification(EVerticalJustification just) {
  mState.SetVerticalJustification(just);
  if (mCurLine && mCurLine->GetWidth() == 0) {
    mCurLine->SetVerticalJustification(just);
#else
void CTextExecuteBuffer::AddVerticalJustification(EVerticalJustification justification) {
  mState.SetVerticalJustification(justification);
  if (mCurrentLine && mCurrentLine->GetWidth() == 0) {
    mCurrentLine->SetVerticalJustification(justification);
#endif

  }
}

void CTextExecuteBuffer::AddPushState() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CPushStateInstruction();
  Add(instruction);
#else
  rstl::ncrc_ptr< CInstruction > inst(rs_new CPushStateInstruction());
  Add(inst);
#endif

  mStateStack.push_front(mState);
}

void CTextExecuteBuffer::AddPopState() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CPopStateInstruction();
  Add(instruction);
#else
  rstl::ncrc_ptr< CInstruction > inst(rs_new CPopStateInstruction());
  Add(inst);
#endif

  mState = mStateStack.front();
  mStateStack.pop_front();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (mCurLine->GetWidth() == 0) {
    mCurLine->SetJustification(mState.GetJustification());
    mCurLine->SetVerticalJustification(mState.GetVerticalJustification());
#else
  if (mCurrentLine->GetWidth() == 0) {
    mCurrentLine->SetJustification(mState.GetJustification());
    mCurrentLine->SetVerticalJustification(mState.GetVerticalJustification());
#endif

  }
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CTextExecuteBuffer::TerminateLineLTR() {
  if (mCurLine->GetY() == 0 && mState.IsFinishedLoading()) {
    mCurLine->SetHeight(
        rstl::max_val((*mState.GetFont())->GetCarriageAdvance(), mCurLine->GetHeight()));
#else
void CTextExecuteBuffer::TerminateLineLTR(bool lastLine) {
  if (mCurrentLine->GetY() == 0 && mState.IsFinishedLoading()) {
    mCurrentLine->SetHeight(
        rstl::max_val(mState.GetFont()->GetCarriageAdvance(), mCurrentLine->GetHeight()));
#endif

  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mCurBlock->AddHeight(
      mCurBlock->GetVerticalJustification() == kVerticalJustification_Full
          ? mCurLine->GetY()
#else
  mCurrentBlock->AddHeight(
      mCurrentBlock->GetVerticalJustification() == kVerticalJustification_Full || lastLine
          ? mCurrentLine->GetY()
#endif

          : mState.GetLineExtraSpacing() +
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                static_cast< int >(mCurLine->GetY() * mState.GetLineSpacing()));
#else
                static_cast< int >(mCurrentLine->GetY() * mState.GetLineSpacing()));
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CTextExecuteBuffer::TerminateLine() {
  if (mCurBlock->GetTextDirection() == kTD_Horizontal) {
    TerminateLineLTR();
#else
void CTextExecuteBuffer::TerminateLine(bool lastLine) {
  if (mCurrentBlock->GetTextDirection() == kTD_Horizontal) {
    TerminateLineLTR(lastLine);
#endif

  }
}

void CTextExecuteBuffer::StartNewWord() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CWordInstruction();
  mCurWordIt = Add(instruction);
  mCurX = 0;
  mCurY = 0;
  mCurWordX = mCurLine->GetWidth();
  mCurWordY = mCurLine->GetY();
  mCurLine->IncWords();
#else
  rstl::ncrc_ptr< CInstruction > inst(rs_new CWordInstruction());
  mCurrentWord = Add(inst);
  mCurrentX = 0;
  mCurrentY = 0;
  mCurrentWordX = mCurrentLine->GetWidth();
  mCurrentWordY = mCurrentLine->GetY();
  mCurrentLine->IncWords();
#endif

}

void CTextExecuteBuffer::StartNewLine() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (mCurLine) {
    TerminateLine();
#else
  if (mCurrentLine) {
    TerminateLine(false);
#endif

  }
  const rstl::ncrc_ptr< CInstruction > instruction =
      rstl::ncrc_ptr< CInstruction >(rs_new CLineInstruction(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
          0, 0, 0, mImageBaseline, mState.GetJustification(), mState.GetVerticalJustification()));
  mCurWordIt = Add(instruction);
  mCurLine = static_cast< CLineInstruction* >(instruction.GetPtr());
#else
          0, 0, 0, mState.GetJustification(), mState.GetVerticalJustification(), mImageBaseline));
  mCurrentWord = Add(instruction);
  mCurrentLine = static_cast< CLineInstruction* >(instruction.GetPtr());
#endif

  mSpaceDistance = 0;
  StartNewWord();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mCurBlock->IncLines();
#else
  mCurrentBlock->IncLines();
#endif

}

#if !RSTL_PRIME1
CLineInstruction::CLineInstruction(int words, int width, int height, EJustification justification,
                                   EVerticalJustification verticalJustification,
                                   const bool imageBaseline)
: mWordCount(words)
, mCurrentX(width)
, mCurrentY(height)
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

void CTextExecuteBuffer::MoveWordLTR() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mCurLine->SubWidth(mCurX + mSpaceDistance);
  if (mCurLine->GetY() > mCurWordY) {
    mCurLine->SetHeight(mCurWordY);
#else
  mCurrentLine->SubWidth(mCurrentX + mSpaceDistance);
  if (mCurrentLine->GetY() > mCurrentWordY) {
    mCurrentLine->SetHeight(mCurrentWordY);
#endif

  }
  mSpaceDistance = 0;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mCurLine->DecWords();
  TerminateLineLTR();
#else
  mCurrentLine->DecWords();
  TerminateLineLTR(false);
#endif

  const rstl::ncrc_ptr< CInstruction > instruction =
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      rs_new CLineInstruction(1, mCurX, mCurY, mImageBaseline, mState.GetJustification(),
                              mState.GetVerticalJustification());
  mCurLine = static_cast< CLineInstruction* >(instruction.GetPtr());
  mInstructions.insert(mCurWordIt, instruction);
  mInstructions.insert(mCurWordIt, rs_new CWordInstruction());
  mCurBlock->IncLines();
#else
      rs_new CLineInstruction(1, mCurrentX, mCurrentY, mState.GetJustification(),
                              mState.GetVerticalJustification(), mImageBaseline);
  mCurrentLine = static_cast< CLineInstruction* >(instruction.GetPtr());
  mInstructions.insert(mCurrentWord, instruction);
  mInstructions.insert(mCurrentWord, rs_new CWordInstruction());
  mCurrentBlock->IncLines();
#endif

}

int CTextExecuteBuffer::WrapOneLTR(const wchar_t* str, int len) {
  int rem = len;
  if (mState.IsFinishedLoading()) {
    int width, height;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    (*mState.GetFont())->GetSize(mState.GetOptions(), width, height, str, len);
#else
    mState.GetFont()->GetSize(mState.GetOptions(), width, height, str, len);
#endif

    if (mState.IsWordWrapping()) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      if (width + mCurLine->GetWidth() > mCurBlock->GetOutputWidth() &&
          mCurLine->GetWordCount() > 1 && mCurX + width < mCurBlock->GetOutputWidth()) {
#else
      if (width + mCurrentLine->GetWidth() > mCurrentBlock->GetOutputWidth() &&
          mCurrentLine->GetWordCount() > 1 && mCurrentX + width < mCurrentBlock->GetOutputWidth()) {
#endif

        MoveWordLTR();
      }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      if (width + mCurLine->GetWidth() > mCurBlock->GetOutputWidth() && len > 1) {
#else
      if (width + mCurrentLine->GetWidth() > mCurrentBlock->GetOutputWidth() && len > 1) {
        const int lineWidth = mCurrentLine->GetWidth();
        const int outputWidth = mCurrentBlock->GetOutputWidth();
        rem = rstl::max_val(1, rstl::min_val(len, 2 * ((outputWidth - lineWidth) /
                                                       mState.GetFont()->GetMonoWidth())));
#endif

        int rank = 5;
        do {
          --rem;
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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        } while (width + mCurLine->GetWidth() > mCurBlock->GetOutputWidth() && rem > 1);
#else
        } while (width + mCurrentLine->GetWidth() > mCurrentBlock->GetOutputWidth() && rem > 1);
#endif

      }
    }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    if ((*mState.GetFont())->GetCarriageAdvance() > mCurY) {
      mCurY = (*mState.GetFont())->GetCarriageAdvance();
#else
    if (mState.GetFont()->GetCarriageAdvance() > mCurrentY) {
      mCurrentY = mState.GetFont()->GetCarriageAdvance();
#endif

    }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    mCurLine->TestLargestFont((*mState.GetFont())->GetMonoWidth(),
                                 (*mState.GetFont())->GetCarriageAdvance(),
                                 (*mState.GetFont())->GetBaseLine());
    mCurLine->AddWidth(width);
    if (mCurLine->GetWidth() > mCurBlock->GetLineX()) {
      mCurBlock->SetWidth(mCurLine->GetWidth());
#else
    mCurrentLine->TestLargestFont(mState.GetFont()->GetMonoWidth(),
                                  mState.GetFont()->GetCarriageAdvance(),
                                  mState.GetFont()->GetBaseLine());
    mCurrentLine->AddWidth(width);
    if (mCurrentLine->GetWidth() > mCurrentBlock->GetLineX()) {
      mCurrentBlock->SetWidth(mCurrentLine->GetWidth());
#endif

    }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    mCurX += width;
#else
    mCurrentX += width;
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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int i = 0;
  if (mCurBlock->GetTextDirection() == kTD_Horizontal) {
    while (i != len) {
      i += WrapOneLTR(str + i, len - i);
#else
  int consumed = 0;
  if (mCurrentBlock->GetTextDirection() == kTD_Horizontal) {
    while (consumed != len) {
      consumed += WrapOneLTR(str + consumed, len - consumed);
#endif

    }
  }
}

void CTextExecuteBuffer::AddString(const wchar_t* str, int len) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (!mCurLine) {
#else
  if (!mCurrentLine) {
#endif

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
          (*mState.GetFont())->GetSize(mState.GetOptions(), width, height, &space, 1);
#else
          mState.GetFont()->GetSize(mState.GetOptions(), width, height, &space, 1);
#endif

        }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        if (mCurBlock->GetTextDirection() == kTD_Horizontal) {
          mCurLine->AddWidth(width);
#else
        if (mCurrentBlock->GetTextDirection() == kTD_Horizontal) {
          mCurrentLine->AddWidth(width);
#endif

          mSpaceDistance = width;
        } else {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
          mCurLine->AddHeight(height);
#else
          mCurrentLine->AddHeight(height);
#endif

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  for (AUTO(it, mInstructions.begin()); it != mInstructions.end(); ++it) {
#else
  for (InstList::const_iterator it = mInstructions.begin(); it != mInstructions.end(); ++it) {
#endif

    count += (*it)->GetAssetCount();
  }
  rstl::vector< CToken > assets;
  if (count > 0) {
    assets.reserve(count);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    for (AUTO(it, mInstructions.begin()); it != mInstructions.end(); ++it) {
#else
    for (InstList::const_iterator it = mInstructions.begin(); it != mInstructions.end(); ++it) {
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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    for (AUTO(it, mInstructions.begin()); it != mInstructions.end(); ++it) {
#else
    for (InstList::const_iterator it = mInstructions.begin(); it != mInstructions.end(); ++it) {
#endif

      (*it)->Invoke(state, &buffer);
    }
  }
  buffer.SetMode(CTextRenderBuffer::kM_BufferFill);
  {
    CFontRenderState state;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    for (AUTO(it, mInstructions.begin()); it != mInstructions.end(); ++it) {
#else
    for (InstList::const_iterator it = mInstructions.begin(); it != mInstructions.end(); ++it) {
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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      for (AUTO(it2, mInstructions.begin()); it2 != mInstructions.end(); ++it2) {
#else
      for (InstList::const_iterator it2 = mInstructions.begin(); it2 != mInstructions.end();
           ++it2) {
#endif

        (*it2)->Invoke(state, &buffer);
      }
    }
    buffer.SetMode(CTextRenderBuffer::kM_BufferFill);
    CFontRenderState state;
    InstList::const_iterator pageEnd = it;
    bool seeking = true;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    for (AUTO(it2, mInstructions.begin()); it2 != mInstructions.end(); ++it2) {
#else
    for (InstList::const_iterator it2 = mInstructions.begin(); it2 != mInstructions.end(); ++it2) {
#endif

      if (it2 == it) {
        seeking = false;
      }
      if (seeking) {
        (*it2)->PageInvoke(state, &buffer);
      } else {
        (*it2)->Invoke(state, &buffer);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        if (!buffer.HasSpaceAvailable(CVector2i(0, 0), extent)) {
#else
        if ((*it2)->IsLineInstruction() && state.GetY() > extent.GetY()) {
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

