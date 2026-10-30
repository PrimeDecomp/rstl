#ifndef _CLINEEXTRASPACINGINSTRUCTION
#define _CLINEEXTRASPACINGINSTRUCTION
#include "rstl/RstlVersions.h"

#include "Kyoto/Text/CInstruction.hpp"

class CLineExtraSpaceInstruction : public CInstruction {
public:
  CLineExtraSpaceInstruction(int spacing) : mSpacing(spacing) {}
  ~CLineExtraSpaceInstruction() {}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
#else
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
#endif

private:
  int mSpacing;
};

#if !RSTL_PRIME1
CHECK_SIZEOF(CLineExtraSpaceInstruction, 0x8)
#endif

#endif // _CLINEEXTRASPACINGINSTRUCTION
