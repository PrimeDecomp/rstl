#include "rstl/RstlVersions.h"
#if RSTL_PRIME1
#include "Kyoto/TToken.hpp"
#include "Kyoto/Text/CRasterFont.hpp"
#endif

#include "Kyoto/Text/CFontInstruction.hpp"
#include "Kyoto/Text/CFontRenderState.hpp"
#include "Kyoto/Text/CTextRenderBuffer.hpp"

void CFontInstruction::Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  if (buf != nullptr) {
    buf->AddFontChange(mFont);
  }
  state.SetFont(mFont);
  state.RefreshPalette();
}

void CFontInstruction::GetAssets(rstl::vector< CToken >& assets) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  assets.push_back(mFont);
  if(mFont.IsLoaded()) {
    assets.push_back(TToken<CRasterFont> (mFont)->mTexture.data());
#else
  assets.push_back_unsafe(mFont);
  if (mFont.IsLoaded()) {
    assets.push_back_unsafe(TToken< CRasterFont >(mFont)->GetTexture());
#endif

  }
}

uint CFontInstruction::GetAssetCount() const { return 2; }

void CFontInstruction::PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const {
  Invoke(state, buf);
}

