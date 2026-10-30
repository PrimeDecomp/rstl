#ifndef _CCUBEMODEL
#define _CCUBEMODEL
#include "rstl/RstlVersions.h"

#include "types.h"
#include "Kyoto/Graphics/CCubeMaterial.hpp"

#include "Kyoto/Graphics/CCubeSurface.hpp"

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/vector.hpp"

class CStopwatch;

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
class IObjectStore;
#else
class CCubeSurface;
class CModelFlags;
#endif

class CTexture;
class CTransform4f;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
class CCubeSurface;
#else
class CVector3f;
class IObjectStore;
#endif

enum ESurfaceSelection {
  kSS_Unsorted,
  kSS_Sorted,
  kSS_All,
};

class CCubeModel {
public:
  class ModelInstance {
  public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    ModelInstance(rstl::vector< void* >& surfaces, const void* materialData, const void* positions,
                  const void* normals, const void* colors, const void* uvs,
#else
    ModelInstance(rstl::vector< CCubeSurface >& surfaces, const void* materialData,
                  const void* positions, const void* normals, const void* colors, const void* uvs,
#endif

                  const void* packedTexCoords)
    : mSurfacePtrs(surfaces)
    , mMaterialData(materialData)
    , mPositions(positions)
    , mNormals(normals)
    , mColors(colors)
    , mTexCoords(uvs)
    , mPackedTexCoords(packedTexCoords) {}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    rstl::vector< void* >& Surfaces() { return mSurfacePtrs; }
    const rstl::vector< void* >& GetSurfaces() const { return mSurfacePtrs; }
#else
    rstl::vector< CCubeSurface >& Surfaces() { return mSurfacePtrs; }
    const rstl::vector< CCubeSurface >& Surfaces() const { return mSurfacePtrs; }
#endif

    const void* GetMaterialPointer() const { return mMaterialData; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    void SetMaterialPointer(const void* mat) { mMaterialData = mat; }
#else
    void SetMaterialPointer(const void* data) { mMaterialData = data; }
#endif

    const void* GetVertexPointer() const { return mPositions; }
    const void* GetNormalPointer() const { return mNormals; }
    const void* GetColorPointer() const { return mColors; }
    const void* GetTCPointer() const { return mTexCoords; }
    const void* GetPackedTCPointer() const { return mPackedTexCoords; }

  private:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    rstl::vector< void* >& mSurfacePtrs;
#else
    rstl::vector< CCubeSurface >& mSurfacePtrs;
#endif

    const void* mMaterialData;
    const void* mPositions;
    const void* mNormals;
    const void* mColors;
    const void* mTexCoords;
    const void* mPackedTexCoords;
  };
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CCubeModel(rstl::vector< void* >* surfaces, rstl::vector< TCachedToken< CTexture > >* textures,
             const void* materialData, const void* positions, const void* normals,
             const void* colors, const void* uvs, const void* compressedUvs, const CAABox& bounds,
             uchar visorFlags, bool texturesLoaded, uint idx);
#else
  // Echoes method names below are inferred from Prime and call sites.
  CCubeModel(rstl::vector< CCubeSurface >* surfaces,
             rstl::vector< TCachedToken< CTexture > >* textures, const void* materialData,
             const void* positions, const void* normals, const void* colors, const void* uvs,
             const void* compressedUvs, const CAABox& bounds, uchar visorFlags, bool texturesLoaded,
             uint idx);
#endif

  static void SetRenderModelBlack(bool v);
  static void SetModelWireframe(bool v);
#if RSTL_PRIME1
  void UnlockTextures() const;
  void RemapMaterialData(const void* data, rstl::vector< TCachedToken< CTexture > >* texture);
  void DrawNormal(const float* positions, const float* normals, ESurfaceSelection which) const;
#endif

  static void DisableShadowMaps();
  static void EnableShadowMaps(const CTexture*, const CTransform4f&, unsigned char, unsigned char);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static void SetNewPlayerPositionAndTime(const CVector3f&, const CStopwatch&);
#else
  static void SetNewPlayerPositionAndTime(const CVector3f& pos, const CStopwatch& stopwatch);
#endif

  static void SetDrawingOccluders(bool);
  static void MakeTexturesFromMats(const void* data,
                                   rstl::vector< TCachedToken< CTexture > >& textures,
                                   IObjectStore& store, bool cache);

#if !RSTL_PRIME1
  void UnlockTextures() const;
  void RemapMaterialData(const void* data, rstl::vector< TCachedToken< CTexture > >* textures);
  bool TryLockTextures() const;
  void DrawNormal(ESurfaceSelection which) const;
  void DrawFlat(int flags) const;
  void Draw(const CModelFlags& flags) const;
  void Draw(u64 mask, const CModelFlags& flags) const;
  void DrawNormal(const CModelFlags& flags) const;
  void DrawAlpha(const CModelFlags& flags) const;
  void SetArraysCurrent() const;
  void SetStaticArraysCurrent() const;
  void DrawSurface(const CCubeSurface& surface, const CModelFlags& flags) const;
  void DrawSurfaceWireframe(const CCubeSurface& surface) const;
  void DrawSurfaceFlat(const CCubeSurface& surface) const; // Guessed name.
  void DrawSurfaces(const CModelFlags& flags) const;
  void DrawSurfaces(u64 mask, const CModelFlags& flags) const;
  void DrawNormalSurfaces(const CModelFlags& flags) const;
  void DrawAlphaSurfaces(const CModelFlags& flags) const;

  void SetUsingPackedLightmaps(bool v) const;
  static bool IsUsingPackedLightmaps() { return sUsingPackedLightmaps; }
#endif

  const ModelInstance& GetModelInstance() const { return mInstance; }
#if RSTL_PRIME1
  bool AreTexturesLoaded() const { return !mLoadTextures; }
#endif

  const void* GetPositions() const { return mInstance.GetVertexPointer(); }
  const void* GetNormals() const { return mInstance.GetNormalPointer(); }
#if !RSTL_PRIME1
  bool AreTexturesLoaded() const { return !mLoadTextures; }
  uchar GetModelFlags() const { return mVisorFlags; }
  int GetModelIndex() const { return mIdx; }
#endif

#if !RSTL_PRIME1
  const rstl::vector< TCachedToken< CTexture > >& GetTextures() const { return *mTextures; }
  const rstl::vector< TCachedToken< CTexture > >* GetTexturesPtr() const { return mTextures; }
#endif

  const CAABox& GetBoundingBox() const { return mBounds; }
  const CCubeSurface& GetNormalSurfaces() const { return mFirstUnsorted; }
  const CCubeSurface& GetAlphaSurfaces() const { return mFirstSorted; }
#if RSTL_PRIME1
  bool GetShouldDrawWorldFlag() const { return mVisible; }
  void SetShouldDrawWorldFlag(bool shouldDraw) { mVisible = shouldDraw; }
  uchar GetModelFlags() const { return mVisorFlags; }
  int GetModelIndex() const { return mIdx; } // TODO: name
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CCubeMaterial GetMaterialByIndex(const int idx) const;
  void SetStaticArraysCurrent() const;
  void SetArraysCurrent() const;
  void SetSkinningArraysCurrent(const float* positions, const float* normals) const;
  void SetUsingPackedLightmaps(const bool use) const;
  static bool IsUsingPackedLightmaps() { return sUsingPackedLightmaps; }
  void DrawSurface(const CCubeSurface& surface, const CModelFlags& modelFlags) const;
  void DrawSurfaceWireframe(const CCubeSurface& surface) const;
  void DrawFlat(const float* positions, const float* normals, ESurfaceSelection which) const;
  bool TryLockTextures() const;
  void Draw(const CModelFlags& flags) const;
  void Draw(const float* positions, const float* normals, const CModelFlags& flags) const;
  void DrawNormal(const CModelFlags& flags) const;
  void DrawAlpha(const CModelFlags& flags) const;
  void DrawSurfaces(const CModelFlags& flags) const;
  void DrawNormalSurfaces(const CModelFlags& flags) const;
  void DrawAlphaSurfaces(const CModelFlags& flags) const;
#else
  // Guessed name; the target passes a surface handle by value for material lookup.
  CCubeMaterial GetMaterial(CCubeSurface surface) const {
    return GetMaterialByIndex(surface.GetMaterialIndex());
  }
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::vector< TCachedToken< CTexture > >& GetTextures() const { return *mTextures; };
#else
  CCubeMaterial GetMaterialByIndex(int index) const {
    const uint offset = index != 0 ? CBasics::SwapBytes(mMaterialOffsets[index - 1]) : 0;
    return CCubeMaterial(mMaterialData + offset);
  }
#endif

private:
#if !RSTL_PRIME1
  void CacheMaterialData(); // Guessed name; caches the offsets and material payload.
#endif

  ModelInstance mInstance;
  rstl::vector< TCachedToken< CTexture > >* mTextures;
  CAABox mBounds;
  CCubeSurface mFirstUnsorted;
  CCubeSurface mFirstSorted;
  mutable bool mLoadTextures : 1;
  bool mVisible : 1;
#if !RSTL_PRIME1
  uchar x40_ : 6;
#endif

  uchar mVisorFlags;
  int mIdx;
#if !RSTL_PRIME1
  const uint* mMaterialOffsets;
  const uchar* mMaterialData;
#endif

  static bool sUsingPackedLightmaps;
};
#if !RSTL_PRIME1
CHECK_SIZEOF(CCubeModel, 0x50)
#endif

#endif // _CCUBEMODEL
