#ifndef _CTEXTEXECUTEBUFFER
#define _CTEXTEXECUTEBUFFER
#include "rstl/RstlVersions.h"

#include "Kyoto/Text/TextCommon.hpp"

#include "rstl/list.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/string.hpp"

#include "Kyoto/Text/CSaveableState.hpp"
#include "Kyoto/Text/CInstruction.hpp"

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
#if RSTL_PRIME1
  void AddLineSpacing(float spacing);
  void AddLineExtraSpace(int space);
  void AddCharacterExtraSpace(int space);
  void AddJustification(EJustification just);
  void AddVerticalJustification(EVerticalJustification just);
  void AddWordWrapping(const bool wrap) { mState.SetWordWrapping(wrap); }
  void AddPushState();
  void AddPopState();
#endif

  void AddImage(const CFontImageDef& image);
  void AddColor(EColorType type, const CTextColor& color);
  void AddColor(EColorType type, float r, float g, float b, float a) {
    AddColor(type, CTextColor(static_cast< uchar >(255.f * r), static_cast< uchar >(255.f * g),
                              static_cast< uchar >(255.f * b), static_cast< uchar >(255.f * a)));
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void AddRemoveColorOverride(int idx);
  void AddColorOverride(int idx, const CTextColor& color);
#else
  void AddColorOverride(int idx, const CTextColor& color);
  void AddRemoveColorOverride(int idx);
  void AddLineSpacing(float spacing);
  void AddLineExtraSpace(int spacing);
  void AddCharacterExtraSpace(int spacing); // Guessed name
  void AddJustification(EJustification justification);
  void AddVerticalJustification(EVerticalJustification justification);
  void AddWordWrapping(bool wrap) { mState.SetWordWrapping(wrap); }
  void AddPushState();
  void AddPopState();
  void AddString(const wchar_t* str, int len);
#endif

  void AddString(const rstl::wstring& str) { AddString(str.data(), str.size()); }
#if RSTL_PRIME1
  void AddString(const wchar_t* str, const int len);
#endif

  void BeginBlock(int x, int y, int width, int height, const bool imageBaseline, ETextDirection dir,
                  EJustification just, EVerticalJustification vjust);

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
  CBlockInstruction* mCurBlock;
  CLineInstruction* mCurLine;
  InstList::iterator mCurWordIt;
  int mCurY;
  int mCurX;
  int mCurWordX;
  int mCurWordY;

  int mSpaceDistance;
  bool mImageBaseline;
  rstl::list< CSaveableState > mStateStack;
};

CHECK_SIZEOF(CTextExecuteBuffer, (RSTL_VERSION >= RSTL_GM8P_00 ? 0xe0 : 0xdc))

#endif // _CTEXTEXECUTEBUFFER
