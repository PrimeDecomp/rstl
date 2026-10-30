#include "rstl/RstlVersions.h"
#include "Kyoto/Text/CTextExecuteBuffer.hpp"

#include "Kyoto/Math/CVector2i.hpp"

#include "Kyoto/Text/CBlockInstruction.hpp"
#if !RSTL_PRIME1
#include "Kyoto/Text/CCharacterExtraSpaceInstruction.hpp"
#endif

#include "Kyoto/Text/CColorInstruction.hpp"
#include "Kyoto/Text/CColorOverrideInstruction.hpp"
#include "Kyoto/Text/CFontImageDef.hpp"

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
: mCurBlock(nullptr)
, mCurLine(nullptr)
, mCurWordIt(mInstructions.end())
, mCurWordX(0)
, mCurWordY(0)
, mSpaceDistance(0)
, mImageBaseline(false) {}

void CTextExecuteBuffer::Clear() {
  mInstructions.clear();
  mState = CSaveableState();
  mCurBlock = nullptr;
  mCurLine = nullptr;
  mCurWordIt = mInstructions.end();
  mCurWordX = 0;
  mCurWordY = 0;
  mSpaceDistance = 0;
}

void CTextExecuteBuffer::BeginBlock(int x, int y, int width, int height, bool imageBaseline,
                                    ETextDirection dir, EJustification just,
                                    EVerticalJustification vjust) {
  mImageBaseline = imageBaseline;
  const rstl::ncrc_ptr< CInstruction > instruction =
      rs_new CBlockInstruction(x, y, width, height, dir, just, vjust);
  mCurBlock = static_cast< CBlockInstruction* >(instruction.GetPtr());
  if (mState.IsFinishedLoading()) {
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
  }
  Add(instruction);
  mState.GetOptions().SetTextDirection(dir);
  mState.SetJustification(just);
  mState.SetVerticalJustification(vjust);
}

void CTextExecuteBuffer::EndBlock() {
  if (mCurLine) {
    TerminateLine(
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
        true
#endif
    );
  }
  mCurLine = nullptr;
  mCurBlock = nullptr;
}

void CTextExecuteBuffer::AddFont(const TToken< CRasterFont >& font) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CFontInstruction(font);
#else
  rstl::ncrc_ptr< CInstruction > instruction(rs_new CFontInstruction(font));
#endif
  Add(instruction);
  mState.SetFont(font);
  if (font.IsLoaded()) {
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
    }
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
  if (!mCurLine) {
    StartNewLine();
  }
  if (mCurBlock && image.IsLoaded()) {
// TODO: verify that this Prime/Echoes difference is real
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
      StartNewLine();
    }
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

void CTextExecuteBuffer::AddColorOverride(int idx, const CTextColor& color) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CColorOverrideInstruction(idx, color);
  Add(instruction);
#else
  Add(rs_new CColorOverrideInstruction(idx, color));
#endif
}

void CTextExecuteBuffer::AddRemoveColorOverride(int idx) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CRemoveColorOverrideInstruction(idx);
  Add(instruction);
#else
  Add(rs_new CRemoveColorOverrideInstruction(idx));
#endif
}

void CTextExecuteBuffer::AddLineSpacing(float spacing) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CLineSpacingInstruction(spacing);
#else
  rstl::ncrc_ptr< CInstruction > instruction(rs_new CLineSpacingInstruction(spacing));
#endif
  Add(instruction);
  mState.SetLineSpacing(spacing);
}

void CTextExecuteBuffer::AddLineExtraSpace(int space) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CLineExtraSpaceInstruction(space);
#else
  rstl::ncrc_ptr< CInstruction > instruction(rs_new CLineExtraSpaceInstruction(space));
#endif
  Add(instruction);
  mState.SetLineExtraSpace(space);
}

#if !RSTL_PRIME1
void CTextExecuteBuffer::AddCharacterExtraSpace(int spacing) {
  rstl::ncrc_ptr< CInstruction > inst(rs_new CCharacterExtraSpaceInstruction(spacing));
  Add(inst);
  mState.GetOptions().SetCharacterExtraSpace(spacing);
}
#endif

void CTextExecuteBuffer::AddJustification(EJustification just) {
  mState.SetJustification(just);
  if (mCurLine && mCurLine->GetWidth() == 0) {
    mCurLine->SetJustification(just);
  }
}

void CTextExecuteBuffer::AddVerticalJustification(EVerticalJustification just) {
  mState.SetVerticalJustification(just);
  if (mCurLine && mCurLine->GetWidth() == 0) {
    mCurLine->SetVerticalJustification(just);
  }
}

void CTextExecuteBuffer::AddPushState() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CPushStateInstruction();
#else
  rstl::ncrc_ptr< CInstruction > instruction(rs_new CPushStateInstruction());
#endif
  Add(instruction);
  mStateStack.push_front(mState);
}

void CTextExecuteBuffer::AddPopState() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const rstl::ncrc_ptr< CInstruction > instruction = rs_new CPopStateInstruction();
#else
  rstl::ncrc_ptr< CInstruction > instruction(rs_new CPopStateInstruction());
#endif
  Add(instruction);
  mState = mStateStack.front();
  mStateStack.pop_front();
  if (mCurLine->GetWidth() == 0) {
    mCurLine->SetJustification(mState.GetJustification());
    mCurLine->SetVerticalJustification(mState.GetVerticalJustification());
  }
}

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
  }
  mCurBlock->AddHeight(mCurBlock->GetVerticalJustification() == kVerticalJustification_Full
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
                               || lastLine
#endif
                           ? mCurLine->GetY()
                           : mState.GetLineExtraSpacing() +
                                 static_cast< int >(mCurLine->GetY() * mState.GetLineSpacing()));
}

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
  }
}

void CTextExecuteBuffer::StartNewWord() {
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
}

void CTextExecuteBuffer::StartNewLine() {
  if (mCurLine) {
    TerminateLine(
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
        false
#endif
    );
  }
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
  mSpaceDistance = 0;
  StartNewWord();
  mCurBlock->IncLines();
}

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

void CTextExecuteBuffer::MoveWordLTR() {
  mCurLine->SubWidth(mCurX + mSpaceDistance);
  if (mCurLine->GetY() > mCurWordY) {
    mCurLine->SetHeight(mCurWordY);
  }
  mSpaceDistance = 0;
  mCurLine->DecWords();
  TerminateLineLTR(
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
      false
#endif
  );
  const rstl::ncrc_ptr< CInstruction > instruction =
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
      if (width + mCurLine->GetWidth() > mCurBlock->GetOutputWidth() &&
          mCurLine->GetWordCount() > 1 && mCurX + width < mCurBlock->GetOutputWidth()) {
        MoveWordLTR();
      }
      if (width + mCurLine->GetWidth() > mCurBlock->GetOutputWidth() && len > 1) {
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
        const int lineWidth = mCurLine->GetWidth();
        const int outputWidth = mCurBlock->GetOutputWidth();
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
        } while (width + mCurLine->GetWidth() > mCurBlock->GetOutputWidth() && rem > 1);
      }
    }
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
    }
    mCurX += width;

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
  if (mCurBlock->GetTextDirection() == kTD_Horizontal) {
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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
          (*mState.GetFont())->GetSize(mState.GetOptions(), width, height, &space, 1);
#else
          mState.GetFont()->GetSize(mState.GetOptions(), width, height, &space, 1);
#endif
        }
        if (mCurBlock->GetTextDirection() == kTD_Horizontal) {
          mCurLine->AddWidth(width);
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
  for (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
       AUTO(it, mInstructions.begin());
#else
       InstList::const_iterator it = mInstructions.begin();
#endif
       it != mInstructions.end(); ++it) {
    count += (*it)->GetAssetCount();
  }
  rstl::vector< CToken > assets;
  if (count > 0) {
    assets.reserve(count);
    for (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
         AUTO(it, mInstructions.begin());
#else
         InstList::const_iterator it = mInstructions.begin();
#endif
         it != mInstructions.end(); ++it) {
      (*it)->GetAssets(assets);
    }
  }
  return assets;
}

CTextRenderBuffer CTextExecuteBuffer::BuildRenderBuffer() const {
  CTextRenderBuffer buffer(CTextRenderBuffer::kM_AllocTally);
  {
    CFontRenderState state;
    for (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
         AUTO(it, mInstructions.begin());
#else
         InstList::const_iterator it = mInstructions.begin();
#endif
         it != mInstructions.end(); ++it) {
      (*it)->Invoke(state, &buffer);
    }
  }
  buffer.SetMode(CTextRenderBuffer::kM_BufferFill);
  {
    CFontRenderState state;
    for (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
         AUTO(it, mInstructions.begin());
#else
         InstList::const_iterator it = mInstructions.begin();
#endif
         it != mInstructions.end(); ++it) {
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
      for (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
           AUTO(it2, mInstructions.begin());
#else
           InstList::const_iterator it2 = mInstructions.begin();
#endif
           it2 != mInstructions.end(); ++it2) {
        (*it2)->Invoke(state, &buffer);
      }
    }
    buffer.SetMode(CTextRenderBuffer::kM_BufferFill);
    CFontRenderState state;
    InstList::const_iterator pageEnd = it;
    bool seeking = true;
    for (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
         AUTO(it2, mInstructions.begin());
#else
         InstList::const_iterator it2 = mInstructions.begin();
#endif
         it2 != mInstructions.end(); ++it2) {
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

