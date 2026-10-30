#ifndef _CCUBEMATERIAL
#define _CCUBEMATERIAL
#include "rstl/RstlVersions.h"
#include "types.h"

#include "Kyoto/Basics/CBasics.hpp"

#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#if !RSTL_PRIME1
#include "dolphin/gx.h"
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
class CCubeMaterial {
public:
  explicit CCubeMaterial(const void* data) : mData(data) {}
  static void ResetCachedMaterials();
  static void EnsureViewDepStateCached(const CCubeSurface* surface);
  static void EnsureTevsDirect();
  static void KillCachedViewDepState();

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  inline const uchar* GetData() const { return static_cast< const uchar* >(mData); }
#else
  static void UseThermalTevs();
  static void UseNormalTevs();
  static void ResetTransparencyKColor();
  static int GetTransparencyKColor();
  static void ResetExtraTexCoord();
  static uint GetExtraTexCoord();
  static uint GetExtraPostTexMtx();

  const uchar* GetData() const { return static_cast< const uchar* >(mData); }
#endif

  uint GetFlags() const { return CBasics::SwapBytes(*reinterpret_cast< const uint* >(GetData())); }
  bool IsFlagSet(const EStateFlags flag) const { return (GetFlags() & flag) != 0; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void SetCurrent(const CModelFlags& flags, const CCubeSurface& surface,
                  const CCubeModel& mode) const;
  void SetCurrentBlack() const;
  uint GetTextureCount() const {
    const uchar* data = GetData();
    data += sizeof(uint);
    const int ret = CBasics::SwapBytes(*reinterpret_cast< const uint* >(data));
    return ret;
  }
#else
  uint GetTextureCount() const { return CBasics::SwapBytes(static_cast< const uint* >(mData)[1]); }
#endif

  uint GetVertexDesc() const {
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

  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  // TODO: Figure out wtf is going on here
  uint GetVertexDescLwzx() const {
    return CBasics::SwapBytes(static_cast< const uint* >(mData)[GetTextureCount() + 2]);
#else
  // Guessed name. Echoes material masks follow the compressed vertex descriptor.
  u64 GetMaterialMask() const {
#ifdef TARGET_PC
    const uint* words = static_cast< const uint* >(mData) + GetTextureCount() + 3;
    return (static_cast< u64 >(BE_SWAP(words[0])) << 32) | BE_SWAP(words[1]);
#else
    return *reinterpret_cast< const u64* >(static_cast< const uint* >(mData) + GetTextureCount() + 3);
#endif
#endif

  }

#if !RSTL_PRIME1
  void SetCurrent(const CModelFlags& flags, const CCubeSurface& surface,
                  const CCubeModel& model) const;
  void SetCurrentBlack() const;
#endif

  uint GetCompressedBlend() const;

  static const CVector3f& GetViewingReflection() { return sViewingFrom; }

private:
  static void SetupBlendMode(uint blendFactors, const CModelFlags& flags, bool alphaTest);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static uint HandleReflection(bool usesTevReg2, GXTexMapID indTexSlot, int indMtxScaleExp,
                               uint tevCount, uint texCount, uint tcgCount, uint finalKColorCount,
                               uint& finalCCFlags, uint& finalACFlags);
#else
  static uint HandleReflection(GXTexMapID indTexSlot, int indMtxScaleExp, uint tevCount,
                               uint texCount, uint tcgCount, uint kColorCount);
#endif

  static const CCubeModel* sLastModelCached;
  static const CCubeModel* sRenderingModel;
  static CVector3f sViewingFrom;
  const void* mData;
};

#endif // _CCUBEMATERIAL
