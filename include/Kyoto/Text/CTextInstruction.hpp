#ifndef _CTEXTINSTRUCTION
#define _CTEXTINSTRUCTION
#include "rstl/RstlVersions.h"

#include "Kyoto/Text/CInstruction.hpp"

class CTextInstruction : public CInstruction {
public:
  static CTextInstruction* Create(const wchar_t* str, const int len);
  CTextInstruction(const wchar_t* str, int length);
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buffer) const override;

private:
  int mLength;
  wchar_t mString[1];
};

#if !RSTL_PRIME1
CHECK_SIZEOF(CTextInstruction, 0xc)
#endif

#endif // _CTEXTINSTRUCTION
