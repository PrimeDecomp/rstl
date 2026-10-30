#ifndef _CPUSHSTATEINSTRUCTION
#define _CPUSHSTATEINSTRUCTION
#include "rstl/RstlVersions.h"

#include "Kyoto/Text/CInstruction.hpp"

class CPushStateInstruction : public CInstruction {
public:
  CPushStateInstruction() {}
  ~CPushStateInstruction() {}

  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
};

#if !RSTL_PRIME1
CHECK_SIZEOF(CPushStateInstruction, 0x4)
#endif

#endif // _CPUSHSTATEINSTRUCTION
