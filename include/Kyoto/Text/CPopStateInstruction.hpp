#ifndef _CPOPSTATEINSTRUCTION
#define _CPOPSTATEINSTRUCTION
#include "rstl/RstlVersions.h"

#include "Kyoto/Text/CInstruction.hpp"

class CPopStateInstruction : public CInstruction {
public:
  CPopStateInstruction() {}
  ~CPopStateInstruction() {}

  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
};

#if !RSTL_PRIME1
CHECK_SIZEOF(CPopStateInstruction, 0x4)
#endif

#endif // _CPOPSTATEINSTRUCTION
