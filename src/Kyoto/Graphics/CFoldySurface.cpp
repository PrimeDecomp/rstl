#include "rstl/RstlVersions.h"
#include "Kyoto/Graphics/CFoldySurface.hpp"

#if RSTL_CORRUPTION
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#endif
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/Graphics/CCubeMaterial.hpp"
#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CCubeSurface.hpp"
#include "Kyoto/Graphics/CDisplayListReader.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#if !RSTL_CORRUPTION
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#endif

#include "rstl/vector.hpp"

#include "dolphin/os.h"
#include "string.h"

CFoldySurface::CFoldySurface(const TToken< CModel >& model, float lowerX, float upperX,
                             const CVector3f& lowerOffset, const CVector3f& middleOffset,
                             const CVector3f& upperOffset)
: mModel(model, true), mPositions(nullptr), mVertexCount(0), mDisplayListSize(0) {
#if RSTL_CORRUPTION
  BuildMatrixIndexAttributes();
#endif
  const CCubeModel& cubeModel = *mModel.GetObject()->GetModelInstance();
#if RSTL_CORRUPTION
  const CCubeSurface firstSurface(cubeModel.GetModelInstance().Surfaces().front());
  const CMaterialDataFactory::SMaterial& material = cubeModel.GetMaterial(firstSurface);
#else
  const CCubeSurface firstSurface = cubeModel.GetSurface(0);
  const CCubeMaterial material = cubeModel.GetMaterial(firstSurface);
#endif
  uint displayListSize = 0;
#if RSTL_CORRUPTION
  const uint vertexStride = CDisplayListReader::GetVertexStride(material.mHeader.mVertexDesc);
  const uint outputStride = vertexStride + mMatrixIndexAttributes.size();
#else
  const uint vertexStride = CDisplayListReader::GetVertexStride(material.GetVertexDesc());
  const uint outputStride = vertexStride + 1;
#endif
  int maxVertexIndex = 0;
  union {
    ushort value;
    uchar bytes[2];
  } shortValue;
#if RSTL_CORRUPTION
  for (int i = 0; i < cubeModel.GetModelInstance().Surfaces().size(); ++i) {
    const CCubeSurface surface(cubeModel.GetModelInstance().Surfaces()[i]);
#else
  for (int i = 0; i < cubeModel.GetSurfaceCount(); ++i) {
    const CCubeSurface surface(cubeModel.GetSurface(i));
#endif
    const uchar* source = static_cast< const uchar* >(surface.GetDisplayList());
    const uchar* end = source + surface.GetDisplayListSize();
    const uchar* cursor = source;
    while (cursor < end) {
      if (*cursor == 0) {
        break;
      }
      shortValue.bytes[0] = cursor[1];
      shortValue.bytes[1] = cursor[2];
      const int count = CBasics::SwapBytes(shortValue.value);
      displayListSize += 3;
      displayListSize += outputStride * count;
      cursor += 3;
      for (int j = 0; j < count; ++j) {
        shortValue.bytes[0] = cursor[0];
        shortValue.bytes[1] = cursor[1];
        const ushort index = CBasics::SwapBytes(shortValue.value);
        if (index > maxVertexIndex) {
          maxVertexIndex = index;
        }
        cursor += vertexStride;
      }
    }
  }

  rstl::vector< uchar > segmentMatrixIndices(maxVertexIndex + 1);
  mPositions = static_cast< CVector3f* >(
#if RSTL_CORRUPTION
      rs_aligned_new_line_in("CFoldySurface.cpp", 105, (maxVertexIndex + 1) * sizeof(CVector3f)));
#else
      CMemory::Alloc((maxVertexIndex + 1) * sizeof(CVector3f), IAllocator::kHI_RoundUpLen));
#endif
  mVertexCount = maxVertexIndex + 1;
  const CVector3f* positions =
      static_cast< const CVector3f* >(cubeModel.GetModelInstance().GetVertexPointer());
  for (int i = 0; i < maxVertexIndex + 1; ++i) {
    const CVector3f position = positions[i];
    uchar segment;
    if (position.GetX() < lowerX) {
      mPositions.get()[i] = position + lowerOffset;
      segment = kS_LowerX * 3;
    } else if (position.GetX() < upperX) {
      mPositions.get()[i] = position + middleOffset;
      segment = kS_MiddleX * 3;
    } else {
      mPositions.get()[i] = position + upperOffset;
      segment = kS_UpperX * 3;
    }
    segmentMatrixIndices[i] = segment;
  }
  DCFlushRange(mPositions.get(), mVertexCount * sizeof(CVector3f));

#if RSTL_CORRUPTION
  uint alignedSize = (displayListSize + 31) & ~31;
#else
  size_t alignedSize = (displayListSize + 31) & ~31;
#endif
  for (int list = 0; list < kS_Count; ++list) {
    rstl::single_ptr< uchar > data(
#if RSTL_CORRUPTION
        static_cast< uchar* >(rs_aligned_new_line_in("CFoldySurface.cpp", 148, alignedSize)));
#else
        static_cast< uchar* >(CMemory::Alloc(alignedSize, IAllocator::kHI_RoundUpLen)));
#endif
    uchar* output = data.get();
#if RSTL_CORRUPTION
    for (int i = 0; i < cubeModel.GetModelInstance().Surfaces().size(); ++i) {
      const CCubeSurface surface(cubeModel.GetModelInstance().Surfaces()[i]);
#else
    for (int i = 0; i < cubeModel.GetSurfaceCount(); ++i) {
      const CCubeSurface surface(cubeModel.GetSurface(i));
#endif
      const uchar* const source = static_cast< const uchar* >(surface.GetDisplayList());
      const size_t size = surface.GetDisplayListSize();
      const uchar* cursor = source;
      while (cursor < source + size) {
        if (*cursor == 0) {
          break;
        }
        output[0] = cursor[0];
        output[1] = cursor[1];
        output[2] = cursor[2];
        output += 3;
        shortValue.bytes[0] = cursor[1];
        shortValue.bytes[1] = cursor[2];
        const int count = CBasics::SwapBytes(shortValue.value);
        cursor += 3;
        for (int j = 0; j < count; ++j) {
          shortValue.bytes[0] = cursor[0];
          shortValue.bytes[1] = cursor[1];
          const ushort index = CBasics::SwapBytes(shortValue.value);
#if RSTL_CORRUPTION
          uchar matrixIndex = GX_PNMTX1 + list * kS_Count * 3 + segmentMatrixIndices[index];
          for (int attr = 0; attr < mMatrixIndexAttributes.size(); ++attr) {
            output[attr] = matrixIndex;
          }
          memcpy(output + mMatrixIndexAttributes.size(), cursor, vertexStride);
#else
          output[0] = GX_PNMTX1 + list * kS_Count * 3 + segmentMatrixIndices[index];
          memcpy(output + 1, cursor, vertexStride);
#endif
          cursor += vertexStride;
          output += outputStride;
        }
      }
      DCInvalidateRange(const_cast< uchar* >(source), size);
    }
    uchar* const dataStart = data.get();
    for (uchar* dataEnd = dataStart + alignedSize; output < dataEnd; ++output) {
      *output = 0;
    }
    DCFlushRange(dataStart, alignedSize);
    mDisplayLists[list] = data.release();
  }
  mDisplayListSize = alignedSize;
}

CFoldySurface::~CFoldySurface() {
  CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_NextFrame,
                                        mPositions.release());
  CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_NextFrame,
#if RSTL_CORRUPTION
                                        mDisplayLists[kS_LowerX].release());
#else
                                        mDisplayLists[0].release());
#endif
  CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_NextFrame,
#if RSTL_CORRUPTION
                                        mDisplayLists[kS_MiddleX].release());
#else
                                        mDisplayLists[1].release());
#endif
  CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_NextFrame,
#if RSTL_CORRUPTION
                                        mDisplayLists[kS_UpperX].release());
#else
                                        mDisplayLists[2].release());
#endif
}

void CFoldySurface::SetSegmentTransforms(int matrixGroup, const CTransform4f& lower,
                                         const CTransform4f& middle,
                                         const CTransform4f& upper) const {
  Mtx lowerModelView;
  Mtx middleModelView;
  Mtx upperModelView;
  PSMTXConcat(CGraphics::GetCameraMtx(), lower.GetCStyleMatrix(), lowerModelView);
  PSMTXConcat(CGraphics::GetCameraMtx(), middle.GetCStyleMatrix(), middleModelView);
  PSMTXConcat(CGraphics::GetCameraMtx(), upper.GetCStyleMatrix(), upperModelView);
  const int firstMatrix = matrixGroup * kS_Count * 3;
#if RSTL_CORRUPTION
  CGX::LoadPosMtxImm(lowerModelView, firstMatrix + GX_PNMTX1);
  CGX::LoadPosMtxImm(middleModelView, firstMatrix + GX_PNMTX2);
  CGX::LoadPosMtxImm(upperModelView, firstMatrix + GX_PNMTX3);
#else
  GXLoadPosMtxImm(lowerModelView, firstMatrix + GX_PNMTX1);
  GXLoadPosMtxImm(middleModelView, firstMatrix + GX_PNMTX2);
  GXLoadPosMtxImm(upperModelView, firstMatrix + GX_PNMTX3);
#endif
}

void CFoldySurface::DrawDisplayList(int index) const {
  CGX::CallDisplayList(mDisplayLists[index].get(), mDisplayListSize);
}

void CFoldySurface::SetMaterialCurrent(const CModelFlags& flags) const {
  mModel.GetObject()->PreDrawModel(flags);
  mModel.GetObject()->Touch(0);
  const CCubeModel& cubeModel = *mModel.GetObject()->GetModelInstance();
#if RSTL_CORRUPTION
  const CCubeSurface surface(cubeModel.GetModelInstance().Surfaces().front());
  CGraphics::SetModelMatrix(CTransform4f::Identity());
  CCubeMaterial::DrawMaterialSurface(CModelFlags(flags, flags.GetOtherFlags() | 0x1800), surface,
                                     cubeModel);
#else
  const CCubeSurface surface(cubeModel.GetSurface(0));
  cubeModel.GetMaterial(surface).SetCurrent(flags, surface, cubeModel);
#endif
  cubeModel.SetArraysCurrent();
  if (mVertexCount != 0) {
#if RSTL_CORRUPTION
    CGX::SetArray(GX_VA_POS, mPositions.get(), sizeof(CVector3f));
#else
    CGX_SET_ARRAY(GX_VA_POS, mPositions.get(), sizeof(CVector3f), mVertexCount * sizeof(CVector3f), true);
#endif
  }
#if RSTL_CORRUPTION
  for (int i = 0; i < mMatrixIndexAttributes.size(); ++i) {
    CGX::SetVtxDesc(mMatrixIndexAttributes[i], GX_DIRECT);
  }
#else
  CGX::SetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
#endif
}

void CFoldySurface::ResetRenderState() const {
#if RSTL_CORRUPTION
  for (int i = 0; i < mMatrixIndexAttributes.size(); ++i) {
    CGX::SetVtxDesc(mMatrixIndexAttributes[i], GX_NONE);
#else
  CGX::SetVtxDesc(GX_VA_PNMTXIDX, GX_NONE);
  GXSetCurrentMtx(GX_PNMTX0);
#endif
}
#if RSTL_CORRUPTION
  GXSetCurrentMtx(GX_PNMTX0);
  CCubeMaterial::EnsureTevsDirect();
}

void CFoldySurface::BuildMatrixIndexAttributes() {
  mMatrixIndexAttributes.clear();
  mMatrixIndexAttributes.push_back(GX_VA_PNMTXIDX);
  CCubeMaterial::ResetCachedMaterials();
  const CCubeModel& model = *mModel.GetObject()->GetModelInstance();
  const CCubeSurface surface(model.GetModelInstance().Surfaces().front());
  CMaterialDrawStateGuard guard(CModelFlags::Normal(), surface, model);
  const uchar count = CGX::GetNumTexGens();
  for (int i = 0; i < count; ++i) {
    GXTexGenSrc source;
    int matrixIndex;
    CGX::GetTexCoordGen(static_cast< GXTexCoordID >(i), nullptr, &source,
                                      &matrixIndex, nullptr, nullptr);
    if (source == GX_TG_POS && matrixIndex == 0) {
      mMatrixIndexAttributes.push_back(static_cast< GXAttr >(GX_VA_TEX0MTXIDX + i));
    }
  }
}
#endif
