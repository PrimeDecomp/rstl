#ifndef _CIMAGEINSTRUCTION
#define _CIMAGEINSTRUCTION
#include "rstl/RstlVersions.h"

#include "Kyoto/Text/CFontImageDef.hpp"
#include "Kyoto/Text/CInstruction.hpp"

class CImageInstruction : public CInstruction {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const override;
  void GetAssets(rstl::vector< CToken >& assetsOut) const override;
#else
  explicit CImageInstruction(const CFontImageDef& image) : mImage(image) {}

  // CInstruction
  void Invoke(CFontRenderState& state, CTextRenderBuffer* buffer) const override;
  void GetAssets(rstl::vector< CToken >& assets) const override;
#endif

  uint GetAssetCount() const override { return mImage.GetImages().size(); }
#if RSTL_PRIME1
  explicit CImageInstruction(const CFontImageDef& image) : mImage(image) {}
#endif

private:
  CFontImageDef mImage;
};

CHECK_SIZEOF(CImageInstruction, 0x20)

#endif // _CIMAGEINSTRUCTION
