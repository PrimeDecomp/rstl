#ifndef _CPUSHSTATEINSTRUCTION
#define _CPUSHSTATEINSTRUCTION
#include "rstl/RstlVersions.h"

#include "Kyoto/Text/CInstruction.hpp"

class CPushStateInstruction : public CInstruction {
public:
  CPushStateInstruction() {}
  ~CPushStateInstruction() {}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
#else
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
#endif

};

#if !RSTL_PRIME1
CHECK_SIZEOF(CPushStateInstruction, 0x4)
#endif

#endif // _CPUSHSTATEINSTRUCTION
