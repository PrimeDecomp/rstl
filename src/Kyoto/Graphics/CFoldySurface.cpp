#include "Kyoto/Graphics/CFoldySurface.hpp"

#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/Graphics/CCubeMaterial.hpp"
#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CCubeSurface.hpp"
#include "Kyoto/Graphics/CDisplayListReader.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"

#include "rstl/vector.hpp"

#include "dolphin/os.h"
#include "string.h"

CFoldySurface::CFoldySurface(const TToken< CModel >& model, float lowerX, float upperX,
                             const CVector3f& lowerOffset, const CVector3f& middleOffset,
                             const CVector3f& upperOffset)
: mModel(model, true), mPositions(nullptr), mVertexCount(0), mDisplayListSize(0) {
  const CCubeModel& cubeModel = *mModel.GetObject()->GetModelInstance();
  const CCubeSurface firstSurface = cubeModel.GetModelInstance().Surfaces().front();
  const CCubeMaterial material = cubeModel.GetMaterial(firstSurface);
  uint displayListSize = 0;
  const uint vertexStride = CDisplayListReader::GetVertexStride(material.GetVertexDesc());
  const uint outputStride = vertexStride + 1;
  int maxVertexIndex = 0;
  union {
    ushort value;
    uchar bytes[2];
  } shortValue;
  for (int i = 0; i < cubeModel.GetModelInstance().Surfaces().size(); ++i) {
    const CCubeSurface surface(cubeModel.GetModelInstance().Surfaces()[i]);
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
      CMemory::Alloc((maxVertexIndex + 1) * sizeof(CVector3f), IAllocator::kHI_RoundUpLen));
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

  size_t alignedSize = (displayListSize + 31) & ~31;
  for (int list = 0; list < kS_Count; ++list) {
    rstl::single_ptr< uchar > data(
        static_cast< uchar* >(CMemory::Alloc(alignedSize, IAllocator::kHI_RoundUpLen)));
    uchar* output = data.get();
    for (int i = 0; i < cubeModel.GetModelInstance().Surfaces().size(); ++i) {
      const CCubeSurface surface(cubeModel.GetModelInstance().Surfaces()[i]);
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
          output[0] = GX_PNMTX1 + list * kS_Count * 3 + segmentMatrixIndices[index];
          memcpy(output + 1, cursor, vertexStride);
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
                                        mDisplayLists[0].release());
  CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_NextFrame,
                                        mDisplayLists[1].release());
  CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_NextFrame,
                                        mDisplayLists[2].release());
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
  GXLoadPosMtxImm(lowerModelView, firstMatrix + GX_PNMTX1);
  GXLoadPosMtxImm(middleModelView, firstMatrix + GX_PNMTX2);
  GXLoadPosMtxImm(upperModelView, firstMatrix + GX_PNMTX3);
}

void CFoldySurface::DrawDisplayList(int index) const {
  CGX::CallDisplayList(mDisplayLists[index].get(), mDisplayListSize);
}

void CFoldySurface::SetMaterialCurrent(const CModelFlags& flags) const {
  mModel.GetObject()->PreDrawModel(flags);
  mModel.GetObject()->Touch(0);
  const CCubeModel& cubeModel = *mModel.GetObject()->GetModelInstance();
  const CCubeSurface surface(cubeModel.GetModelInstance().Surfaces().front());
  cubeModel.GetMaterial(surface).SetCurrent(flags, surface, cubeModel);
  cubeModel.SetArraysCurrent();
  if (mVertexCount != 0) {
    CGX::SetArray(GX_VA_POS, mPositions.get(), sizeof(CVector3f));
  }
  CGX::SetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
}

void CFoldySurface::ResetRenderState() const {
  CGX::SetVtxDesc(GX_VA_PNMTXIDX, GX_NONE);
  GXSetCurrentMtx(GX_PNMTX0);
}

