#include "rstl/RstlVersions.h"
#include "Kyoto/Graphics/CCubeModel.hpp"
#if !RSTL_CORRUPTION
#include "rstl/macros.hpp"
#endif
#include "Kyoto/Basics/CBasics.hpp"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif

#include "Kyoto/Graphics/CCubeSurface.hpp"
#if !RSTL_CORRUPTION
#include "Kyoto/Graphics/CGX.hpp"
#endif
#include "Kyoto/Graphics/CGX_Impl.hpp" // IWYU pragma: keep

#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#if !RSTL_CORRUPTION
#include "Kyoto/Platform.h"
#include "Kyoto/ByteOrder.hpp"
#endif
#include "dolphin/gx/GXVert.h"

#include "string.h"

#if RSTL_CORRUPTION

// Guessed name; bits 0x200 and 0x8000 reject material flags.
// Guessed name
static uint sSkippedMaterialFlags = 0x100;
bool CCubeModel::sDrawingWireframe = false;
#else
#if RSTL_PRIME1
static bool sDrawingOccluders = false;

#define GET_MATERIAL_FROM_SURFACE(surface) GetMaterialByIndex(surface.GetMaterialIndex())
#else
// Guessed name; bits 0x200 and 0x8000 reject material flags.
static uint sSkippedMaterialFlags = 0x8200;
// Guessed name
#define GET_MATERIAL_FROM_SURFACE(surface) GetMaterial(surface)
#endif

static bool sDrawingWireframe = false;
#endif
bool CCubeModel::sUsingPackedLightmaps = false;

#if RSTL_CORRUPTION
CCubeModel::CCubeModel(CSimplePool* pool, rstl::vector< void* >* surfaces,
                       const void* materialData, const void* positions, const void* normals,
                       const void* colors, const void* uvs, const CAABox& bounds,
                       const uchar visorFlags, const bool deferTextureLoad, const uint idx)
: mInstance(*surfaces, materialData, positions, normals, colors, uvs)
, mMaterials(CMaterialDataFactory::BuildMaterialRecords(materialData))
, mTextureState(mMaterials, pool)
#else
inline uint GetMaterialOffset(const uchar* materialData, const int idx) {
  materialData += (idx * 4);
  return CBasics::SwapBytes(*reinterpret_cast< const uint* >(materialData - 4));
}

#if IS_TARGET_PC
CCubeModel::CCubeModel(std::span< const TModelData > surfaces,
                       rstl::vector< TCachedToken< CTexture > >* textures, TModelData materialData,
                       const SModelArrays& arrays, const CAABox& bounds, const uchar visorFlags,
                       const bool texturesLoaded, const uint idx)
: mInstance(materialData, arrays)
#else
CCubeModel::CCubeModel(SurfaceList* surfaces, rstl::vector< TCachedToken< CTexture > >* textures,
                       const void* materialData, const void* positions, const void* normals,
                       const void* colors, const void* uvs, const void* compressedUvs,
                       const CAABox& bounds, const uchar visorFlags, const bool texturesLoaded,
                       const uint idx)
: mInstance(*surfaces, materialData, positions, normals, colors, uvs, compressedUvs)
#endif
, mTextures(textures)
#endif
, mBounds(bounds)
, mFirstUnsorted(nullptr)
, mFirstSorted(nullptr)
#if RSTL_CORRUPTION
, mTexturesReady(static_cast< uchar >(!deferTextureLoad))
#else
, mLoadTextures(static_cast< uchar >(!texturesLoaded))
#endif
, mVisible(false)
#if RSTL_CORRUPTION
, x70_(0)
#else
#if !RSTL_PRIME1
, x40_(0)
#endif

#endif
, mVisorFlags(visorFlags)
#if RSTL_CORRUPTION
, mIdx(idx) {
  rstl::vector< void* >& surf = mInstance.Surfaces();
  for (rstl::vector< void* >::iterator it = surf.begin(); it != surf.end(); ++it) {
    CCubeSurface::SSurfaceData* data = static_cast< CCubeSurface::SSurfaceData* >(*it);
    data->mParent = this;
  }

  for (int i = surf.size(); i > 0; --i) {
    void*& data = surf[i - 1];
    if (GetMaterial(CCubeSurface(data)).mHeader.mFlags & 0x28) {
      static_cast< CCubeSurface::SSurfaceData* >(data)->mNextSurface = mFirstSorted.mData;
      mFirstSorted.mData = static_cast< CCubeSurface::SSurfaceData* >(data);
    } else {
      static_cast< CCubeSurface::SSurfaceData* >(data)->mNextSurface = mFirstUnsorted.mData;
      mFirstUnsorted.mData = static_cast< CCubeSurface::SSurfaceData* >(data);
    }
  }
#else
, mIdx(idx)
#if !RSTL_PRIME1
, mMaterialOffsets(nullptr)
, mMaterialData(nullptr)
#endif
{
#if !RSTL_PRIME1
  CacheMaterialData();
#endif

#if defined(TARGET_PC)
  const uint materialCount = ValidateModelMaterials(materialData);
  mSurfaces.reserve(surfaces.size());
  for (auto data : surfaces) {
    mSurfaces.push_back(CCubeSurface::ReadData(data, this, materialCount));
  }
  for (int i = mSurfaces.size(); i > 0; --i) {
    auto& data = mSurfaces[i - 1];
    auto& first = GetMaterialByIndex(data.mMaterialIndex).IsFlagSet(kStateFlag_DepthSorting)
                      ? mFirstSorted
                      : mFirstUnsorted;
    data.mNextSurface = first.mRawdata;
    first = CCubeSurface(&data);
  }
#else
  SurfaceList& surf = mInstance.Surfaces();

  for (AUTO(it, surf.begin()); it != surf.end(); ++it) {
    AUTO(data, IF_RSTL_PRIME1_ELSE(static_cast< CCubeSurface::SSurfaceData* >(*it), it->mData));
    data->mParent = this;
  }

  for (int i = surf.size(); i > 0; i--) {
    RSTL_ECHOES_CONST SurfaceListType& surface = surf[i - 1];
#if RSTL_PRIME1
    uint materialIndex = static_cast< CCubeSurface::SSurfaceData* >(surface)->mMaterialIndex;
    if (GetMaterialByIndex(materialIndex).IsFlagSet(kStateFlag_DepthSorting)) {
      static_cast< CCubeSurface::SSurfaceData* >(surface)->mNextSurface = mFirstSorted.mRawdata;
      mFirstSorted.mRawdata = static_cast< uchar* >(surface);
    }
#else
    if (GetMaterial(surface).IsFlagSet(kStateFlag_DepthSorting)) {
      surface.mData->mNextSurface = mFirstSorted.mData;
      mFirstSorted = surface;
    }
#endif
    else {
#if RSTL_PRIME1
      static_cast< CCubeSurface::SSurfaceData* >(surface)->mNextSurface = mFirstUnsorted.mRawdata;
      mFirstUnsorted.mRawdata = static_cast< uchar* >(surface);
#else
      surface.mData->mNextSurface = mFirstUnsorted.mData;
      mFirstUnsorted = surface;
#endif
    }
  }
#endif
}
#endif

#if RSTL_CORRUPTION
  if (mTexturesReady) {
    mTextureState.LockTextures();
    mTextureState.TryCacheTextures();
#else
void CCubeModel::MakeTexturesFromMats(TModelData data,
                                      rstl::vector< TCachedToken< CTexture > >& textures,
                                      IObjectStore& store, const bool cache) {
#if defined(TARGET_PC)
  ValidateModelMaterials(data);
#endif
  const uint* textureIds = static_cast< const uint* >(GetModelDataPointer(data));
  const uint textureCount =
      CBasics::SwapBytes(*static_cast< const int* >(GetModelDataPointer(data)));
  textureIds++;
  textures.reserve(textureCount);

  for (int i = 0; i < textureCount; i++) {
    IF_RSTL_PRIME1_ELSE(textures.push_back, textures.push_back_unsafe)(
        store.GetObj(SObjectTag('TXTR', CBasics::SwapBytes(*textureIds))));
    if (!cache) {
      textures.back().ForceCache();
    }
    ++textureIds;
#endif
  }
}

void CCubeModel::SetStaticArraysCurrent() const {
#if RSTL_CORRUPTION
  CGX::SetArray(GX_VA_CLR0, mInstance.GetColorPointer(), sizeof(CColor));
  const void* texCoords = mInstance.GetTCPointer();
  if (texCoords) {
    for (int i = 0; i <= GX_VA_TEX7 - GX_VA_TEX0; ++i) {
      CGX::SetArray(static_cast< GXAttr >(i + GX_VA_TEX0), texCoords, sizeof(CVector2f));
#else
  CGX_SET_ARRAY(GX_VA_CLR0, mInstance.GetColorPointer(),
                sizeof(CColor), mInstance.GetArrays().colors.size(), false);
  const void* packed = mInstance.GetPackedTCPointer();
  const void* unpacked = mInstance.GetTCPointer();
  if (!packed) {
    if (IS_TARGET_PC && sUsingPackedLightmaps) {
      CCubeMaterial::ResetCachedMaterials();
    }
    sUsingPackedLightmaps = false;
  }

  if (sUsingPackedLightmaps) {
    CGX_SET_ARRAY(
        GX_VA_TEX0, packed,
        sizeof(ushort) * 2, mInstance.GetArrays().packedTexCoords.size(), TARGET_LITTLE_ENDIAN);
  } else {
    CGX_SET_ARRAY(GX_VA_TEX0, unpacked,
                  sizeof(CVector2f), mInstance.GetArrays().texCoords.size(), TARGET_LITTLE_ENDIAN);
  }

  if (unpacked) {
    for (int i = 1; i <= GX_VA_TEX7 - GX_VA_TEX0; ++i) {
      CGX_SET_ARRAY(static_cast< GXAttr >(i + GX_VA_TEX0), unpacked,
                    sizeof(CVector2f), mInstance.GetArrays().texCoords.size(), TARGET_LITTLE_ENDIAN);
#endif
    }
  }

  CCubeMaterial::KillCachedViewDepState();
}

void CCubeModel::SetArraysCurrent() const {
#if RSTL_CORRUPTION
  CGX::SetArray(GX_VA_POS, mInstance.GetVertexPointer(), sizeof(CVector3f));
#else
  CGX_SET_ARRAY(GX_VA_POS, mInstance.GetVertexPointer(),
                sizeof(CVector3f), mInstance.GetArrays().positions.size(), TARGET_LITTLE_ENDIAN);
#endif
  const int stride = (mVisorFlags & 1) ? sizeof(short) * 3 : sizeof(CVector3f);
#if RSTL_CORRUPTION
  CGX::SetArray(GX_VA_NRM, mInstance.GetNormalPointer(), stride);
#else
  CGX_SET_ARRAY(GX_VA_NRM, mInstance.GetNormalPointer(),
                stride, mInstance.GetArrays().normals.size(), TARGET_LITTLE_ENDIAN);
#endif
  SetStaticArraysCurrent();
}

#if RSTL_CORRUPTION
void CCubeModel::DrawSurface(CCubeSurface surface, const CModelFlags& modelFlags) const {
  const CMaterialDataFactory::SMaterial& material = GetMaterial(surface);
  if (material.mHeader.mFlags & sSkippedMaterialFlags) {
#else
void CCubeModel::SetSkinningArraysCurrent(TModelPositions positions, TModelNormals normals) const {
#if defined(TARGET_PC)
  const auto& arrays = mInstance.GetArrays();
  if (positions.empty()) {
    CGX_SET_ARRAY(GX_VA_POS, arrays.positions.data(), sizeof(CVector3f), arrays.positions.size(),
                  TARGET_LITTLE_ENDIAN);
  } else {
    CGX_SET_ARRAY(GX_VA_POS, positions.data(), sizeof(CVector3f), positions.size_bytes(),
                  TARGET_LITTLE_ENDIAN);
  }
  if (normals.empty()) {
    const int stride = (mVisorFlags & 1) ? sizeof(short) * 3 : sizeof(CVector3f);
    CGX_SET_ARRAY(GX_VA_NRM, arrays.normals.data(), stride, arrays.normals.size(),
                  TARGET_LITTLE_ENDIAN);
  } else {
    CGX_SET_ARRAY(GX_VA_NRM, normals.data(), sizeof(CVector3f), normals.size_bytes(),
                  TARGET_LITTLE_ENDIAN);
  }
#else
  CGraphics::sRenderState.SetVtxState(positions, normals,
                                      static_cast< const uint* >(mInstance.GetColorPointer()));
#endif
  SetStaticArraysCurrent();
}

void CCubeModel::SetUsingPackedLightmaps(const bool use) const {
  sUsingPackedLightmaps = use;
  if (sUsingPackedLightmaps) {
    CGX_SET_ARRAY(
        GX_VA_TEX0, mInstance.GetPackedTCPointer(),
        sizeof(ushort) * 2, mInstance.GetArrays().packedTexCoords.size(), TARGET_LITTLE_ENDIAN);
  } else {
    CGX_SET_ARRAY(GX_VA_TEX0, mInstance.GetTCPointer(),
                  sizeof(CVector2f), mInstance.GetArrays().texCoords.size(), TARGET_LITTLE_ENDIAN);
  }
}

#if RSTL_PRIME1
CCubeMaterial CCubeModel::GetMaterialByIndex(const int idx) const {
#if defined(TARGET_PC)
  return CCubeMaterial(GetModelMaterial(mInstance.GetMaterialData(), idx).data());
#else
  uint materialCount = 0;
  uint materialOffset = 0;
  const uchar* materialData =
      static_cast< const uchar* >(mInstance.GetMaterialPointer()) + (mTextures->size() + 1) * 4;
  materialCount = *reinterpret_cast< const uint* >(materialData++);
  materialCount = CBasics::SwapBytes(materialCount);
  materialData++;
  materialData++;
  materialData++;
  if (idx != 0) {
    materialOffset = GetMaterialOffset(materialData, idx);
  }

  materialData += (materialCount * 4);
  materialData += materialOffset;
  return CCubeMaterial(materialData);
#endif
}
#endif

void CCubeModel::DrawSurface(const CCubeSurface& surface, const CModelFlags& modelFlags) const {
  const CCubeMaterial material = GET_MATERIAL_FROM_SURFACE(surface);
  if (IF_RSTL_PRIME1_ELSE(material.IsFlagSet(kStateFlag_ShadowOccluderMesh) && !sDrawingOccluders,
                          material.GetFlags() & sSkippedMaterialFlags)) {
#endif
    return;
  }

#if RSTL_CORRUPTION
  CCubeMaterial::DrawMaterialSurface(modelFlags, surface, *this);
#else
  material.SetCurrent(modelFlags, surface, *this);
  CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
#endif
}

#if RSTL_CORRUPTION
void CCubeModel::DrawSurfaceFlat(const CCubeSurface& surface) const {
  const CMaterialDataFactory::SMaterial& material = GetMaterial(surface);
  if (material.mHeader.mFlags & sSkippedMaterialFlags) {
    return;
  }
  CGX::SetVtxDescv_Compressed(material.mHeader.mVertexDesc);
  CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
}
#else
#if !RSTL_PRIME1
void CCubeModel::DrawSurfaceFlat(const CCubeSurface& surface) const {
  const CCubeMaterial material = GetMaterial(surface);
  if (material.GetFlags() & sSkippedMaterialFlags) {
    return;
  }
  CGX::SetVtxDescv_Compressed(material.GetVertexDesc());
  CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
}
#endif
#endif

static inline const ushort proxy_to_uint(const uchar* data) {
  uchar bytes[2];
  bytes[0] = data[0];
  bytes[1] = data[1];
#ifdef __MWERKS__
  return CBasics::SwapBytes(*reinterpret_cast< const ushort* >(bytes));
#else
  ushort value;
  memcpy(&value, bytes, sizeof(value));
  return CBasics::SwapBytes(value);
#endif
}

#if RSTL_CORRUPTION
void CCubeModel::DrawSurfaceWireframe(CCubeSurface surface) const {
  const CMaterialDataFactory::SMaterial& material = GetMaterial(surface);
  CGraphics::CAlphaContextGuard alphaGuard(0);
#else
void CCubeModel::DrawSurfaceWireframe(const CCubeSurface& surface) const {
  const CCubeMaterial material = GET_MATERIAL_FROM_SURFACE(surface);

#endif
  uint vertexAttributes;
  static uint sLastDesc = 0;
  static uint sAttrCount = 0;
#if RSTL_CORRUPTION
  static uint sMatrixIndexCount = 0; // Guessed name
  static bool sUseMatrixIndex = false; // Guessed name
  CGraphics::EnableDestinationAlpha(0, 0);
  vertexAttributes = material.mHeader.mVertexDesc;
#else
  vertexAttributes = material.GetVertexDesc();
#endif

  if (vertexAttributes != sLastDesc) {
    sAttrCount = 0;
#if RSTL_CORRUPTION
    sMatrixIndexCount = 0;
    sUseMatrixIndex = false;
    for (int i = GX_VA_POS; i <= GX_VA_TEX7; ++i) {
      if ((vertexAttributes >> ((i - GX_VA_POS) * 2)) & 3) {
#else
    for (int i = 0; i < 16; ++i) {
      if ((vertexAttributes >> (i * 2)) & 3) {
#endif
        sAttrCount++;
#if RSTL_CORRUPTION
      }
    }
    for (int i = 0; i < 8; ++i) {
      if ((vertexAttributes >> (i + 24)) & 1) {
        sMatrixIndexCount++;
#endif
      }
    }
    sLastDesc = vertexAttributes;
  }

#if RSTL_CORRUPTION
  const int matrixIndexCount = sMatrixIndexCount;
  const int vertexStride = sAttrCount * 2 + matrixIndexCount;
#else
  const int attrCountTimes2 = sAttrCount * 2;
#endif
  static const GXVtxDescList sDesc[] = {
      {GX_VA_POS, GX_INDEX16},
      {GX_VA_NULL, GX_NONE},
  };
#if RSTL_CORRUPTION
  static const GXVtxDescList sMatrixDesc[] = {
      {GX_VA_PNMTXIDX, GX_DIRECT},
      {GX_VA_POS, GX_INDEX16},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(sUseMatrixIndex ? sMatrixDesc : sDesc);
#else
  CGX::SetVtxDescv(sDesc);
#endif
  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetNumIndStages(0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ONE);
  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
  CGX::SetNumChans(0);
  CGX::SetNumTexGens(1);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);

#if !RSTL_CORRUPTION
  if (!RSTL_PRIME1) {
#endif
    CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
#if !RSTL_CORRUPTION
  }
#endif

  const int displayListSize = surface.GetDisplayListSize();
  const uchar* dispList = static_cast< const uchar* >(surface.GetDisplayList());
  for (int bytesRead = 0; bytesRead < displayListSize;) {
    const uchar pType = *dispList & 0xf8;
    if (!pType) {
      break;
    }
    bytesRead += 3;
#if RSTL_CORRUPTION
    int elementCount = proxy_to_uint(dispList + 1);
#else
    IF_RSTL_PRIME1_ELSE(ushort, int) elementCount = proxy_to_uint(dispList + 1);
#endif
    dispList += 3;
    if (elementCount < 3U) {
      break;
    }

    CGX::Begin(GX_LINESTRIP, GX_VTXFMT0, 4);
#if RSTL_CORRUPTION
    if (sUseMatrixIndex) {
      GXMatrixIndex1u8(0);
    }
    GXPosition1x16(proxy_to_uint(dispList + matrixIndexCount));
    if (sUseMatrixIndex) {
      GXMatrixIndex1u8(0);
    }
    GXPosition1x16(proxy_to_uint(dispList + vertexStride + matrixIndexCount));
    if (sUseMatrixIndex) {
      GXMatrixIndex1u8(0);
    }
    GXPosition1x16(proxy_to_uint(dispList + vertexStride * 2 + matrixIndexCount));
    if (sUseMatrixIndex) {
      GXMatrixIndex1u8(0);
    }
    GXPosition1x16(proxy_to_uint(dispList + matrixIndexCount));
#else
    GXPosition1x16(proxy_to_uint(dispList));
    GXPosition1x16(proxy_to_uint(dispList + attrCountTimes2));
    GXPosition1x16(proxy_to_uint(dispList + attrCountTimes2 * 2));
    GXPosition1x16(proxy_to_uint(dispList));
    bytesRead += elementCount * attrCountTimes2;
    dispList += attrCountTimes2 * 3;
#endif
    CGX::End();
#if RSTL_CORRUPTION
    bytesRead += elementCount * vertexStride;
    dispList += vertexStride * 3;
#endif
    if (pType == GX_TRIANGLES) {
#if RSTL_CORRUPTION
      const ushort remaining = elementCount - 3;
      for (int j = 0; j < remaining; j += 3) {
#else
#if RSTL_PRIME1
      elementCount -= 3;
#else
      const ushort remaining = elementCount - 3;
#endif
      for (int j = 0; j < IF_RSTL_PRIME1_ELSE(elementCount, remaining); j += 3) {
#endif
        CGX::Begin(GX_LINESTRIP, GX_VTXFMT0, 4);
#if RSTL_CORRUPTION
        if (sUseMatrixIndex) {
          GXMatrixIndex1u8(0);
        }
        GXPosition1x16(proxy_to_uint(dispList + matrixIndexCount));
        if (sUseMatrixIndex) {
          GXMatrixIndex1u8(0);
        }
        GXPosition1x16(proxy_to_uint(dispList + vertexStride + matrixIndexCount));
        if (sUseMatrixIndex) {
          GXMatrixIndex1u8(0);
        }
        GXPosition1x16(proxy_to_uint(dispList + vertexStride * 2 + matrixIndexCount));
        if (sUseMatrixIndex) {
          GXMatrixIndex1u8(0);
        }
        GXPosition1x16(proxy_to_uint(dispList + matrixIndexCount));
#else
        GXPosition1x16(proxy_to_uint(dispList));
        GXPosition1x16(proxy_to_uint(dispList + attrCountTimes2));
        GXPosition1x16(proxy_to_uint(dispList + attrCountTimes2 * 2));
        GXPosition1x16(proxy_to_uint(dispList));
        dispList += attrCountTimes2 * 3;
#endif
        CGX::End();
#if RSTL_CORRUPTION
        dispList += vertexStride * 3;
#endif
      }
    } else if (pType == GX_TRIANGLESTRIP) {
#if RSTL_CORRUPTION
      const ushort remaining = elementCount - 3;
#else
#if RSTL_PRIME1
      elementCount -= 3;
#else
      const ushort remaining = elementCount - 3;
#endif

#endif
      uint winding = 1;
#if RSTL_CORRUPTION
      for (int j = 0; j < remaining; ++j) {
#else
      for (int j = 0; j < IF_RSTL_PRIME1_ELSE(elementCount, remaining); ++j) {
#endif
        CGX::Begin(GX_LINESTRIP, GX_VTXFMT0, 3);
#if RSTL_CORRUPTION
        const uchar* last = dispList - vertexStride * ((winding ^ 1) + 1);
        const uchar* first = dispList - vertexStride * (winding + 1);
        if (sUseMatrixIndex) {
          GXMatrixIndex1u8(0);
        }
        GXPosition1x16(proxy_to_uint(first + matrixIndexCount));
        if (sUseMatrixIndex) {
          GXMatrixIndex1u8(0);
        }
        GXPosition1x16(proxy_to_uint(dispList + matrixIndexCount));
        if (sUseMatrixIndex) {
          GXMatrixIndex1u8(0);
        }
        GXPosition1x16(proxy_to_uint(last + matrixIndexCount));
#else
        const uchar* last = dispList - attrCountTimes2 * ((winding ^ 1) + 1);
        const uchar* first = dispList - attrCountTimes2 * (winding + 1);
        winding ^= 1;
        GXPosition1x16(proxy_to_uint(first));
        GXPosition1x16(proxy_to_uint(dispList));
        dispList += attrCountTimes2;
        GXPosition1x16(proxy_to_uint(last));
#endif
        CGX::End();
#if RSTL_CORRUPTION
        dispList += vertexStride;
        winding ^= 1;
#endif
      }
    } else {
      if (pType != GX_TRIANGLEFAN) {
#if RSTL_CORRUPTION
        break;
#else
        return;
#endif
      }
#if RSTL_CORRUPTION
      const ushort remaining = elementCount - 3;
      const uchar* indices = dispList - vertexStride * 3 + matrixIndexCount;
#else
#if RSTL_PRIME1
      elementCount -= 3;
#else
      const ushort remaining = elementCount - 3;
#endif
#endif

#if RSTL_CORRUPTION
      for (int j = 0; j < remaining; ++j) {
        const uchar* previous = dispList - vertexStride;
#else
      const uchar* indices = dispList - attrCountTimes2 * 3;

      for (int j = 0; j < IF_RSTL_PRIME1_ELSE(elementCount, remaining); ++j) {
        const uchar* previous = dispList - attrCountTimes2;
#endif
        CGX::Begin(GX_LINESTRIP, GX_VTXFMT0, 3);
#if RSTL_CORRUPTION
        if (sUseMatrixIndex) {
          GXMatrixIndex1u8(0);
        }
        GXPosition1x16(proxy_to_uint(previous + matrixIndexCount));
        if (sUseMatrixIndex) {
          GXMatrixIndex1u8(0);
        }
        GXPosition1x16(proxy_to_uint(dispList + matrixIndexCount));
        if (sUseMatrixIndex) {
          GXMatrixIndex1u8(0);
        }
#else
        GXPosition1x16(proxy_to_uint(previous));
        GXPosition1x16(proxy_to_uint(dispList));
        dispList += attrCountTimes2;
#endif
        GXPosition1x16(proxy_to_uint(indices));
        CGX::End();
#if RSTL_CORRUPTION
        dispList += vertexStride;
#endif
      }
    }
  }
}

bool CCubeModel::TryLockTextures() const {
#if RSTL_CORRUPTION
  if (!mTexturesReady) {
    CMaterialTextureState& textures = mTextureState;
    textures.LockTextures();
    if (textures.TryCacheTextures()) {
      mTexturesReady = true;
#else
  if (!mLoadTextures) {
    bool texturesLoading = false;
    for (int i = 0; i < mTextures->size(); ++i) {
      (*mTextures)[i].Lock();
      if (!(*mTextures)[i].TryCache()) {

        texturesLoading = true;
      } else if (!(*mTextures)[i].GetObject()->LoadToMRAM()) {

        texturesLoading = true;
#endif
      }
    }
#if RSTL_CORRUPTION
  return !!mTexturesReady;
#else

    if (!texturesLoading) {
      mLoadTextures = true;
    }
#endif
  }

#if RSTL_CORRUPTION
bool CCubeModel::AreRequestedTexturesLoaded() const {
  return mTextureState.HasLockedTextures() ? TryLockTextures() : false;
#else
  return !!mLoadTextures;
#endif
}

void CCubeModel::DrawSurfaces(const CModelFlags& flags) const {
  if (sDrawingWireframe) {
    for (CCubeSurface surface = GetNormalSurfaces(); surface.IsValid();
         surface = surface.GetNextSurface()) {
      DrawSurfaceWireframe(surface);
    }
    for (CCubeSurface surface = GetAlphaSurfaces(); surface.IsValid();
         surface = surface.GetNextSurface()) {
      DrawSurfaceWireframe(surface);
    }
  } else if ((flags.GetOtherFlags() & CModelFlags::kF_NoTextureLock) || TryLockTextures()) {
    for (CCubeSurface surface = GetNormalSurfaces(); surface.IsValid();
         surface = surface.GetNextSurface()) {
      DrawSurface(surface, flags);
    }
    for (CCubeSurface surface = GetAlphaSurfaces(); surface.IsValid();
         surface = surface.GetNextSurface()) {
      DrawSurface(surface, flags);
    }
#if RSTL_CORRUPTION
    CCubeMaterial::EnsureTevsDirect();
#endif
  }
}

#if RSTL_CORRUPTION
void CCubeModel::DrawSurfaces(u64 mask, const CModelFlags& flags) const {
  if (sDrawingWireframe) {
    for (CCubeSurface surface = GetNormalSurfaces(); surface.IsValid();
         surface = surface.GetNextSurface()) {
      DrawSurfaceWireframe(surface);
    }
    for (CCubeSurface surface = GetAlphaSurfaces(); surface.IsValid();
         surface = surface.GetNextSurface()) {
      DrawSurfaceWireframe(surface);
    }
  } else if ((flags.GetOtherFlags() & CModelFlags::kF_NoTextureLock) || TryLockTextures()) {
    for (CCubeSurface surface = GetNormalSurfaces(); surface.IsValid();
         surface = surface.GetNextSurface()) {
      if (GetMaterial(surface).mHeader.mCollisionFlags & mask) {
        DrawSurface(surface, flags);
      }
    }
    for (CCubeSurface surface = GetAlphaSurfaces(); surface.IsValid();
         surface = surface.GetNextSurface()) {
      if (GetMaterial(surface).mHeader.mCollisionFlags & mask) {
        DrawSurface(surface, flags);
      }
    }
  }
}
#else
#if !RSTL_PRIME1
void CCubeModel::DrawSurfaces(u64 mask, const CModelFlags& flags) const {
  if (sDrawingWireframe) {
    for (CCubeSurface surface = GetNormalSurfaces(); surface.IsValid();
         surface = surface.GetNextSurface()) {
      DrawSurfaceWireframe(surface);
    }
    for (CCubeSurface surface = GetAlphaSurfaces(); surface.IsValid();
         surface = surface.GetNextSurface()) {
      DrawSurfaceWireframe(surface);
    }
  } else if ((flags.GetOtherFlags() & CModelFlags::kF_NoTextureLock) || TryLockTextures()) {
    for (CCubeSurface surface = GetNormalSurfaces(); surface.IsValid();
         surface = surface.GetNextSurface()) {
      if (GetMaterial(surface).GetMaterialMask() & mask) {
        DrawSurface(surface, flags);
      }
    }
    for (CCubeSurface surface = GetAlphaSurfaces(); surface.IsValid();
         surface = surface.GetNextSurface()) {
      if (GetMaterial(surface).GetMaterialMask() & mask) {
        DrawSurface(surface, flags);
      }
    }
  }
}
#endif
#endif

void CCubeModel::DrawNormalSurfaces(const CModelFlags& flags) const {
  if (sDrawingWireframe) {
    for (CCubeSurface surface = GetNormalSurfaces(); surface.IsValid();
         surface = surface.GetNextSurface()) {
      DrawSurfaceWireframe(surface);
    }
  } else if (TryLockTextures()) {
    for (CCubeSurface surface = GetNormalSurfaces(); surface.IsValid();
         surface = surface.GetNextSurface()) {
      DrawSurface(surface, flags);
    }
#if RSTL_CORRUPTION
    CCubeMaterial::EnsureTevsDirect();
#endif
  }
}

void CCubeModel::DrawAlphaSurfaces(const CModelFlags& flags) const {
  if (sDrawingWireframe) {
    for (CCubeSurface surface = GetAlphaSurfaces(); surface.IsValid();
         surface = surface.GetNextSurface()) {
      DrawSurfaceWireframe(surface);
    }
  } else if (TryLockTextures()) {
    for (CCubeSurface surface = GetAlphaSurfaces(); surface.IsValid();
         surface = surface.GetNextSurface()) {
      DrawSurface(surface, flags);
    }
#if RSTL_CORRUPTION
    CCubeMaterial::EnsureTevsDirect();
#endif
  }
}

#if RSTL_CORRUPTION
void CCubeModel::DrawFlat(int which) const {
    SetArraysCurrent();

#else
#if RSTL_PRIME1
void CCubeModel::DrawFlat(TModelPositions positions, TModelNormals normals,
                          ESurfaceSelection which) const
#else
void CCubeModel::DrawFlat(int which) const
#endif
{
#if RSTL_PRIME1
  if (IF_TARGET_PC_OR(true, positions != nullptr)) {
    SetSkinningArraysCurrent(positions, normals);
  } else {
    SetArraysCurrent();
  }
#else
  SetArraysCurrent();
#endif
#endif
  if (which != kSS_Sorted) {
    for (CCubeSurface surface = mFirstUnsorted; surface.IsValid();
         surface = surface.GetNextSurface()) {
#if RSTL_CORRUPTION
      const CMaterialDataFactory::SMaterial& material = GetMaterial(surface);
      CGX::SetVtxDescv_Compressed(material.mHeader.mVertexDesc);
#else
      CCubeMaterial material = GET_MATERIAL_FROM_SURFACE(surface);
      CGX::SetVtxDescv_Compressed(material.GetVertexDescLwzx());
#endif
      CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
    }
  }

  if (which != kSS_Unsorted) {
    for (CCubeSurface surface = mFirstSorted; surface.IsValid();
         surface = surface.GetNextSurface()) {
#if RSTL_CORRUPTION
      const CMaterialDataFactory::SMaterial& material = GetMaterial(surface);
      CGX::SetVtxDescv_Compressed(material.mHeader.mVertexDesc);
#else
      CCubeMaterial material = GET_MATERIAL_FROM_SURFACE(surface);
      CGX::SetVtxDescv_Compressed(material.GetVertexDescLwzx());
#endif
      CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
    }
  }
}

void CCubeModel::Draw(const CModelFlags& flags) const {
  CCubeMaterial::KillCachedViewDepState();
  SetArraysCurrent();
  DrawSurfaces(flags);
}

#if RSTL_CORRUPTION
void CCubeModel::Draw(u64 mask, const CModelFlags& flags) const {
#else
#if RSTL_PRIME1
void CCubeModel::Draw(TModelPositions positions, TModelNormals normals,
                      const CModelFlags& flags) const
#else
void CCubeModel::Draw(u64 mask, const CModelFlags& flags) const
#endif
{

#endif
  CCubeMaterial::KillCachedViewDepState();
#if RSTL_CORRUPTION
  SetArraysCurrent();
  DrawSurfaces(mask, flags);
#else
#if RSTL_PRIME1
  SetSkinningArraysCurrent(positions, normals);
  DrawSurfaces(flags);
#else
  SetArraysCurrent();
  DrawSurfaces(mask, flags);
#endif
#endif
}

void CCubeModel::DrawNormal(const CModelFlags& flags) const {
  CCubeMaterial::KillCachedViewDepState();
  SetArraysCurrent();
  DrawNormalSurfaces(flags);
}

void CCubeModel::DrawAlpha(const CModelFlags& flags) const {
  CCubeMaterial::KillCachedViewDepState();
  SetArraysCurrent();
  DrawAlphaSurfaces(flags);
}

void CCubeModel::SetDrawingOccluders(const bool drawOccluders) {
#if RSTL_CORRUPTION
  if (drawOccluders) {
    sSkippedMaterialFlags &= ~0x100;
  } else {
    sSkippedMaterialFlags |= 0x100;
  }
#else
#if RSTL_PRIME1
  sDrawingOccluders = drawOccluders;
#else
  if (drawOccluders) {
    sSkippedMaterialFlags &= ~kStateFlag_ShadowOccluderMesh;
  } else {
    sSkippedMaterialFlags |= kStateFlag_ShadowOccluderMesh;
  }
#endif
#endif
}

#if RSTL_CORRUPTION
void CCubeModel::DrawNormal(ESurfaceSelection which) const {
#else
void CCubeModel::SetModelWireframe(const bool drawWireframe) { sDrawingWireframe = drawWireframe; }

void CCubeModel::UnlockTextures() const {
  for (rstl::vector< TCachedToken< CTexture > >::iterator texture = mTextures->begin();
       texture != mTextures->end(); ++texture) {

    texture->Unlock();
  }

  mLoadTextures = false;
}

void CCubeModel::RemapMaterialData(TModelData data,
                                   rstl::vector< TCachedToken< CTexture > >* texture) {
  mInstance.SetMaterialPointer(data);
  mTextures = texture;
  mLoadTextures = false;
#if !RSTL_PRIME1
  CacheMaterialData();
#endif
}

void CCubeModel::DrawNormal(IF_RSTL_PRIME1(TModelPositions positions, TModelNormals normals, )
                                ESurfaceSelection which) const {
#endif
  CGX::SetNumIndStages(0);
  CGX::SetNumTevStages(1);
  CGX::SetNumTexGens(1);
  CGX::SetZMode(true, GX_LEQUAL, true);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_ONE, GX_LO_CLEAR);
#if RSTL_CORRUPTION
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  DrawFlat(which);
#else
#if RSTL_PRIME1
  DrawFlat(positions, normals, which);
#else
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  DrawFlat(which);
#endif
#endif
}
#if !RSTL_CORRUPTION

#if !RSTL_PRIME1
void CCubeModel::CacheMaterialData() {
  const uint* data = static_cast< const uint* >(mInstance.GetMaterialPointer());
  data += mTextures->size() + 1;
  const uint materialCount = CBasics::SwapBytes(*data++);
  mMaterialOffsets = data;
  data += materialCount;
  mMaterialData = reinterpret_cast< const uchar* >(data);
}
#endif
#endif
