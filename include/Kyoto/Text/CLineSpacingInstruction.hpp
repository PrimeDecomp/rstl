#ifndef _CLINESPACINGINSTRUCTION
#define _CLINESPACINGINSTRUCTION
#include "rstl/RstlVersions.h"

#include "Kyoto/Text/CInstruction.hpp"

class CLineSpacingInstruction : public CInstruction {
public:
  CLineSpacingInstruction(float spacing) : mSpacing(spacing) {}
  ~CLineSpacingInstruction() {}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
#else
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
#endif

private:
  float mSpacing;
};

#if !RSTL_PRIME1
CHECK_SIZEOF(CLineSpacingInstruction, 0x8)
#endif

#endif // _CLINESPACINGINSTRUCTION
