#include "rstl/RstlVersions.h"

#include "ByteOrder.hpp"
#include "WorldFormat/CMetroidModelInstance.hpp"

#include "Kyoto/Basics/CBasics.hpp"
#if !RSTL_PRIME1
#include "Kyoto/Streams/CInputStream.hpp"
#include "WorldFormat/SAreaSurface.hpp"
#endif
#include <string.h>

#if TARGET_BIG_ENDIAN
static const CTransform4f& TransformFromData(const void* ptr) {
  return *static_cast< const CTransform4f* >(ptr);
}
#else
static CTransform4f TransformFromData(const void* ptr) {
  float values[12];
  memcpy(values, ptr, sizeof(values));
  for (int i = 0; i < 12; ++i) {
    values[i] = CBasics::SwapBytes(values[i]);
  }
  return CTransform4f(values[0], values[1], values[2], values[3], values[4], values[5], values[6],
                      values[7], values[8], values[9], values[10], values[11]);
}
#endif

static CAABox BoundingBoxFromData(const void* ptr) {
  float out[6];
  const float* tmp = reinterpret_cast< const float* >(ptr);
  for (int i = 0; i < 6; ++i) {
    out[i] = CBasics::SwapBytes(tmp[i]);
  }

#ifdef __MWERKS__
  return *reinterpret_cast< const CAABox* >(out);
#else
  return CAABox(CVector3f(out[0], out[1], out[2]), CVector3f(out[3], out[4], out[5]));
#endif
}

CMetroidModelInstance::CMetroidModelInstance(const void* header, const void* materials,
                                             const void* positions, const void* normals,
                                             const void* colors, const void* texCoords,
                                             const void* packedTexCoords,
                                             const rstl::vector< TSurface >& surfaces
#if !RSTL_PRIME1
                                             ,
                                             const void* const& section1,
                                             const void* const& section2
#endif
                                             )
#if RSTL_PRIME1
: mVisorFlags(CBasics::SwapBytes(*reinterpret_cast< const uint* >(header)))
, mWorldTransform(TransformFromData((uchar*)header + sizeof(uint)))
, mWorldBounds(BoundingBoxFromData((uchar*)header + sizeof(CTransform4f) + sizeof(uint)))
#else
: mVisorFlags(*static_cast< const uint* >(header))
, mWorldTransform(TransformFromData(static_cast< const uchar* >(header) + sizeof(uint)))
, mWorldBounds(BoundingBoxFromData(static_cast< const uchar* >(header) + sizeof(CTransform4f) +
                                   sizeof(uint)))
#endif
, mMaterialData(materials)
, mSurfaces(surfaces)
, mPositions(positions)
, mNormals(normals)
, mColors(colors)
, mTexCoords(texCoords)
#if RSTL_PRIME1
, mPackedTexCoords(packedTexCoords) {}
#else
, mPackedTexCoords(packedTexCoords)
, x74_(section1)
, mSurfaceGroups(section2) {}

const ushort* CMetroidModelInstance::CSurfaceGroups::GetSurfaceIndices(int group) const {
  const int start = group != 0 ? mData[group] : 0;
  return mData + (mData[0] + 1) + start;
}

ushort CMetroidModelInstance::CSurfaceGroups::GetSurfaceCount(int group) const {
  const ushort count = mData[group + 1];
  if (group == 0) {
    return count;
  }
  return count - mData[group];
}

SAreaSurface::SAreaSurface(CInputStream& in)
: mBounds(in)
, mModelIndex(in.ReadInt16())
, mSurfaceGroupIndex(in.ReadInt16())
, x1c_(in.ReadInt16())
, x1e_(in.ReadInt16()) {}
#endif
