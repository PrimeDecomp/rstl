#ifndef _CREMOVECOLOROVERRIDEINSTRUCTION
#define _CREMOVECOLOROVERRIDEINSTRUCTION
#include "rstl/RstlVersions.h"

#include "Kyoto/Text/CInstruction.hpp"

class CRemoveColorOverrideInstruction : public CInstruction {
public:
  explicit CRemoveColorOverrideInstruction(int idx) : mIdx(idx) {}
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
#else
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
#endif

private:
  int mIdx;
};

#if !RSTL_PRIME1
CHECK_SIZEOF(CRemoveColorOverrideInstruction, 0x8)
#endif

#endif // _CREMOVECOLOROVERRIDEINSTRUCTION
