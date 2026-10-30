#include "rstl/RstlVersions.h"
#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Basics/CBasics.hpp"

#include "Kyoto/Graphics/CCubeSurface.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGX_Impl.hpp" // IWYU pragma: keep

#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CVector2f.hpp"

#include "dolphin/gx/GXVert.h"

#include "string.h"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
static bool sDrawingOccluders = false;
#else
// Guessed name; bits 0x200 and 0x8000 reject material flags.
static uint sSkippedMaterialFlags = 0x8200;
#endif

static bool sDrawingWireframe = false;
bool CCubeModel::sUsingPackedLightmaps = false;

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
inline uint GetMaterialOffset(const uchar* materialData, const int idx) {
  materialData += (idx * 4);
  return CBasics::SwapBytes(*reinterpret_cast< const uint* >(materialData - 4));
}
#endif

CCubeModel::CCubeModel(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    rstl::vector< void* >* surfaces,
#else
    rstl::vector< CCubeSurface >* surfaces,
#endif
    rstl::vector< TCachedToken< CTexture > >* textures, const void* materialData,
    const void* positions, const void* normals, const void* colors, const void* uvs,
    const void* compressedUvs, const CAABox& bounds, const uchar visorFlags,
    const bool texturesLoaded, const uint idx)
: mInstance(*surfaces, materialData, positions, normals, colors, uvs, compressedUvs)
, mTextures(textures)
, mBounds(bounds)
, mFirstUnsorted(nullptr)
, mFirstSorted(nullptr)
, mLoadTextures(static_cast< uchar >(!texturesLoaded))
, mVisible(false)
#if !RSTL_PRIME1
, x40_(0)
#endif

, mVisorFlags(visorFlags)
, mIdx(idx)
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
, mMaterialOffsets(nullptr)
, mMaterialData(nullptr)
#endif
{
#if !RSTL_PRIME1
  CacheMaterialData();
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::vector< void* >& surf = mInstance.Surfaces();
  for (AUTO(it, surf.begin()); it != surf.end(); ++it) {
    CCubeSurface::SSurfaceData* data = static_cast< CCubeSurface::SSurfaceData* >(*it);
#else
  rstl::vector< CCubeSurface >& surf = mInstance.Surfaces();
  for (rstl::vector< CCubeSurface >::iterator it = surf.begin(); it != surf.end(); ++it) {
    const CCubeSurface::SSurfaceData* data = it->mData;
#endif

    data->mParent = this;
  }

  for (int i = surf.size(); i > 0; i--) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    void*& data = surf[i - 1];
    uint materialIndex = static_cast< CCubeSurface::SSurfaceData* >(data)->mMaterialIndex;
    if (GetMaterialByIndex(materialIndex).IsFlagSet(kStateFlag_DepthSorting)) {
      static_cast< CCubeSurface::SSurfaceData* >(data)->mNextSurface = mFirstSorted.mRawdata;
      mFirstSorted.mRawdata = static_cast< uchar* >(data);
#else
    const CCubeSurface& surface = surf[i - 1];
    if (GetMaterial(surface).IsFlagSet(kStateFlag_DepthSorting)) {
      surface.mData->mNextSurface = mFirstSorted.mData;
      mFirstSorted = surface;
#endif

    } else {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      static_cast< CCubeSurface::SSurfaceData* >(data)->mNextSurface = mFirstUnsorted.mRawdata;
      mFirstUnsorted.mRawdata = static_cast< uchar* >(data);
#else
      surface.mData->mNextSurface = mFirstUnsorted.mData;
      mFirstUnsorted = surface;
#endif

    }
  }
}

void CCubeModel::MakeTexturesFromMats(const void* data,
                                      rstl::vector< TCachedToken< CTexture > >& textures,
                                      IObjectStore& store, const bool cache) {
  const uint* textureIds = static_cast< const uint* >(data);
  const uint textureCount = CBasics::SwapBytes(*static_cast< const int* >(data));
  textureIds++;
  textures.reserve(textureCount);

  for (int i = 0; i < textureCount; i++) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    textures.push_back(store.GetObj(SObjectTag('TXTR', CBasics::SwapBytes(*textureIds))));
#else
    textures.push_back_unsafe(store.GetObj(SObjectTag('TXTR', CBasics::SwapBytes(*textureIds))));
#endif

    if (!cache) {
      textures.back().ForceCache();
    }
    ++textureIds;
  }
}

void CCubeModel::SetStaticArraysCurrent() const {
  CGX::SetArray(GX_VA_CLR0, mInstance.GetColorPointer(), sizeof(CColor));
  const void* packed = mInstance.GetPackedTCPointer();
  const void* unpacked = mInstance.GetTCPointer();
  if (!packed) {
    sUsingPackedLightmaps = false;
  }

  if (sUsingPackedLightmaps) {
    CGX::SetArray(GX_VA_TEX0, packed, sizeof(ushort) * 2);
  } else {
    CGX::SetArray(GX_VA_TEX0, unpacked, sizeof(CVector2f));
  }

  if (unpacked) {
    for (int i = 1; i <= GX_VA_TEX7 - GX_VA_TEX0; ++i) {
      CGX::SetArray(static_cast< GXAttr >(i + GX_VA_TEX0), unpacked, sizeof(CVector2f));
    }
  }

  CCubeMaterial::KillCachedViewDepState();
}

void CCubeModel::SetArraysCurrent() const {
  CGX::SetArray(GX_VA_POS, mInstance.GetVertexPointer(), sizeof(CVector3f));
  const int stride = (mVisorFlags & 1) ? sizeof(short) * 3 : sizeof(CVector3f);
  CGX::SetArray(GX_VA_NRM, mInstance.GetNormalPointer(), stride);
  SetStaticArraysCurrent();
}

#if RSTL_PRIME1
void CCubeModel::SetSkinningArraysCurrent(const float* positions, const float* normals) const {
  CGraphics::sRenderState.SetVtxState(positions, normals,
                                      static_cast< const uint* >(mInstance.GetColorPointer()));
  SetStaticArraysCurrent();
}
#endif

void CCubeModel::SetUsingPackedLightmaps(const bool use) const {
  sUsingPackedLightmaps = use;
  if (sUsingPackedLightmaps) {
    CGX::SetArray(GX_VA_TEX0, mInstance.GetPackedTCPointer(), sizeof(ushort) * 2);
  } else {
    CGX::SetArray(GX_VA_TEX0, mInstance.GetTCPointer(), sizeof(CVector2f));
  }
}

#if RSTL_PRIME1
CCubeMaterial CCubeModel::GetMaterialByIndex(const int idx) const {
  uint materialCount = 0;
  uint materialOffset = 0;
  const uchar* materialData = static_cast< const uchar* >(mInstance.GetMaterialPointer()) +
                              (mTextures->size() + 1) * 4;
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
}
#endif

void CCubeModel::DrawSurface(const CCubeSurface& surface, const CModelFlags& modelFlags) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const CCubeMaterial material = GetMaterialByIndex(surface.GetMaterialIndex());
  if (material.IsFlagSet(kStateFlag_ShadowOccluderMesh) && !sDrawingOccluders) {
#else
  const CCubeMaterial material = GetMaterial(surface);
  if (material.GetFlags() & sSkippedMaterialFlags) {
#endif

    return;
  }

  material.SetCurrent(modelFlags, surface, *this);
#if !RSTL_PRIME1
  CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
}

void CCubeModel::DrawSurfaceFlat(const CCubeSurface& surface) const {
  const CCubeMaterial material = GetMaterial(surface);
  if (material.GetFlags() & sSkippedMaterialFlags) {
    return;
  }
  CGX::SetVtxDescv_Compressed(material.GetVertexDesc());
#endif

  CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
}

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

void CCubeModel::DrawSurfaceWireframe(const CCubeSurface& surface) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const CCubeMaterial material = GetMaterialByIndex(surface.GetMaterialIndex());
#else
  const CCubeMaterial material = GetMaterial(surface);
#endif

  uint vertexAttributes;
  static uint sLastDesc = 0;
  static uint sAttrCount = 0;
  vertexAttributes = material.GetVertexDesc();

  if (vertexAttributes != sLastDesc) {
    sAttrCount = 0;
    for (int i = 0; i < 16; ++i) {
      if ((vertexAttributes >> (i * 2)) & 3) {
        sAttrCount++;
      }
    }
    sLastDesc = vertexAttributes;
  }

  const int attrCountTimes2 = sAttrCount * 2;
  static const GXVtxDescList sDesc[] = {
      {GX_VA_POS, GX_INDEX16},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(sDesc);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetNumIndStages(0);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ONE);
  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
  CGX::SetNumChans(0);
  CGX::SetNumTexGens(1);
  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);

#if !RSTL_PRIME1
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
#endif

  const int displayListSize = surface.GetDisplayListSize();
  const uchar* dispList = static_cast< const uchar* >(surface.GetDisplayList());
  for (int bytesRead = 0; bytesRead < displayListSize;) {
    const uchar pType = *dispList & 0xf8;
    if (!pType) {
      break;
    }
    bytesRead += 3;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    ushort elementCount = proxy_to_uint(dispList + 1);
#else
    int elementCount = proxy_to_uint(dispList + 1);
#endif

    dispList += 3;
    if (elementCount < 3U) {
      break;
    }

    CGX::Begin(GX_LINESTRIP, GX_VTXFMT0, 4);
    GXPosition1x16(proxy_to_uint(dispList));
    GXPosition1x16(proxy_to_uint(dispList + attrCountTimes2));
    GXPosition1x16(proxy_to_uint(dispList + attrCountTimes2 * 2));
    GXPosition1x16(proxy_to_uint(dispList));
    bytesRead += elementCount * attrCountTimes2;
    dispList += attrCountTimes2 * 3;
    CGX::End();
    if (pType == GX_TRIANGLES) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      elementCount -= 3;
      for (int j = 0; j < elementCount; j += 3) {
#else
      const ushort remaining = elementCount - 3;
      for (int j = 0; j < remaining; j += 3) {
#endif

        CGX::Begin(GX_LINESTRIP, GX_VTXFMT0, 4);
        GXPosition1x16(proxy_to_uint(dispList));
        GXPosition1x16(proxy_to_uint(dispList + attrCountTimes2));
        GXPosition1x16(proxy_to_uint(dispList + attrCountTimes2 * 2));
        GXPosition1x16(proxy_to_uint(dispList));
        dispList += attrCountTimes2 * 3;
        CGX::End();
      }
    } else if (pType == GX_TRIANGLESTRIP) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      elementCount -= 3;
#else
      const ushort remaining = elementCount - 3;
#endif

      uint winding = 1;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      for (int j = 0; j < elementCount; ++j) {
#else
      for (int j = 0; j < remaining; ++j) {
#endif

        CGX::Begin(GX_LINESTRIP, GX_VTXFMT0, 3);
        const uchar* last = dispList - attrCountTimes2 * ((winding ^ 1) + 1);
        const uchar* first = dispList - attrCountTimes2 * (winding + 1);
        winding ^= 1;
        GXPosition1x16(proxy_to_uint(first));
        GXPosition1x16(proxy_to_uint(dispList));
        dispList += attrCountTimes2;
        GXPosition1x16(proxy_to_uint(last));
        CGX::End();
      }
    } else {
      if (pType != GX_TRIANGLEFAN) {
        return;
      }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      elementCount -= 3;
#else
      const ushort remaining = elementCount - 3;
#endif

      const uchar* indices = dispList - attrCountTimes2 * 3;

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      for (int j = 0; j < elementCount; ++j) {
#else
      for (int j = 0; j < remaining; ++j) {
#endif

        const uchar* previous = dispList - attrCountTimes2;
        CGX::Begin(GX_LINESTRIP, GX_VTXFMT0, 3);
        GXPosition1x16(proxy_to_uint(previous));
        GXPosition1x16(proxy_to_uint(dispList));
        dispList += attrCountTimes2;
        GXPosition1x16(proxy_to_uint(indices));
        CGX::End();
      }
    }
  }
}

bool CCubeModel::TryLockTextures() const {
  if (!mLoadTextures) {
    bool texturesLoading = false;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    for (int i = 0; i < GetTextures().size(); ++i) {
      GetTextures()[i].Lock();
      if (!GetTextures()[i].TryCache()) {
#else
    for (int i = 0; i < mTextures->size(); ++i) {
      (*mTextures)[i].Lock();
      if (!(*mTextures)[i].TryCache()) {
#endif

        texturesLoading = true;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      } else if (!GetTextures()[i].GetObject()->LoadToMRAM()) {
#else
      } else if (!(*mTextures)[i].GetObject()->LoadToMRAM()) {
#endif

        texturesLoading = true;
      }
    }

    if (!texturesLoading) {
      mLoadTextures = true;
    }
  }

  return !!mLoadTextures;
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
  }
}

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
  }
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CCubeModel::DrawFlat(const float* positions, const float* normals,
                          ESurfaceSelection which) const {
  if (positions != nullptr) {
    SetSkinningArraysCurrent(positions, normals);
  } else {
#else
void CCubeModel::DrawFlat(int which) const {
#endif

    SetArraysCurrent();
#if RSTL_PRIME1
  }
#endif

  if (which != kSS_Sorted) {
    for (CCubeSurface surface = mFirstUnsorted; surface.IsValid();
         surface = surface.GetNextSurface()) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      CCubeMaterial material = GetMaterialByIndex(surface.GetMaterialIndex());
      CGX::SetVtxDescv_Compressed(material.GetVertexDescLwzx());
#else
      CCubeMaterial material = GetMaterial(surface);
      CGX::SetVtxDescv_Compressed(CBasics::SwapBytes(
          reinterpret_cast< const uint* >(material.GetData())[material.GetTextureCount() + 2]));
#endif

      CGX::CallDisplayList(surface.GetDisplayList(), surface.GetDisplayListSize());
    }
  }

  if (which != kSS_Unsorted) {
    for (CCubeSurface surface = mFirstSorted; surface.IsValid();
         surface = surface.GetNextSurface()) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      CCubeMaterial material = GetMaterialByIndex(surface.GetMaterialIndex());
      CGX::SetVtxDescv_Compressed(material.GetVertexDescLwzx());
#else
      CCubeMaterial material = GetMaterial(surface);
      CGX::SetVtxDescv_Compressed(CBasics::SwapBytes(
          reinterpret_cast< const uint* >(material.GetData())[material.GetTextureCount() + 2]));
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

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CCubeModel::Draw(const float* positions, const float* normals,
                      const CModelFlags& flags) const {
#else
void CCubeModel::Draw(u64 mask, const CModelFlags& flags) const {
#endif

  CCubeMaterial::KillCachedViewDepState();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  SetSkinningArraysCurrent(positions, normals);
  DrawSurfaces(flags);
#else
  SetArraysCurrent();
  DrawSurfaces(mask, flags);
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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  sDrawingOccluders = drawOccluders;
#else
  if (drawOccluders) {
    sSkippedMaterialFlags &= ~kStateFlag_ShadowOccluderMesh;
  } else {
    sSkippedMaterialFlags |= kStateFlag_ShadowOccluderMesh;
  }
#endif

}

void CCubeModel::SetModelWireframe(const bool drawWireframe) { sDrawingWireframe = drawWireframe; }

void CCubeModel::UnlockTextures() const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  for (AUTO(texture, mTextures->begin()); texture != mTextures->end(); ++texture) {
#else
  for (rstl::vector< TCachedToken< CTexture > >::iterator texture = mTextures->begin();
       texture != mTextures->end(); ++texture) {
#endif

    texture->Unlock();
  }

  mLoadTextures = false;
}

void CCubeModel::RemapMaterialData(const void* data,
                                   rstl::vector< TCachedToken< CTexture > >* texture) {

  mInstance.SetMaterialPointer(data);
  mTextures = texture;
  mLoadTextures = false;
#if !RSTL_PRIME1
  CacheMaterialData();
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CCubeModel::DrawNormal(const float* positions, const float* normals,
                            ESurfaceSelection which) const {
#else
void CCubeModel::DrawNormal(ESurfaceSelection which) const {
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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  DrawFlat(positions, normals, which);
#else
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
  DrawFlat(which);
}

void CCubeModel::CacheMaterialData() {
  const uint* data = static_cast< const uint* >(mInstance.GetMaterialPointer());
  data += mTextures->size() + 1;
  const uint materialCount = CBasics::SwapBytes(*data++);
  mMaterialOffsets = data;
  data += materialCount;
  mMaterialData = reinterpret_cast< const uchar* >(data);
#endif

}

