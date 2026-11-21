#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif
#include "Kyoto/Graphics/CCubeMaterial.hpp"

#if RSTL_CORRUPTION
#include "Kyoto/Basics/CCast.hpp"
#else
#include "Kyoto/Basics/CBasics.hpp"
#endif
#include "Kyoto/Basics/CStopwatch.hpp"

#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CCubeSurface.hpp"
#if !RSTL_CORRUPTION
#if !RSTL_PRIME1
#include "Kyoto/Graphics/CDisplayListReader.hpp"
#endif

#include "Kyoto/Graphics/CGX.hpp"
#endif
#include "Kyoto/Graphics/CGX_Impl.hpp" // IWYU pragma: keep

#include "Kyoto/Graphics/CGraphics.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Graphics/CMaterialTextureState.hpp"
#endif
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#if !RSTL_PRIME1
#include "Kyoto/Graphics/PortalPlane.hpp"
#endif

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Math/CVector3i.hpp"
#include "rstl/algorithm.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/set.hpp"
#else
#include "Kyoto/Basics/CCast.hpp"
#endif

#if RSTL_CORRUPTION
#include "Corruption/MetaRender/CCubeRenderer.hpp"
#else
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
#endif

#include "dolphin/mtx.h"
#if RSTL_CORRUPTION
#include "string.h"
#else
#if !RSTL_PRIME1
#include "string.h"
#endif

#include "Kyoto/MemoryCopy.hpp"
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_CORRUPTION
// Material data is big-endian; keep direct loads on big-endian targets.
extern "C" int fn_8033B70C(uint vtxDesc, int attr);

typedef void (*TTevHandler)(const uint*& materialData, uint firstTev, uint& tevCount,
                            uint& tcgCount);
#else
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
#if RSTL_CORRUPTION
static CVector3f sPlayerPosition(CVector3f::Zero());
#else
#if !RSTL_PRIME1
static CVector3f sPlayerPosition(CVector3f::Zero());
#endif

static CTransform4f sTextureProjectionTransform(CTransform4f::Identity());
#endif
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

#if RSTL_CORRUPTION
bool CCubeMaterial::sMaterialCacheEnabled = true;
int CCubeMaterial::sLastMaterialIndex = -1;
int CCubeMaterial::sLastParsedMaterialUnique = -1;
const CCubeModel* CCubeMaterial::sCachedMaterialModel = nullptr;
bool CCubeMaterial::sMaterialStateUnknown = false;
bool CCubeMaterial::sResetAlphaCompare = false;
// Guessed name
static CCubeMaterial::SMaterialTevSwapState sMaterialTevSwaps;
rstl::optional_object< CGraphics::SAlphaContextState > CCubeMaterial::sSavedAlphaContext;
int CCubeMaterial::sMaterialStageCount = 0;
CCubeMaterial::TMaterialPassFinalizer CCubeMaterial::sMaterialPassFinalizer =
    CCubeMaterial::FinalizePassStageCounts;
int CCubeMaterial::sGuiColorKRegister = -1;
static CTransform4f sTextureProjectionTransform(CTransform4f::Identity());
CColor CCubeMaterial::sTevRegister1Color(CColor::White());
CColor CCubeMaterial::sLastMaterialRegisterColor(CColor::White());
// Guessed name; eight quantized alpha levels, selected by the upper three bits.
const GXTevKAlphaSel CCubeMaterial::sQuantizedAlphaSelectors[] = {
    GX_TEV_KASEL_1_8, GX_TEV_KASEL_1_4, GX_TEV_KASEL_3_8, GX_TEV_KASEL_1_2,
    GX_TEV_KASEL_5_8, GX_TEV_KASEL_3_4, GX_TEV_KASEL_7_8, GX_TEV_KASEL_1,
};

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

#else
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

#endif
void CCubeMaterial::SetupBlendMode(const uint blendFactors, const CModelFlags& flags,
                                   bool alphaTest) {
  GXBlendFactor newSrcFactor = static_cast< GXBlendFactor >(blendFactors & 0xFFFF);
  GXBlendFactor newDstFactor = static_cast< GXBlendFactor >(blendFactors >> 0x10);
#if RSTL_CORRUPTION
  CModelFlags::ETrans blendMode = flags.GetTrans();
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CModelFlags::ETrans blendMode = flags.GetTrans();
#else
  CModelFlags::ETrans blendMode = flags.GetTransSigned();
#endif
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
#if RSTL_CORRUPTION
  const uint colorArgs = shadowMapsEnabled ? 0x7a04f : materialDataCur[0];
  const uint alphaArgs = materialDataCur[1];
  const uint colorOps = materialDataCur[2];
  const uint alphaOps = materialDataCur[3];
#else
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
#endif

  CGX::SetStandardDirectTev_Compressed(stage, colorArgs, alphaArgs, colorOps, alphaOps);

#if RSTL_CORRUPTION
  uint tmtcFlags = *texMapTexCoordFlags;
  uint matFlags = materialDataCur[4];
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  uint tmtcFlags = SBig(*texMapTexCoordFlags);
  uint matFlags = SBig(materialDataCur[4]);
#else
  uint tmtcFlags = ReadWord(texMapTexCoordFlags);
  uint matFlags = ReadWord(&materialDataCur[4]);
#endif

#endif
  CGX::SetTevOrder(stage, static_cast< GXTexCoordID >(tmtcFlags & 0xFF),
                   static_cast< GXTexMapID >(tmtcFlags >> 8 & 0xFF),
                   static_cast< GXChannelID >(matFlags & 0xFF));
  CGX::SetTevKColorSel(stage, static_cast< GXTevKColorSel >(matFlags >> 0x8 & 0xFF));
  CGX::SetTevKAlphaSel(stage, static_cast< GXTevKAlphaSel >(matFlags >> 0x10 & 0xFF));
}

#if RSTL_CORRUPTION
static uint HandleThermalTev(int tevCur, const uint* materialDataCur,
                             const uint* texMapTexCoordFlags) {
  const GXTevStageID stage = static_cast< GXTevStageID >(tevCur);
  switch (materialDataCur[0]) {
  case 0x7b84f:
    CGX::SetTevColorIn(stage, GX_CC_ZERO, GX_CC_ONE, GX_CC_KONST, GX_CC_ZERO);
    break;
  default:
    CGX::SetTevColorIn(stage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_TEXC, GX_CC_ZERO);
    break;
#else
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

#endif
  }
#if RSTL_CORRUPTION
  CGX::SetTevColorOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, GX_TRUE, GX_TEVPREV);
  CGX::SetTevAlphaIn_Compressed(stage, materialDataCur[1]);
  CGX::SetTevAlphaOp_Compressed(stage, materialDataCur[3]);

  uint tmtcFlags = *texMapTexCoordFlags;
  uint matFlags = materialDataCur[4];
  CGX::SetTevOrder(stage, GX_TEXCOORD0, static_cast< GXTexMapID >(tmtcFlags >> 8 & 0xFF),
                   GX_COLOR_NULL);
  CGX::SetTevKColorSel(stage, static_cast< GXTevKColorSel >(matFlags >> 0x8 & 0xFF));
  CGX::SetTevKAlphaSel(stage, static_cast< GXTevKAlphaSel >(matFlags >> 0x10 & 0xFF));
  return tmtcFlags & 0xFF;
#else
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
#if RSTL_CORRUPTION
static uint HandleUVScroll(const float* params, GXTexMtx texMtx) {
  const float f1 = params[0];
  const float f2 = params[1];
  const float f3 = params[2];
  const float f4 = params[3];
#else
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
#if RSTL_CORRUPTION
static uint HandleUVRotation(const float* params, GXTexMtx texMtx) {
  const float f1 = params[0];
  const float f2 = params[1];
#else
#if RSTL_PRIME1
  case 3: {
    const float f1 = SBig(params[0]);
    const float f2 = SBig(params[1]);
#else
static uint HandleUVRotation(const float* params, GXTexMtx texMtx) {
  const float f1 = ReadFloat(&params[0]);
  const float f2 = ReadFloat(&params[1]);
#endif

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
#if RSTL_CORRUPTION
static uint HandleUVFilmstrip(const float* params, uint type, GXTexMtx texMtx) {
  const float f1 = params[0];
  const float f2 = params[1];
  const float f3 = params[2];
  const float f4 = params[3];
#else
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
#if RSTL_CORRUPTION
  SMtx tmpPtMtx = sPtMtx;
  float scale = params[0];
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    Mtx tmpPtMtx;
    memcpy(&tmpPtMtx, &sPtMtx, sizeof(Mtx));
    float scale = SBig(params[0]);
#else
  SMtx tmpPtMtx = sPtMtx;
  float scale = ReadFloat(&params[0]);
#endif

#endif
    scale = 0.5f * scale;
#if RSTL_CORRUPTION
  tmpPtMtx.m[0][0] = scale;
  tmpPtMtx.m[0][3] = CMath::FastFmod(0.025f * (vm.Get03() + vm.Get13()) * params[1], 1.f);
  tmpPtMtx.m[1][2] = scale;
  tmpPtMtx.m[1][3] = CMath::FastFmod(0.05f * vm.Get23() * params[1], 1.f);
#else
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
#if RSTL_CORRUPTION

// Guessed name
static uint LoadAnimatedTextureMatrix(const uint* params, GXTexMtx texMtx, GXPTTexMtx postTexMtx) {
  const uint mode = params[0];
  const float* values = reinterpret_cast< const float* >(params + 1);
  const float scaleS = values[0];
  const float scaleT = values[1];
  const float angleOffset = values[2];
  const float angularSpeed = values[3];
  // The native reader skips values[4] and uses this offset for both axes.
  const float offset = values[5];
  const float speedS = values[6];
  const float speedT = values[7];
  const float seconds = CGraphics::GetSecondsMod900();
  const float angle = seconds * angularSpeed + angleOffset;
  const float sine = CMath::FastSinR(angle);
  const float cosine = CMath::FastCosR(angle);
  const float translationS = scaleS * (speedS * seconds + offset);
  const float translationT = scaleT * (speedT * seconds + offset);
  CTransform4f matrix(scaleS * cosine, scaleT * -sine, 0.f,
                      translationS + (0.5f - 0.5f * (cosine - sine)), scaleS * sine,
                      scaleT * cosine, 0.f, translationT + (0.5f - 0.5f * (sine + cosine)), 0.f,
                      0.f, 0.f, 1.f);

  static const CTransform4f xyProjection(1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f,
                                         1.f);
  static const CTransform4f yzProjection(0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f,
                                         1.f);
  static const CTransform4f xzProjection(1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f,
                                         1.f);
  const CTransform4f inverseCamera = CGraphics::GetCameraTransform().GetInverse();
  switch (static_cast< int >(mode)) {
  case 0:
    break;
  case 3:
    matrix *= xzProjection * inverseCamera;
    break;
  case 2:
    matrix *= yzProjection * inverseCamera;
    break;
  case 1:
    matrix *= xyProjection * inverseCamera;
    break;
  case 4:
    matrix *= PortalPlane::GetMaterialProjectionTransform() * inverseCamera;
    break;
  }
  ConstMtxPtr textureMatrix = matrix.GetCStyleMatrix();
  GXLoadTexMtxImm(textureMatrix, mode == 0 ? texMtx : postTexMtx, GX_MTX3x4);
  return 10;
}

static uint HandleAnimatedUV(const uint* uvAnim, GXTexMtx texMtx, GXPTTexMtx ptTexMtx) {
  const uint type = *uvAnim;
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
  case 8:
    return LoadAnimatedTextureMatrix(uvAnim + 1, texMtx, ptTexMtx);
#else
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

#endif
  default:
    return 0;
  }
}

#if RSTL_CORRUPTION
void CCubeMaterial::ConfigurePositionTexCoordGenerator(SMaterialPassState& state,
                                                       GXPTTexMtx postTexMtx) {
  CGX::SetTexCoordGen(static_cast< GXTexCoordID >(state.mTexCoordCount), GX_TG_MTX3x4, GX_TG_POS,
                      static_cast< GXTexMtx >(GX_PNMTX0), GX_FALSE, postTexMtx);
}

void CCubeMaterial::ConfigureMaterialPassTextureStage(
    SMaterialPassState& state, int materialIndex, int passIndex,
    const CMaterialDataFactory::SMaterial& material, const CMaterialTextureState& textures,
    GXChannelID channel) {
  const CMaterialDataFactory::SPass& pass = material.mPasses[passIndex];
  GXTexCoordID coord;
  if (pass.x14_ != 0) {
    const ushort* header = static_cast< const ushort* >(pass.mAnimationData);
    const int generatorMode = header[0];
    const int matrixMode = header[1];
    HandleAnimatedUV(reinterpret_cast< const uint* >(header + 2), state.mTexMtx, state.mPostTexMtx);
    if (generatorMode == 3) {
      ConfigurePositionTexCoordGenerator(state, state.mPostTexMtx);
      state.mPostTexMtx = static_cast< GXPTTexMtx >(state.mPostTexMtx + 3);
    } else {
      GXTexGenSrc source = GX_TG_TEX0;
      switch (generatorMode) {
      case 2:
        source = static_cast< GXTexGenSrc >(GX_TG_TEX0 + pass.mUvIndex);
        break;
      case 0:
        source = GX_TG_POS;
        break;
      case 1:
        source = GX_TG_NRM;
        break;
      }
      switch (matrixMode) {
      case 0:
        CGX::SetTexCoordGen(static_cast< GXTexCoordID >(state.mTexCoordCount), GX_TG_MTX3x4, source,
                            GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
        break;
      case 1:
        CGX::SetTexCoordGen(static_cast< GXTexCoordID >(state.mTexCoordCount), GX_TG_MTX3x4, source,
                            state.mTexMtx, GX_FALSE, GX_PTIDENTITY);
        state.mTexMtx = static_cast< GXTexMtx >(state.mTexMtx + 3);
        break;
      case 2:
        CGX::SetTexCoordGen(static_cast< GXTexCoordID >(state.mTexCoordCount), GX_TG_MTX3x4, source,
                            GX_IDENTITY, GX_TRUE, state.mPostTexMtx);
        state.mPostTexMtx = static_cast< GXPTTexMtx >(state.mPostTexMtx + 3);
        break;
      case 3:
        CGX::SetTexCoordGen(static_cast< GXTexCoordID >(state.mTexCoordCount), GX_TG_MTX3x4, source,
                            state.mTexMtx, GX_TRUE, state.mPostTexMtx);
        state.mTexMtx = static_cast< GXTexMtx >(state.mTexMtx + 3);
        state.mPostTexMtx = static_cast< GXPTTexMtx >(state.mPostTexMtx + 3);
        break;
      }
    }
    coord = static_cast< GXTexCoordID >(state.mTexCoordCount++);
  } else {
    coord = static_cast< GXTexCoordID >(textures.GetPassTexCoordIndex(materialIndex, passIndex));
  }
  const GXTexMapID map =
      static_cast< GXTexMapID >(textures.GetPassTexMapIndex(materialIndex, passIndex));
  CGX::SetTevOrder(static_cast< GXTevStageID >(state.mTevCount), coord, map, channel);
  if (channel == GX_COLOR0A0) {
    state.mUsesRasterColor = true;
  }
}

void CCubeMaterial::AppendCLRTextureColorStage(SMaterialPassState& state, bool useTextureAlpha,
                                               uchar opacity, bool useRegisterColor,
                                               const CMaterialDataFactory::SMaterial& material,
                                               int materialIndex,
                                               const CMaterialTextureState& textures) {
  bool usesRasterAlpha = false;
  CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO,
                     useRegisterColor ? GX_CC_C0 : GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO);
  CGX::SetTevColorOp(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_ADD, GX_TB_ZERO,
                     useRegisterColor ? GX_CS_SCALE_2 : GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
  if (useTextureAlpha) {
    if (opacity == 255) {
      CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                         GX_CA_ZERO, GX_CA_TEXA);
    } else {
      GXColor color;
      color.a = opacity; // The native path initializes only the alpha used by this stage.
      CGX::SetTevKColor(static_cast< GXTevKColorID >(state.mKonstCount), color);
      CGX::SetTevKAlphaSel(static_cast< GXTevStageID >(state.mTevCount),
                           static_cast< GXTevKAlphaSel >(GX_TEV_KASEL_K0_A + state.mKonstCount));
      CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_TEXA,
                         GX_CA_KONST, GX_CA_ZERO);
      ++state.mKonstCount;
    }
  } else {
    CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                       GX_CA_ZERO, state.x1a_ ? GX_CA_APREV : GX_CA_RASA);
    usesRasterAlpha = !state.x1a_;
    state.x1a_ = true;
  }
  CGX::SetTevAlphaOp(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_ADD, GX_TB_ZERO,
                     GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
  ConfigureMaterialPassTextureStage(
      state, materialIndex, CMaterialDataFactory::kP_CLR, material, textures,
      (usesRasterAlpha || !useRegisterColor) ? GX_COLOR0A0 : GX_COLOR_NULL);
  ++state.mTevCount;
}

void CCubeMaterial::AppendCLRTextureRedAlphaStage(SMaterialPassState& state,
                                                  const CMaterialDataFactory::SMaterial& material,
                                                  int materialIndex,
                                                  const CMaterialTextureState& textures) {
  CGX::SetTevKAlphaSel(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_KASEL_K0_A);
  CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_ZERO,
                     GX_CC_ZERO, GX_CC_ZERO);
  CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                     GX_CA_ZERO, GX_CA_TEXA);
  CGX::SetTevColorOp(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_ADD, GX_TB_ZERO,
                     GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
  CGX::SetTevAlphaOp(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_SUB, GX_TB_ZERO,
                     GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
  ConfigureMaterialPassTextureStage(state, materialIndex, CMaterialDataFactory::kP_CLR, material,
                                    textures, GX_COLOR_NULL);
  CGX::SetTevSwapMode(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_SWAP0, GX_TEV_SWAP1);
  sMaterialTevSwaps.AddStage(state.mTevCount);
  ++state.mTevCount;
}

bool CCubeMaterial::AppendTRANTextureAlphaMultiplyStage(
    SMaterialPassState& state, const CMaterialDataFactory::SMaterial& material, int materialIndex,
    const CMaterialTextureState& textures) {
  if (material.mHeader.mPassMask & (1 << CMaterialDataFactory::kP_TRAN)) {
    CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_ZERO,
                       GX_CC_ZERO, GX_CC_CPREV);
    CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_APREV,
                       GX_CA_TEXA, GX_CA_ZERO);
    CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(state.mTevCount));
    ConfigureMaterialPassTextureStage(state, materialIndex, CMaterialDataFactory::kP_TRAN, material,
                                      textures, GX_COLOR_NULL);
    ConfigureTrackedTextureSwap(static_cast< GXTevStageID >(state.mTevCount),
                                material.mPasses[CMaterialDataFactory::kP_TRAN].mFlags);
    ++state.mTevCount;
    return true;
  }
  return false;
}

int CCubeMaterial::AllocateOptionalINCAKonstColor(SMaterialPassState& state,
                                                  const CMaterialDataFactory::SMaterial& material,
                                                  const CModelFlags& flags) {
  if ((material.mPasses[CMaterialDataFactory::kP_INCA].mFlags & 8) &&
      (flags.GetOtherFlags() & 0x4000)) {
    const GXColor color = PortalPlane::sOptionalINCAColor.GetGXColor();
    CGX::SetTevKColor(static_cast< GXTevKColorID >(state.mKonstCount), color);
    ++state.mKonstCount;
    return state.mKonstCount - 1;
  }
  return -1;
}

bool CCubeMaterial::AppendBLOITextureAlphaAddStage(SMaterialPassState& state,
                                                   const CMaterialDataFactory::SMaterial& material,
                                                   int materialIndex,
                                                   const CMaterialTextureState& textures) {
  if (!(material.mHeader.mPassMask & (1 << CMaterialDataFactory::kP_BLOI))) {
    return false;
  }
  CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_ZERO,
                     GX_CC_ZERO, GX_CC_CPREV);
  CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(state.mTevCount));
  ConfigureMaterialPassTextureStage(state, materialIndex, CMaterialDataFactory::kP_BLOI, material,
                                    textures, GX_COLOR_NULL);
  ConfigureTrackedTextureSwap(static_cast< GXTevStageID >(state.mTevCount),
                              material.mPasses[CMaterialDataFactory::kP_BLOI].mFlags);
  CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_TEXA, GX_CA_ZERO,
                     GX_CA_ZERO, GX_CA_APREV);
  ++state.mTevCount;
  return true;
}

bool CCubeMaterial::AppendBLOLTextureAlphaMultiplyStage(
    SMaterialPassState& state, bool enabled, const CMaterialDataFactory::SMaterial& material,
    int materialIndex, const CMaterialTextureState& textures) {
  if (!enabled || !(material.mHeader.mPassMask & (1 << CMaterialDataFactory::kP_BLOL))) {
    return false;
  }
  ConfigureTrackedTextureSwap(static_cast< GXTevStageID >(state.mTevCount),
                              material.mPasses[CMaterialDataFactory::kP_BLOL].mFlags);
  CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(state.mTevCount));
  CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_ZERO,
                     GX_CC_ZERO, GX_CC_CPREV);
  CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_APREV,
                     GX_CA_TEXA, GX_CA_ZERO);
  ConfigureMaterialPassTextureStage(state, materialIndex, CMaterialDataFactory::kP_BLOL, material,
                                    textures, GX_COLOR_NULL);
  ++state.mTevCount;
  return true;
}

bool CCubeMaterial::AppendSelectedKonstAlphaAdditionStage(SMaterialPassState& state, uchar alpha) {
  CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_ZERO,
                     GX_CC_ZERO, GX_CC_CPREV);
  CGX::SetTevKAlphaSel(static_cast< GXTevStageID >(state.mTevCount),
                       sQuantizedAlphaSelectors[alpha >> 5]);
  CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_KONST, GX_CA_ZERO,
                     GX_CA_ZERO, GX_CA_APREV);
  CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(state.mTevCount));
  CGX::SetTevOrder(static_cast< GXTevStageID >(state.mTevCount), GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
                   GX_COLOR_NULL);
  ++state.mTevCount;
  return true;
}

void CCubeMaterial::AppendColorAdditionStage(SMaterialPassState& state, CColor color) {
  if ((color.GetColor_u32() & 0xffffff00) == 0) {
    return;
  }
  CGX::SetTevKColor(static_cast< GXTevKColorID >(state.mKonstCount), color.GetGXColor());
  CGX::SetTevKColorSel(static_cast< GXTevStageID >(state.mTevCount),
                       static_cast< GXTevKColorSel >(GX_TEV_KCSEL_K0 + state.mKonstCount));
  ++state.mKonstCount;
  CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_ONE,
                     GX_CC_KONST, GX_CC_CPREV);
  CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                     GX_CA_ZERO, GX_CA_APREV);
  CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(state.mTevCount));
  CGX::SetTevOrder(static_cast< GXTevStageID >(state.mTevCount), GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
                   GX_COLOR_NULL);
  ++state.mTevCount;
}

void CCubeMaterial::AppendColorModulationStage(SMaterialPassState& state, CColor color) {
  if ((color.GetColor_u32() & 0xffffff00) == 0xffffff00) {
    return;
  }
  CGX::SetTevKColor(static_cast< GXTevKColorID >(state.mKonstCount), color.GetGXColor());
  CGX::SetTevKColorSel(static_cast< GXTevStageID >(state.mTevCount),
                       static_cast< GXTevKColorSel >(GX_TEV_KCSEL_K0 + state.mKonstCount));
  ++state.mKonstCount;
  CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_CPREV,
                     GX_CC_KONST, GX_CC_ZERO);
  CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                     GX_CA_ZERO, GX_CA_APREV);
  CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(state.mTevCount));
  CGX::SetTevOrder(static_cast< GXTevStageID >(state.mTevCount), GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
                   GX_COLOR_NULL);
  ++state.mTevCount;
}

void CCubeMaterial::AppendThresholdAlphaColorAndAlphaModulationStage(SMaterialPassState& state,
                                                                     CColor color) {
  const uchar alpha = color.GetAlphau8();
  const uchar threshold = alpha > 127 ? static_cast< uchar >((alpha - 128) * 2) : 0;
  GXColor thresholdColor = {0, 0, 0, 0};
  thresholdColor.r = threshold;
  thresholdColor.g = threshold;
  thresholdColor.b = threshold;
  thresholdColor.a = threshold;
  CGX::SetTevKColor(static_cast< GXTevKColorID >(state.mKonstCount), thresholdColor);
  CGX::SetTevKColorSel(static_cast< GXTevStageID >(state.mTevCount),
                       static_cast< GXTevKColorSel >(GX_TEV_KCSEL_K0 + state.mKonstCount));
  CGX::SetTevKAlphaSel(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_KASEL_K0_A);
  ++state.mKonstCount;
  CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_CPREV,
                     GX_CC_KONST, GX_CC_ZERO);
  CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_APREV,
                     GX_CA_KONST, GX_CA_ZERO);
  CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(state.mTevCount));
  CGX::SetTevOrder(static_cast< GXTevStageID >(state.mTevCount), GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
                   GX_COLOR_NULL);
  ++state.mTevCount;
}

void CCubeMaterial::AppendRasterOrRegisterColorStage(
    SMaterialPassState& state, bool useConstantAlpha, uchar opacity, bool useRegisterColor,
    const CMaterialDataFactory::SMaterial& material) {
  const GXTevColorArg colorArg = useRegisterColor ? GX_CC_C0 : GX_CC_RASC;
  GXColor color = material.mColors.mColor.GetGXColor();
  color.a = opacity;
  bool usesRasterAlpha = false;
  CGX::SetTevKColor(static_cast< GXTevKColorID >(state.mKonstCount), color);
  CGX::SetTevKColorSel(static_cast< GXTevStageID >(state.mTevCount),
                       static_cast< GXTevKColorSel >(GX_TEV_KCSEL_K0 + state.mKonstCount));
  CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, colorArg,
                     GX_CC_KONST, GX_CC_ZERO);
  CGX::SetTevColorOp(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_ADD, GX_TB_ZERO,
                     GX_CS_SCALE_2, GX_TRUE, GX_TEVPREV);
  if (useConstantAlpha) {
    CGX::SetTevKAlphaSel(static_cast< GXTevStageID >(state.mTevCount),
                         static_cast< GXTevKAlphaSel >(GX_TEV_KASEL_K0_A + state.mKonstCount));
    CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                       GX_CA_ZERO, GX_CA_KONST);
  } else {
    CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                       GX_CA_ZERO, state.x1a_ ? GX_CA_APREV : GX_CA_RASA);
    usesRasterAlpha = !state.x1a_;
    state.x1a_ = true;
  }
  CGX::SetTevAlphaOp(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_ADD, GX_TB_ZERO,
                     GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
  CGX::SetTevOrder(static_cast< GXTevStageID >(state.mTevCount), GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
                   (usesRasterAlpha || !useRegisterColor) ? GX_COLOR0A0 : GX_COLOR_NULL);
  ++state.mKonstCount;
  ++state.mTevCount;
}

bool CCubeMaterial::AppendINCATextureColorAdditionStage(
    SMaterialPassState& state, const CMaterialDataFactory::SMaterial& material, int materialIndex,
    const CModelFlags& flags, const CMaterialTextureState& textures) {
  if (!(material.mHeader.mPassMask & (1 << CMaterialDataFactory::kP_INCA))) {
    return false;
  }
  const int konst = AllocateOptionalINCAKonstColor(state, material, flags);
  if (konst != -1) {
    CGX::SetTevKColorSel(static_cast< GXTevStageID >(state.mTevCount),
                         static_cast< GXTevKColorSel >(GX_TEV_KCSEL_K0 + konst));
  } else {
    CGX::SetTevKColorSel(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_KCSEL_1);
  }
  CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_KONST,
                     GX_CC_TEXC, GX_CC_CPREV);
  CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                     GX_CA_ZERO, GX_CA_APREV);
  CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(state.mTevCount));
  ConfigureMaterialPassTextureStage(state, materialIndex, CMaterialDataFactory::kP_INCA, material,
                                    textures, GX_COLOR_NULL);
  ++state.mTevCount;
  return true;
}

bool CCubeMaterial::AppendINCAColorAndAlphaAdditionStages(
    SMaterialPassState& state, const CMaterialDataFactory::SMaterial& material, int materialIndex,
    const CModelFlags& flags, const CMaterialTextureState& textures) {
  const uchar alpha = material.mIntegers[2]; // BLOI
  if (!(material.mHeader.mPassMask & (1 << CMaterialDataFactory::kP_INCA))) {
    if (alpha != 0) {
      return AppendSelectedKonstAlphaAdditionStage(state, alpha);
    }
    return false;
  }
  const int konst = AllocateOptionalINCAKonstColor(state, material, flags);
  if (konst != -1) {
    CGX::SetTevKColorSel(static_cast< GXTevStageID >(state.mTevCount),
                         static_cast< GXTevKColorSel >(GX_TEV_KCSEL_K0 + konst));
  } else {
    CGX::SetTevKColorSel(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_KCSEL_1);
  }
  CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_KONST,
                     GX_CC_TEXC, GX_CC_CPREV);
  CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(state.mTevCount));
  ConfigureMaterialPassTextureStage(state, materialIndex, CMaterialDataFactory::kP_INCA, material,
                                    textures, GX_COLOR_NULL);
  if (material.mPasses[CMaterialDataFactory::kP_INCA].mFlags & 4) {
    ConfigureTrackedTextureSwap(static_cast< GXTevStageID >(state.mTevCount),
                                material.mPasses[CMaterialDataFactory::kP_INCA].mFlags);
    CGX::SetTevKAlphaSel(static_cast< GXTevStageID >(state.mTevCount),
                         sQuantizedAlphaSelectors[material.mIntegers[3] >> 5]); // BNIF
    CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_TEXA,
                       GX_CA_KONST, GX_CA_APREV);
    ++state.mTevCount;
    if (alpha != 0) {
      AppendSelectedKonstAlphaAdditionStage(state, alpha);
    }
  } else {
    if (alpha != 0) {
      CGX::SetTevKAlphaSel(static_cast< GXTevStageID >(state.mTevCount),
                           sQuantizedAlphaSelectors[alpha >> 5]);
      CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_KONST, GX_CA_ZERO,
                         GX_CA_ZERO, GX_CA_APREV);
    } else {
      CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                         GX_CA_ZERO, GX_CA_APREV);
    }
    ++state.mTevCount;
  }
  return true;
}

bool CCubeMaterial::AppendRFLDTextureColorStages(SMaterialPassState& state, GXTevColorArg baseColor,
                                                 bool useRegisterColor,
                                                 const CMaterialDataFactory::SMaterial& material,
                                                 int materialIndex,
                                                 const CMaterialTextureState& textures) {
  if (!(material.mHeader.mPassMask & (1 << CMaterialDataFactory::kP_RFLD)) ||
      (material.mHeader.mFlags & 0x400)) {
    return false;
  }
  GXTevColorArg multiplier = GX_CC_ONE;
  if (material.mHeader.mPassMask &
      ((1 << CMaterialDataFactory::kP_RFLV) | (1 << CMaterialDataFactory::kP_LRLD) |
       (1 << CMaterialDataFactory::kP_LURD))) {
    CGX::SetTevColorOp(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_ADD, GX_TB_ZERO,
                       GX_CS_SCALE_1, GX_TRUE, GX_TEVREG2);
    CGX::SetTevAlphaOp(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_ADD, GX_TB_ZERO,
                       GX_CS_SCALE_1, GX_TRUE, GX_TEVREG2);
    if (material.mHeader.mPassMask & (1 << CMaterialDataFactory::kP_RFLV)) {
      CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_ZERO,
                         GX_CC_ZERO, GX_CC_TEXC);
      CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                         GX_CA_ZERO, GX_CA_ZERO);
      ConfigureMaterialPassTextureStage(state, materialIndex, CMaterialDataFactory::kP_RFLV,
                                        material, textures, GX_COLOR_NULL);
    } else if (material.mHeader.mPassMask & (1 << CMaterialDataFactory::kP_LRLD)) {
      CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO,
                         useRegisterColor ? GX_CC_C0 : GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO);
      CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                         GX_CA_ZERO, GX_CA_ZERO);
      ConfigureMaterialPassTextureStage(state, materialIndex, CMaterialDataFactory::kP_LRLD,
                                        material, textures,
                                        useRegisterColor ? GX_COLOR_NULL : GX_COLOR0A0);
    } else if (material.mHeader.mPassMask & (1 << CMaterialDataFactory::kP_LURD)) {
      CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO,
                         useRegisterColor ? GX_CC_C0 : GX_CC_RASC, GX_CC_TEXA, GX_CC_TEXC);
      CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                         GX_CA_ZERO, GX_CA_ZERO);
      ConfigureMaterialPassTextureStage(state, materialIndex, CMaterialDataFactory::kP_LURD,
                                        material, textures,
                                        useRegisterColor ? GX_COLOR_NULL : GX_COLOR0A0);
    }
    ++state.mTevCount;
    multiplier = GX_CC_C2;
  }
  if (material.mHeader.mPassMask & (1 << CMaterialDataFactory::kP_RFLD)) {
    CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_TEXC,
                       multiplier, baseColor);
    CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                       GX_CA_ZERO, GX_CA_APREV);
    CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(state.mTevCount));
    ConfigureMaterialPassTextureStage(state, materialIndex, CMaterialDataFactory::kP_RFLD, material,
                                      textures, GX_COLOR_NULL);
    ++state.mTevCount;
  }
  return true;
}

bool CCubeMaterial::AppendBLODAlphaStages(SMaterialPassState& state, bool useRegisterColor,
                                          const CMaterialDataFactory::SMaterial& material,
                                          int materialIndex,
                                          const CMaterialTextureState& textures) {
  if (!useRegisterColor) {
    state.x18_ = true;
    return false;
  }
  bool needsConstant = false;
  bool addedStage = false;
  if (!(material.mHeader.mPassMask & (1 << CMaterialDataFactory::kP_BLOD))) {
    needsConstant = !state.x18_;
  } else {
    ConfigureTrackedTextureSwap(static_cast< GXTevStageID >(state.mTevCount),
                                material.mPasses[CMaterialDataFactory::kP_BLOD].mFlags);
    CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(state.mTevCount));
    CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_ZERO,
                       GX_CC_ZERO, GX_CC_CPREV);
    const GXTevAlphaArg alphaSource = state.x1a_ ? GX_CA_APREV : GX_CA_RASA;
    // The native function retains this branch despite its early return above.
    if (!useRegisterColor) {
      const uchar bloom = material.mIntegers[1]; // BLOD
      if (bloom == 255 || state.x18_) {
        CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_TEXA,
                           alphaSource, GX_CA_ZERO);
        state.x18_ = true;
      } else if (bloom == 0) {
        CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                           GX_CA_ZERO, GX_CA_ZERO);
        state.x18_ = true;
      } else {
        CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_TEXA,
                           alphaSource, GX_CA_ZERO);
        needsConstant = !state.x18_;
      }
      state.x1a_ = true;
    } else {
      needsConstant = !state.x18_;
      CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_APREV,
                         GX_CA_TEXA, GX_CA_ZERO);
    }
    ConfigureMaterialPassTextureStage(state, materialIndex, CMaterialDataFactory::kP_BLOD, material,
                                      textures,
                                      alphaSource == GX_CA_RASA ? GX_COLOR0A0 : GX_COLOR_NULL);
    ++state.mTevCount;
    addedStage = true;
  }
  if (needsConstant) {
    const uchar bloom = material.mIntegers[1]; // BLOD
    if (bloom != 255) {
      CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(state.mTevCount));
      CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_ZERO,
                         GX_CC_ZERO, GX_CC_CPREV);
      if (bloom == 0) {
        CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                           GX_CA_ZERO, GX_CA_ZERO);
        CGX::SetTevOrder(static_cast< GXTevStageID >(state.mTevCount), GX_TEXCOORD_NULL,
                         GX_TEXMAP_NULL, GX_COLOR_NULL);
      } else {
        const GXTevAlphaArg alphaSource = state.x1a_ ? GX_CA_APREV : GX_CA_RASA;
        CGX::SetTevKAlphaSel(static_cast< GXTevStageID >(state.mTevCount),
                             sQuantizedAlphaSelectors[bloom >> 5]);
        CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, alphaSource,
                           GX_CA_KONST, GX_CA_ZERO);
        // G2MEAB selects the raster channel for APREV here, unlike the texture stage.
        CGX::SetTevOrder(static_cast< GXTevStageID >(state.mTevCount), GX_TEXCOORD_NULL,
                         GX_TEXMAP_NULL, alphaSource == GX_CA_APREV ? GX_COLOR0A0 : GX_COLOR_NULL);
        state.x1a_ = true;
      }
      ++state.mTevCount;
      addedStage = true;
    }
    state.x18_ = true;
  }
  return addedStage;
}

bool CCubeMaterial::AppendMaterialColorRegister0Stage(
    SMaterialPassState& state, bool useConstantAlpha,
    const CMaterialDataFactory::SMaterial& material, int materialIndex,
    const CMaterialTextureState& textures) {
  const bool hasDiffuse = material.mHeader.mPassMask & (1 << CMaterialDataFactory::kP_DIFF);
  const bool hasConstantDiffuse = (material.mHeader.mFlags & 0x2c) == 4;
  if (!hasDiffuse && !hasConstantDiffuse) {
    return false;
  }
  if (!(material.mColors.mDiffuseBloom == sLastMaterialRegisterColor)) {
    sLastMaterialRegisterColor = material.mColors.mDiffuseBloom;
    GXSetTevColor(GX_TEVREG1,
                  CColor::Modulate(sLastMaterialRegisterColor, sTevRegister1Color).GetGXColor());
  }
  if (hasDiffuse) {
    CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_TEXC,
                       GX_CC_C1, GX_CC_RASC);
    CGX::SetTevColorOp(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_ADD, GX_TB_ZERO,
                       GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
    if (useConstantAlpha) {
      CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                         GX_CA_ZERO, GX_CA_KONST);
      CGX::SetTevKAlphaSel(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_KASEL_1);
      CGX::SetTevAlphaOp(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_ADD, GX_TB_ZERO,
                         GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    } else {
      const uchar bloom = material.mIntegers[1]; // BLOD
      state.x18_ = true;
      CGX::SetTevAlphaOp(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_ADD, GX_TB_ZERO,
                         GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
      if (bloom == 0) {
        CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                           GX_CA_ZERO, GX_CA_RASA);
      } else if (bloom == 255) {
        CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_TEXA,
                           GX_CA_A1, GX_CA_RASA);
      } else {
        GXColor color;
        color.a = ((bloom | 31) * sTevRegister1Color.GetAlphau8()) / 255;
        CGX::SetTevKColor(static_cast< GXTevKColorID >(state.mKonstCount), color);
        CGX::SetTevKAlphaSel(static_cast< GXTevStageID >(state.mTevCount),
                             static_cast< GXTevKAlphaSel >(GX_TEV_KASEL_K0_A + state.mKonstCount));
        ++state.mKonstCount;
        CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_TEXA,
                           GX_CA_KONST, GX_CA_RASA);
      }
      state.x1a_ = true;
    }
    ConfigureMaterialPassTextureStage(state, materialIndex, CMaterialDataFactory::kP_DIFF, material,
                                      textures, GX_COLOR0A0);
  } else {
    CGX::SetTevColorOp(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_ADD, GX_TB_ZERO,
                       GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
    CGX::SetTevAlphaOp(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_ADD, GX_TB_ZERO,
                       GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_C1,
                       GX_CC_KONST, GX_CC_RASC);
    CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                       GX_CA_ZERO, useConstantAlpha ? GX_CA_KONST : GX_CA_ZERO);
    CGX::SetTevKColorSel(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_KCSEL_3_8);
    CGX::SetTevKAlphaSel(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_KASEL_1);
    CGX::SetTevOrder(static_cast< GXTevStageID >(state.mTevCount), GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
                     GX_COLOR0A0);
    state.mUsesRasterColor = true;
    state.x1a_ = true;
  }
  ++state.mTevCount;
  return true;
}

bool CCubeMaterial::AppendTRANTextureAlphaMultiplyIntoRegister2Stage(
    SMaterialPassState& state, const CMaterialDataFactory::SMaterial& material, int materialIndex,
    const CMaterialTextureState& textures) {
  if (material.mHeader.mPassMask & (1 << CMaterialDataFactory::kP_TRAN)) {
    CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_ZERO,
                       GX_CC_ZERO, GX_CC_CPREV);
    CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_A2,
                       GX_CA_TEXA, GX_CA_ZERO);
    CGX::SetTevColorOp(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_ADD, GX_TB_ZERO,
                       GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    CGX::SetTevAlphaOp(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_ADD, GX_TB_ZERO,
                       GX_CS_SCALE_1, GX_TRUE, GX_TEVREG2);
    ConfigureMaterialPassTextureStage(state, materialIndex, CMaterialDataFactory::kP_TRAN, material,
                                      textures, GX_COLOR_NULL);
    ConfigureTrackedTextureSwap(static_cast< GXTevStageID >(state.mTevCount),
                                material.mPasses[CMaterialDataFactory::kP_TRAN].mFlags);
    ++state.mTevCount;
    return true;
  }
  return false;
}

bool CCubeMaterial::AppendRFLDTextureRedAlphaMultiplyStage(
    SMaterialPassState& state, const CMaterialDataFactory::SMaterial& material, int materialIndex,
    const CMaterialTextureState& textures) {
  if ((material.mHeader.mFlags & 0x400) &&
      (material.mHeader.mPassMask & (1 << CMaterialDataFactory::kP_RFLD))) {
    CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_ZERO,
                       GX_CC_ZERO, GX_CC_CPREV);
    CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_APREV,
                       GX_CA_TEXA, GX_CA_ZERO);
    CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(state.mTevCount));
    ConfigureMaterialPassTextureStage(state, materialIndex, CMaterialDataFactory::kP_RFLD, material,
                                      textures, GX_COLOR_NULL);
    CGX::SetTevSwapMode(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_SWAP0, GX_TEV_SWAP1);
    sMaterialTevSwaps.AddStage(state.mTevCount);
    ++state.mTevCount;
    return true;
  }
  return false;
}

bool CCubeMaterial::AppendRFLDTextureRedAlphaMultiplyIntoRegister2Stage(
    SMaterialPassState& state, const CMaterialDataFactory::SMaterial& material, int materialIndex,
    const CMaterialTextureState& textures) {
  if ((material.mHeader.mFlags & 0x400) &&
      (material.mHeader.mPassMask & (1 << CMaterialDataFactory::kP_RFLD))) {
    CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_ZERO,
                       GX_CC_ZERO, GX_CC_CPREV);
    CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_A2,
                       GX_CA_TEXA, GX_CA_ZERO);
    CGX::SetTevColorOp(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_ADD, GX_TB_ZERO,
                       GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    CGX::SetTevAlphaOp(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_ADD, GX_TB_ZERO,
                       GX_CS_SCALE_1, GX_TRUE, GX_TEVREG2);
    ConfigureMaterialPassTextureStage(state, materialIndex, CMaterialDataFactory::kP_RFLD, material,
                                      textures, GX_COLOR_NULL);
    CGX::SetTevSwapMode(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_SWAP0, GX_TEV_SWAP1);
    sMaterialTevSwaps.AddStage(state.mTevCount);
    ++state.mTevCount;
    return true;
  }
  return false;
}

void CCubeMaterial::AppendThresholdAlphaColorModulationStage(SMaterialPassState& state,
                                                             CColor color) {
  const uchar alpha = color.GetAlphau8();
  const uchar threshold = alpha > 127 ? static_cast< uchar >((alpha - 128) * 2) : 0;
  GXColor thresholdColor = {0, 0, 0, 0};
  thresholdColor.r = threshold;
  thresholdColor.g = threshold;
  thresholdColor.b = threshold;
  thresholdColor.a = threshold;
  CGX::SetTevKColor(static_cast< GXTevKColorID >(state.mKonstCount), thresholdColor);
  CGX::SetTevKColorSel(static_cast< GXTevStageID >(state.mTevCount),
                       static_cast< GXTevKColorSel >(GX_TEV_KCSEL_K0 + state.mKonstCount));
  ++state.mKonstCount;
  CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_CPREV,
                     GX_CC_KONST, GX_CC_ZERO);
  CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                     GX_CA_ZERO, GX_CA_APREV);
  CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(state.mTevCount));
  CGX::SetTevOrder(static_cast< GXTevStageID >(state.mTevCount), GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
                   GX_COLOR_NULL);
  ++state.mTevCount;
}

void CCubeMaterial::AppendPremultipliedColorModulationStage(SMaterialPassState& state,
                                                            CColor color) {
  if (color.GetColor_u32() == 0xffffffff) {
    return;
  }
  if (color.GetAlphau8() != 255) {
    GXColor premultiplied = color.GetGXColor();
    const uchar alpha = premultiplied.a;
    premultiplied.r = premultiplied.r * alpha / 255;
    premultiplied.g = premultiplied.g * alpha / 255;
    premultiplied.b = premultiplied.b * alpha / 255;
    CGX::SetTevKColor(static_cast< GXTevKColorID >(state.mKonstCount), premultiplied);
  } else {
    CGX::SetTevKColor(static_cast< GXTevKColorID >(state.mKonstCount), color.GetGXColor());
  }
  CGX::SetTevKColorSel(static_cast< GXTevStageID >(state.mTevCount),
                       static_cast< GXTevKColorSel >(GX_TEV_KCSEL_K0 + state.mKonstCount));
  ++state.mKonstCount;
  CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_CPREV,
                     GX_CC_KONST, GX_CC_ZERO);
  CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                     GX_CA_ZERO, GX_CA_APREV);
  CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(state.mTevCount));
  CGX::SetTevOrder(static_cast< GXTevStageID >(state.mTevCount), GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
                   GX_COLOR_NULL);
  ++state.mTevCount;
}

bool CCubeMaterial::AppendBLOLAndBLODAlphaStages(SMaterialPassState& state,
                                                 const CMaterialDataFactory::SMaterial& material,
                                                 int materialIndex,
                                                 const CMaterialTextureState& textures) {
  if (!(material.mHeader.mPassMask & (1 << CMaterialDataFactory::kP_BLOL))) {
    return AppendBLODAlphaStages(state, false, material, materialIndex, textures);
  }
  CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_A1, GX_CA_TEXA,
                     GX_CA_RASA);
  CGX::SetTevAlphaOp(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_ADD, GX_TB_ZERO,
                     GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
  ConfigureTrackedTextureSwap(static_cast< GXTevStageID >(state.mTevCount),
                              material.mPasses[CMaterialDataFactory::kP_BLOL].mFlags);
  ConfigureMaterialPassTextureStage(state, materialIndex, CMaterialDataFactory::kP_BLOL, material,
                                    textures, GX_COLOR0A0);
  ++state.mTevCount;
  state.x1a_ = true;
  AppendBLODAlphaStages(state, true, material, materialIndex, textures);
  return true;
}

void CCubeMaterial::AppendINCAModulatedTextureStage(SMaterialPassState& state,
                                                    const CMaterialDataFactory::SMaterial& material,
                                                    int materialIndex, const CModelFlags& flags,
                                                    const CMaterialTextureState& textures) {
  const int mode = static_cast< char >(flags.GetTrans());
  if (mode == CModelFlags::kT_Two || mode == CModelFlags::kT_Opaque) {
    CGX::SetTevKColorSel(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_KCSEL_1);
    CGX::SetTevKAlphaSel(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_KASEL_1);
  } else {
    CGX::SetTevKColor(static_cast< GXTevKColorID >(state.mKonstCount),
                      flags.GetColorRef().GetGXColor());
    CGX::SetTevKColorSel(static_cast< GXTevStageID >(state.mTevCount),
                         static_cast< GXTevKColorSel >(GX_TEV_KCSEL_K0 + state.mKonstCount));
    CGX::SetTevKAlphaSel(static_cast< GXTevStageID >(state.mTevCount),
                         static_cast< GXTevKAlphaSel >(GX_TEV_KASEL_K0_A + state.mKonstCount));
    ++state.mKonstCount;
  }
  CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_KONST,
                     GX_CC_TEXC, GX_CC_ZERO);
  CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                     GX_CA_ZERO, GX_CA_KONST);
  CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(state.mTevCount));
  ConfigureMaterialPassTextureStage(state, materialIndex, CMaterialDataFactory::kP_INCA, material,
                                    textures, GX_COLOR_NULL);
  ++state.mTevCount;
}

bool CCubeMaterial::AppendInitialINCAColorAndAlphaStages(
    SMaterialPassState& state, const CMaterialDataFactory::SMaterial& material, int materialIndex,
    const CModelFlags& flags, const CMaterialTextureState& textures) {
  bool appendAlpha = false;
  const uchar alpha = material.mIntegers[2]; // BLOI
  state.x1a_ = true;
  if (!(material.mHeader.mPassMask & (1 << CMaterialDataFactory::kP_INCA))) {
    if (alpha != 0) {
      CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_ZERO,
                         GX_CC_ZERO, GX_CC_ZERO);
      CGX::SetTevKAlphaSel(static_cast< GXTevStageID >(state.mTevCount),
                           sQuantizedAlphaSelectors[alpha >> 5]);
      CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                         GX_CA_ZERO, GX_CA_KONST);
      CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(state.mTevCount));
      CGX::SetTevOrder(static_cast< GXTevStageID >(state.mTevCount), GX_TEXCOORD_NULL,
                       GX_TEXMAP_NULL, GX_COLOR_NULL);
      ++state.mTevCount;
      return true;
    }
    return false;
  }
  const int konst = AllocateOptionalINCAKonstColor(state, material, flags);
  if (konst != -1) {
    CGX::SetTevKColorSel(static_cast< GXTevStageID >(state.mTevCount),
                         static_cast< GXTevKColorSel >(GX_TEV_KCSEL_K0 + konst));
  } else {
    CGX::SetTevKColorSel(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_KCSEL_1);
  }
  CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_TEXC,
                     GX_CC_KONST, GX_CC_ZERO);
  if (material.mPasses[CMaterialDataFactory::kP_INCA].mFlags & 4) {
    ConfigureTrackedTextureSwap(static_cast< GXTevStageID >(state.mTevCount),
                                material.mPasses[CMaterialDataFactory::kP_INCA].mFlags);
    const uchar bnif = material.mIntegers[3];
    if (alpha != 0 && bnif == 0) {
      CGX::SetTevKAlphaSel(static_cast< GXTevStageID >(state.mTevCount),
                           sQuantizedAlphaSelectors[alpha >> 5]);
      CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_KONST, GX_CA_ZERO,
                         GX_CA_ZERO, GX_CA_TEXA);
    } else {
      CGX::SetTevKAlphaSel(static_cast< GXTevStageID >(state.mTevCount),
                           sQuantizedAlphaSelectors[bnif >> 5]);
      CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_TEXA,
                         GX_CA_KONST, GX_CA_ZERO);
      appendAlpha = alpha != 0;
    }
  } else if (alpha != 0) {
    CGX::SetTevKAlphaSel(static_cast< GXTevStageID >(state.mTevCount),
                         sQuantizedAlphaSelectors[alpha >> 5]);
    CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                       GX_CA_ZERO, GX_CA_KONST);
  } else {
    CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                       GX_CA_ZERO, GX_CA_ZERO);
  }
  CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(state.mTevCount));
  ConfigureMaterialPassTextureStage(state, materialIndex, CMaterialDataFactory::kP_INCA, material,
                                    textures, GX_COLOR_NULL);
  ++state.mTevCount;
  if (appendAlpha) {
    AppendSelectedKonstAlphaAdditionStage(state, alpha);
  }
  return true;
}

void CCubeMaterial::AppendTextureOrConstantAlphaStages(
    bool multiplyPrevious, SMaterialPassState& state, uchar opacity,
    const CMaterialDataFactory::SMaterial& material, int materialIndex,
    const CMaterialTextureState& textures) {
  if (material.mHeader.mPassMask &
      ((1 << CMaterialDataFactory::kP_CLR) | (1 << CMaterialDataFactory::kP_TRAN))) {
    if (opacity < 255) {
      GXColor color = {255, 255, 255, 0};
      color.a = opacity;
      CGX::SetTevKColor(static_cast< GXTevKColorID >(state.mKonstCount), color);
      CGX::SetTevKAlphaSel(static_cast< GXTevStageID >(state.mTevCount),
                           static_cast< GXTevKAlphaSel >(GX_TEV_KASEL_K0_A + state.mKonstCount));
      if (multiplyPrevious) {
        CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_APREV,
                           GX_CA_KONST, GX_CA_ZERO);
        CGX::SetTevAlphaOp(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_ADD, GX_TB_ZERO,
                           GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        CGX::SetTevOrder(static_cast< GXTevStageID >(state.mTevCount), GX_TEXCOORD_NULL,
                         GX_TEXMAP_NULL, GX_COLOR_NULL);
        ++state.mTevCount;
        CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_TEXA,
                           GX_CA_APREV, GX_CA_ZERO);
      } else {
        CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_KONST,
                           GX_CA_TEXA, GX_CA_ZERO);
      }
    } else if (multiplyPrevious) {
      CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_TEXA,
                         GX_CA_APREV, GX_CA_ZERO);
    } else {
      CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                         GX_CA_ZERO, GX_CA_TEXA);
    }
    CGX::SetTevAlphaOp(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_ADD, GX_TB_ZERO,
                       GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    if (material.mHeader.mPassMask & (1 << CMaterialDataFactory::kP_TRAN)) {
      ConfigureMaterialPassTextureStage(state, materialIndex, CMaterialDataFactory::kP_TRAN,
                                        material, textures, GX_COLOR_NULL);
      ConfigureTrackedTextureSwap(static_cast< GXTevStageID >(state.mTevCount),
                                  material.mPasses[CMaterialDataFactory::kP_TRAN].mFlags);
    } else {
      ConfigureMaterialPassTextureStage(state, materialIndex, CMaterialDataFactory::kP_CLR,
                                        material, textures, GX_COLOR_NULL);
    }
    ++state.mTevCount;
  } else {
    GXColor color = {255, 255, 255, 0};
    color.a = opacity;
    CGX::SetTevKColor(static_cast< GXTevKColorID >(state.mKonstCount), color);
    CGX::SetTevKAlphaSel(static_cast< GXTevStageID >(state.mTevCount),
                         static_cast< GXTevKAlphaSel >(GX_TEV_KASEL_K0_A + state.mKonstCount));
    if (multiplyPrevious) {
      CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_KONST,
                         GX_CA_APREV, GX_CA_ZERO);
    } else {
      CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                         GX_CA_ZERO, GX_CA_KONST);
    }
    CGX::SetTevAlphaOp(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_ADD, GX_TB_ZERO,
                       GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    CGX::SetTevOrder(static_cast< GXTevStageID >(state.mTevCount), GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
                     GX_COLOR_NULL);
    ++state.mKonstCount;
    ++state.mTevCount;
  }
}

int CCubeMaterial::DrawOpaqueMaterialPass(const CMaterialDataFactory::SMaterial& material,
                                          CCubeSurface surface, int index, const CModelFlags& flags,
                                          CMaterialTextureState& textures) {
  CGraphics::DisableDestinationAlpha(0);
  SMaterialPassState state;
  state.mTexCoordCount = textures.SetupTexCoordGenerators(index);
  bool useRegisterColor = false;
  if (AppendMaterialColorRegister0Stage(state, false, material, index, textures)) {
    useRegisterColor = true;
    AppendBLOLTextureAlphaMultiplyStage(state, true, material, index, textures);
  }
  AppendBLODAlphaStages(state, useRegisterColor, material, index, textures);
  if (material.mHeader.mPassMask & (1 << CMaterialDataFactory::kP_CLR)) {
    AppendCLRTextureColorStage(state, false, 255, useRegisterColor, material, index, textures);
  } else {
    AppendRasterOrRegisterColorStage(state, false, 255, useRegisterColor, material);
  }
  AppendRFLDTextureColorStages(state, GX_CC_CPREV, useRegisterColor, material, index, textures);
  AppendINCAColorAndAlphaAdditionStages(state, material, index, flags, textures);
  AppendBLOITextureAlphaAddStage(state, material, index, textures);
  switch (static_cast< char >(flags.GetTrans())) {
  case CModelFlags::kT_Two:
    AppendColorAdditionStage(state, flags.GetColor());
    break;
  case CModelFlags::kT_One:
  case CModelFlags::kT_Blend:
    AppendColorModulationStage(state, flags.GetColor());
    break;
  case CModelFlags::kT_ThresholdAlpha:
    AppendThresholdAlphaColorAndAlphaModulationStage(state, flags.GetColor());
    break;
  }
  ConfigureDepthModeFromModelFlags(flags, GX_TRUE, GX_LEQUAL, GX_TRUE);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
  sMaterialPassFinalizer(state);
  if (!(flags.GetOtherFlags() & 0x1000)) {
    CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
  }
  return 1;
}

int CCubeMaterial::DrawAlphaThresholdMaterialPass(const CMaterialDataFactory::SMaterial& material,
                                                  CCubeSurface surface, int index,
                                                  const CModelFlags& flags,
                                                  CMaterialTextureState& textures) {
  CGraphics::EnableDestinationAlpha(0, 0);
  SMaterialPassState state;
  state.mTexCoordCount = textures.SetupTexCoordGenerators(index);
  if (material.mHeader.mPassMask & (1 << CMaterialDataFactory::kP_CLR)) {
    AppendCLRTextureRedAlphaStage(state, material, index, textures);
  } else {
    AppendRasterOrRegisterColorStage(state, false, 255, false, material);
  }
  const uchar alpha = flags.GetColorRef().GetAlphau8();
  ConfigureDepthModeFromModelFlags(flags, GX_TRUE, GX_LEQUAL, GX_TRUE);
  CGX::SetAlphaCompare(GX_GREATER, static_cast< uchar >(255 - alpha * 2), GX_AOP_AND, GX_ALWAYS, 0);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
  sResetAlphaCompare = true;
  sMaterialPassFinalizer(state);
  if (!(flags.GetOtherFlags() & 0x1000)) {
    CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
  }
  return 1;
}

int CCubeMaterial::DrawAlphaBlendedMaterialPassWithoutAlphaUpdate(
    const CMaterialDataFactory::SMaterial& material, CCubeSurface surface, int index, uchar opacity,
    const CModelFlags& flags, CMaterialTextureState& textures) {
  SMaterialPassState state;
  state.mTexCoordCount = textures.SetupTexCoordGenerators(index);
  bool useRegisterColor = false;
  if (AppendMaterialColorRegister0Stage(state, true, material, index, textures)) {
    useRegisterColor = true;
  }
  if (material.mHeader.mPassMask & (1 << CMaterialDataFactory::kP_CLR)) {
    AppendCLRTextureColorStage(state, true, opacity, useRegisterColor, material, index, textures);
  } else {
    AppendRasterOrRegisterColorStage(state, true, opacity, useRegisterColor, material);
  }
  AppendTRANTextureAlphaMultiplyStage(state, material, index, textures);
  AppendRFLDTextureRedAlphaMultiplyStage(state, material, index, textures);
  AppendRFLDTextureColorStages(state, GX_CC_CPREV, useRegisterColor, material, index, textures);
  switch (static_cast< char >(flags.GetTrans())) {
  case CModelFlags::kT_Two:
    AppendColorAdditionStage(state, flags.GetColor());
    break;
  case CModelFlags::kT_One:
  case CModelFlags::kT_Blend:
    AppendColorModulationStage(state, flags.GetColor());
    break;
  }
  ConfigureDepthModeFromModelFlags(flags, GX_TRUE, GX_LEQUAL, GX_FALSE);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
  sMaterialPassFinalizer(state);
  CGraphics::DisableAlphaUpdate(0);
  if (!(flags.GetOtherFlags() & 0x1000)) {
    CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
  }
  return 1;
}

int CCubeMaterial::DrawAlphaCutoff192BlendedMaterialPass(
    const CMaterialDataFactory::SMaterial& material, CCubeSurface surface, int index,
    const CModelFlags& flags, CMaterialTextureState& textures) {
  SMaterialPassState state;
  state.mTexCoordCount = textures.SetupTexCoordGenerators(index);
  bool useRegisterColor = false;
  if (AppendMaterialColorRegister0Stage(state, true, material, index, textures)) {
    useRegisterColor = true;
  }
  if (material.mHeader.mPassMask & (1 << CMaterialDataFactory::kP_CLR)) {
    AppendCLRTextureColorStage(state, true, 255, useRegisterColor, material, index, textures);
  } else {
    AppendRasterOrRegisterColorStage(state, true, 255, useRegisterColor, material);
  }
  AppendTRANTextureAlphaMultiplyStage(state, material, index, textures);
  AppendRFLDTextureRedAlphaMultiplyStage(state, material, index, textures);
  AppendRFLDTextureColorStages(state, GX_CC_CPREV, useRegisterColor, material, index, textures);
  AppendINCATextureColorAdditionStage(state, material, index, flags, textures);
  ConfigureDepthModeFromModelFlags(flags, GX_TRUE, GX_LEQUAL, GX_TRUE);
  CGX::SetAlphaCompare(GX_GREATER, 192, GX_AOP_AND, GX_ALWAYS, 0);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
  CGraphics::EnableDestinationAlpha(0, 0);
  sResetAlphaCompare = true;
  sMaterialPassFinalizer(state);
  if (!(flags.GetOtherFlags() & 0x1000)) {
    CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
  }
  return 1;
}

int CCubeMaterial::DrawMaterialWithINCAAdditiveOverlay(
    const CMaterialDataFactory::SMaterial& material, CCubeSurface surface, int index, uchar opacity,
    const CModelFlags& flags, CMaterialTextureState& textures) {
  const int passes = DrawAlphaBlendedMaterialPassWithoutAlphaUpdate(material, surface, index,
                                                                    opacity, flags, textures);
  if (flags.GetOtherFlags() & 0x1800) {
    return passes;
  }
  SMaterialPassState state;
  state.mTexCoordCount = textures.SetupTexCoordGenerators(index);
  AppendINCAModulatedTextureStage(state, material, index, flags, textures);
  ConfigureDepthModeFromModelFlags(flags, GX_TRUE, GX_LEQUAL, GX_FALSE);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_CLEAR);
  sMaterialPassFinalizer(state);
  CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
  return passes + 1;
}

int CCubeMaterial::DrawAdditiveINCAMaterialPass(const CMaterialDataFactory::SMaterial& material,
                                                CCubeSurface surface, int index,
                                                const CModelFlags& flags,
                                                CMaterialTextureState& textures) {
  CGraphics::DisableDestinationAlpha(0);
  SMaterialPassState state;
  state.mTexCoordCount = textures.SetupTexCoordGenerators(index);
  AppendInitialINCAColorAndAlphaStages(state, material, index, flags, textures);
  AppendBLOITextureAlphaAddStage(state, material, index, textures);
  AppendRFLDTextureColorStages(state, GX_CC_CPREV, false, material, index, textures);
  switch (static_cast< char >(flags.GetTrans())) {
  case CModelFlags::kT_One:
    AppendColorModulationStage(state, flags.GetColor());
    break;
  case CModelFlags::kT_Blend:
  case CModelFlags::kT_Additive:
    AppendPremultipliedColorModulationStage(state, flags.GetColor());
    break;
  case CModelFlags::kT_ThresholdAlpha:
    AppendThresholdAlphaColorModulationStage(state, flags.GetColor());
    break;
  }
  ConfigureDepthModeFromModelFlags(flags, GX_TRUE, GX_LEQUAL, GX_FALSE);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  sMaterialPassFinalizer(state);
  if (!(flags.GetOtherFlags() & 0x1000)) {
    CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
  }
  return 1;
}

int CCubeMaterial::DrawMaterialWithOneOneINCAOverlay(
    const CMaterialDataFactory::SMaterial& material, CCubeSurface surface, int index, uchar opacity,
    const CModelFlags& flags, CMaterialTextureState& textures) {
  const int passes = DrawAlphaBlendedMaterialWithAuxiliaryPasses(material, surface, index, opacity,
                                                                 flags, textures);
  if (flags.GetOtherFlags() & 0x1000) {
    return passes;
  }
  ResetTrackedTevSwapModes(sMaterialTevSwaps);
  SMaterialPassState state;
  state.mTexCoordCount = textures.SetupTexCoordGenerators(index);
  AppendInitialINCAColorAndAlphaStages(state, material, index, flags, textures);
  AppendBLOITextureAlphaAddStage(state, material, index, textures);
  switch (static_cast< char >(flags.GetTrans())) {
  case CModelFlags::kT_One:
    AppendColorModulationStage(state, flags.GetColor());
    break;
  case CModelFlags::kT_Blend:
    AppendPremultipliedColorModulationStage(state, flags.GetColor());
    break;
  case CModelFlags::kT_ThresholdAlpha:
    AppendThresholdAlphaColorModulationStage(state, flags.GetColor());
    break;
  }
  ConfigureDepthModeFromModelFlags(flags, GX_TRUE, GX_LEQUAL, GX_FALSE);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  sMaterialPassFinalizer(state);
  CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
  return passes + 1;
}

int CCubeMaterial::DrawAlphaBlendedMaterialWithAuxiliaryPasses(
    const CMaterialDataFactory::SMaterial& material, CCubeSurface surface, int index, uchar opacity,
    const CModelFlags& flags, CMaterialTextureState& textures) {
  CGraphics::DisableDestinationAlpha(0);
  SMaterialPassState state;
  state.mTexCoordCount = textures.SetupTexCoordGenerators(index);
  bool useRegisterColor = AppendMaterialColorRegister0Stage(state, true, material, index, textures);
  AppendBLOLTextureAlphaMultiplyStage(state, useRegisterColor, material, index, textures);
  if (material.mHeader.mPassMask & (1 << CMaterialDataFactory::kP_CLR)) {
    AppendCLRTextureColorStage(state, true, opacity, useRegisterColor, material, index, textures);
  } else {
    AppendRasterOrRegisterColorStage(state, true, opacity, useRegisterColor, material);
  }
  AppendTRANTextureAlphaMultiplyStage(state, material, index, textures);
  AppendRFLDTextureRedAlphaMultiplyStage(state, material, index, textures);
  AppendRFLDTextureColorStages(state, GX_CC_CPREV, useRegisterColor, material, index, textures);
  switch (static_cast< char >(flags.GetTrans())) {
  case CModelFlags::kT_One:
  case CModelFlags::kT_Blend:
    AppendColorModulationStage(state, flags.GetColor());
    break;
  case CModelFlags::kT_Two:
    AppendColorAdditionStage(state, flags.GetColor());
    break;
  }
  ConfigureDepthModeFromModelFlags(flags, GX_TRUE, GX_LEQUAL, GX_FALSE);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
  sMaterialPassFinalizer(state);
  CGraphics::DisableAlphaUpdate(0);
  if (flags.GetOtherFlags() & 0x1000) {
    return 1;
  }
  CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
  ResetTrackedTevSwapModes(sMaterialTevSwaps);
  state = SMaterialPassState();
  state.mTexCoordCount = textures.SetupTexCoordGenerators(index);
  CGraphics::DisableDestinationAlpha(0);
  AppendTextureOrConstantAlphaStages(false, state, opacity, material, index, textures);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
  sMaterialPassFinalizer(state);
  CGX::SetColorUpdate(false);
  CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
  int passes = 2;
  ResetTrackedTevSwapModes(sMaterialTevSwaps);
  state = SMaterialPassState();
  state.mTexCoordCount = textures.SetupTexCoordGenerators(index);
  if (AppendBLOLAndBLODAlphaStages(state, material, index, textures)) {
    AppendTextureOrConstantAlphaStages(true, state, opacity, material, index, textures);
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
    CGX::SetColorUpdate(false);
    sMaterialPassFinalizer(state);
    CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
    passes = 3;
  }
  CGX::SetColorUpdate(true);
  return passes;
}

int CCubeMaterial::DrawAdditiveMaterialPass(const CMaterialDataFactory::SMaterial& material,
                                            CCubeSurface surface, int index,
                                            const CModelFlags& flags,
                                            CMaterialTextureState& textures) {
  CGraphics::DisableDestinationAlpha(0);
  SMaterialPassState state;
  state.mTexCoordCount = textures.SetupTexCoordGenerators(index);
  bool useRegisterColor =
      AppendMaterialColorRegister0Stage(state, false, material, index, textures);
  AppendBLOLTextureAlphaMultiplyStage(state, useRegisterColor, material, index, textures);
  AppendBLODAlphaStages(state, useRegisterColor, material, index, textures);
  const uchar opacity = material.mIntegers[0]; // OPAC
  const bool useTextureAlpha = material.mHeader.mFlags & 8;
  if (opacity == 255 && !useTextureAlpha) {
    if (material.mHeader.mPassMask & (1 << CMaterialDataFactory::kP_CLR)) {
      AppendCLRTextureColorStage(state, false, 255, useRegisterColor, material, index, textures);
    } else {
      AppendRasterOrRegisterColorStage(state, false, 255, useRegisterColor, material);
    }
    AppendRFLDTextureColorStages(state, GX_CC_CPREV, useRegisterColor, material, index, textures);
    AppendINCAColorAndAlphaAdditionStages(state, material, index, flags, textures);
  } else {
    if (!useRegisterColor) {
      CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_ZERO,
                         GX_CC_ZERO, GX_CC_CPREV);
      CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                         GX_CA_ZERO, GX_CA_RASA);
      CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(state.mTevCount));
      CGX::SetTevOrder(static_cast< GXTevStageID >(state.mTevCount), GX_TEXCOORD_NULL,
                       GX_TEXMAP_NULL, GX_COLOR0A0);
      ++state.mTevCount;
    }
    CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_TEXC,
                       useRegisterColor ? GX_CC_C0 : GX_CC_RASC, GX_CC_ZERO);
    if (opacity < 255) {
      GXColor color;
      color.a = opacity; // Only alpha is consumed by this stage.
      CGX::SetTevKColor(static_cast< GXTevKColorID >(state.mKonstCount), color);
      CGX::SetTevKAlphaSel(static_cast< GXTevStageID >(state.mTevCount),
                           static_cast< GXTevKAlphaSel >(GX_TEV_KASEL_K0_A + state.mKonstCount));
      ++state.mKonstCount;
      CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_KONST,
                         GX_CA_TEXA, GX_CA_ZERO);
    } else {
      CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                         GX_CA_ZERO, GX_CA_TEXA);
    }
    CGX::SetTevColorOp(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_ADD, GX_TB_ZERO,
                       useRegisterColor ? GX_CS_SCALE_2 : GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    CGX::SetTevAlphaOp(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_ADD, GX_TB_ZERO,
                       GX_CS_SCALE_1, GX_TRUE, GX_TEVREG2);
    ConfigureMaterialPassTextureStage(state, index, CMaterialDataFactory::kP_CLR, material,
                                      textures, useRegisterColor ? GX_COLOR_NULL : GX_COLOR0A0);
    ++state.mTevCount;
    AppendTRANTextureAlphaMultiplyIntoRegister2Stage(state, material, index, textures);
    AppendRFLDTextureRedAlphaMultiplyIntoRegister2Stage(state, material, index, textures);
    AppendRFLDTextureColorStages(state, GX_CC_CPREV, useRegisterColor, material, index, textures);
    CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(state.mTevCount));
    const uchar alpha = material.mIntegers[2]; // BLOI
    if (material.mHeader.mPassMask & (1 << CMaterialDataFactory::kP_INCA)) {
      CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_A2,
                         GX_CC_CPREV, GX_CC_TEXC);
      ConfigureMaterialPassTextureStage(state, index, CMaterialDataFactory::kP_INCA, material,
                                        textures, GX_COLOR_NULL);
      if (material.mPasses[CMaterialDataFactory::kP_INCA].mFlags & 4) {
        ConfigureTrackedTextureSwap(static_cast< GXTevStageID >(state.mTevCount),
                                    material.mPasses[CMaterialDataFactory::kP_INCA].mFlags);
        CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_TEXA, GX_CA_ZERO,
                           GX_CA_ZERO, GX_CA_APREV);
        ++state.mTevCount;
        if (alpha != 0) {
          AppendSelectedKonstAlphaAdditionStage(state, alpha);
        }
      } else {
        if (alpha != 0) {
          CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_KONST, GX_CA_ZERO,
                             GX_CA_ZERO, GX_CA_APREV);
          CGX::SetTevKAlphaSel(static_cast< GXTevStageID >(state.mTevCount),
                               sQuantizedAlphaSelectors[alpha >> 5]);
        } else {
          CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                             GX_CA_ZERO, GX_CA_APREV);
        }
        ++state.mTevCount;
      }
    } else {
      CGX::SetTevColorIn(static_cast< GXTevStageID >(state.mTevCount), GX_CC_ZERO, GX_CC_A2,
                         GX_CC_CPREV, GX_CC_ZERO);
      CGX::SetTevOrder(static_cast< GXTevStageID >(state.mTevCount), GX_TEXCOORD_NULL,
                       GX_TEXMAP_NULL, GX_COLOR_NULL);
      if (alpha != 0) {
        CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_KONST, GX_CA_ZERO,
                           GX_CA_ZERO, GX_CA_APREV);
        CGX::SetTevKAlphaSel(static_cast< GXTevStageID >(state.mTevCount),
                             sQuantizedAlphaSelectors[alpha >> 5]);
      } else {
        CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                           GX_CA_ZERO, GX_CA_APREV);
      }
      ++state.mTevCount;
    }
  }
  AppendBLOITextureAlphaAddStage(state, material, index, textures);
  switch (static_cast< char >(flags.GetTrans())) {
  case CModelFlags::kT_One:
    AppendColorModulationStage(state, flags.GetColor());
    break;
  case CModelFlags::kT_Blend:
  case CModelFlags::kT_Additive:
    AppendPremultipliedColorModulationStage(state, flags.GetColor());
    break;
  case CModelFlags::kT_ThresholdAlpha:
    AppendThresholdAlphaColorModulationStage(state, flags.GetColor());
    break;
  }
  ConfigureDepthModeFromModelFlags(flags, GX_TRUE, GX_LEQUAL, GX_FALSE);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  sMaterialPassFinalizer(state);
  if (!(flags.GetOtherFlags() & 0x1000)) {
    CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
  }
  return 1;
}

static void HandleAlphaMask(uint vtxDesc, uint& tevCount, uint& texCount, uint& tcgCount) {
  const GXTevStageID stage = static_cast< GXTevStageID >(tevCount);
  const GXTexMapID texMap = static_cast< GXTexMapID >(texCount);
  sAlphaMaskTexCoord = tcgCount;
  const CTexture& texture = CCubeRenderer::That()->GetAlphaMaskRamp();
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
  if (fn_8033B70C(vtxDesc, GX_VA_TEX6MTXIDX) == GX_DIRECT && sAlphaMaskTexCoord < 8) {
    CGX::SetVtxDesc(GX_VA_TEX6MTXIDX, GX_NONE);
    CGX::SetVtxDesc(static_cast< GXAttr >(sAlphaMaskTexCoord + GX_VA_TEX0MTXIDX), GX_DIRECT);
  }
}

static void ModulateKColor(const CModelFlags& flags) {
  CGX::SetTevKColor(
      GX_KCOLOR0, CColor::Modulate(flags.GetColor(),
                                   reinterpret_cast< const CColor& >(CGX::GetTevKColor(GX_KCOLOR0)))
                      .GetGXColor());
}

static bool TryModulateKColor(uint tevCount, uint& kColorCount, const CModelFlags& flags) {
  const CModelFlags::ETrans blendMode = flags.GetTrans();
  if (blendMode != CModelFlags::kT_Additive && blendMode != CModelFlags::kT_Blend) {
    return false;
  }
  if (tevCount != 1) {
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
#else
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
#endif

static void HandleTransparency(uint& finalTevCount, uint& finalKColorCount,
#if RSTL_CORRUPTION
                               const CModelFlags& modelFlags, uint blendFactors) {
  if ((modelFlags.GetOtherFlags() & CModelFlags::kF_Unknown400) != 0 &&
      TryModulateKColor(finalTevCount, finalKColorCount, modelFlags)) {
    return;
  }

  const CModelFlags::ETrans blendMode = modelFlags.GetTrans();
  const CColor color = modelFlags.GetColor();
#else
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
#endif

  if (blendMode == 2) {
#if RSTL_CORRUPTION
    if (static_cast< GXBlendFactor >(blendFactors >> 16) == 1) {
#else
    const GXBlendFactor srcFactor = static_cast< GXBlendFactor >(blendFactors >> 16);
    if (srcFactor == 1) {
#endif
      return;
    }
  }

  if (blendMode == 3) {
#if RSTL_CORRUPTION
    const uint stage = finalTevCount;
    const uint stage2 = stage + 1;
    CGX::SetTevColorIn_Compressed(static_cast< GXTevStageID >(stage), 0x73def);
    CGX::SetTevAlphaIn_Compressed(static_cast< GXTevStageID >(stage), 0x1ce7);
    CGX::SetTevColorOp(static_cast< GXTevStageID >(stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                       GX_TRUE, GX_TEVREG0);
    CGX::SetTevKColorSel(static_cast< GXTevStageID >(stage),
#else
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
#endif
                        static_cast< GXTevKColorSel >(finalKColorCount + GX_TEV_KCSEL_K0_A));
#if RSTL_CORRUPTION
    CGX::SetTevAlphaOp(static_cast< GXTevStageID >(stage), GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                       GX_TRUE, GX_TEVPREV);
    CGX::SetTevOrder(static_cast< GXTevStageID >(stage), GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
                     GX_COLOR_NULL);
    CGX::SetTevDirect(static_cast< GXTevStageID >(stage));
#else
    CGX::SetTevAlphaOp(stage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    CGX::SetTevOrder(stage, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
#endif

#if RSTL_CORRUPTION
    CGX::SetTevColorIn_Compressed(static_cast< GXTevStageID >(stage2), 0x7b840);
    CGX::SetTevAlphaIn_Compressed(static_cast< GXTevStageID >(stage2), 0x1ce7);
    CGX::SetTevKColorSel(static_cast< GXTevStageID >(stage),
                         static_cast< GXTevKColorSel >(finalKColorCount + GX_TEV_KCSEL_K0));
    CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(stage2));
    CGX::SetTevDirect(static_cast< GXTevStageID >(stage2));
    CGX::SetTevOrder(static_cast< GXTevStageID >(stage2), GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
                     GX_COLOR_NULL);
    CGX::SetTevKColor(static_cast< GXTevKColorID >(finalKColorCount), color.GetGXColor());
#else
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

#if RSTL_CORRUPTION
    const uint stage = finalTevCount;
    CGX::SetTevColorIn_Compressed(static_cast< GXTevStageID >(stage), colorArgs);
    CGX::SetTevAlphaIn_Compressed(static_cast< GXTevStageID >(stage), alphaArgs);
    CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(stage));
    CGX::SetTevDirect(static_cast< GXTevStageID >(stage));
    CGX::SetTevOrder(static_cast< GXTevStageID >(stage), GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
                     GX_COLOR_NULL);
    sTransparencyKColor = finalKColorCount;
    CGX::SetTevKColor(static_cast< GXTevKColorID >(finalKColorCount), color.GetGXColor());
    CGX::SetTevKColorSel(static_cast< GXTevStageID >(stage),
                         static_cast< GXTevKColorSel >(finalKColorCount + GX_TEV_KCSEL_K0));
    CGX::SetTevKAlphaSel(static_cast< GXTevStageID >(stage),
#else
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
#endif
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

#if RSTL_CORRUPTION
uint CCubeMaterial::HandleReflection(const GXTexMapID indTexSlot, const int indMtxScaleExp,
                                     const uint tevCount, const uint texCount, const uint tcgCount,
                                     const uint kColorCount) {
  bool usesTevReg2 = false;
  for (uint i = 0; i < tevCount; ++i) {
    if ((CGX::GetTevState(static_cast< GXTevStageID >(i)).mColorOps >> 9 & 3) == GX_TEVREG2) {
      usesTevReg2 = true;
      break;
    }
  }

#else
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

#endif
  uint out = 0;
#if RSTL_CORRUPTION
  GXTevStageID finalTevCount = static_cast< GXTevStageID >(tevCount);
  GXTevColorArg colorArg;
  const GXTevKColorID finalKColorCount = static_cast< GXTevKColorID >(kColorCount);
  const GXTexMapID texMap = static_cast< GXTexMapID >(texCount);
#else
#if RSTL_PRIME1
  GXTevColorArg colorArg = GX_CC_CPREV;
#endif

  const GXTevStageID firstStage = static_cast< GXTevStageID >(tevCount);
  GXTevStageID finalTevCount;

#endif
  if (usesTevReg2) {
    colorArg = GX_CC_C2;
#if RSTL_CORRUPTION
    CGX::SetTevColorIn(finalTevCount, GX_CC_ZERO, GX_CC_C2, GX_CC_KONST, GX_CC_ZERO);
    CGX::SetTevAlphaIn(finalTevCount, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A2);
    CGX::SetTevColorOp(finalTevCount, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG2);
    CGX::SetTevAlphaOp(finalTevCount, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG2);
    CGX::SetTevOrder(finalTevCount, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_ZERO);
#else
    CGX::SetTevColorIn(firstStage, GX_CC_ZERO, GX_CC_C2, GX_CC_KONST, GX_CC_ZERO);
    CGX::SetTevAlphaIn(firstStage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A2);
    CGX::SetTevColorOp(firstStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG2);
    CGX::SetTevAlphaOp(firstStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG2);
    CGX::SetTevOrder(firstStage, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_ZERO);
#endif
    out = 1;
  } else {
    colorArg = GX_CC_KONST;
  }

#if !RSTL_CORRUPTION
#if !RSTL_PRIME1
  const GXTevKColorID finalKColorCount = static_cast< GXTevKColorID >(kColorCount);
#endif

  const GXTexMapID texMap = static_cast< GXTexMapID >(texCount);
#if RSTL_PRIME1
  const GXTevKColorID finalKColorCount = static_cast< GXTevKColorID >(kColorCount);
#endif

#endif
  CColor reflectionColor;
  reflectionColor.Set(sReflectionAlpha, sReflectionAlpha, sReflectionAlpha, sReflectionAlpha);
#if RSTL_CORRUPTION
  CGX::SetTevKColor(finalKColorCount, reflectionColor.GetGXColor());
  CGX::SetTevKColorSel(finalTevCount,
#else
  CGX::SetTevKColor(finalKColorCount, reflectionColor);
  CGX::SetTevKColorSel(firstStage,
#endif
                      static_cast< GXTevKColorSel >(finalKColorCount + GX_TEV_KCSEL_K0));

#if RSTL_CORRUPTION
  finalTevCount = static_cast< GXTevStageID >(finalTevCount + out);
#else
  finalTevCount = static_cast< GXTevStageID >(tevCount + out);
#endif

#if RSTL_CORRUPTION
  CCubeRenderer::That()->GetRealReflection()->Load(texMap, CTexture::kCM_Clamp);
#else
  CUBE_RENDERER::That()->GetRealReflection()->Load(texMap, CTexture::kCM_Clamp);
#endif

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
#if RSTL_CORRUPTION
  if (sReflectionStage != -1) {
#else
  if (sReflectionStage == -1) {
    return;
  }

#endif
  CGX::SetNumIndStages(0);
  CGX::SetTevDirect(static_cast< GXTevStageID >(sReflectionStage));
  sReflectionStage = -1;
}
#if RSTL_CORRUPTION
  ResetTrackedTevSwapModes(sMaterialTevSwaps);
  if (sResetAlphaCompare) {
    CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    sResetAlphaCompare = false;
  }
  if (sSavedAlphaContext.valid()) {
    CGraphics::RestoreAlphaContextState(0, *sSavedAlphaContext);
  }
}
#endif

void CCubeMaterial::SetCurrentBlack() const {
  const uint* data = reinterpret_cast< const uint* >(mData);
#if RSTL_CORRUPTION
  const uint texCount = data[1];
  const uint flags = data[0];
  const uint vertexDesc = data[texCount + 2];
#else
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
#endif

  if ((flags & (kStateFlag_DepthSorting | kStateFlag_AlphaTest)) != 0) {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_ONE, GX_LO_CLEAR);
  } else {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
  }

  CGX::SetVtxDescv_Compressed(vertexDesc);

#if RSTL_CORRUPTION
  GXTevColorArg colorArg = GX_CC_ZERO;
  if (sbRenderModelBlackKonst) {
    colorArg = GX_CC_ONE;
  }
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  GXTevColorArg colorArg = GX_CC_ZERO;
  if (lbl_805A9559) {
    colorArg = GX_CC_ONE;
  }
#else
  const GXTevColorArg colorArg = sbRenderModelBlackKonst ? GX_CC_ONE : GX_CC_ZERO;
#endif

#endif
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, colorArg);

#if RSTL_CORRUPTION
  GXTevAlphaArg alphaArg = GX_CA_ZERO;
  if (sbRenderModelBlackKonst) {
    alphaArg = GX_CA_KONST;
  }
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  GXTevAlphaArg alphaArg = GX_CA_ZERO;
  if (lbl_805A9559) {
    alphaArg = GX_CA_KONST;
  }
#else
  const GXTevAlphaArg alphaArg = sbRenderModelBlackKonst ? GX_CA_KONST : GX_CA_ZERO;
#endif

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
#if RSTL_CORRUPTION
  GXCompare func;
  if ((modelFlags & CModelFlags::kF_Unknown200) == 0) {
    func = GX_LEQUAL;
  } else if ((modelFlags & CModelFlags::kF_DepthGreater) != 0) {
#else
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

#endif
    if ((modelFlags & CModelFlags::kF_DepthNonInclusive) != 0) {
      func = GX_GREATER;
    } else {
      func = GX_GEQUAL;
    }
  } else if ((modelFlags & CModelFlags::kF_DepthNonInclusive) != 0) {
    func = GX_LESS;
#if RSTL_CORRUPTION
    } else {
      func = GX_EQUAL;
    }
  CGX::SetZMode((modelFlags & CModelFlags::kF_NoDepthCompare) == 0, func,
                (modelFlags & CModelFlags::kF_NoDepthUpdate) == 0 &&
                    ((modelFlags & CModelFlags::kF_ForceDepthUpdate) != 0 ||
                     (matFlags & kStateFlag_DepthWrite) != 0));
#else
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
#endif
}

static void DoPassthru(const uint finalTevCount) {
  const GXTevStageID stage = static_cast< GXTevStageID >(finalTevCount);
  CGX::SetTevColorIn(stage, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
  CGX::SetTevAlphaIn(stage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
  CGX::SetTevOrder(stage, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  CGX::SetTevDirect(stage);
  CGX::SetStandardTevColorAlphaOp(stage);
}

#if RSTL_CORRUPTION
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
  const uint finalTcgCount = *uvAnim.words;
  for (uint i = 0; i < finalTcgCount; ++i) {
    CGX::SetTexCoordGen_Compressed(static_cast< GXTexCoordID >(i), uvAnim.words[i + 1]);
  }
  uvAnim.words += finalTcgCount + 1;

  scanner_t end;
  end.bytes = uvAnim.bytes + *uvAnim.words + 4;
  const uint animCount = uvAnim.words[1];
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
  if ((materialDataCur.words[2] >> 9 & 3) == GX_TEVREG0) {
    materialDataCur.words += 5;
    texMapTexCoordFlags.words += 1;
  }
  const int texCoord = HandleThermalTev(firstTev, materialDataCur.words, texMapTexCoordFlags.words);

  scanner_t uvAnim;
  uvAnim.words = savedTexMapTexCoordFlags.words + matTevCount;
  const uint fullTcgCount = *uvAnim.words;
  if (texCoord != GX_TEXCOORD_NULL) {
    for (int i = 0; i < fullTcgCount; ++i) {
      if (texCoord == i) {
        CGX::SetTexCoordGen_Compressed(GX_TEXCOORD0, uvAnim.words[i + 1]);
        break;
      }
    }
  }
  uvAnim.words += fullTcgCount + 1;

  scanner_t end;
  end.bytes = uvAnim.bytes + *uvAnim.words + 4;
  const uint animCount = uvAnim.words[1];
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
#else
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

#if RSTL_CORRUPTION
  const uint* materialDataCur = reinterpret_cast< const uint* >(GetData());
#else
#if !RSTL_PRIME1
  scanner_t materialDataCur;
  materialDataCur.bytes = GetData();
#endif

#endif
  uint numIndStages = 0;
#if RSTL_CORRUPTION
  sLastMaterialCached = GetData();
#endif
  GXTexMapID indTexSlot = GX_TEXMAP0;
#if RSTL_CORRUPTION
  const uint matFlags = materialDataCur[0];
  uint texCount = materialDataCur[1];
#else
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

#endif
  const bool reflection =
      (matFlags & (kStateFlag_Reflection | kStateFlag_ReflectionSurfaceEye)) != 0;
#if RSTL_CORRUPTION
  sRenderingModel = &model;
#else
#if !RSTL_PRIME1
  sLastMaterialCached = GetData();
  sRenderingModel = &model;
#endif

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
#if RSTL_CORRUPTION
    materialDataCur += 2;
#else
    materialDataCur.words += 2;
#endif
    for (uint i = 0; i < texCount; ++i) {
#if RSTL_CORRUPTION
      textures[*materialDataCur].GetObject()->Load(static_cast< GXTexMapID >(i),
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      textures[SBig(*materialDataCur.words)].GetObject()->Load(static_cast< GXTexMapID >(i),
#else
      textures[ReadWord(materialDataCur.words)].GetObject()->Load(static_cast< GXTexMapID >(i),
#endif

#endif
                                                               CTexture::kCM_Repeat);
#if RSTL_CORRUPTION
      ++materialDataCur;
#else
      ++materialDataCur.words;
#endif
    }
  } else {
#if RSTL_CORRUPTION
    materialDataCur += texCount + 2;
#else
    materialDataCur.words += texCount + 2;
#endif
  }

#if RSTL_CORRUPTION
  const uint vertexDesc = *materialDataCur++;
  materialDataCur += 2;
  const int groupIdx = static_cast< int >(*materialDataCur++);

  uint finalKColorCount = 0;
  if ((matFlags & kStateFlag_KonstValues) != 0) {
    finalKColorCount = materialDataCur[0];
    for (uint i = 0; i < finalKColorCount; ++i) {
      CGX::SetTevKColor(static_cast< GXTevKColorID >(i), CColor::ToGX(materialDataCur[i + 1]));
    }
    materialDataCur += finalKColorCount + 1;
  }
#else
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

#if RSTL_CORRUPTION
  const uint blendFactors = *materialDataCur;
#else
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

#endif
    SetupBlendMode(blendFactors, flags, (matFlags & kStateFlag_AlphaTest) != 0);
#if RSTL_PRIME1
  }
#endif

#if RSTL_CORRUPTION
  ++materialDataCur;
#else
  ++materialDataCur.words;
#endif
  const bool indTex = (matFlags & kStateFlag_ReflectionIndirectTexture) != 0;
  if (indTex) {
#if RSTL_CORRUPTION
    indTexSlot = static_cast< GXTexMapID >(*materialDataCur++);
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    indTexSlot = static_cast< GXTexMapID >(SBig(*materialDataCur.words++));
#else
    indTexSlot = static_cast< GXTexMapID >(ReadWord(materialDataCur.words));
    ++materialDataCur.words;
#endif

#endif
  }

  HandleDepth(flags.GetOtherFlags(), matFlags);

#if RSTL_CORRUPTION
  const uint chanCount = materialDataCur[0];
  const uint firstChan = materialDataCur[1];
  materialDataCur += chanCount + 1;
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const uint chanCount = SBig(materialDataCur.words[0]);
  const uint firstChan = SBig(materialDataCur.words[1]);
#else
  const uint chanCount = ReadWord(&materialDataCur.words[0]);
  const uint firstChan = ReadWord(&materialDataCur.words[1]);
#endif

  materialDataCur.words += chanCount + 1;
#endif
  const uint finalNumColorChans = HandleColorChannels(chanCount, firstChan);

#if RSTL_PRIME1
  uint matTevCount;
#endif

  uint firstTev = 0;
  if (sbRenderModelShadow) {
    firstTev = 2;
  }

#if RSTL_CORRUPTION
  uint finalTevCount = *materialDataCur++;
  uint tcgCount = 0;
  sTevHandler(materialDataCur, firstTev, finalTevCount, tcgCount);

  if (flags.GetTrans() != CModelFlags::kT_Opaque) {
    HandleTransparency(finalTevCount, finalKColorCount, flags, blendFactors);
  }

  if ((flags.GetOtherFlags() & CModelFlags::kF_Unknown80) != 0) {
    HandleAlphaMask(vertexDesc, finalTevCount, texCount, tcgCount);
#else
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
#if RSTL_CORRUPTION
    } else if (finalTevCount != 0 &&
               (CGX::GetTevState(static_cast< GXTevStageID >(finalTevCount - 1)).mColorOps >> 9 &
                3) != 0) {
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    } else if ((finalCCFlags >> 9 & 3) != 0) {
#else
    } else if (finalTevCount != 0 &&
               CGX::MaskAndShiftLeft(
                   CGX::GetTevState(static_cast< GXTevStageID >(finalTevCount - 1)).mColorOps, 3,
                   9) != 0) {
#endif

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
#if !RSTL_CORRUPTION
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
#endif
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

#if RSTL_CORRUPTION
  const CTransform4f& modelMtx = CGraphics::GetModelMatrix();
  const CVector3f& playerPos =
      modelMtx.TransposeRotate(sPlayerPosition - modelMtx.GetTranslation());
#else
  const CVector3f& playerPos = GetReflectionPlayerPosition();
#endif
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

#if RSTL_CORRUPTION
  CCubeRenderer* renderer = CCubeRenderer::That();
#else
  CUBE_RENDERER* renderer = CUBE_RENDERER::That();
#endif
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
#if RSTL_CORRUPTION
      sViewingFrom = modelPoint;
#else
      const float pointX = modelPoint.GetX();
      const float pointY = modelPoint.GetY();
      const float pointZ = modelPoint.GetZ();
      sViewingFrom = CVector3f(pointX, pointY, pointZ);
#endif
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

#if RSTL_CORRUPTION
  CVector3f dir = distVec / reflDist;
  CVector3f right = CVector3f::Cross(dir, CVector3f(0.f, 0.f, 1.f));
#else
#if RSTL_PRIME1
  CVector3f right = CVector3f::Cross(distVec / reflDist, CVector3f(0.f, 0.f, 1.f));
#endif

#endif
  float xScale = 0.32258067f;
  float yScale = 0.32258067f;
#if !RSTL_CORRUPTION
#if !RSTL_PRIME1
  CVector3f dir = (1.f / reflDist) * distVec;
  CVector3f right = CVector3f::Cross(dir, CVector3f(0.f, 0.f, 1.f));
#endif

#endif
  if (rs_can_normalize(right)) {
    right.Normalize();
  } else {
#if !RSTL_PRIME1
    dir = CVector3f::Forward();
#endif

    right = CVector3f::Right();
  }

#if !RSTL_CORRUPTION
  const float playerZ = playerPoint.GetZ();
#endif
  const float scale = 0.02f * reflDist + 1.f;
#if !RSTL_CORRUPTION
  const float rightDist = CVector3f::Dot(modelPoint, right);

#endif
  xScale *= scale * sThrobX;
  yScale *= scale * sThrobY;

#if RSTL_CORRUPTION
  SMtx texMtx = texMtx2;
  texMtx.m[0][0] = xScale * right.GetX();
  texMtx.m[0][1] = xScale * right.GetY();
  texMtx.m[0][3] = -CVector3f::Dot(modelPoint, right) * xScale + 0.5f;
  texMtx.m[1][2] = yScale;
  texMtx.m[1][3] = -playerPoint.GetZ() * yScale;
  CGX::LoadTexMtxImm(texMtx.m, GX_TEXMTX7, GX_MTX3x4);
#else
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

#endif
}

void CCubeMaterial::KillCachedViewDepState() {
#if RSTL_CORRUPTION
  sCachedMaterialModel = nullptr;
#else
  sLastModelCached = nullptr;
#if !RSTL_PRIME1
  sMaterialCachedState = 0;
#endif
#endif
}

void CCubeMaterial::ResetCachedMaterials() {
  KillCachedViewDepState();
#if RSTL_CORRUPTION
  sLastMaterialIndex = -1;
  sLastParsedMaterialUnique = -1;
  sMaterialStateUnknown = false;
#else
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

#endif
}
#if RSTL_CORRUPTION

uint CCubeMaterial::GetCompressedBlend() const {
  const uint* ptr = reinterpret_cast< const uint* >(mData);
  const uint flags = ptr[0];
  const uint texCount = ptr[1];
  const uint* blend = ptr + texCount + 6;
  if ((flags & kStateFlag_KonstValues) != 0) {
    blend += *blend + 1;
  }
  return *blend;
}

uint CCubeMaterial::GetExtraPostTexMtx() { return sAlphaMaskPostTexMtx; }

uint CCubeMaterial::GetExtraTexCoord() { return sAlphaMaskTexCoord; }

void CCubeMaterial::ResetExtraTexCoord() { sAlphaMaskTexCoord = GX_TEXCOORD_NULL; }
#else
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
#if RSTL_CORRUPTION
  sThrobX = 1. / (1. - throbAmplitudeX * sin(time * frequency + phaseX));
  sThrobY = 1. / (1. - throbAmplitudeY * sin(sLastTime * frequency + phaseY));
#else
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
#if RSTL_CORRUPTION
  if (CCubeRenderer::That()->GetMaterialMode() != 0) {
    return;
  }
#else
#if !RSTL_PRIME1
  if (CUBE_RENDERER::That()->GetMaterialMode() != 0) {
    return;
  }
#endif

#endif
  sbRenderModelShadow = true;
  spShadowTexture = const_cast< CTexture* >(shadowTex);
  sTextureProjectionTransform = textureProjXf;
  sChannel0DisableLightMask = chan0DisableMask;
  sChannel1EnableLightMask = chan1EnableLightMask;
}

void CCubeModel::DisableShadowMaps() { sbRenderModelShadow = false; }

#if RSTL_CORRUPTION
int CCubeMaterial::GetTransparencyKColor() { return sTransparencyKColor; }

void CCubeMaterial::ResetTransparencyKColor() { sTransparencyKColor = -1; }

void CCubeMaterial::UseNormalTevs() { sTevHandler = HandleNormalTevs; }

void CCubeMaterial::UseThermalTevs() { sTevHandler = HandleThermalTevs; }

CMaterialTextureState::CMaterialTextureState(
    const rstl::vector< CMaterialDataFactory::SMaterial >& materials, CSimplePool* pool)
: mFirstUncachedTexture(0)
, mTexturesLocked(false)
, mTexturesLoaded(false)
, mMappingsFinalized(false) {
  rstl::set< CAssetId > textureIds;
  const CMaterialDataFactory::SMaterial* materialData = materials.data();
  int materialCount = materials.size();
  for (int i = 0; i < materialCount; ++i) {
    const CMaterialDataFactory::SMaterial& material = materialData[i];
    for (int pass = 0; pass < CMaterialDataFactory::kP_Count; ++pass) {
      CAssetId id = material.mPasses[pass].mTextureId;
      if (id != kInvalidAssetId) {
        textureIds.insert(id);
      }
    }
  }

  rstl::vector< CAssetId > identifiers(textureIds.begin(), textureIds.end());
  mTextures.reserve(identifiers.size());
  for (int i = 0; i < identifiers.size(); ++i) {
    mTextures.push_back(pool->GetObj(SObjectTag('TXTR', identifiers[i])));
  }

  mMaterials.reserve(materials.size());
  materialData = materials.data();
  materialCount = materials.size();
  for (int i = 0; i < materialCount; ++i) {
    const CMaterialDataFactory::SMaterial& material = materialData[i];
    SMaterialTextures textures;
    textures.mUniqueIndex = material.mHeader.mUniqueIndex;
    for (int pass = 0; pass < CMaterialDataFactory::kP_Count; ++pass) {
      const CMaterialDataFactory::SPass& materialPass = material.mPasses[pass];
      CAssetId id = materialPass.mTextureId;
      if (id != kInvalidAssetId) {
        bool found = false;
        for (int j = 0; j < textures.mTextureIndices.size(); ++j) {
          if (identifiers[textures.mTextureIndices[j]] == id) {
            textures.mPassTexMapIndices[pass] = j;
            found = true;
            break;
          }
        }
        if (!found) {
          textures.mTextureIndices.push_back(
              rstl::binary_find(identifiers.begin(), identifiers.end(), id) - identifiers.begin());
          textures.mPassTexMapIndices[pass] = textures.mTextureIndices.size() - 1;
        }
        if (materialPass.mAnimationData == nullptr) {
          textures.mPassTexCoordIndices[pass] = materialPass.mUvIndex;
        } else {
          textures.mPassTexCoordIndices[pass] = -1;
        }
      } else {
        textures.mPassTexMapIndices[pass] = -1;
        textures.mPassTexCoordIndices[pass] = -1;
      }
    }
    mMaterials.push_back(textures);
  }
}

void CMaterialTextureState::LockTextures() {
  if (!mTexturesLocked) {
    mTexturesLocked = true;
    mFirstUncachedTexture = 0;
    for (int i = 0; i < mTextures.size(); ++i) {
      mTextures[i].Lock();
    }
  }
}

bool CMaterialTextureState::TryCacheTextures() {
  if (!mTexturesLocked) {
    return false;
  }
  if (mTexturesLoaded) {
    return true;
  }
  for (int i = mFirstUncachedTexture; i < mTextures.size(); ++i) {
    if (!mTextures[i].TryCache()) {
      mFirstUncachedTexture = i;
      return false;
    }
  }
  FinalizeMappings();
  return true;
}

void CMaterialTextureState::FinalizeMappings() {
  mTexturesLoaded = true;
  if (!mMappingsFinalized) {
    mMappingsFinalized = true;
    int maxUniqueIndex = -1;
    for (rstl::vector< SMaterialTextures >::iterator it = mMaterials.begin();
         it != mMaterials.end(); ++it) {
      SMaterialTextures& material = *it;
      if (material.mUniqueIndex > maxUniqueIndex) {
        maxUniqueIndex = material.mUniqueIndex;
      }
      material.mTexCoordSources.clear();
      rstl::reserved_vector< CVector3i, CMaterialDataFactory::kP_Count > coordinates;
      for (int pass = 0; pass < CMaterialDataFactory::kP_Count; ++pass) {
        if (material.mPassTexCoordIndices[pass] != -1) {
          if (material.mPassTexMapIndices[pass] == -1) {
            material.mPassTexCoordIndices[pass] = 0;
          } else {
            const CTexture* texture =
                mTextures[material.mTextureIndices[material.mPassTexMapIndices[pass]]].GetObject();
            CVector3i coordinate(texture->GetWidth(), texture->GetHeight(),
                                 material.mPassTexCoordIndices[pass]);
            int index = rstl::find(coordinates.begin(), coordinates.end(), coordinate) -
                        coordinates.begin();
            if (index == coordinates.size()) {
              coordinates.push_back(coordinate);
              material.mTexCoordSources.push_back(
                  static_cast< GXTexGenSrc >(GX_TG_TEX0 + material.mPassTexCoordIndices[pass]));
            }
            material.mPassTexCoordIndices[pass] = index;
          }
        }
      }
    }

    if (maxUniqueIndex > -1) {
      rstl::vector< int > uniqueIndices;
      uniqueIndices.reserve(mMaterials.size());
      int lastUniqueIndex = -1;
      for (int i = 0; i < mMaterials.size(); ++i) {
        const SMaterialTextures& material = mMaterials[i];
        int maxPreviousIndex = -1;
        int uniqueIndex = -1;
        for (int j = 0; j < i; ++j) {
          const SMaterialTextures& previous = mMaterials[j];
          if (material.mUniqueIndex == previous.mUniqueIndex) {
            const int previousIndex = uniqueIndices[j];
            if (previousIndex <= maxPreviousIndex) {
              continue;
            }
            if (material.mPassTexCoordIndices == previous.mPassTexCoordIndices) {
              uniqueIndex = previousIndex;
              break;
            }
            maxPreviousIndex = previousIndex;
          }
        }
        if (uniqueIndex == -1) {
          uniqueIndex = lastUniqueIndex + 1;
          ++lastUniqueIndex;
        }
        uniqueIndices.push_back(uniqueIndex);
      }
      for (int i = 0; i < mMaterials.size(); ++i) {
        mMaterials[i].mUniqueIndex = uniqueIndices[i];
      }
    }
  }
}

void CMaterialTextureState::BindTextures(int materialIndex) {
  TCachedToken< CTexture >* textures = mTextures.data();
  const SMaterialTextures& material = mMaterials[materialIndex];
  for (int i = 0; i < material.mTextureIndices.size(); ++i) {
    TCachedToken< CTexture >& texture = textures[material.mTextureIndices[i]];
    if (texture.IsLocked() && texture.IsLoaded()) {
      texture.GetObject()->Load(static_cast< GXTexMapID >(i), CTexture::kCM_Repeat);
    }
  }
}

int CMaterialTextureState::GetPassTexMapIndex(int materialIndex, int pass) const {
  return mMaterials[materialIndex].mPassTexMapIndices[pass];
}

int CMaterialTextureState::SetupTexCoordGenerators(int materialIndex) const {
  const SMaterialTextures& material = mMaterials[materialIndex];
  for (int i = 0; i < material.mTexCoordSources.size(); ++i) {
    CGX::SetTexCoordGen(static_cast< GXTexCoordID >(i), GX_TG_MTX2x4, material.mTexCoordSources[i],
                        GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
  }
  return material.mTexCoordSources.size();
}

int CMaterialTextureState::GetPassTexCoordIndex(int materialIndex, int pass) const {
  return mMaterials[materialIndex].mPassTexCoordIndices[pass];
}

void CCubeMaterial::SelectMaterialTevHandler(int mode) {
  switch (mode) {
  case 0:
    sMaterialPassFinalizer = FinalizePassStageCounts;
    return;
  case 1:
    sMaterialPassFinalizer = ApplyStageCountPalette;
    return;
  case 2:
    sMaterialPassFinalizer = ApplyAdditiveStageCountVisualization;
    return;
  }
}

void CCubeMaterial::ApplyTevRegister1Color() {
  GXSetTevColor(GX_TEVREG1, sTevRegister1Color.GetGXColor());
  sLastMaterialRegisterColor = CColor::White();
}

void CCubeMaterial::SetTevRegister1Color(GXColor color) {
  sTevRegister1Color.Set(*reinterpret_cast< const uint* >(&color));
  ApplyTevRegister1Color();
}

void CCubeMaterial::SetDefaultTevSwapTables() {
  CGX::SetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
  CGX::SetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_RED);
  CGX::SetTevSwapModeTable(GX_TEV_SWAP2, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_GREEN);
  CGX::SetTevSwapModeTable(GX_TEV_SWAP3, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_BLUE);
}

void CCubeMaterial::ResetGuiColorKRegister() { sGuiColorKRegister = -1; }

int CCubeMaterial::GetGuiColorKRegister() { return sGuiColorKRegister; }

void CCubeMaterial::DrawMaterialSurface(const CModelFlags& flags, const CCubeSurface& surface,
                                        const CCubeModel& model) {
  const int materialIndex = surface.GetMaterialIndex();
  CMaterialTextureState& textures = model.GetTextureState();
  const int uniqueIndex = textures.GetUniqueIndex(materialIndex);
  const CMaterialDataFactory::SMaterial& material = model.GetMaterialByIndex(materialIndex);
  if (sMaterialCacheEnabled && sCachedMaterialModel == &model) {
    if (sLastMaterialIndex == materialIndex) {
      if (!(flags.GetOtherFlags() & 0x1000)) {
        CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
      }
      return;
    }
    if (sLastMaterialIndex != -1 && sLastParsedMaterialUnique != -1 &&
        sLastParsedMaterialUnique == uniqueIndex) {
      textures.BindTextures(materialIndex);
      if (!(flags.GetOtherFlags() & 0x1000)) {
        CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
      }
      sLastMaterialIndex = materialIndex;
      return;
    }
  }

  sCachedMaterialModel = &model;
  sLastMaterialIndex = materialIndex;
  sLastParsedMaterialUnique = uniqueIndex;
  CCubeMaterial::EnsureTevsDirect();
  sSavedAlphaContext = CGraphics::GetAlphaContextState(0);
  textures.BindTextures(materialIndex);
  CGX::SetVtxDescv_Compressed(material.mHeader.mVertexDesc);

  const int blendMode = static_cast< signed char >(flags.GetTrans());
  uchar opacity = material.mIntegers[0];
  if (blendMode > 5) {
    opacity = opacity * flags.GetColorRef().GetAlphau8() / 255;
  }
  const uint materialFlags = material.mHeader.mFlags;
  const uint flag3 = (materialFlags >> 3) & 1;
  const bool opaqueFlag4 = (materialFlags & 0x10) && opacity == 255;
  const bool alpha = (flag3 || opacity < 255) && !opaqueFlag4;
  const uint flag5 = (materialFlags >> 5) & 1;
  const bool hasIncandescence = (material.mHeader.mPassMask & 0x410) != 0;
  const bool keepAlpha = (flags.GetOtherFlags() & 0x800) || (materialFlags & 1);
  const bool additive = blendMode > 7;
  const bool defaultAlphaContext = CGraphics::GetAlphaContext() == 0;
  const uint flag9 = (material.mHeader.mFlags >> 9) & 1;
  int path = 2;
  if (sbRenderModelBlack) {
    path = 9;
  } else if (flag9) {
    path = 8;
  } else if (flag5 && !flag3) {
    path = 3;
  } else if (additive) {
    path = 4;
  } else if (flag5) {
    if (keepAlpha || !defaultAlphaContext) {
      path = 1;
    } else {
      path = 6;
    }
  } else if (opaqueFlag4) {
    path = 7;
  } else if (alpha) {
    if (keepAlpha || !defaultAlphaContext) {
      if (hasIncandescence) {
        path = 1;
      } else {
        path = 0;
      }
    } else {
      if (hasIncandescence) {
        path = 6;
      } else {
        path = 5;
      }
    }
  }
  if (flags.GetTrans() == CModelFlags::kT_ThresholdAlpha) {
    const uchar modelAlpha = flags.GetColorRef().GetAlphau8();
    if (modelAlpha > 127 && modelAlpha < 255) {
      opacity = (modelAlpha - 128) * 2;
    } else if (modelAlpha != 255) {
      if (path == 2) {
        path = 10;
      } else {
        path = 8;
      }
    }
  }

  int result = 0;
  switch (path) {
  case 0:
    result = DrawAlphaBlendedMaterialPassWithoutAlphaUpdate(material, surface, materialIndex,
                                                            opacity, flags, textures);
    break;
  case 1:
    result = DrawMaterialWithINCAAdditiveOverlay(material, surface, materialIndex, opacity, flags,
                                                 textures);
    break;
  case 2:
    result = DrawOpaqueMaterialPass(material, surface, materialIndex, flags, textures);
    break;
  case 3:
    result = DrawAdditiveINCAMaterialPass(material, surface, materialIndex, flags, textures);
    break;
  case 4:
    result = DrawAdditiveMaterialPass(material, surface, materialIndex, flags, textures);
    break;
  case 5:
    result = DrawAlphaBlendedMaterialWithAuxiliaryPasses(material, surface, materialIndex, opacity,
                                                         flags, textures);
    break;
  case 6:
    result = DrawMaterialWithOneOneINCAOverlay(material, surface, materialIndex, opacity, flags,
                                               textures);
    break;
  case 7:
    result =
        DrawAlphaCutoff192BlendedMaterialPass(material, surface, materialIndex, flags, textures);
    break;
  case 8:
    result = DrawWhiteZeroAlphaBasePass(surface, flags);
    break;
  case 9:
    result = DrawConstantColorZeroAlphaBasePass(
        surface, flags, sbRenderModelBlackKonst ? CColor::White() : CColor::Black());
    break;
  case 10:
    result = DrawAlphaThresholdMaterialPass(material, surface, materialIndex, flags, textures);
    break;
  }
  if (result != 1) {
    sLastMaterialIndex = -1;
    sLastParsedMaterialUnique = -1;
  }
  sMaterialStateUnknown = false;
  sMaterialStageCount = 0;
}

int CCubeMaterial::DrawWhiteZeroAlphaBasePass(CCubeSurface surface, const CModelFlags& flags) {
  SMaterialPassState state;
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ONE);
  CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                     GX_CA_ZERO, GX_CA_ZERO);
  CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(state.mTevCount));
  CGX::SetTevOrder(static_cast< GXTevStageID >(state.mTevCount), GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
                   GX_COLOR_NULL);
  ++state.mTevCount;
  ConfigureDepthModeFromModelFlags(flags, GX_TRUE, GX_LEQUAL, GX_FALSE);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_ONE, GX_LO_CLEAR);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  sMaterialPassFinalizer(state);
  if (flags.GetOtherFlags() & 0x1000) {
    return 1;
  }
  CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
  return 1;
}

int CCubeMaterial::DrawConstantColorZeroAlphaBasePass(CCubeSurface surface,
                                                      const CModelFlags& flags, CColor color) {
  CGraphics::DisableDestinationAlpha(0);
  SMaterialPassState state;
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
  CGX::SetTevAlphaIn(static_cast< GXTevStageID >(state.mTevCount), GX_CA_ZERO, GX_CA_ZERO,
                     GX_CA_ZERO, GX_CA_ZERO);
  CGX::SetTevKColorSel(static_cast< GXTevStageID >(state.mTevCount), GX_TEV_KCSEL_K0);
  CGX::SetTevKColor(GX_KCOLOR0, color.GetGXColor());
  CGX::SetStandardTevColorAlphaOp(static_cast< GXTevStageID >(state.mTevCount));
  CGX::SetTevOrder(static_cast< GXTevStageID >(state.mTevCount), GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
                   GX_COLOR_NULL);
  ++state.mTevCount;
  CGX::SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  ConfigureDepthModeFromModelFlags(flags, GX_TRUE, GX_LEQUAL, GX_TRUE);
  sMaterialPassFinalizer(state);
  if (flags.GetOtherFlags() & 0x1000) {
    return 1;
  }
  CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
  return 1;
}

void CCubeMaterial::ResetTrackedTevSwapModes(SMaterialTevSwapState& state) {
  state.mMask = 0;
  for (int i = 0; i < state.mStages.size(); ++i) {
    CGX::SetTevSwapMode(static_cast< GXTevStageID >(state.mStages[i]), GX_TEV_SWAP0, GX_TEV_SWAP0);
  }
  state.mStages.clear();
}

void CCubeMaterial::ConfigureTrackedTextureSwap(GXTevStageID stage, uint flags) {
  const int selector = flags & 3;
  if (selector == 3) {
    return;
  }
  CGX::SetTevSwapMode(stage, GX_TEV_SWAP0, static_cast< GXTevSwapSel >(selector + 1));
  sMaterialTevSwaps.AddStage(stage);
}

void CCubeMaterial::ConfigureDepthModeFromModelFlags(const CModelFlags& flags, GXBool compare,
                                                     GXCompare func, GXBool update) {
  const uint modelFlags = flags.GetOtherFlags();
  if (modelFlags & 0x2203) {
    const bool enabled = !(modelFlags & CModelFlags::kF_NoDepthCompare) && compare;
    if ((modelFlags & CModelFlags::kF_Unknown200) && (modelFlags & CModelFlags::kF_DepthGreater)) {
      if (modelFlags & CModelFlags::kF_DepthGreater) {
        if (modelFlags & CModelFlags::kF_DepthNonInclusive) {
          func = GX_GREATER;
        } else {
          func = GX_GEQUAL;
        }
      } else {
        if (modelFlags & CModelFlags::kF_DepthNonInclusive) {
          func = GX_LESS;
        } else {
          func = GX_EQUAL;
        }
      }
    }
    if (modelFlags & CModelFlags::kF_NoDepthUpdate) {
      update = GX_FALSE;
    } else if (modelFlags & CModelFlags::kF_ForceDepthUpdate) {
      update = GX_TRUE;
    }
    CGX::SetZMode(enabled, func, update);
  } else {
    CGX::SetZMode(compare, func, update);
  }
}

void CCubeMaterial::ApplyAdditiveStageCountVisualization(const SMaterialPassState& state) {
  CGX::SetNumTevStages(1);
  CGX::SetNumTexGens(0);
  GXColor color = {0, 0, 0, 0};
  const uchar intensity = state.mTevCount * 8;
  color.r = intensity;
  color.g = intensity;
  color.b = intensity;
  CGX::SetNumChans(1);
  CGX::SetTevKColor(GX_KCOLOR0, color);
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  CGX::SetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
  CGX::SetColorUpdate(true);
}

void CCubeMaterial::ApplyStageCountPalette(const SMaterialPassState& state) {
  CGX::SetNumTevStages(1);
  CGX::SetNumTexGens(0);
  CGX::SetNumChans(1);
  static const CColor palette[] = {CColor::Yellow(), CColor::Green(), CColor::Blue(),
                                   CColor::Purple(), CColor::Red(),   CColor::Orange()};
  sMaterialStageCount += state.mTevCount;
  if (sMaterialStageCount < 1 || sMaterialStageCount > sizeof(palette) / sizeof(palette[0])) {
    CGX::SetTevKColor(GX_KCOLOR0, CColor::White().GetGXColor());
  } else {
    CGX::SetTevKColor(GX_KCOLOR0, palette[sMaterialStageCount - 1].GetGXColor());
  }
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  CGX::SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
  CGX::SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
  CGX::SetColorUpdate(true);
}

void CCubeMaterial::FinalizePassStageCounts(const SMaterialPassState& state) {
  CGX::SetNumTevStages(state.mTevCount);
  CGX::SetNumTexGens(state.mTexCoordCount);
  const bool channels = state.mUsesRasterColor || state.mTexCoordCount == 0;
  CGX::SetNumChans(channels ? 1 : 0);
}

CMaterialDrawStateGuard::CMaterialDrawStateGuard(const CModelFlags& flags,
                                                 CCubeSurface surface,
                                                 const CCubeModel& model) {
  CCubeMaterial::DrawMaterialSurface(CModelFlags(flags, flags.GetOtherFlags() | 0x1800), surface,
                                     model);
}

CMaterialDrawStateGuard::~CMaterialDrawStateGuard() { CCubeMaterial::EnsureTevsDirect(); }
#else
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
#endif
