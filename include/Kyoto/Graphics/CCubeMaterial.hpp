#ifndef _CCUBEMATERIAL
#define _CCUBEMATERIAL
#include "rstl/RstlVersions.h"
#include "rstl/RstlVersions.h"
#include "rstl/types.h"

#include "Kyoto/Basics/CBasics.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CMaterialDataFactory.hpp"
#endif
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#if RSTL_CORRUPTION
#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"

#include "dolphin/gx.h"


#else
#if !RSTL_PRIME1
#include "dolphin/gx.h"
#endif
#endif

enum EStateFlags {
  kStateFlag_KonstValues = (1 << 3),
  kStateFlag_DepthSorting = (1 << 4),
  kStateFlag_AlphaTest = (1 << 5),
  kStateFlag_Reflection = (1 << 6),
  kStateFlag_DepthWrite = (1 << 7),
  kStateFlag_ReflectionSurfaceEye = (1 << 8),
  kStateFlag_ShadowOccluderMesh = (1 << 9),
  kStateFlag_ReflectionIndirectTexture = (1 << 10),
  kStateFlag_Lightmap = (1 << 11),
  kStateFlag_LightmapUvArray = (1 << 13),
  kStateFlag_TextureSlotMask = static_cast< uint >(~kStateFlag_LightmapUvArray),
};

class CCubeSurface;
class CCubeModel;
#if RSTL_CORRUPTION
class CMaterialTextureState;

// Guessed name
class CMaterialDrawStateGuard {
public:
  CMaterialDrawStateGuard(const CModelFlags& flags, CCubeSurface surface,
                          const CCubeModel& model);
  ~CMaterialDrawStateGuard();
};

#endif
class CCubeMaterial {
public:
#if RSTL_CORRUPTION
  // Guessed name
  static void SelectMaterialTevHandler(int mode);

  // Guessed name
  static void SetDefaultTevSwapTables();

  // Guessed name
  static void ResetGuiColorKRegister();

  // Guessed name
  static int GetGuiColorKRegister();

  // Guessed name
  static void DrawMaterialSurface(const CModelFlags& flags, const CCubeSurface& surface,
                                  const CCubeModel& model);

#endif
  explicit CCubeMaterial(const void* data) : mData(data) {}
  static void ResetCachedMaterials();
  static void EnsureViewDepStateCached(const CCubeSurface* surface);
  static void EnsureTevsDirect();
  static void KillCachedViewDepState();

  static void UseThermalTevs();
  static void UseNormalTevs();
  static void ResetTransparencyKColor();
  static int GetTransparencyKColor();
  static void ResetExtraTexCoord();
  static uint GetExtraTexCoord();
  static uint GetExtraPostTexMtx();
  // Guessed name
#if RSTL_CORRUPTION
  static void ApplyTevRegister1Color();
#else
  const uchar* GetData() const { return static_cast< const uchar* >(mData); }
#endif
  // Guessed name
#if RSTL_CORRUPTION
  static void SetTevRegister1Color(GXColor color);

  const uchar* GetData() const { return static_cast< const uchar* >(mData); }
#endif
  uint GetFlags() const { return CBasics::SwapBytes(*reinterpret_cast< const uint* >(GetData())); }
  bool IsFlagSet(const EStateFlags flag) const { return (GetFlags() & flag) != 0; }
  uint GetTextureCount() const {
#if RSTL_PRIME1
    const uchar* data = GetData();
    data += sizeof(uint);
    const int ret = CBasics::SwapBytes(*reinterpret_cast< const uint* >(data));
    return ret;
#else
    return CBasics::SwapBytes(static_cast< const uint* >(mData)[1]);
#endif
  }

  uint GetVertexDesc() const {
#if RSTL_CORRUPTION
    const uchar* data = GetData();
    data += GetTextureCount() * sizeof(uint);
    data += sizeof(uint) + sizeof(uint);
    return CBasics::SwapBytes(*reinterpret_cast< const uint* >(data));
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const uchar* data = GetData();
    data += (GetTextureCount() * sizeof(uint));
    data += sizeof(uint) + sizeof(uint);
    return CBasics::SwapBytes(*reinterpret_cast< const uint* >(data));
#else
    const uint* words = static_cast< const uint* >(mData);
    return CBasics::SwapBytes(words[GetTextureCount() + 2]);
#endif
#endif
  }

  // Guessed name. Echoes material masks follow the compressed vertex descriptor.
#if RSTL_CORRUPTION
  u64 GetMaterialMask() const {
    return *reinterpret_cast< const u64* >(static_cast< const uint* >(mData) + GetTextureCount() +
                                           3);
  }
#else
  uint GetVertexDescLwzx() const {
    return CBasics::SwapBytes(static_cast< const uint* >(mData)[GetTextureCount() + 2]);
  }
#if !RSTL_PRIME1
  // Guessed name. Echoes material masks follow the compressed vertex descriptor.
  u64 GetMaterialMask() const {
#ifdef TARGET_PC
    const uint* words = static_cast< const uint* >(mData) + GetTextureCount() + 3;
    return (static_cast< u64 >(BE_SWAP(words[0])) << 32) | BE_SWAP(words[1]);
#else
    return *reinterpret_cast< const u64* >(static_cast< const uint* >(mData) + GetTextureCount() + 3);
#endif
  }
#endif
#endif

  void SetCurrent(const CModelFlags& flags, const CCubeSurface& surface,
                  const CCubeModel& model) const;
  void SetCurrentBlack() const;

  uint GetCompressedBlend() const;

  static const CVector3f& GetViewingReflection() { return sViewingFrom; }

#if !RSTL_CORRUPTION
private:
  static void SetupBlendMode(uint blendFactors, const CModelFlags& flags, bool alphaTest);
#endif
  // Guessed names; tracks the stages whose texture swap must be restored.
#if RSTL_CORRUPTION
  struct SMaterialTevSwapState {
    // Guessed name
    void AddStage(int stage) {
      if (!(mMask & (1 << stage))) {
        mMask |= 1 << stage;
        mStages.push_back(stage);
      }
    }

    uint mMask;
    rstl::reserved_vector< int, 16 > mStages;
  };

private:
  // Guessed names; native pass construction and consumers establish this 0x1c-byte state.
  struct SMaterialPassState {
    SMaterialPassState()
    : mTevCount(0)
    , mTexCoordCount(0)
    , mTexMtx(GX_TEXMTX0)
    , mPostTexMtx(GX_PTTEXMTX0)
    , x10_(GX_TEXMAP_NULL)
    , mKonstCount(0)
    , x18_(false)
    , mUsesRasterColor(false)
    , x1a_(false) {}

    uint mTevCount;
    int mTexCoordCount;
    GXTexMtx mTexMtx;
    GXPTTexMtx mPostTexMtx;
    GXTexMapID x10_;
    uint mKonstCount;
    bool x18_;
    bool mUsesRasterColor;
    bool x1a_;
  };

  typedef char SMaterialPassStateSizeCheck[sizeof(SMaterialPassState) == 0x1c ? 1 : -1];
  typedef char SMaterialTevSwapStateSizeCheck[sizeof(SMaterialTevSwapState) == 0x48 ? 1 : -1];

  // Guessed name
  static void ApplyStageCountPalette(const SMaterialPassState& state);

  // Guessed name
  static void ApplyAdditiveStageCountVisualization(const SMaterialPassState& state);

  // Guessed name
  static void ConfigureDepthModeFromModelFlags(const CModelFlags& flags, GXBool compare,
                                               GXCompare func, GXBool update);

  // Guessed name
  static void FinalizePassStageCounts(const SMaterialPassState& state);

  // Guessed name
  static void ResetTrackedTevSwapModes(SMaterialTevSwapState& state);

  // Guessed name
  static void ConfigureTrackedTextureSwap(GXTevStageID stage, uint flags);

  // Guessed name
  static void ConfigurePositionTexCoordGenerator(SMaterialPassState& state, GXPTTexMtx postTexMtx);

  // Guessed name
  static void ConfigureMaterialPassTextureStage(SMaterialPassState& state, int materialIndex,
                                                int passIndex,
                                                const CMaterialDataFactory::SMaterial& material,
                                                const CMaterialTextureState& textures,
                                                GXChannelID channel);

  // Guessed name
  static void AppendCLRTextureColorStage(SMaterialPassState& state, bool useTextureAlpha,
                                         uchar opacity, bool useRegisterColor,
                                         const CMaterialDataFactory::SMaterial& material,
                                         int materialIndex, const CMaterialTextureState& textures);

  // Guessed name
  static void AppendCLRTextureRedAlphaStage(SMaterialPassState& state,
                                            const CMaterialDataFactory::SMaterial& material,
                                            int materialIndex,
                                            const CMaterialTextureState& textures);

  // Guessed name
  static bool AppendTRANTextureAlphaMultiplyStage(SMaterialPassState& state,
                                                  const CMaterialDataFactory::SMaterial& material,
                                                  int materialIndex,
                                                  const CMaterialTextureState& textures);

  // Guessed name
  static int AllocateOptionalINCAKonstColor(SMaterialPassState& state,
                                            const CMaterialDataFactory::SMaterial& material,
                                            const CModelFlags& flags);

  // Guessed name
  static bool AppendBLOITextureAlphaAddStage(SMaterialPassState& state,
                                             const CMaterialDataFactory::SMaterial& material,
                                             int materialIndex,
                                             const CMaterialTextureState& textures);

  // Guessed name
  static bool AppendBLOLTextureAlphaMultiplyStage(SMaterialPassState& state, bool enabled,
                                                  const CMaterialDataFactory::SMaterial& material,
                                                  int materialIndex,
                                                  const CMaterialTextureState& textures);

  // Guessed name
  static bool AppendSelectedKonstAlphaAdditionStage(SMaterialPassState& state, uchar alpha);

  // Guessed name
  static void AppendColorAdditionStage(SMaterialPassState& state, CColor color);

  // Guessed name
  static void AppendColorModulationStage(SMaterialPassState& state, CColor color);

  // Guessed name
  static void AppendThresholdAlphaColorAndAlphaModulationStage(SMaterialPassState& state,
                                                               CColor color);

  // Guessed name
  static void AppendRasterOrRegisterColorStage(SMaterialPassState& state, bool useConstantAlpha,
                                               uchar opacity, bool useRegisterColor,
                                               const CMaterialDataFactory::SMaterial& material);

  // Guessed name
  static bool AppendINCATextureColorAdditionStage(SMaterialPassState& state,
                                                  const CMaterialDataFactory::SMaterial& material,
                                                  int materialIndex, const CModelFlags& flags,
                                                  const CMaterialTextureState& textures);

  // Guessed name
  static bool AppendINCAColorAndAlphaAdditionStages(SMaterialPassState& state,
                                                    const CMaterialDataFactory::SMaterial& material,
                                                    int materialIndex, const CModelFlags& flags,
                                                    const CMaterialTextureState& textures);

  // Guessed name
  static bool AppendRFLDTextureColorStages(SMaterialPassState& state, GXTevColorArg baseColor,
                                           bool useRegisterColor,
                                           const CMaterialDataFactory::SMaterial& material,
                                           int materialIndex,
                                           const CMaterialTextureState& textures);

  // Guessed name
  static bool AppendBLODAlphaStages(SMaterialPassState& state, bool useRegisterColor,
                                    const CMaterialDataFactory::SMaterial& material,
                                    int materialIndex, const CMaterialTextureState& textures);

  // Guessed name
  static bool AppendMaterialColorRegister0Stage(SMaterialPassState& state, bool useConstantAlpha,
                                                const CMaterialDataFactory::SMaterial& material,
                                                int materialIndex,
                                                const CMaterialTextureState& textures);

  // Guessed name
  static bool AppendTRANTextureAlphaMultiplyIntoRegister2Stage(
      SMaterialPassState& state, const CMaterialDataFactory::SMaterial& material, int materialIndex,
      const CMaterialTextureState& textures);

  // Guessed name
  static bool
  AppendRFLDTextureRedAlphaMultiplyStage(SMaterialPassState& state,
                                         const CMaterialDataFactory::SMaterial& material,
                                         int materialIndex, const CMaterialTextureState& textures);

  // Guessed name
  static bool AppendRFLDTextureRedAlphaMultiplyIntoRegister2Stage(
      SMaterialPassState& state, const CMaterialDataFactory::SMaterial& material, int materialIndex,
      const CMaterialTextureState& textures);

  // Guessed name
  static void AppendThresholdAlphaColorModulationStage(SMaterialPassState& state, CColor color);

  // Guessed name
  static void AppendPremultipliedColorModulationStage(SMaterialPassState& state, CColor color);

  // Guessed name
  static bool AppendBLOLAndBLODAlphaStages(SMaterialPassState& state,
                                           const CMaterialDataFactory::SMaterial& material,
                                           int materialIndex,
                                           const CMaterialTextureState& textures);

  // Guessed name
  static void AppendINCAModulatedTextureStage(SMaterialPassState& state,
                                              const CMaterialDataFactory::SMaterial& material,
                                              int materialIndex, const CModelFlags& flags,
                                              const CMaterialTextureState& textures);

  // Guessed name
  static bool AppendInitialINCAColorAndAlphaStages(SMaterialPassState& state,
                                                   const CMaterialDataFactory::SMaterial& material,
                                                   int materialIndex, const CModelFlags& flags,
                                                   const CMaterialTextureState& textures);

  // Guessed name
  static void AppendTextureOrConstantAlphaStages(bool multiplyPrevious, SMaterialPassState& state,
                                                 uchar opacity,
                                                 const CMaterialDataFactory::SMaterial& material,
                                                 int materialIndex,
                                                 const CMaterialTextureState& textures);

  // Guessed name
  static int DrawWhiteZeroAlphaBasePass(CCubeSurface surface, const CModelFlags& flags);

  // Guessed name
  static int DrawConstantColorZeroAlphaBasePass(CCubeSurface surface, const CModelFlags& flags,
                                                CColor color);

  // Guessed name
  static int DrawOpaqueMaterialPass(const CMaterialDataFactory::SMaterial& material,
                                    CCubeSurface surface, int index, const CModelFlags& flags,
                                    CMaterialTextureState& textures);

  // Guessed name
  static int DrawAdditiveINCAMaterialPass(const CMaterialDataFactory::SMaterial& material,
                                          CCubeSurface surface, int index, const CModelFlags& flags,
                                          CMaterialTextureState& textures);

  // Guessed name
  static int DrawAdditiveMaterialPass(const CMaterialDataFactory::SMaterial& material,
                                      CCubeSurface surface, int index, const CModelFlags& flags,
                                      CMaterialTextureState& textures);

  // Guessed name
  static int DrawAlphaCutoff192BlendedMaterialPass(const CMaterialDataFactory::SMaterial& material,
                                                   CCubeSurface surface, int index,
                                                   const CModelFlags& flags,
                                                   CMaterialTextureState& textures);

  // Guessed name
  static int DrawAlphaThresholdMaterialPass(const CMaterialDataFactory::SMaterial& material,
                                            CCubeSurface surface, int index,
                                            const CModelFlags& flags,
                                            CMaterialTextureState& textures);

  // Guessed name
  static int DrawAlphaBlendedMaterialPassWithoutAlphaUpdate(
      const CMaterialDataFactory::SMaterial& material, CCubeSurface surface, int index,
      uchar opacity, const CModelFlags& flags, CMaterialTextureState& textures);

  // Guessed name
  static int DrawMaterialWithINCAAdditiveOverlay(const CMaterialDataFactory::SMaterial& material,
                                                 CCubeSurface surface, int index, uchar opacity,
                                                 const CModelFlags& flags,
                                                 CMaterialTextureState& textures);

  // Guessed name
  static int DrawAlphaBlendedMaterialWithAuxiliaryPasses(
      const CMaterialDataFactory::SMaterial& material, CCubeSurface surface, int index,
      uchar opacity, const CModelFlags& flags, CMaterialTextureState& textures);

  // Guessed name
  static int DrawMaterialWithOneOneINCAOverlay(const CMaterialDataFactory::SMaterial& material,
                                               CCubeSurface surface, int index, uchar opacity,
                                               const CModelFlags& flags,
                                               CMaterialTextureState& textures);

  // Guessed names; shared state for parsed-material rendering.
  static bool sMaterialCacheEnabled;
  static int sLastMaterialIndex;
  static int sLastParsedMaterialUnique;
  static const CCubeModel* sCachedMaterialModel;
  static bool sMaterialStateUnknown;
  static bool sResetAlphaCompare;
  static rstl::optional_object< CGraphics::SAlphaContextState > sSavedAlphaContext;
  static int sMaterialStageCount;
  typedef void (*TMaterialPassFinalizer)(const SMaterialPassState& state);
  static TMaterialPassFinalizer sMaterialPassFinalizer;
  static int sGuiColorKRegister;
  static CColor sTevRegister1Color;
  static CColor sLastMaterialRegisterColor;
  static const GXTevKAlphaSel sQuantizedAlphaSelectors[];

  static void SetupBlendMode(uint blendFactors, const CModelFlags& flags, bool alphaTest);
  static uint HandleReflection(GXTexMapID indTexSlot, int indMtxScaleExp, uint tevCount,
                               uint texCount, uint tcgCount, uint kColorCount);
#else
#if RSTL_PRIME1
  static uint HandleReflection(bool usesTevReg2, GXTexMapID indTexSlot, int indMtxScaleExp,
                               uint tevCount, uint texCount, uint tcgCount, uint finalKColorCount,
                               uint& finalCCFlags, uint& finalACFlags);
#else
  static uint HandleReflection(GXTexMapID indTexSlot, int indMtxScaleExp, uint tevCount,
                               uint texCount, uint tcgCount, uint kColorCount);
#endif
#endif

  static const CCubeModel* sLastModelCached;
  static const CCubeModel* sRenderingModel;
  static CVector3f sViewingFrom;
  const void* mData;
};

#endif // _CCUBEMATERIAL
