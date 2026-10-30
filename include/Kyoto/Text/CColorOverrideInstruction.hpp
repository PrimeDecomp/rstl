#ifndef _CCOLOROVERRIDEINSTRUCTION
#define _CCOLOROVERRIDEINSTRUCTION
#include "rstl/RstlVersions.h"

#include "Kyoto/Text/CInstruction.hpp"
#include "Kyoto/Text/CTextColor.hpp"

class CColorOverrideInstruction : public CInstruction {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  explicit CColorOverrideInstruction(int idx, const CTextColor& color)
  : mIdx(idx), mColor(color) {}
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
#else
  explicit CColorOverrideInstruction(int idx, const CTextColor& color) : mIdx(idx), mColor(color) {}
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
#endif

private:
  int mIdx;
  CTextColor mColor;
};

#if !RSTL_PRIME1
CHECK_SIZEOF(CColorOverrideInstruction, 0xc)
#endif

#endif // _CCOLOROVERRIDEINSTRUCTION
