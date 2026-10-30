#ifndef _CPOPSTATEINSTRUCTION
#define _CPOPSTATEINSTRUCTION
#include "rstl/RstlVersions.h"

#include "Kyoto/Text/CInstruction.hpp"

class CPopStateInstruction : public CInstruction {
public:
  CPopStateInstruction() {}
  ~CPopStateInstruction() {}

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
CHECK_SIZEOF(CPopStateInstruction, 0x4)
#endif

#endif // _CPOPSTATEINSTRUCTION
