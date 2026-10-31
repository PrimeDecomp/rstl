#include "rstl/RstlVersions.h"
#include "Kyoto/Graphics/CCubeMaterial.hpp"

#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Basics/CStopwatch.hpp"

#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CCubeSurface.hpp"
#if !RSTL_PRIME1
#include "Kyoto/Graphics/CDisplayListReader.hpp"
#endif

#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGX_Impl.hpp" // IWYU pragma: keep

#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#if !RSTL_PRIME1
#include "Kyoto/Graphics/PortalPlane.hpp"
#endif

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Basics/CCast.hpp"

#include "rstl/math.hpp"
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "Prime/MetaRender/CCubeRenderer.hpp"
#include "Prime/MetroidPrime/CGameDebug.hpp"

#define CUBE_RENDERER PRIME_NAMESPACE CCubeRenderer
#else
#include "Echoes/MetaRender/CCubeRenderer.hpp"

#define CUBE_RENDERER ECHOES_NAMESPACE CCubeRenderer
#endif

#include "dolphin/mtx.h"
#if !RSTL_PRIME1
#include "string.h"
#endif

#include "Kyoto/MemoryCopy.hpp"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
// Material data is big-endian; keep direct loads on big-endian targets.
#if TARGET_LITTLE_ENDIAN
#define SBig(x) CBasics::SwapBytes(x)
#else
#define SBig(x) x
#endif
#else
typedef void (*TTevHandler)(const uint*& materialData, uint firstTev, uint& tevCount,
                            uint& tcgCount);
#endif

#if !RSTL_PRIME1
static void HandleThermalTevs(const uint*& materialData, uint firstTev, uint& tevCount,
                              uint& tcgCount);
static void HandleNormalTevs(const uint*& materialData, uint firstTev, uint& tevCount,
                             uint& tcgCount);
static void SetupAlphaMaskVtxDesc(uint vtxDesc);
#endif

static const float gkEpsilon32 = FLT_EPSILON;

#if RSTL_PRIME1
static CVector3f sPlayerPosition(0.f, 0.f, 0.f);
#endif

CVector3f CCubeMaterial::sViewingFrom(0.f, 0.f, 0.f);
#if !RSTL_PRIME1
static CVector3f sPlayerPosition(CVector3f::Zero());
#endif

static CTransform4f sTextureProjectionTransform(CTransform4f::Identity());
int sLastMaterialUnique = -1;
#if RSTL_PRIME1
static int sMaterialCachedState = 0;
const CCubeModel* CCubeMaterial::sLastModelCached = nullptr;
const CCubeModel* CCubeMaterial::sRenderingModel = nullptr;
const uchar* sLastMaterialCached = nullptr;
static float sReflectionAlpha = 0.f;
static float sLastTime = 0.f;
#endif

static float sThrobX = 1.f;
static float sThrobY = 1.f;
static int sReflectionStage = -1;
#if !RSTL_PRIME1
static int sAlphaMaskPostTexMtx = GX_PTIDENTITY;
static int sAlphaMaskTexCoord = GX_TEXCOORD_NULL;
static int sTransparencyKColor = -1;
static TTevHandler sTevHandler = HandleNormalTevs;
const CCubeModel* CCubeMaterial::sLastModelCached = nullptr;
const CCubeModel* CCubeMaterial::sRenderingModel = nullptr;
static int sMaterialCachedState = 0;
const uchar* sLastMaterialCached = nullptr;
static bool sKColorModulated = false;
static float sReflectionAlpha = 0.f;
static float sLastTime = 0.f;
#endif

static bool sbRenderModelBlack = false;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
static bool lbl_805A9559 = false;
#else
static bool sbRenderModelBlackKonst = false;
#endif

static bool sbRenderModelShadow = false;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
static CTexture* spShadowTexture = NULL;
#else
static CTexture* spShadowTexture = nullptr;
#endif

static uchar sChannel0DisableLightMask = 0;
static uchar sChannel1EnableLightMask = 0;
static const GXColor sGXBlack = {0, 0, 0, 255};
static const GXColor sGXWhite = {0xFF, 0xFF, 0xFF, 0xFF};

#if !RSTL_PRIME1
static const Mtx sEnvPostMtx = {
    {0.5f, 0.0f, 0.0f, 0.5f},
    {0.0f, 0.0f, 0.5f, 0.5f},
    {0.0f, 0.0f, 0.0f, 1.0f},
};
static Mtx sScrollTexMtx = {
    {1.f, 0.f, 0.f, 0.f},
    {0.f, 1.f, 0.f, 0.f},
    {0.f, 0.f, 1.f, 0.f},
};

struct SMtx {
  Mtx m;
};

union scanner_t {
  const uint* words;
  const uchar* bytes;
};

// Material data is stored big-endian; these are no-ops on a big-endian host.
#define ReadWord(word) BE_SWAP(*(word))
#define ReadFloat(word) BE_SWAP(*(word))
#endif

void CCubeMaterial::SetupBlendMode(const uint blendFactors, const CModelFlags& flags,
                                   bool alphaTest) {
  GXBlendFactor newSrcFactor = static_cast< GXBlendFactor >(blendFactors & 0xFFFF);
  GXBlendFactor newDstFactor = static_cast< GXBlendFactor >(blendFactors >> 0x10);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CModelFlags::ETrans blendMode = flags.GetTrans();
#else
  CModelFlags::ETrans blendMode = flags.GetTransSigned();
#endif

#if !RSTL_PRIME1
  GXCompare alphaCompare;
#endif

  if (alphaTest) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CGX::SetAlphaCompare(GX_GEQUAL, 64, GX_AOP_OR, GX_NEVER, 0);
#else
    alphaCompare = GX_GEQUAL;
#endif

    newSrcFactor = GX_BL_ONE;
    newDstFactor = GX_BL_ZERO;
  } else {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
#else
    alphaCompare = GX_ALWAYS;
#endif

  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  // TODO: how to check
  if (blendMode > CModelFlags::kT_Four && newSrcFactor == GX_BL_ONE) {
#else
  CGX::SetAlphaCompare(alphaCompare, 64, GX_AOP_AND, GX_ALWAYS, 0);

  if (blendMode > 4 && newSrcFactor == GX_BL_ONE) {
#endif

    newSrcFactor = GX_BL_SRCALPHA;
    if (newDstFactor == GX_BL_ZERO) {
      newDstFactor = blendMode > 6 ? GX_BL_ONE : GX_BL_INVSRCALPHA;
    }
  }
  CGX::SetBlendMode(GX_BM_BLEND, newSrcFactor, newDstFactor, GX_LO_CLEAR);
}

static void HandleTev(int tevCur, const uint* materialDataCur, const uint* texMapTexCoordFlags,
                      bool shadowMapsEnabled) {
  const GXTevStageID stage = static_cast< GXTevStageID >(tevCur);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const uint colorArgs = shadowMapsEnabled ? 0x7a04f : SBig(materialDataCur[0]);
  const uint alphaArgs = SBig(materialDataCur[1]);
  const uint colorOps = SBig(materialDataCur[2]);
  const uint alphaOps = SBig(materialDataCur[3]);
#else
  const uint colorArgs = shadowMapsEnabled ? 0x7a04f : ReadWord(&materialDataCur[0]);
  const uint alphaArgs = ReadWord(&materialDataCur[1]);
  const uint colorOps = ReadWord(&materialDataCur[2]);
  const uint alphaOps = ReadWord(&materialDataCur[3]);
#endif

  CGX::SetStandardDirectTev_Compressed(stage, colorArgs, alphaArgs, colorOps, alphaOps);

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  uint tmtcFlags = SBig(*texMapTexCoordFlags);
  uint matFlags = SBig(materialDataCur[4]);
#else
  uint tmtcFlags = ReadWord(texMapTexCoordFlags);
  uint matFlags = ReadWord(&materialDataCur[4]);
#endif

  CGX::SetTevOrder(stage, static_cast< GXTexCoordID >(tmtcFlags & 0xFF),
                   static_cast< GXTexMapID >(tmtcFlags >> 8 & 0xFF),
                   static_cast< GXChannelID >(matFlags & 0xFF));
  CGX::SetTevKColorSel(stage, static_cast< GXTevKColorSel >(matFlags >> 0x8 & 0xFF));
  CGX::SetTevKAlphaSel(stage, static_cast< GXTevKAlphaSel >(matFlags >> 0x10 & 0xFF));
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
static void SetFullBrightWorldTev(uint stage, uint colorArgs, bool lightmap) {
  // Retain the material's TEV operations and alpha path; replace only lighting inputs.
  uint fullBrightArgs = 0;
  for (uint shift = 0; shift < 20; shift += 5) {
    uint arg = (colorArgs >> shift) & 0x1f;
    if (arg == GX_CC_RASC || arg == GX_CC_RASA || arg == GX_CC_C1 || arg == GX_CC_A1 ||
        (lightmap && (arg == GX_CC_TEXC || arg == GX_CC_TEXA)))
      arg = GX_CC_ONE;
    fullBrightArgs |= arg << shift;
#else
static uint HandleThermalTev(int tevCur, const uint* materialDataCur,
                             const uint* texMapTexCoordFlags) {
  const GXTevStageID stage = static_cast< GXTevStageID >(tevCur);
  if (ReadWord(&materialDataCur[0]) == 0x7b84f) {
    CGX::SetTevColorIn(stage, GX_CC_ZERO, GX_CC_ONE, GX_CC_KONST, GX_CC_ZERO);
  } else {
    CGX::SetTevColorIn(stage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_TEXC, GX_CC_ZERO);
#endif

  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CGX::SetTevColorIn_Compressed(static_cast< GXTevStageID >(stage), fullBrightArgs);
#else
  CGX::SetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, GX_TRUE, GX_TEVPREV);
  CGX::SetTevAlphaIn_Compressed(stage, ReadWord(&materialDataCur[1]));
  CGX::SetTevAlphaOp_Compressed(stage, ReadWord(&materialDataCur[3]));

  uint tmtcFlags = ReadWord(texMapTexCoordFlags);
  uint matFlags = ReadWord(&materialDataCur[4]);
  const uint texCoord = tmtcFlags & 0xFF;
  CGX::SetTevOrder(stage, GX_TEXCOORD0, static_cast< GXTexMapID >(tmtcFlags >> 8 & 0xFF),
                   GX_COLOR_NULL);
  CGX::SetTevKColorSel(stage, static_cast< GXTevKColorSel >(matFlags >> 0x8 & 0xFF));
  CGX::SetTevKAlphaSel(stage, static_cast< GXTevKAlphaSel >(matFlags >> 0x10 & 0xFF));
  return texCoord;
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
static uint HandleAnimatedUV(const uint* uvAnim, GXTexMtx texMtx, GXPTTexMtx ptTexMtx) {
  static const Mtx postMtx = {
      {0.5f, 0.0f, 0.0f, 0.5f},
      {0.0f, 0.0f, 0.5f, 0.5f},
      {0.0f, 0.0f, 0.0f, 1.0f},
  };
  static Mtx texMtx1 = {
      {1.f, 0.f, 0.f, 0.f},
      {0.f, 1.f, 0.f, 0.f},
      {0.f, 0.f, 1.f, 0.f},
  };
  uint type = SBig(*uvAnim);
  const float* params = reinterpret_cast< const float* >(uvAnim + 1);
  switch (type) {
  case 0: {
#else
static uint HandleUVInvModelView(GXTexMtx texMtx, GXPTTexMtx ptTexMtx) {
#endif

    CTransform4f xf = CGraphics::GetViewMatrix().GetQuickInverse().MultiplyIgnoreTranslation(
        CGraphics::GetModelMatrix());
    xf.SetTranslation(CVector3f::Zero());
    CGX::LoadTexMtxImm(xf.GetCStyleMatrix(), texMtx, GX_MTX3x4);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CGX::LoadTexMtxImm(postMtx, ptTexMtx, GX_MTX3x4);
#else
  CGX::LoadTexMtxImm(sEnvPostMtx, ptTexMtx, GX_MTX3x4);
#endif

    return 1;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  case 1: {
#else
static uint HandleUVInvModelViewTranslated(GXTexMtx texMtx, GXPTTexMtx ptTexMtx) {
#endif

    CTransform4f xf = CGraphics::GetViewMatrix().GetQuickInverse() * CGraphics::GetModelMatrix();
    CGX::LoadTexMtxImm(xf.GetCStyleMatrix(), texMtx, GX_MTX3x4);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CGX::LoadTexMtxImm(postMtx, ptTexMtx, GX_MTX3x4);
#else
  CGX::LoadTexMtxImm(sEnvPostMtx, ptTexMtx, GX_MTX3x4);
#endif

    return 1;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  case 2: {
    const float f1 = SBig(params[0]);
    const float f2 = SBig(params[1]);
    const float f3 = SBig(params[2]);
    const float f4 = SBig(params[3]);
#else
static uint HandleUVScroll(const float* params, GXTexMtx texMtx) {
  const float f1 = ReadFloat(&params[0]);
  const float f2 = ReadFloat(&params[1]);
  const float f3 = ReadFloat(&params[2]);
  const float f4 = ReadFloat(&params[3]);
#endif

    const float seconds = CGraphics::GetSecondsMod900();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    texMtx1[0][3] = f1 + seconds * f3;
    texMtx1[1][3] = f2 + seconds * f4;
    CGX::LoadTexMtxImm(texMtx1, texMtx, GX_MTX3x4);
#else
  sScrollTexMtx[0][3] = f1 + seconds * f3;
  sScrollTexMtx[1][3] = f2 + seconds * f4;
  CGX::LoadTexMtxImm(sScrollTexMtx, texMtx, GX_MTX3x4);
#endif

    return 5;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  case 3: {
    const float f1 = SBig(params[0]);
    const float f2 = SBig(params[1]);
#else
static uint HandleUVRotation(const float* params, GXTexMtx texMtx) {
  const float f1 = ReadFloat(&params[0]);
  const float f2 = ReadFloat(&params[1]);
#endif

    const float seconds = CGraphics::GetSecondsMod900();
    const float angle = f1 + seconds * f2;
    const float asin = CMath::FastSinR(angle);
    const float acos = CMath::FastCosR(angle);
    Mtx mtx = {
        {acos, -asin, 0.f, 0.5f * (1.f - (acos - asin))},
        {asin, acos, 0.f, 0.5f * (1.f - (asin + acos))},
        {0.f, 0.f, 1.f, 0.f},
    };
    CGX::LoadTexMtxImm(mtx, texMtx, GX_MTX3x4);
    return 3;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  case 4:
  case 5: {
    const float f1 = SBig(params[0]);
    const float f2 = SBig(params[1]);
    const float f3 = SBig(params[2]);
    const float f4 = SBig(params[3]);
#else
static uint HandleUVFilmstrip(const float* params, uint type, GXTexMtx texMtx) {
  const float f1 = ReadFloat(&params[0]);
  const float f2 = ReadFloat(&params[1]);
  const float f3 = ReadFloat(&params[2]);
  const float f4 = ReadFloat(&params[3]);
#endif

    const float value = (f4 + CGraphics::GetSecondsMod900()) * f1 * f3;
    const float fmod = CMath::FastFmod(value, 1.f);
    const float fs = CCast::FtoS(fmod * f2);
    const float v2 = fs * f3;
    if (type == 4) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      texMtx1[0][3] = v2;
      texMtx1[1][3] = 0.f;
#else
    sScrollTexMtx[0][3] = v2;
    sScrollTexMtx[1][3] = 0.f;
#endif

    } else {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      texMtx1[0][3] = 0.f;
      texMtx1[1][3] = v2;
#else
    sScrollTexMtx[0][3] = 0.f;
    sScrollTexMtx[1][3] = v2;
#endif

    }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CGX::LoadTexMtxImm(texMtx1, texMtx, GX_MTX3x4);
#else
  CGX::LoadTexMtxImm(sScrollTexMtx, texMtx, GX_MTX3x4);
#endif

    return 5;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  case 6: {
    static const Mtx sTexMtx = {
        {0.f, 0.f, 0.f, 0.f},
        {0.f, 0.f, 0.f, 0.f},
        {0.f, 0.f, 0.f, 0.f},
    };
    static const Mtx sPtMtx = {
#else
static uint HandleUVModelMatrix(GXTexMtx texMtx, GXPTTexMtx ptTexMtx) {
  static const SMtx sPtMtx = {{
#endif

        {0.5f, 0.f, 0.f, 0.f},
        {0.f, 0.f, 0.5f, 0.f},
        {0.f, 0.f, 0.f, 1.f},
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    };
    const CTransform4f& mm = CGraphics::GetModelMatrix();
    const CVector3f& rowX = mm.GetRow(kDX);
    const CVector3f& rowY = mm.GetRow(kDY);
    const CVector3f& rowZ = mm.GetRow(kDZ);
    Mtx tmpTexMtx;
    memcpy(&tmpTexMtx, &sTexMtx, sizeof(Mtx));
    tmpTexMtx[0][0] = rowX[kDX];
    tmpTexMtx[0][1] = rowX[kDY];
    tmpTexMtx[0][2] = rowX[kDZ];
    tmpTexMtx[1][0] = rowY[kDX];
    tmpTexMtx[1][1] = rowY[kDY];
    tmpTexMtx[1][2] = rowY[kDZ];
    tmpTexMtx[2][0] = rowZ[kDX];
    tmpTexMtx[2][1] = rowZ[kDY];
    tmpTexMtx[2][2] = rowZ[kDZ];
    Mtx tmpPtMtx;
    memcpy(&tmpPtMtx, &sPtMtx, sizeof(Mtx));
    tmpPtMtx[0][3] = mm.Get03() * 0.05f;
    tmpPtMtx[1][3] = mm.Get13() * 0.05f;
    CGX::LoadTexMtxImm(tmpTexMtx, texMtx, GX_MTX3x4);
    CGX::LoadTexMtxImm(tmpPtMtx, ptTexMtx, GX_MTX3x4);
#else
  }};
  CTransform4f xf = CGraphics::GetModelMatrix();
  xf.SetTranslation(CVector3f::Zero());
  SMtx tmpPtMtx = sPtMtx;
  tmpPtMtx.m[0][3] = CGraphics::GetModelMatrix().Get03() * 0.05f;
  tmpPtMtx.m[1][3] = CGraphics::GetModelMatrix().Get13() * 0.05f;
  CGX::LoadTexMtxImm(xf.GetCStyleMatrix(), texMtx, GX_MTX3x4);
  CGX::LoadTexMtxImm(tmpPtMtx.m, ptTexMtx, GX_MTX3x4);
#endif

    return 1;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  case 7: {
    static const Mtx sPtMtx = {
#else
static uint HandleUVCylinder(const float* params, GXTexMtx texMtx, GXPTTexMtx ptTexMtx) {
  static const SMtx sPtMtx = {{
#endif

        {0.f, 0.f, 0.f, 0.f},
        {0.f, 0.f, 0.f, 0.f},
        {0.f, 0.f, 0.f, 1.f},
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    };
#else
  }};
#endif

    const CTransform4f& vm = CGraphics::GetViewMatrix();
    CTransform4f xf = CGraphics::GetViewMatrix().GetQuickInverse().MultiplyIgnoreTranslation(
        CGraphics::GetModelMatrix());
    xf.SetTranslation(CVector3f::Zero());
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    Mtx tmpPtMtx;
    memcpy(&tmpPtMtx, &sPtMtx, sizeof(Mtx));
    float scale = SBig(params[0]);
#else
  SMtx tmpPtMtx = sPtMtx;
  float scale = ReadFloat(&params[0]);
#endif

    scale = 0.5f * scale;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    tmpPtMtx[0][0] = scale;
    tmpPtMtx[0][3] = CMath::FastFmod(0.025f * (vm.Get03() + vm.Get13()) * SBig(params[1]), 1.f);
    tmpPtMtx[1][2] = scale;
    tmpPtMtx[1][3] = CMath::FastFmod(0.05f * vm.Get23() * SBig(params[1]), 1.f);
#else
  tmpPtMtx.m[0][0] = scale;
  tmpPtMtx.m[0][3] =
      CMath::FastFmod(0.025f * (vm.Get03() + vm.Get13()) * ReadFloat(&params[1]), 1.f);
  tmpPtMtx.m[1][2] = scale;
  tmpPtMtx.m[1][3] = CMath::FastFmod(0.05f * vm.Get23() * ReadFloat(&params[1]), 1.f);
#endif

    CGX::LoadTexMtxImm(xf.GetCStyleMatrix(), texMtx, GX_MTX3x4);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CGX::LoadTexMtxImm(tmpPtMtx, ptTexMtx, GX_MTX3x4);
#else
  CGX::LoadTexMtxImm(tmpPtMtx.m, ptTexMtx, GX_MTX3x4);
#endif

    return 3;
  }
#if !RSTL_PRIME1
static uint HandleAnimatedUV(const uint* uvAnim, GXTexMtx texMtx, GXPTTexMtx ptTexMtx) {
  const uint type = ReadWord(uvAnim);
  switch (type) {
  case 0:
    return HandleUVInvModelView(texMtx, ptTexMtx);
  case 1:
    return HandleUVInvModelViewTranslated(texMtx, ptTexMtx);
  case 2:
    return HandleUVScroll(reinterpret_cast< const float* >(uvAnim + 1), texMtx);
  case 3:
    return HandleUVRotation(reinterpret_cast< const float* >(uvAnim + 1), texMtx);
  case 4:
  case 5:
    return HandleUVFilmstrip(reinterpret_cast< const float* >(uvAnim + 1), type, texMtx);
  case 6:
    return HandleUVModelMatrix(texMtx, ptTexMtx);
  case 7:
    return HandleUVCylinder(reinterpret_cast< const float* >(uvAnim + 1), texMtx, ptTexMtx);
#endif

  default:
    return 0;
  }
}

#if !RSTL_PRIME1
static void HandleAlphaMask(uint vtxDesc, uint& tevCount, uint& texCount, uint& tcgCount) {
  const GXTevStageID stage = static_cast< GXTevStageID >(tevCount);
  const GXTexMapID texMap = static_cast< GXTexMapID >(texCount);
  sAlphaMaskTexCoord = tcgCount;
  const CTexture& texture = CUBE_RENDERER::That()->GetAlphaMaskRamp();
  sAlphaMaskPostTexMtx = tcgCount * 3 + GX_PTTEXMTX0;

  GXTexObj texObj;
  GXInitTexObj(&texObj, const_cast< void* >(texture.GetConstBitMapData(0)), texture.GetWidth(),
               texture.GetHeight(), GX_TF_I4, GX_CLAMP, GX_CLAMP, GX_FALSE);
  GXInitTexObjLOD(&texObj, GX_NEAR, GX_NEAR, 0.f, 0.f, 0.f, GX_FALSE, GX_FALSE, GX_ANISO_1);
  GXLoadTexObj(&texObj, texMap);
  CTexture::InvalidateTexmap(texMap);

  CGX::SetNumTevStages(tevCount + 1);
  CGX::SetNumTexGens(tcgCount + 1);
  CGX::SetStandardTevColorAlphaOp(stage);
  CGX::SetTevColorIn(stage, GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO);
  CGX::SetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_APREV, GX_CA_TEXA, GX_CA_ZERO);
  CGX::SetTevOrder(stage, static_cast< GXTexCoordID >(sAlphaMaskTexCoord), texMap, GX_COLOR_NULL);
  CGraphics::SetAlphaCompare(kAF_Greater, 0, kAO_And, kAF_Always, 0);
  CGX::LoadTexMtxImm(PortalPlane::GetTextureTransform().GetCStyleMatrix(), sAlphaMaskPostTexMtx,
                     GX_MTX3x4);
  CGX::SetTexCoordGen(static_cast< GXTexCoordID >(sAlphaMaskTexCoord), GX_TG_MTX3x4, GX_TG_POS,
                      static_cast< GXTexMtx >(GX_PNMTX0), GX_FALSE,
                      static_cast< GXPTTexMtx >(sAlphaMaskPostTexMtx));
  SetupAlphaMaskVtxDesc(vtxDesc);

  ++tevCount;
  ++texCount;
  ++tcgCount;
}

static void SetupAlphaMaskVtxDesc(uint vtxDesc) {
  CGX::SetVtxDescv_Compressed(vtxDesc);
  if (CDisplayListReader::GetAttributeType(vtxDesc, GX_VA_TEX6MTXIDX) == GX_DIRECT &&
      sAlphaMaskTexCoord < 8) {
    CGX::SetVtxDesc(GX_VA_TEX6MTXIDX, GX_NONE);
    CGX::SetVtxDesc(static_cast< GXAttr >(sAlphaMaskTexCoord + GX_VA_TEX0MTXIDX), GX_DIRECT);
  }
}

static void ModulateKColor(const CModelFlags& flags) {
  const CColor flagsColor = flags.GetColor();
  const CColor color = CColor::Modulate(
      flagsColor, reinterpret_cast< const CColor& >(CGX::GetTevKColor(GX_KCOLOR0)));
  CGX::SetTevKColor(GX_KCOLOR0, color.GetGXColor());
}

static bool TryModulateKColor(uint tevCount, uint& kColorCount, const CModelFlags& flags) {
  const char blendMode = flags.GetTrans();
  if ((blendMode != CModelFlags::kT_Additive && blendMode != CModelFlags::kT_Blend) ||
      tevCount != 1) {
    return false;
  }
  if (kColorCount == 1) {
    const CGX::STevState& state = CGX::GetTevState(GX_TEVSTAGE0);
    if (state.mColorInArgs == 0x7b94f &&
        (state.mAlphaInArgs == 0x390c7 || state.mAlphaInArgs == 0x31ce7)) {
      ModulateKColor(flags);
      sKColorModulated = true;
      return true;
    }
  }
  return false;
}
#endif

static void HandleTransparency(uint& finalTevCount, uint& finalKColorCount,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                               const CModelFlags& modelFlags, uint blendFactors, uint& finalCCFlags,
                               uint& finalACFlags) {
  const CModelFlags::ETrans blendMode = modelFlags.GetTrans();
  const CColor& color = modelFlags.GetColorRef();
#else
                               const CModelFlags& modelFlags, uint blendFactors) {
  if ((modelFlags.GetOtherFlags() & CModelFlags::kF_Unknown400) != 0 &&
      TryModulateKColor(finalTevCount, finalKColorCount, modelFlags)) {
    return;
  }

  const CModelFlags::ETrans blendMode = modelFlags.GetTransSigned();
  const CColor color = modelFlags.GetColor();
#endif

  if (blendMode == 2) {
    const GXBlendFactor srcFactor = static_cast< GXBlendFactor >(blendFactors >> 16);
    if (srcFactor == 1) {
      return;
    }
  }

  if (blendMode == 3) {
    const uint stageIndex = finalTevCount;
    const GXTevStageID stage = static_cast< GXTevStageID >(stageIndex);
    const GXTevStageID stage2 = static_cast< GXTevStageID >(stageIndex + 1);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CGX::SetTevColorIn(stage, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
    CGX::SetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
#else
    CGX::SetTevColorIn_Compressed(stage, 0x73def);
    CGX::SetTevAlphaIn_Compressed(stage, 0x1ce7);
#endif
    CGX::SetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);

    CGX::SetTevKColorSel(stage,
                        static_cast< GXTevKColorSel >(finalKColorCount + GX_TEV_KCSEL_K0_A));
    CGX::SetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    CGX::SetTevOrder(stage, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);

    CGX::SetTevDirect(stage);

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CGX::SetTevColorIn(stage2, GX_CC_CPREV, GX_CC_C0, GX_CC_KONST, GX_CC_ZERO);
    CGX::SetTevAlphaIn(stage2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
#else
    CGX::SetTevColorIn_Compressed(stage2, 0x7b840);
    CGX::SetTevAlphaIn_Compressed(stage2, 0x1ce7);
#endif
    CGX::SetTevKColorSel(stage, static_cast< GXTevKColorSel >(finalKColorCount + GX_TEV_KCSEL_K0));

    CGX::SetStandardTevColorAlphaOp(stage2);
    CGX::SetTevDirect(stage2);
    CGX::SetTevOrder(stage2, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CGX::SetTevKColor(static_cast< GXTevKColorID >(finalKColorCount), color.GetGXColor());
#else
    CGX::SetTevKColor(static_cast< GXTevKColorID >(finalKColorCount), color);
#endif

    finalKColorCount += 1;
    finalTevCount += 2;
  } else {
    uint alphaArgs = 0x380c7;
    if (blendMode == 8) {
      alphaArgs = 0x31ce7;
    }

    uint colorArgs = 0x781cf;
    if (blendMode == 2) {
      colorArgs = 0x7018f;
    }

    const GXTevStageID stage = static_cast< GXTevStageID >(finalTevCount);
    CGX::SetTevColorIn_Compressed(stage, colorArgs);
    CGX::SetTevAlphaIn_Compressed(stage, alphaArgs);
    CGX::SetStandardTevColorAlphaOp(stage);
#if RSTL_PRIME1
    finalCCFlags = 0x100;
    finalACFlags = 0x100;
#endif

    CGX::SetTevDirect(stage);
    CGX::SetTevOrder(stage, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CGX::SetTevKColor(static_cast< GXTevKColorID >(finalKColorCount), color.GetGXColor());
#else
    sTransparencyKColor = finalKColorCount;
    CGX::SetTevKColor(static_cast< GXTevKColorID >(finalKColorCount), color);
#endif
    CGX::SetTevKColorSel(stage, static_cast< GXTevKColorSel >(finalKColorCount + GX_TEV_KCSEL_K0));

    CGX::SetTevKAlphaSel(stage,
                        static_cast< GXTevKAlphaSel >(finalKColorCount + GX_TEV_KASEL_K0_A));

    finalTevCount += 1;
    finalKColorCount += 1;
  }
}

static void DoModelShadow(uint texCount, uint tcgCount) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static const Mtx identity2D = {
#else
  static const SMtx identity2D = {{
#endif

      {0.f, 0.f, 0.f, 0.f},
      {0.f, 0.f, 0.f, 0.f},
      {0.f, 0.f, 0.f, 1.f},
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  };
#else
  }};
#endif

  spShadowTexture->Load(static_cast< GXTexMapID >(texCount), CTexture::kCM_Repeat);

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  Mtx mtx;
  memcpy(&mtx, &identity2D, sizeof(Mtx));
  mtx[0][0] = sTextureProjectionTransform.Get00();
  mtx[0][1] = sTextureProjectionTransform.Get01();
  mtx[0][2] = sTextureProjectionTransform.Get02();
  mtx[0][3] = sTextureProjectionTransform.Get03();
  mtx[1][0] = sTextureProjectionTransform.Get20();
  mtx[1][1] = sTextureProjectionTransform.Get21();
  mtx[1][2] = sTextureProjectionTransform.Get22();
  mtx[1][3] = sTextureProjectionTransform.Get23();
  CGX::LoadTexMtxImm(mtx, GX_TEXMTX5, GX_MTX3x4);
#else
  SMtx mtx = identity2D;
  mtx.m[0][0] = sTextureProjectionTransform.Get00();
  mtx.m[0][1] = sTextureProjectionTransform.Get01();
  mtx.m[0][2] = sTextureProjectionTransform.Get02();
  mtx.m[0][3] = sTextureProjectionTransform.Get03();
  mtx.m[1][0] = sTextureProjectionTransform.Get20();
  mtx.m[1][1] = sTextureProjectionTransform.Get21();
  mtx.m[1][2] = sTextureProjectionTransform.Get22();
  mtx.m[1][3] = sTextureProjectionTransform.Get23();
  CGX::LoadTexMtxImm(mtx.m, GX_TEXMTX5, GX_MTX3x4);
#endif

  CGX::SetTexCoordGen(static_cast< GXTexCoordID >(tcgCount), GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX5,
                      GX_FALSE, GX_PTIDENTITY);

  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
  CGX::SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
  CGX::SetTevOrder(GX_TEVSTAGE0, static_cast< GXTexCoordID >(tcgCount),
                   static_cast< GXTexMapID >(texCount), GX_COLOR1A1);

  CGX::SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
  CGX::SetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
  CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_C0);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_RASA, GX_CA_KONST, GX_CA_A0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_1);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
uint CCubeMaterial::HandleReflection(bool usesTevReg2, const GXTexMapID indTexSlot,
                                     const int indMtxScaleExp, const uint tevCount,
                                     const uint texCount, const uint tcgCount,
                                     const uint kColorCount, uint& finalCCFlags,
                                     uint& finalACFlags) {
#else
uint CCubeMaterial::HandleReflection(const GXTexMapID indTexSlot, const int indMtxScaleExp,
                                     const uint tevCount, const uint texCount, const uint tcgCount,
                                     const uint kColorCount) {
  bool usesTevReg2 = false;
  for (uint i = 0; i < tevCount; ++i) {
    if (static_cast< GXTevRegID >(CGX::MaskAndShiftLeft(
            CGX::GetTevState(static_cast< GXTevStageID >(i)).mColorOps, 3, 9)) == GX_TEVREG2) {
      usesTevReg2 = true;
      break;
    }
  }

  GXTevColorArg colorArg = GX_CC_CPREV;
#endif

  uint out = 0;
#if RSTL_PRIME1
  GXTevColorArg colorArg = GX_CC_CPREV;
#endif

  const GXTevStageID firstStage = static_cast< GXTevStageID >(tevCount);
  GXTevStageID finalTevCount;

  if (usesTevReg2) {
    colorArg = GX_CC_C2;
    CGX::SetTevColorIn(firstStage, GX_CC_ZERO, GX_CC_C2, GX_CC_KONST, GX_CC_ZERO);
    CGX::SetTevAlphaIn(firstStage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A2);
    CGX::SetTevColorOp(firstStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG2);
    CGX::SetTevAlphaOp(firstStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG2);
    CGX::SetTevOrder(firstStage, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_ZERO);
    out = 1;
  } else {
    colorArg = GX_CC_KONST;
  }

#if !RSTL_PRIME1
  const GXTevKColorID finalKColorCount = static_cast< GXTevKColorID >(kColorCount);
#endif

  const GXTexMapID texMap = static_cast< GXTexMapID >(texCount);
#if RSTL_PRIME1
  const GXTevKColorID finalKColorCount = static_cast< GXTevKColorID >(kColorCount);
#endif

  CColor reflectionColor;
  reflectionColor.Set(sReflectionAlpha, sReflectionAlpha, sReflectionAlpha, sReflectionAlpha);
  CGX::SetTevKColor(finalKColorCount, reflectionColor);
  CGX::SetTevKColorSel(firstStage,
                      static_cast< GXTevKColorSel >(finalKColorCount + GX_TEV_KCSEL_K0));

  finalTevCount = static_cast< GXTevStageID >(tevCount + out);

  CUBE_RENDERER::That()->GetRealReflection()->Load(texMap, CTexture::kCM_Clamp);

  GXTexCoordID texCoord;
  if (indTexSlot != GX_TEXMAP_NULL) {
    texCoord = static_cast< GXTexCoordID >(tcgCount + 1);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, static_cast< GXTexCoordID >(tcgCount), indTexSlot);
    CGX::SetTexCoordGen(static_cast< GXTexCoordID >(tcgCount), GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX6,
                        GX_TRUE, GX_PTTEXMTX6);
    CGX::SetTevIndWarp(finalTevCount, GX_INDTEXSTAGE0, 1, 0, GX_ITM_0);
    CGX::SetIndTexMtxSTPointFive(GX_ITM_0, static_cast< s8 >(indMtxScaleExp));
    if (sReflectionStage > static_cast< int >(finalTevCount)) {
      CGX::SetTevDirect(static_cast< GXTevStageID >(sReflectionStage));
    }
    sReflectionStage = finalTevCount;
  } else {
    texCoord = static_cast< GXTexCoordID >(tcgCount);
  }

  CGX::SetTexCoordGen(texCoord, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX7, GX_FALSE, GX_PTIDENTITY);

  CGX::SetTevColorIn(finalTevCount, GX_CC_ZERO, colorArg, GX_CC_TEXC, GX_CC_CPREV);
  CGX::SetTevAlphaIn(finalTevCount, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
  CGX::SetStandardTevColorAlphaOp(finalTevCount);
#if RSTL_PRIME1
  finalACFlags = 0;
  finalCCFlags = 0;
#endif

  CGX::SetTevOrder(finalTevCount, texCoord, texMap, GX_COLOR_NULL);
  return out + 1;
}

void CCubeMaterial::EnsureTevsDirect() {
  if (sReflectionStage == -1) {
    return;
  }

  CGX::SetNumIndStages(0);
  CGX::SetTevDirect(static_cast< GXTevStageID >(sReflectionStage));
  sReflectionStage = -1;
}

void CCubeMaterial::SetCurrentBlack() const {
  const uint* data = reinterpret_cast< const uint* >(mData);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const uint texCount = CBasics::SwapBytes(data[1]);
  const uint flags = CBasics::SwapBytes(data[0]);
  const uint vertexDesc = CBasics::SwapBytes(data[texCount + 2]);
#else
  const uint texCount = ReadWord(&data[1]);
  const uint flags = ReadWord(&data[0]);
  const uint vertexDesc = ReadWord(&data[texCount + 2]);
#endif

  if ((flags & (kStateFlag_DepthSorting | kStateFlag_AlphaTest)) != 0) {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_ONE, GX_LO_CLEAR);
  } else {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
  }

  CGX::SetVtxDescv_Compressed(vertexDesc);

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  GXTevColorArg colorArg = GX_CC_ZERO;
  if (lbl_805A9559) {
    colorArg = GX_CC_ONE;
  }
#else
  const GXTevColorArg colorArg = sbRenderModelBlackKonst ? GX_CC_ONE : GX_CC_ZERO;
#endif

  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, colorArg);

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  GXTevAlphaArg alphaArg = GX_CA_ZERO;
  if (lbl_805A9559) {
    alphaArg = GX_CA_KONST;
  }
#else
  const GXTevAlphaArg alphaArg = sbRenderModelBlackKonst ? GX_CA_KONST : GX_CA_ZERO;
#endif

  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, alphaArg);

  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_1);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_POS, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  CGX::SetNumTevStages(1);
  CGX::SetNumChans(0);
  CGX::SetNumTexGens(1);
  CGX::SetNumIndStages(0);
#if RSTL_PRIME1
  DEBUG_TOOLS_CALL(ApplyMaterialDebug(1));
#endif

}

static uint HandleColorChannels(uint chanCount, uint firstChan) {
  if (sbRenderModelShadow) {
    if (chanCount != 0) {
      CGX::SetChanAmbColor(CGX::Channel1, sGXBlack);
      CGX::SetChanMatColor(CGX::Channel1, sGXWhite);
      CGX::SetChanCtrl(CGX::Channel1, true, GX_SRC_REG, GX_SRC_REG,
                       static_cast< GXLightID >(sChannel1EnableLightMask), GX_DF_CLAMP, GX_AF_SPOT);

      const uchar chan0Lights = CGraphics::GetLightMask() & ~sChannel0DisableLightMask;
      CGX::SetChanCtrl_Compressed(CGX::Channel0, static_cast< GXLightID >(chan0Lights), firstChan);
      if (chan0Lights != 0) {
        CGX::SetChanMatColor(CGX::Channel0, sGXWhite);
      } else {
        CGX::SetChanMatColor(CGX::Channel0, CGX::GetChanAmbColor(CGX::Channel0));
      }
    }
    return 2;
  }

  if (chanCount == 2) {
    CGX::SetChanAmbColor(CGX::Channel1, sGXBlack);
    CGX::SetChanMatColor(CGX::Channel1, sGXWhite);
  } else {
    CGX::SetChanCtrl(CGX::Channel1, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                     GX_AF_NONE);
  }

  if (chanCount >= 1) {
    const uchar lightMask = CGraphics::GetLightMask();
    CGX::SetChanCtrl_Compressed(CGX::Channel0, static_cast< GXLightID >(lightMask), firstChan);
    if (lightMask != 0) {
      CGX::SetChanMatColor(CGX::Channel0, sGXWhite);
    } else {
      CGX::SetChanMatColor(CGX::Channel0, CGX::GetChanAmbColor(CGX::Channel0));
    }
  } else {
    CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                     GX_AF_NONE);
  }

  return chanCount;
}

static void HandleDepth(uint modelFlags, uint matFlags) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  GXCompare func = GX_NEVER;
#else
  GXCompare func;
#endif

  if ((modelFlags & CModelFlags::kF_DepthCompare) == 0) {
    func = GX_ALWAYS;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  } else if ((modelFlags & CModelFlags::kF_DepthGreater) != 0) {
#else
  } else if ((modelFlags & CModelFlags::kF_Unknown200) != 0) {
    if ((modelFlags & CModelFlags::kF_DepthGreater) != 0) {
#endif

    if ((modelFlags & CModelFlags::kF_DepthNonInclusive) != 0) {
      func = GX_GREATER;
    } else {
      func = GX_GEQUAL;
    }
  } else if ((modelFlags & CModelFlags::kF_DepthNonInclusive) != 0) {
    func = GX_LESS;
#if !RSTL_PRIME1
    } else {
      func = GX_EQUAL;
    }
#endif

  } else {
    func = GX_LEQUAL;
  }
  CGX::SetZMode(true, func,
                (modelFlags & CModelFlags::kF_DepthUpdate) == CModelFlags::kF_DepthUpdate &&
                    (matFlags & kStateFlag_DepthWrite) != 0);
}

static void DoPassthru(const uint finalTevCount) {
  const GXTevStageID stage = static_cast< GXTevStageID >(finalTevCount);
  CGX::SetTevColorIn(stage, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
  CGX::SetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
  CGX::SetTevOrder(stage, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  CGX::SetTevDirect(stage);
  CGX::SetStandardTevColorAlphaOp(stage);
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
union scanner_t {
  const uint* words;
  const uchar* bytes;
};
#else
static void HandleNormalTevs(const uint*& materialData, uint firstTev, uint& tevCount,
                             uint& tcgCount) {
  scanner_t materialDataCur;
  materialDataCur.words = materialData;
  scanner_t texMapTexCoordFlags;
  texMapTexCoordFlags = materialDataCur;
  texMapTexCoordFlags.bytes += tevCount * 20;
  const uint finalTevCount = tevCount + firstTev;
  for (uint i = firstTev; i < finalTevCount; ++i) {
    HandleTev(i, materialDataCur.words, texMapTexCoordFlags.words,
              sbRenderModelShadow && i == firstTev);
    materialDataCur.words += 5;
    texMapTexCoordFlags.words += 1;
  }

  scanner_t uvAnim;
  uvAnim = texMapTexCoordFlags;
  const uint finalTcgCount = ReadWord(uvAnim.words);
  for (uint i = 0; i < finalTcgCount; ++i) {
    CGX::SetTexCoordGen_Compressed(static_cast< GXTexCoordID >(i), ReadWord(&uvAnim.words[i + 1]));
  }
  uvAnim.words += finalTcgCount + 1;

  scanner_t end;
  end.bytes = uvAnim.bytes + ReadWord(uvAnim.words) + 4;
  const uint animCount = ReadWord(&uvAnim.words[1]);
  uvAnim.words += 2;
  uint animIdx = 0;
  GXTexMtx texMtx = GX_TEXMTX0;
  GXPTTexMtx ptTexMtx = GX_PTTEXMTX0;
  for (; animIdx < animCount;) {
    const int size = HandleAnimatedUV(uvAnim.words, texMtx, ptTexMtx);
    if (size == 0) {
      break;
    }
    ++animIdx;
    texMtx = static_cast< GXTexMtx >(texMtx + 3);
    ptTexMtx = static_cast< GXPTTexMtx >(ptTexMtx + 3);
    uvAnim.words += size;
  }

  materialData = end.words;
  tevCount = finalTevCount;
  tcgCount = finalTcgCount;
}

static void HandleThermalTevs(const uint*& materialData, uint firstTev, uint& tevCount,
                              uint& tcgCount) {
  scanner_t materialDataCur;
  materialDataCur.words = materialData;
  const uint matTevCount = tevCount;
  scanner_t texMapTexCoordFlags;
  texMapTexCoordFlags = materialDataCur;
  texMapTexCoordFlags.bytes += matTevCount * 20;
  scanner_t savedTexMapTexCoordFlags;
  savedTexMapTexCoordFlags = texMapTexCoordFlags;
  if ((ReadWord(&materialDataCur.words[2]) >> 9 & 3) == GX_TEVREG0) {
    materialDataCur.words += 5;
    texMapTexCoordFlags.words += 1;
  }
  const int texCoord = HandleThermalTev(firstTev, materialDataCur.words, texMapTexCoordFlags.words);

  scanner_t uvAnim;
  uvAnim = savedTexMapTexCoordFlags;
  uvAnim.words += matTevCount;
  const uint fullTcgCount = ReadWord(uvAnim.words);
  if (texCoord != GX_TEXCOORD_NULL) {
    for (int i = 0; i < fullTcgCount; ++i) {
      if (texCoord == i) {
        CGX::SetTexCoordGen_Compressed(GX_TEXCOORD0, ReadWord(&uvAnim.words[i + 1]));
        break;
      }
    }
  }
  uvAnim.words += fullTcgCount + 1;

  scanner_t end;
  end.bytes = uvAnim.bytes + ReadWord(uvAnim.words) + 4;
  const uint animCount = ReadWord(&uvAnim.words[1]);
  uvAnim.words += 2;
  uint animIdx = 0;
  GXTexMtx texMtx = GX_TEXMTX0;
  GXPTTexMtx ptTexMtx = GX_PTTEXMTX0;
  for (; animIdx < animCount;) {
    const int size = HandleAnimatedUV(uvAnim.words, texMtx, ptTexMtx);
    if (size == 0) {
      break;
    }
    ++animIdx;
    texMtx = static_cast< GXTexMtx >(texMtx + 3);
    ptTexMtx = static_cast< GXPTTexMtx >(ptTexMtx + 3);
    uvAnim.words += size;
  }

  materialData = end.words;
  tevCount = firstTev + 1;
  tcgCount = 1;
}
#endif

void CCubeMaterial::SetCurrent(const CModelFlags& flags, const CCubeSurface& surface,
                               const CCubeModel& model) const {
  if (mData == sLastMaterialCached) {
    switch (sMaterialCachedState) {
    case 1:
      if (sLastModelCached == sRenderingModel) {
        return;
      }
      break;
    case 2:
      break;
    default:
      return;
    }
  }

  if (sbRenderModelBlack) {
    SetCurrentBlack();
    return;
  }

#if !RSTL_PRIME1
  scanner_t materialDataCur;
  materialDataCur.bytes = GetData();
#endif

  uint numIndStages = 0;
  GXTexMapID indTexSlot = GX_TEXMAP0;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  scanner_t materialDataCur;
  materialDataCur.bytes = GetData();
  const uint matFlags = SBig(materialDataCur.words[0]);
  uint texCount = SBig(materialDataCur.words[1]);
#else
  const uint matFlags = ReadWord(&materialDataCur.words[0]);
  uint texCount = ReadWord(&materialDataCur.words[1]);
#endif

  const bool reflection =
      (matFlags & (kStateFlag_Reflection | kStateFlag_ReflectionSurfaceEye)) != 0;
#if !RSTL_PRIME1
  sLastMaterialCached = GetData();
  sRenderingModel = &model;
#endif

  if (reflection) {
#if RSTL_PRIME1
    sLastMaterialCached = materialDataCur.bytes;
    sRenderingModel = &model;
#endif

    EnsureViewDepStateCached((matFlags & kStateFlag_ReflectionSurfaceEye) != 0 ? &surface
                                                                               : nullptr);
#if !RSTL_PRIME1
  } else {
    sMaterialCachedState = 0;
#endif

  }

#if RSTL_PRIME1
  sRenderingModel = &model;
  sLastMaterialCached = GetData();
#endif

  if ((flags.GetOtherFlags() & CModelFlags::kF_NoTextureLock) == 0) {
    const rstl::vector< TCachedToken< CTexture > >& textures = model.GetTextures();
    materialDataCur.words += 2;
    for (uint i = 0; i < texCount; ++i) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      textures[SBig(*materialDataCur.words)].GetObject()->Load(static_cast< GXTexMapID >(i),
#else
      textures[ReadWord(materialDataCur.words)].GetObject()->Load(static_cast< GXTexMapID >(i),
#endif

                                                               CTexture::kCM_Repeat);
      ++materialDataCur.words;
    }
  } else {
    materialDataCur.words += texCount + 2;
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const int groupIdx = static_cast< int >(SBig(materialDataCur.words[1]));
#else
  const uint vertexDesc = ReadWord(materialDataCur.words);
  ++materialDataCur.words;
  materialDataCur.words += 2;
  const int groupIdx = static_cast< int >(ReadWord(materialDataCur.words));
  ++materialDataCur.words;

  uint finalKColorCount = 0;
  if ((matFlags & kStateFlag_KonstValues) != 0) {
    finalKColorCount = ReadWord(&materialDataCur.words[0]);
    for (uint i = 0; i < finalKColorCount; ++i) {
      CGX::SetTevKColor(static_cast< GXTevKColorID >(i),
                        CColor::ToGX(ReadWord(&materialDataCur.words[i + 1])));
    }
    materialDataCur.words += finalKColorCount + 1;
  }
#endif

  if (sLastMaterialUnique != -1 && sLastMaterialUnique == groupIdx && sMaterialCachedState == 0) {
#if !RSTL_PRIME1
    if (sKColorModulated) {
      ModulateKColor(flags);
    }
#endif

    return;
  }
  sLastMaterialUnique = groupIdx;
#if !RSTL_PRIME1
  sKColorModulated = false;
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CGX::SetVtxDescv_Compressed(SBig(materialDataCur.words[0]));
  materialDataCur.words += 2;
#else
  CGX::SetVtxDescv_Compressed(vertexDesc);
#endif

  const bool packedLightmaps = (matFlags & kStateFlag_LightmapUvArray) != 0;
  if (packedLightmaps != CCubeModel::IsUsingPackedLightmaps()) {
    model.SetUsingPackedLightmaps(packedLightmaps);
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  uint finalKColorCount = 0;
  if ((matFlags & kStateFlag_KonstValues) != 0) {
    finalKColorCount = SBig(materialDataCur.words[0]);
    for (uint i = 0; i < finalKColorCount; ++i) {
      CGX::SetTevKColor(static_cast< GXTevKColorID >(i),
                        CColor::ToGX(SBig(materialDataCur.words[i + 1])));
    }
    materialDataCur.words += finalKColorCount + 1;
  }

  const uint blendFactors = SBig(*materialDataCur.words);
  if (CUBE_RENDERER::That()->GetInAreaDraw()) {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  } else {
#else
  const uint blendFactors = ReadWord(materialDataCur.words);
#endif

    SetupBlendMode(blendFactors, flags, (matFlags & kStateFlag_AlphaTest) != 0);
#if RSTL_PRIME1
  }
#endif

  ++materialDataCur.words;
  const bool indTex = (matFlags & kStateFlag_ReflectionIndirectTexture) != 0;
  if (indTex) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    indTexSlot = static_cast< GXTexMapID >(SBig(*materialDataCur.words++));
#else
    indTexSlot = static_cast< GXTexMapID >(ReadWord(materialDataCur.words));
    ++materialDataCur.words;
#endif

  }

  HandleDepth(flags.GetOtherFlags(), matFlags);

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const uint chanCount = SBig(materialDataCur.words[0]);
  const uint firstChan = SBig(materialDataCur.words[1]);
#else
  const uint chanCount = ReadWord(&materialDataCur.words[0]);
  const uint firstChan = ReadWord(&materialDataCur.words[1]);
#endif

  materialDataCur.words += chanCount + 1;
  const uint finalNumColorChans = HandleColorChannels(chanCount, firstChan);

#if RSTL_PRIME1
  uint matTevCount;
#endif

  uint firstTev = 0;
  if (sbRenderModelShadow) {
    firstTev = 2;
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  matTevCount = SBig(*materialDataCur.words++);
  uint finalTevCount = matTevCount;
  uint finalCCFlags = 0;
  uint finalACFlags = 0;
  scanner_t texMapTexCoordFlags;
  texMapTexCoordFlags = materialDataCur;
  texMapTexCoordFlags.bytes += matTevCount * 20;
  bool usesTevReg2 = false;
#else
  uint finalTevCount = ReadWord(materialDataCur.words);
  ++materialDataCur.words;
  uint tcgCount = 0;
  sTevHandler(materialDataCur.words, firstTev, finalTevCount, tcgCount);
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const bool thermal = CUBE_RENDERER::That()->GetThermal();
  if (thermal) {
    scanner_t savedTexMapTexCoordFlags;
    savedTexMapTexCoordFlags = texMapTexCoordFlags;
    finalTevCount = firstTev + 1;
    finalCCFlags = SBig(materialDataCur.words[2]);
    if ((finalCCFlags >> 9 & 3) == GX_TEVREG0) {
      materialDataCur.words += 5;
      texMapTexCoordFlags.words += 1;
      const GXColor thermalColor = {0xc0, 0xc0, 0xc0, 0xc0};
      GXSetTevColor(GX_TEVREG0, thermalColor);
      finalCCFlags = SBig(materialDataCur.words[2]);
    }
    finalACFlags = SBig(materialDataCur.words[3]);
    HandleTev(firstTev, materialDataCur.words, texMapTexCoordFlags.words, sbRenderModelShadow);
    usesTevReg2 = false;
    texMapTexCoordFlags.words = savedTexMapTexCoordFlags.words + matTevCount;
  } else {
    finalTevCount = matTevCount + firstTev;
    for (uint i = firstTev; i < finalTevCount; ++i) {
      HandleTev(i, materialDataCur.words, texMapTexCoordFlags.words,
                sbRenderModelShadow && i == firstTev);
      if (DEBUG_TOOLS_GET_BOOL(kDO_FullBrightWorld, false) && model.GetModelIndex() != -1 &&
          !sbRenderModelShadow) {
        // In Prime's lightmapped world materials, texture slot zero contains the lightmap.
        const bool lightmap = (matFlags & kStateFlag_Lightmap) != 0 &&
                              ((SBig(*texMapTexCoordFlags.words) >> 8) & 0xff) == GX_TEXMAP0;
        SetFullBrightWorldTev(i, SBig(materialDataCur.words[0]), lightmap);
      }
      finalCCFlags = SBig(materialDataCur.words[2]);
      finalACFlags = SBig(materialDataCur.words[3]);
      if ((finalCCFlags >> 9 & 3) == GX_TEVREG2) {
        usesTevReg2 = true;
      }
      materialDataCur.words += 5;
      texMapTexCoordFlags.words += 1;
    }
#else
  if (flags.GetTransSigned() != CModelFlags::kT_Opaque) {
    HandleTransparency(finalTevCount, finalKColorCount, flags, blendFactors);
#endif

  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  materialDataCur = texMapTexCoordFlags;
  uint tcgCount = 0;
  if (thermal) {
    const uint fullTcgCount = SBig(materialDataCur.words[0]);
    tcgCount = fullTcgCount <= 2 ? fullTcgCount : 2;
    for (uint i = 0; i < tcgCount; ++i) {
      CGX::SetTexCoordGen_Compressed(static_cast< GXTexCoordID >(i),
                                    SBig(materialDataCur.words[i + 1]));
    }
    materialDataCur.words += fullTcgCount + 1;
  } else {
    tcgCount = SBig(materialDataCur.words[0]);
    for (uint i = 0; i < tcgCount; ++i) {
      CGX::SetTexCoordGen_Compressed(static_cast< GXTexCoordID >(i),
                                    SBig(materialDataCur.words[i + 1]));
    }
    materialDataCur.words += tcgCount + 1;
  }

  const uint animCount = SBig(materialDataCur.words[1]);
  materialDataCur.words += 2;
  uint animIdx = 0;
  GXTexMtx texMtx = GX_TEXMTX0;
  GXPTTexMtx ptTexMtx = GX_PTTEXMTX0;
  for (; animIdx < animCount;) {
    const int size = HandleAnimatedUV(materialDataCur.words, texMtx, ptTexMtx);
    if (size == 0) {
      break;
    }
    ++animIdx;
    texMtx = static_cast< GXTexMtx >(texMtx + 3);
    ptTexMtx = static_cast< GXPTTexMtx >(ptTexMtx + 3);
    materialDataCur.words += size;
  }

  if (flags.GetTrans() != CModelFlags::kT_Opaque) {
    HandleTransparency(finalTevCount, finalKColorCount, flags, blendFactors, finalCCFlags,
                       finalACFlags);
#else
  if ((flags.GetOtherFlags() & CModelFlags::kF_Unknown80) != 0) {
    HandleAlphaMask(vertexDesc, finalTevCount, texCount, tcgCount);
#endif

  }

  if (reflection) {
    if (sReflectionAlpha > 0.f) {
      uint addedTevs = 0;
      if (indTex) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        addedTevs = HandleReflection(usesTevReg2, static_cast< GXTexMapID >(indTexSlot & 7), 0,
                                     finalTevCount, texCount, tcgCount, finalKColorCount,
                                     finalCCFlags, finalACFlags);
#else
        addedTevs = HandleReflection(static_cast< GXTexMapID >(indTexSlot & 7), 0, finalTevCount,
                                     texCount, tcgCount, finalKColorCount);
#endif

        numIndStages = 1;
        tcgCount += 2;
      } else {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        addedTevs = HandleReflection(usesTevReg2, GX_TEXMAP_NULL, 0, finalTevCount, texCount,
                                     tcgCount, finalKColorCount, finalCCFlags, finalACFlags);
#else
        addedTevs = HandleReflection(GX_TEXMAP_NULL, 0, finalTevCount, texCount, tcgCount,
                                     finalKColorCount);
#endif

        tcgCount += 1;
      }
#if !RSTL_PRIME1
      finalTevCount += addedTevs;
#endif

      texCount += 1;
#if RSTL_PRIME1
      finalTevCount += addedTevs;
#endif

      finalKColorCount += 1;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    } else if ((finalCCFlags >> 9 & 3) != 0) {
#else
    } else if (finalTevCount != 0 &&
               CGX::MaskAndShiftLeft(
                   CGX::GetTevState(static_cast< GXTevStageID >(finalTevCount - 1)).mColorOps, 3,
                   9) != 0) {
#endif

      DoPassthru(finalTevCount);
      finalTevCount += 1;
    }
  }

  if (sbRenderModelShadow) {
    DoModelShadow(texCount, tcgCount);
#if !RSTL_PRIME1
    texCount += 1;
#endif

    tcgCount += 1;
  }

  CGX::SetNumIndStages(numIndStages);
  CGX::SetNumTevStages(finalTevCount);
  CGX::SetNumTexGens(tcgCount);
  CGX::SetNumChans(finalNumColorChans);
#if RSTL_PRIME1
  DEBUG_TOOLS_CALL(ApplyMaterialDebug(finalTevCount));
#endif

}

static inline CVector3f TransformReflectionPoint(const CTransform4f& xf, const CVector3f& point) {
  const float x = point.GetX() - xf.Get03();
  const float y = point.GetY() - xf.Get13();
  const float z = point.GetZ() - xf.Get23();
  return xf.TransposeRotate(CVector3f(x, y, z));
#if RSTL_PRIME1
}

static inline float ReflectionDistanceSquared(const CVector3f& point, const CVector3f& origin) {
  const CVector3f& delta = point - origin;
  return delta.MagSquared();
#endif

}

static inline CVector3f GetReflectionPlayerPosition() {
  return TransformReflectionPoint(CGraphics::GetModelMatrix(), sPlayerPosition);
}

void CCubeMaterial::EnsureViewDepStateCached(const CCubeSurface* surface) {
  static const Mtx texMtx1 = {
      {0.5f, 0.f, 0.f, 0.5f},
      {0.f, 0.f, 0.5f, 0.5f},
      {0.f, 0.f, 0.f, 1.f},
  };
#if !RSTL_PRIME1
  static const SMtx texMtx2 = {{
      {0.f, 0.f, 0.f, 0.f},
      {0.f, 0.f, 0.f, 0.f},
      {0.f, 0.f, 0.f, 1.f},
  }};
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if ((surface == NULL && sLastModelCached == sRenderingModel) || sRenderingModel == NULL) {
#else
  if ((surface == nullptr && sLastModelCached == sRenderingModel) || sRenderingModel == nullptr) {
#endif

    return;
  }

  const CVector3f& playerPos = GetReflectionPlayerPosition();
  CVector3f points[2];
  points[1] = playerPos;
  sLastModelCached = sRenderingModel;
  points[0] = CVector3f::Zero();
  CVector3f& modelPoint = points[0];
  CVector3f& playerPoint = points[1];
  float radius = 0.f;

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (surface != NULL) {
#else
  if (surface != nullptr) {
#endif

    sMaterialCachedState = 2;

    const CPlane plane(surface->GetCenter(), surface->GetNormalHint());
    modelPoint = playerPoint - plane.GetHeight(playerPoint) * plane.GetNormal();
  } else {
    sMaterialCachedState = 1;

    const CAABox& bounds = sRenderingModel->GetBoundingBox();
    modelPoint = bounds.GetCenterPoint();
    modelPoint.SetZ(playerPoint.GetZ());
    radius = 0.5f * (bounds.GetWidth() + bounds.GetHeight());
  }

  CUBE_RENDERER* renderer = CUBE_RENDERER::That();
  if (renderer->GetReflectionFlag()) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const float oldMag = ReflectionDistanceSquared(sViewingFrom, sPlayerPosition);
#else
    const CVector3f& oldDelta = sViewingFrom - sPlayerPosition;
#endif

    const CVector3f& newDelta = modelPoint - sPlayerPosition;
#if !RSTL_PRIME1
    const float oldMag = oldDelta.MagSquared();
#endif

    const float newMag = newDelta.MagSquared();
    if (newMag < oldMag) {
      const float pointX = modelPoint.GetX();
      const float pointY = modelPoint.GetY();
      const float pointZ = modelPoint.GetZ();
      sViewingFrom = CVector3f(pointX, pointY, pointZ);
    }
  } else {
    sViewingFrom = modelPoint;
    renderer->SetReflectionFlag();
  }

  const CVector3f distVec = modelPoint - playerPoint;
  const float dist = distVec.Magnitude();
  const float reflDist = CMath::Max(gkEpsilon32, dist - 0.5f * radius);

  if (reflDist >= 5.f) {
    sReflectionAlpha = 0.f;
    return;
  }

  sReflectionAlpha = (5.f - reflDist) / 5.f;

  CTransform4f xf = CGraphics::GetViewMatrix().GetQuickInverse() * CGraphics::GetModelMatrix();
  CGX::LoadTexMtxImm(xf.GetCStyleMatrix(), GX_TEXMTX6, GX_MTX3x4);
  CGX::LoadTexMtxImm(texMtx1, GX_PTTEXMTX6, GX_MTX3x4);

#if RSTL_PRIME1
  CVector3f right = CVector3f::Cross(distVec / reflDist, CVector3f(0.f, 0.f, 1.f));
#endif

  float xScale = 0.32258067f;
  float yScale = 0.32258067f;
#if !RSTL_PRIME1
  CVector3f dir = (1.f / reflDist) * distVec;
  CVector3f right = CVector3f::Cross(dir, CVector3f(0.f, 0.f, 1.f));
#endif

  if (right.CanBeNormalized()) {
    right.Normalize();
  } else {
#if !RSTL_PRIME1
    dir = CVector3f::Forward();
#endif

    right = CVector3f::Right();
  }

  const float playerZ = playerPoint.GetZ();
  const float scale = 0.02f * reflDist + 1.f;
  const float rightDist = CVector3f::Dot(modelPoint, right);

  xScale *= scale * sThrobX;
  yScale *= scale * sThrobY;

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  Mtx texMtx = {
      {0.f, 0.f, 0.f, 0.f},
      {0.f, 0.f, 0.f, 0.f},
      {0.f, 0.f, 0.f, 1.f},
  };
  // The unused initializer is retained after texMtx's initializer in the original rodata.
  Mtx unusedTexMtx = {
      {0.f, 0.f, 0.f, 0.f},
      {0.f, 0.f, 0.f, 0.f},
      {0.f, 0.f, 0.f, 1.f},
  };
  texMtx[0][0] = xScale * right.GetX();
  texMtx[0][1] = xScale * right.GetY();
  texMtx[0][3] = -rightDist * xScale + 0.5f;
  texMtx[1][2] = yScale;
  texMtx[1][3] = -playerZ * yScale;
  CGX::LoadTexMtxImm(texMtx, GX_TEXMTX7, GX_MTX3x4);
#else
  SMtx texMtx = texMtx2;
  texMtx.m[0][0] = xScale * right.GetX();
  texMtx.m[0][1] = xScale * right.GetY();
  texMtx.m[0][3] = -rightDist * xScale + 0.5f;
  texMtx.m[1][2] = yScale;
  texMtx.m[1][3] = -playerZ * yScale;
  CGX::LoadTexMtxImm(texMtx.m, GX_TEXMTX7, GX_MTX3x4);
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CCubeMaterial::KillCachedViewDepState() { sLastModelCached = NULL; }
#else
void CCubeMaterial::KillCachedViewDepState() {
  sLastModelCached = nullptr;
  sMaterialCachedState = 0;
}
#endif

void CCubeMaterial::ResetCachedMaterials() {
  KillCachedViewDepState();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  sRenderingModel = NULL;
  sLastMaterialCached = NULL;
#else
  sRenderingModel = nullptr;
  sLastMaterialCached = nullptr;
#endif

  sLastMaterialUnique = -1;
#if !RSTL_PRIME1
  sKColorModulated = false;
#endif

}
#if !RSTL_PRIME1
uint CCubeMaterial::GetCompressedBlend() const {
  const uint* ptr = reinterpret_cast< const uint* >(mData);
  const uint flags = ReadWord(&ptr[0]);
  const uint texCount = ReadWord(&ptr[1]);
  const uint* blend = ptr + texCount + 6;
  if ((flags & kStateFlag_KonstValues) != 0) {
    blend += ReadWord(blend) + 1;
  }
  return ReadWord(blend);
}

uint CCubeMaterial::GetExtraPostTexMtx() { return sAlphaMaskPostTexMtx; }

uint CCubeMaterial::GetExtraTexCoord() { return sAlphaMaskTexCoord; }

void CCubeMaterial::ResetExtraTexCoord() { sAlphaMaskTexCoord = GX_TEXCOORD_NULL; }
#endif

void CCubeModel::SetNewPlayerPositionAndTime(const CVector3f& pos, const CStopwatch& stopwatch) {
  sPlayerPosition = pos;
  CCubeMaterial::KillCachedViewDepState();

  s64 millis = stopwatch.GetCurrMicros() / 1000;

  float frequency = 1.5f;
  float timeWrapScale = 100000.f;
  float period = M_2PIF / frequency;
  const float time = static_cast< float >(static_cast< uint >(
                         millis % static_cast< uint >(timeWrapScale * period))) /
                     1000.f;
  sLastTime = time;

  float throbAmplitudeX = 0.05f;
  float throbAmplitudeY = 0.015f;
  float phaseX = 0.f;
  float phaseY = 1.f;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  sThrobX = 1. / (1. - throbAmplitudeX * sin(time * frequency + phaseX));
  sThrobY = 1. / (1. - throbAmplitudeY * sin(sLastTime * frequency + phaseY));
#else
  const float sinX = sin(time * frequency + phaseX);
  sThrobX = 1.f / (1.f - throbAmplitudeX * sinX);
  const float sinY = sin(sLastTime * frequency + phaseY);
  sThrobY = 1.f / (1.f - throbAmplitudeY * sinY);
#endif

}

void CCubeModel::SetRenderModelBlack(bool v) {
  sbRenderModelBlack = v;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  lbl_805A9559 = false;
#else
  sbRenderModelBlackKonst = false;
#endif

}

void CCubeModel::EnableShadowMaps(const CTexture* shadowTex, const CTransform4f& textureProjXf,
                                  unsigned char chan0DisableMask,
                                  unsigned char chan1EnableLightMask) {
#if !RSTL_PRIME1
  if (CUBE_RENDERER::That()->GetMaterialMode() != 0) {
    return;
  }
#endif

  sbRenderModelShadow = true;
  spShadowTexture = const_cast< CTexture* >(shadowTex);
  sTextureProjectionTransform = textureProjXf;
  sChannel0DisableLightMask = chan0DisableMask;
  sChannel1EnableLightMask = chan1EnableLightMask;
}

void CCubeModel::DisableShadowMaps() { sbRenderModelShadow = false; }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
uint CCubeMaterial::GetCompressedBlend() const {
  const uint* ptr = reinterpret_cast< const uint* >(mData);
  const uint flags = CBasics::SwapBytes(ptr[0]);
  const uint texCount = CBasics::SwapBytes(ptr[1]);
  const uint* blend = ptr + texCount + 4;
  if ((flags & kStateFlag_KonstValues) != 0) {
    blend += CBasics::SwapBytes(*blend) + 1;
  }
  return CBasics::SwapBytes(*blend);
}
#else
int CCubeMaterial::GetTransparencyKColor() { return sTransparencyKColor; }

void CCubeMaterial::ResetTransparencyKColor() { sTransparencyKColor = -1; }

void CCubeMaterial::UseNormalTevs() { sTevHandler = HandleNormalTevs; }

void CCubeMaterial::UseThermalTevs() { sTevHandler = HandleThermalTevs; }
#endif
