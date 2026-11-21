#ifndef _CCUBEMODEL
#define _CCUBEMODEL
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#endif

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#endif
#include "rstl/types.h"
#if !RSTL_CORRUPTION
#include "Kyoto/Graphics/CCubeSurface.hpp"
#include "Kyoto/Graphics/CCubeMaterial.hpp"
#include "Kyoto/Graphics/ModelTypes.hpp"
#endif

#if RSTL_CORRUPTION


class CSimplePool;

#include "Kyoto/Graphics/CCubeMaterial.hpp"
#include "Kyoto/Graphics/CCubeSurface.hpp"
#include "Kyoto/Graphics/CMaterialTextureState.hpp"
#endif
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/TToken.hpp"
#if !RSTL_CORRUPTION
#include "rstl/vector.hpp"
#endif

#if RSTL_CORRUPTION
#include "rstl/vector.hpp"
#else
class CStopwatch;
#endif

class CCubeSurface;
class CModelFlags;
#if RSTL_CORRUPTION
class CStopwatch;
#endif
class CTexture;
class CTransform4f;
class CVector3f;
class IObjectStore;

enum ESurfaceSelection {
  kSS_Unsorted,
  kSS_Sorted,
  kSS_All,
};

class CCubeModel {
public:
#if !RSTL_CORRUPTION
#if RSTL_PRIME1
  typedef void* SurfaceListType;
#else
  typedef CCubeSurface SurfaceListType;
#endif
  typedef rstl::vector< SurfaceListType > SurfaceList;

#endif
  class ModelInstance {
  public:
#if RSTL_CORRUPTION
    ModelInstance(rstl::vector< void* >& surfaces, const void* materialData, const void* positions,
                  const void* normals, const void* colors, const void* uvs)
    : mSurfacePtrs(surfaces)
    , mMaterialData(materialData)
    , mPositions(positions)
    , mNormals(normals)
    , mColors(colors)
    , mTexCoords(uvs) {}

    rstl::vector< void* >& Surfaces() { return mSurfacePtrs; }
    const rstl::vector< void* >& Surfaces() const { return mSurfacePtrs; }
    const void* GetMaterialPointer() const { return mMaterialData; }
    void SetMaterialPointer(const void* data) { mMaterialData = data; }
    const void* GetVertexPointer() const { return mPositions; }
    const void* GetNormalPointer() const { return mNormals; }
    const void* GetColorPointer() const { return mColors; }
    const void* GetTCPointer() const { return mTexCoords; }
#else
#if defined(TARGET_PC)
    ModelInstance(TModelData materialData, const SModelArrays& arrays)
    : mMaterialData(materialData), mArrays(arrays) {}
#else
    ModelInstance(SurfaceList& surfaces, TModelData materialData, const void* positions,
                  const void* normals, const void* colors, const void* uvs,
                  const void* packedTexCoords)
    : mSurfacePtrs(surfaces)
    , mMaterialData(materialData)
    , mPositions(positions)
    , mNormals(normals)
    , mColors(colors)
    , mTexCoords(uvs)
    , mPackedTexCoords(packedTexCoords) {}

    SurfaceList& Surfaces() { return mSurfacePtrs; }
    const SurfaceList& Surfaces() const { return mSurfacePtrs; }
    const SurfaceList& GetSurfaces() const { return mSurfacePtrs; }
#endif

    const void* GetMaterialPointer() const { return GetModelDataPointer(mMaterialData); }
    TModelData GetMaterialData() const { return mMaterialData; }
    void SetMaterialPointer(TModelData mat) { mMaterialData = mat; }
#if defined(TARGET_PC)
    const SModelArrays& GetArrays() const { return mArrays; }
    const void* GetVertexPointer() const { return mArrays.positions.data(); }
    const void* GetNormalPointer() const { return mArrays.normals.data(); }
    const void* GetColorPointer() const { return mArrays.colors.data(); }
    const void* GetTCPointer() const { return mArrays.texCoords.data(); }
    const void* GetPackedTCPointer() const { return mArrays.packedTexCoords.data(); }
#else
    const void* GetVertexPointer() const { return mPositions; }
    const void* GetNormalPointer() const { return mNormals; }
    const void* GetColorPointer() const { return mColors; }
    const void* GetTCPointer() const { return mTexCoords; }
    const void* GetPackedTCPointer() const { return mPackedTexCoords; }
#endif
#endif

  private:
#if RSTL_CORRUPTION
    rstl::vector< void* >& mSurfacePtrs;
    const void* mMaterialData;
    const void* mPositions;
    const void* mNormals;
    const void* mColors;
    const void* mTexCoords;
#else
#if !defined(TARGET_PC)
    SurfaceList& mSurfacePtrs;
#endif
    TModelData mMaterialData;
#if defined(TARGET_PC)
    SModelArrays mArrays;
#else
    const void* mPositions;
    const void* mNormals;
    const void* mColors;
    const void* mTexCoords;
    const void* mPackedTexCoords;
#endif
#endif
  };

#if RSTL_CORRUPTION
  CCubeModel(CSimplePool* pool, rstl::vector< void* >* surfaces, const void* materialData,
             const void* positions, const void* normals, const void* colors, const void* uvs,
             const CAABox& bounds, uchar visorFlags, bool deferTextureLoad, uint idx);
#else
#if defined(TARGET_PC)
  CCubeModel(std::span< const TModelData > surfaces,
             rstl::vector< TCachedToken< CTexture > >* textures, TModelData materialData,
             const SModelArrays& arrays, const CAABox& bounds, uchar visorFlags,
             bool texturesLoaded, uint idx);
#else
  CCubeModel(SurfaceList* surfaces, rstl::vector< TCachedToken< CTexture > >* textures,
             TModelData materialData, const void* positions, const void* normals,
             const void* colors, const void* uvs, const void* compressedUvs, const CAABox& bounds,
             uchar visorFlags, bool texturesLoaded, uint idx);
#endif
#endif

  static void SetRenderModelBlack(bool v);
#if RSTL_CORRUPTION
  static void SetModelWireframe(bool v) { sDrawingWireframe = v; }
  static bool IsDrawingWireframe() { return sDrawingWireframe; } // Guessed name
#else
  static void SetModelWireframe(bool v);
#endif
  static void DisableShadowMaps();
  static void EnableShadowMaps(const CTexture*, const CTransform4f&, unsigned char, unsigned char);
  static void SetNewPlayerPositionAndTime(const CVector3f& pos, const CStopwatch& stopwatch);
  static void SetDrawingOccluders(bool);
#if !RSTL_CORRUPTION
  static void MakeTexturesFromMats(TModelData data,
                                   rstl::vector< TCachedToken< CTexture > >& textures,
                                   IObjectStore& store, bool cache);
#endif

#if !RSTL_CORRUPTION
  void UnlockTextures() const;
#endif
  bool TryLockTextures() const;
#if RSTL_CORRUPTION
  bool AreRequestedTexturesLoaded() const; // Guessed name
  void DrawNormal(ESurfaceSelection which) const;
  void DrawFlat(int flags) const;
#else
  void RemapMaterialData(TModelData data, rstl::vector< TCachedToken< CTexture > >* textures);
  void SetStaticArraysCurrent() const;
  void SetArraysCurrent() const;
  void SetSkinningArraysCurrent(TModelPositions positions, TModelNormals normals) const;
  void SetUsingPackedLightmaps(bool v) const;
  static bool IsUsingPackedLightmaps() { return sUsingPackedLightmaps; }

#endif
  void Draw(const CModelFlags& flags) const;
  void Draw(u64 mask, const CModelFlags& flags) const;
#if !RSTL_CORRUPTION
  void Draw(TModelPositions positions, TModelNormals normals, const CModelFlags& flags) const;
#endif
  void DrawNormal(const CModelFlags& flags) const;
#if !RSTL_CORRUPTION
  void DrawNormal(ESurfaceSelection which) const;
  void DrawNormal(TModelPositions positions, TModelNormals normals, ESurfaceSelection which) const;
#endif
  void DrawAlpha(const CModelFlags& flags) const;
#if RSTL_CORRUPTION
  void SetArraysCurrent() const;
  void SetStaticArraysCurrent() const;
  void DrawSurface(CCubeSurface surface, const CModelFlags& flags) const;
  void DrawSurfaceWireframe(CCubeSurface surface) const;
#else
  void DrawFlat(int flags) const;
  void DrawFlat(TModelPositions positions, TModelNormals normals, ESurfaceSelection which) const;
  void DrawSurface(const CCubeSurface& surface, const CModelFlags& flags) const;
  void DrawSurfaceWireframe(const CCubeSurface& surface) const;
#endif
  void DrawSurfaceFlat(const CCubeSurface& surface) const; // Guessed name.
  void DrawSurfaces(const CModelFlags& flags) const;
  void DrawSurfaces(u64 mask, const CModelFlags& flags) const;
  void DrawNormalSurfaces(const CModelFlags& flags) const;
  void DrawAlphaSurfaces(const CModelFlags& flags) const;

#if RSTL_CORRUPTION
  void SetUsingPackedLightmaps(bool v) const;
  static bool IsUsingPackedLightmaps() { return sUsingPackedLightmaps; }

#endif
  const ModelInstance& GetModelInstance() const { return mInstance; }
  const void* GetPositions() const { return mInstance.GetVertexPointer(); }
  const void* GetNormals() const { return mInstance.GetNormalPointer(); }
#if RSTL_CORRUPTION
  bool ShouldDeferTextureLoad() const { return !mTexturesReady; } // Guessed name
#else
  bool AreTexturesLoaded() const { return !mLoadTextures; }
#endif
  uchar GetModelFlags() const { return mVisorFlags; }
  int GetModelIndex() const { return mIdx; }
#if RSTL_CORRUPTION

  const rstl::vector< TCachedToken< CTexture > >& GetTextures() const {
    return mTextureState.GetTextures();
  }
  CMaterialTextureState& GetTextureState() const { return mTextureState; } // Guessed name
#endif
  const CAABox& GetBoundingBox() const { return mBounds; }
  const CCubeSurface& GetNormalSurfaces() const { return mFirstUnsorted; }
  const CCubeSurface& GetAlphaSurfaces() const { return mFirstSorted; }
#if !RSTL_CORRUPTION
  bool GetShouldDrawWorldFlag() const { return mVisible; }
  void SetShouldDrawWorldFlag(bool shouldDraw) { mVisible = shouldDraw; }
  const rstl::vector< TCachedToken< CTexture > >* GetTexturesPtr() const { return mTextures; }
#endif

#if !RSTL_CORRUPTION
  CCubeMaterial GetMaterialByIndex(int index) const;
  const rstl::vector< TCachedToken< CTexture > >& GetTextures() const { return *mTextures; }
#endif
  // Guessed name; the target passes a surface handle by value for material lookup.
#if RSTL_CORRUPTION
  const CMaterialDataFactory::SMaterial& GetMaterial(CCubeSurface surface) const {
#else
  CCubeMaterial GetMaterial(CCubeSurface surface) const {
#endif
    return GetMaterialByIndex(surface.GetMaterialIndex());
  }

#if RSTL_CORRUPTION
  const CMaterialDataFactory::SMaterial& GetMaterialByIndex(int index) const {
    return mMaterials[index];
  }
#else
#if defined(TARGET_PC)
  int GetSurfaceCount() const { return mSurfaces.size(); }
  CCubeSurface GetSurface(int index) const { return CCubeSurface(&mSurfaces[index]); }
  size_t GetSurfaceStorageSize() const {
    return mSurfaces.capacity() * sizeof(CCubeSurface::SSurfaceData);
  }
#else
  int GetSurfaceCount() const { return mInstance.Surfaces().size(); }
  CCubeSurface GetSurface(int index) const { return CCubeSurface(mInstance.Surfaces()[index]); }
#endif
#endif

private:
#if !RSTL_CORRUPTION
  void CacheMaterialData(); // Guessed name; caches the offsets and material payload.

#endif
  ModelInstance mInstance;
#if RSTL_CORRUPTION
  rstl::vector< CMaterialDataFactory::SMaterial > mMaterials;
  mutable CMaterialTextureState mTextureState;
#else
  rstl::vector< TCachedToken< CTexture > >* mTextures;
#endif
  CAABox mBounds;
  CCubeSurface mFirstUnsorted;
  CCubeSurface mFirstSorted;
#if RSTL_CORRUPTION
  mutable bool mTexturesReady : 1; // Guessed name
#else
  mutable bool mLoadTextures : 1;
#endif
  bool mVisible : 1;
#if RSTL_CORRUPTION
  uchar x70_ : 6;
#else
#if !RSTL_PRIME1
  uchar x40_ : 6;
#endif

#endif
  uchar mVisorFlags;
  int mIdx;
#if !RSTL_CORRUPTION
#if !RSTL_PRIME1
  const uint* mMaterialOffsets;
  const uchar* mMaterialData;
#endif
#if defined(TARGET_PC)
  rstl::vector< CCubeSurface::SSurfaceData > mSurfaces;
#endif
#endif

  static bool sUsingPackedLightmaps;
#if RSTL_CORRUPTION
  static bool sDrawingWireframe;
#endif
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CCubeModel, 0x78)
NESTED_CHECK_SIZEOF(CCubeModel, ModelInstance, 0x18)
#else
#if !RSTL_PRIME1
CHECK_SIZEOF(CCubeModel, 0x50)
#endif
#endif

#if !RSTL_CORRUPTION
#if !RSTL_PRIME1
inline CCubeMaterial CCubeModel::GetMaterialByIndex(int index) const {
  // FIXME: needs a special variant for PC?
  const uint offset = index != 0 ? CBasics::SwapBytes(mMaterialOffsets[index - 1]) : 0;
  return CCubeMaterial(mMaterialData + offset);
}
#endif

#endif
#endif // _CCUBEMODEL
