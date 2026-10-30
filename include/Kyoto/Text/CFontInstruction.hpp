#ifndef _CFONTINSTRUCTION
#define _CFONTINSTRUCTION
#include "rstl/RstlVersions.h"

#include "Kyoto/TToken.hpp"
#include "Kyoto/Text/CInstruction.hpp"
#include "Kyoto/Text/CRasterFont.hpp"

class CFontInstruction : public CInstruction {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CFontInstruction(const TToken< CRasterFont >& font) : mFont(font) { mFont.Lock(); }
#else
  explicit CFontInstruction(const TToken< CRasterFont >& font) : mFont(font) { mFont.Lock(); }
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void GetAssets(rstl::vector< CToken >& assetsOut) const override;
#else
  // CInstruction
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buffer) const override;
  void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buffer) const override;
  void GetAssets(rstl::vector< CToken >& assets) const override;
#endif

  uint GetAssetCount() const override;

private:
  TToken< CRasterFont > mFont;
};

#if !RSTL_PRIME1
CHECK_SIZEOF(CFontInstruction, 0xc)
#endif

#endif // _CFONTINSTRUCTION
