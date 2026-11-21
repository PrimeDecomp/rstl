#ifndef _CTEXTEXECUTEBUFFER
#define _CTEXTEXECUTEBUFFER
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#endif

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#else
#include "Kyoto/Text/TextCommon.hpp"

#endif
#include "rstl/list.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/string.hpp"

#include "Kyoto/Text/CSaveableState.hpp"
#if !RSTL_CORRUPTION
#include "Kyoto/Text/CInstruction.hpp"
#endif

#if RSTL_CORRUPTION


class CInstruction;
#endif
class CBlockInstruction;
class CLineInstruction;
class CFontImageDef;
class CTextRenderBuffer;
class CVector2i;

class CTextExecuteBuffer {
  typedef rstl::list< rstl::ncrc_ptr< CInstruction > > InstList;

public:
  CTextExecuteBuffer();

  CTextRenderBuffer BuildRenderBuffer() const;
  rstl::list< CTextRenderBuffer > BuildRenderBufferPages(const CVector2i& extent) const;
  rstl::vector< CToken > GetAssets() const;

  void AddFont(const TToken< CRasterFont >& font);
  void AddImage(const CFontImageDef& image);
  void AddColor(EColorType type, const CTextColor& color);
  void AddColor(EColorType type, float r, float g, float b, float a) {
    AddColor(type, CTextColor(static_cast< uchar >(255.f * r), static_cast< uchar >(255.f * g),
                              static_cast< uchar >(255.f * b), static_cast< uchar >(255.f * a)));
  }
  void AddColorOverride(int idx, const CTextColor& color);
  void AddRemoveColorOverride(int idx);
  void AddLineSpacing(float spacing);
  void AddLineExtraSpace(int spacing);
  void AddCharacterExtraSpace(int spacing); // Guessed name
  void AddJustification(EJustification justification);
  void AddVerticalJustification(EVerticalJustification justification);
#if RSTL_CORRUPTION
  void AddWordWrapping(bool wrap) { mState.SetWordWrapping(wrap); }
#else
  void AddWordWrapping(RSTL_PRIME1_CONST bool wrap) { mState.SetWordWrapping(wrap); }
#endif
  void AddPushState();
  void AddPopState();
  void AddString(const wchar_t* str, int len);
  void AddString(const rstl::wstring& str) { AddString(str.data(), str.size()); }

#if RSTL_CORRUPTION
  void BeginBlock(int x, int y, int width, int height, bool imageBaseline, ETextDirection direction,
                  EJustification justification, EVerticalJustification verticalJustification);
#else
  void BeginBlock(int x, int y, int width, int height, const bool imageBaseline, ETextDirection dir,
                  EJustification just, EVerticalJustification vjust);

#endif
  void EndBlock();

  void Clear();

private:
  static CTextRenderBuffer BuildRenderBufferPage(InstList::const_iterator start,
                                                 InstList::const_iterator pageStart,
                                                 InstList::const_iterator pageEnd);
  InstList::iterator Add(const rstl::ncrc_ptr< CInstruction >& instruction) {
    mInstructions.push_back(instruction);
    return rstl::advance_iterator(mInstructions.begin(), -1);
  }
  void AddStringFragment(const wchar_t* str, int len);
  int WrapOneLTR(const wchar_t* str, int len);
  void MoveWordLTR();
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_GM8P_00
  bool ShouldWrapImage(const CFontImageDef& image) const;
#endif
#endif

  void StartNewLine();
  void StartNewWord();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void TerminateLine();
  void TerminateLineLTR();
#else
  void TerminateLine(bool lastLine);
  void TerminateLineLTR(bool lastLine);
#endif

  InstList mInstructions;
  CSaveableState mState;
#if RSTL_CORRUPTION
  CBlockInstruction* mCurrentBlock;
  CLineInstruction* mCurLine;
  InstList::iterator mCurWordIt;
  int mCurrentY;
  int mCurrentX;
  int mCurWordX;
  int mCurWordY;
#else
  CBlockInstruction* mCurBlock;
  CLineInstruction* mCurLine;
  InstList::iterator mCurWordIt;
  int mCurY;
  int mCurX;
  int mCurWordX;
  int mCurWordY;

#endif
  int mSpaceDistance;
  bool mImageBaseline;
  rstl::list< CSaveableState > mStateStack;
};

#if RSTL_CORRUPTION
CHECK_SIZEOF(CTextExecuteBuffer, 0xe0)
#else
CHECK_SIZEOF(CTextExecuteBuffer, (RSTL_VERSION >= RSTL_GM8P_00 ? 0xe0 : 0xdc))
#endif

#endif // _CTEXTEXECUTEBUFFER
