#ifndef _CWORDINSTRUCTION
#define _CWORDINSTRUCTION
#include "rstl/RstlVersions.h"

#include "Kyoto/Text/CInstruction.hpp"

class CWordInstruction : public CInstruction {
public:
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;

  void InvokeLTR(CFontRenderState& state) const;
};

#if !RSTL_PRIME1
CHECK_SIZEOF(CWordInstruction, 0x4)
#endif

#endif // _CWORDINSTRUCTION
