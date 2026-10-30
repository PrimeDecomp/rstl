#include "rstl/RstlVersions.h"
#include "Kyoto/Graphics/CTevCombiners.hpp"

#if !RSTL_PRIME1
#include "Kyoto/Graphics/CColor.hpp"
#endif

#include "Kyoto/Graphics/CGX.hpp"
#if RSTL_PRIME1
#include "Kyoto/Graphics/CGraphics.hpp"
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#define TEV_MAX_PASSES 2
#else
#include "dolphin/gx/GXTev.h"
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
bool CTevCombiners::sValidPasses[2] = {false, false};
#else
namespace {
const int kMaxPasses = 2;
}
#endif

int CTevCombiners::sNextUniquePass = 0;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
const CTevCombiners::AlphaVar CTevCombiners::skAlphaOne(CTevCombiners::kAS_Konst);
const CTevCombiners::ColorVar CTevCombiners::skColorOne(CTevCombiners::kCS_One);
#else
const CTevCombiners::AlphaVar CTevCombiners::skAlphaOne(kAS_Konst);
const CTevCombiners::ColorVar CTevCombiners::skColorOne(kCS_One);
const CTevCombiners::CTevPass
    CTevCombiners::kEnvPassthru(ColorPass(ColorVar(kCS_Zero), ColorVar(kCS_Zero),
                                          ColorVar(kCS_Zero), ColorVar(kCS_RasterColor)),
                                AlphaPass(AlphaVar(kAS_Zero), AlphaVar(kAS_Zero),
                                          AlphaVar(kAS_Zero), AlphaVar(kAS_RasterAlpha)));
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
const CTevCombiners::CTevPass CTevCombiners::kEnvPassthru(
    CTevCombiners::ColorPass((CTevCombiners::ColorVar(CTevCombiners::kCS_Zero)),
                             CTevCombiners::ColorVar(CTevCombiners::kCS_Zero),
                             CTevCombiners::ColorVar(CTevCombiners::kCS_Zero),
                             CTevCombiners::ColorVar(CTevCombiners::kCS_RasterColor)),
    CTevCombiners::AlphaPass(CTevCombiners::AlphaVar(CTevCombiners::kAS_Zero),
                             CTevCombiners::AlphaVar(CTevCombiners::kAS_Zero),
                             CTevCombiners::AlphaVar(CTevCombiners::kAS_Zero),
                             CTevCombiners::AlphaVar(CTevCombiners::kAS_RasterAlpha)));
#else
bool CTevCombiners::sValidPasses[kMaxPasses] = {false, false};
#endif

uint CTevCombiners::sNumEnabledPasses = -1;

CTevCombiners::AlphaVar::AlphaVar(EAlphaSrc src) : mSrc(src) {}
CTevCombiners::ColorVar::ColorVar(EColorSrc src) : mSrc(src) {}

void CTevCombiners::RecomputePasses() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_R3ME_00
  const uchar n = (sValidPasses[TEV_MAX_PASSES - 1] != 0) + 1;
#else
  uchar n = sValidPasses[TEV_MAX_PASSES - 1] != 0;
  n++;
#endif
  sNumEnabledPasses = n;
  CGX::SetNumTevStages(n);
#else
  uchar count = sValidPasses[kMaxPasses - 1] != false;
  sNumEnabledPasses = ++count;
  CGX::SetNumTevStages(count);
#endif

}

void CTevCombiners::Init() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  for (int i = 0; i < TEV_MAX_PASSES; ++i) {
#else
  for (int i = 0; i < kMaxPasses; ++i) {
#endif

    sValidPasses[i] = true;
  }
#if !RSTL_PRIME1
  sNumEnabledPasses = kMaxPasses;
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  sNumEnabledPasses = TEV_MAX_PASSES;
  for (int i = 0; i < TEV_MAX_PASSES; ++i) {
    DeletePass(static_cast< ERglTevStage >(i));
#else
  for (int i = 0; i < kMaxPasses; ++i) {
    DeletePass(i);
#endif

  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  for (int i = 0; i < TEV_MAX_PASSES; ++i) {
#else
  for (int i = 0; i < kMaxPasses; ++i) {
#endif

    sValidPasses[i] = false;
  }
  RecomputePasses();
}

void CTevCombiners::DeletePass(int stage) {
  SetPassCombiners(stage, kEnvPassthru);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  sValidPasses[static_cast< size_t >(stage)] = false;
#else
  sValidPasses[stage] = false;
#endif

  RecomputePasses();
}

void CTevCombiners::SetupPass(int stage, const CTevPass& pass) {
  if (&pass == &kEnvPassthru) {
    DeletePass(stage);
    return;
  }
  if (SetPassCombiners(stage, pass)) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    sValidPasses[static_cast< size_t >(stage)] = true;
#else
    sValidPasses[stage] = true;
#endif

    RecomputePasses();
  }
}

bool CTevCombiners::SetPassCombiners(int stage, const CTevPass& pass) {
  pass.Execute(stage);
  return true;
}

void CTevCombiners::CTevPass::Execute(int stage) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const GXTevStageID stageId = (GXTevStageID)(stage);
  CGX::SetTevColorIn(stageId, (GXTevColorArg)mColorPass.GetA().GetSource(),
                     (GXTevColorArg)mColorPass.GetB().GetSource(),
                     (GXTevColorArg)mColorPass.GetC().GetSource(),
                     (GXTevColorArg)mColorPass.GetD().GetSource());
  CGX::SetTevAlphaIn(stageId, (GXTevAlphaArg)mAlphaPass.GetA().GetSource(),
                     (GXTevAlphaArg)mAlphaPass.GetB().GetSource(),
                     (GXTevAlphaArg)mAlphaPass.GetC().GetSource(),
                     (GXTevAlphaArg)mAlphaPass.GetD().GetSource());
  CGX::SetTevColorOp(stageId, (GXTevOp)mColorOp.GetOp(), (GXTevBias)mColorOp.GetBias(),
                     (GXTevScale)mColorOp.GetScale(), mColorOp.GetClamp(),
                     (GXTevRegID)mColorOp.GetOutput());
  CGX::SetTevAlphaOp(stageId, (GXTevOp)mAlphaOp.GetOp(), (GXTevBias)mAlphaOp.GetBias(),
                     (GXTevScale)mAlphaOp.GetScale(), mAlphaOp.GetClamp(),
                     (GXTevRegID)mAlphaOp.GetOutput());
#else
  const GXTevStageID stageId = static_cast< GXTevStageID >(stage);
  CGX::SetTevColorIn(stageId, static_cast< GXTevColorArg >(mColorPass.GetA().GetSource()),
                     static_cast< GXTevColorArg >(mColorPass.GetB().GetSource()),
                     static_cast< GXTevColorArg >(mColorPass.GetC().GetSource()),
                     static_cast< GXTevColorArg >(mColorPass.GetD().GetSource()));
  CGX::SetTevAlphaIn(stageId, static_cast< GXTevAlphaArg >(mAlphaPass.GetA().GetSource()),
                     static_cast< GXTevAlphaArg >(mAlphaPass.GetB().GetSource()),
                     static_cast< GXTevAlphaArg >(mAlphaPass.GetC().GetSource()),
                     static_cast< GXTevAlphaArg >(mAlphaPass.GetD().GetSource()));
  CGX::SetTevColorOp(stageId, static_cast< GXTevOp >(mColorOp.GetOp()),
                     static_cast< GXTevBias >(mColorOp.GetBias()),
                     static_cast< GXTevScale >(mColorOp.GetScale()), mColorOp.GetClamp(),
                     static_cast< GXTevRegID >(mColorOp.GetOutput()));
  CGX::SetTevAlphaOp(stageId, static_cast< GXTevOp >(mAlphaOp.GetOp()),
                     static_cast< GXTevBias >(mAlphaOp.GetBias()),
                     static_cast< GXTevScale >(mAlphaOp.GetScale()), mAlphaOp.GetClamp(),
                     static_cast< GXTevRegID >(mAlphaOp.GetOutput()));
#endif

  CGX::SetTevKColorSel(stageId, GX_TEV_KCSEL_8_8);
  CGX::SetTevKAlphaSel(stageId, GX_TEV_KASEL_8_8);
}

#if !RSTL_PRIME1
// Guessed name.
void CTevCombiners::SetTevRegisterColor(int index, const CColor& color) {
  GXSetTevColor(static_cast< GXTevRegID >(index + 1), color.GetGXColor());
}
#endif

void CTevCombiners::ResetStates() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  for (int i = 0; i < TEV_MAX_PASSES; ++i) {
#else
  for (int i = 0; i < kMaxPasses; ++i) {
#endif

    sValidPasses[i] = false;
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  kEnvPassthru.Execute(kTS_Stage0);
#else
  kEnvPassthru.Execute(0);
#endif

  sNumEnabledPasses = 1;
  CGX::SetNumTevStages(1);
}

