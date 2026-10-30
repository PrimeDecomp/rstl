#include "rstl/RstlVersions.h"
#include "Kyoto/Text/CRemoveColorOverrideInstruction.hpp"

#include "Kyoto/Text/CFontRenderState.hpp"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CRemoveColorOverrideInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
#else
void CRemoveColorOverrideInstruction::Invoke(CFontRenderState& state,
                                             CTextRenderBuffer* buf) const {
#endif

  state.GetOverride()[mIdx] = false;
  state.RefreshPalette();
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CRemoveColorOverrideInstruction::PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
#else
void CRemoveColorOverrideInstruction::PageInvoke(CFontRenderState& state,
                                                 CTextRenderBuffer* buf) const {
#endif

  Invoke(state, buf);
}

