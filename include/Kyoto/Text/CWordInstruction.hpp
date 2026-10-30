#ifndef _CWORDINSTRUCTION
#define _CWORDINSTRUCTION
#include "rstl/RstlVersions.h"

#include "Kyoto/Text/CInstruction.hpp"

class CWordInstruction : public CInstruction {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const;
#else
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
#endif

  void InvokeLTR(CFontRenderState& state) const;
};

#if !RSTL_PRIME1
CHECK_SIZEOF(CWordInstruction, 0x4)
#endif

#endif // _CWORDINSTRUCTION
