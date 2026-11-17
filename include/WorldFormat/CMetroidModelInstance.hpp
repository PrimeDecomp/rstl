#ifndef _CMETROIDMODELINSTANCE
#define _CMETROIDMODELINSTANCE

#include "rstl/RstlVersions.h"

#include "rstl/static_assert.hpp"
#include "Kyoto/Graphics/CCubeMaterial.hpp"
#include "Kyoto/Graphics/CCubeSurface.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "rstl/vector.hpp"


class CMetroidModelInstance {
public:
#if RSTL_PRIME1
  typedef void* TSurface;
#else
  typedef CCubeSurface TSurface;
#endif

#if !RSTL_PRIME1
  // Reconstructed name for the borrowed serialized group-count/index table.
  class CSurfaceGroups {
  public:
    explicit CSurfaceGroups(const void* data) : mData(static_cast< const ushort* >(data)) {}
    ushort GetSurfaceCount(int group) const;
    const ushort* GetSurfaceIndices(int group) const;

  private:
    const ushort* mData;
  };
#endif

  CMetroidModelInstance(const void* header, const void* materials, const void* positions,
                        const void* normals, const void* colors, const void* texCoords,
                        const void* packedTexCoords, const rstl::vector< TSurface >& surfaces
#if !RSTL_PRIME1
                        ,
                        const void* const& section1, const void* const& section2
#endif
  );
#if RSTL_PRIME1
  ~CMetroidModelInstance() {}
#endif

  const rstl::vector< TSurface >& GetSurfaces() const { return mSurfaces; }
  const void* GetVertexPointer() const { return mPositions; }
  const void* GetMaterialPointer() const { return mMaterialData; }
  const void* GetNormalPointer() const { return mNormals; }
  const void* GetColorPointer() const { return mColors; }
  const void* GetTCPointer() const { return mTexCoords; }
  const void* GetPackedTCPointer() const { return mPackedTexCoords; }
  const CAABox& GetBoundingBox() const { return mWorldBounds; }
  int GetFlags() const { return mVisorFlags; }
#if !RSTL_PRIME1
  CSurfaceGroups GetSurfaceGroups() const { return CSurfaceGroups(mSurfaceGroups); }

  // Guessed name. Each serialized surface record is two halfwords after the header.
  ushort GetSurfaceAreaIndex(int surface) const {
    return static_cast< const ushort* >(x74_)[surface * 2 + 2];
  }
  const ushort* GetSurfaceRecords() const {
    return static_cast< const ushort* >(x74_);
  } // Guessed name

  // Guessed names. The resource stores cumulative surface counts followed by indices.
  ushort GetSurfaceCountInGroup(int group) const {
    const ushort* table = static_cast< const ushort* >(mSurfaceGroups);
    return group == 0 ? table[group + 1] : table[group + 1] - table[group];
  }
  const ushort* GetSurfaceIndices(int group) const {
    const ushort* table = static_cast< const ushort* >(mSurfaceGroups);
    const ushort start = group == 0 ? 0 : table[group];
    return table + table[0] + 1 + start;
  }

  // Guessed name. The material section starts with the texture count and texture IDs; surface
  // loops read the count once per model and pass it to GetMaterialByIndex.
  uint GetTextureCount() const {
    return CBasics::SwapBytes(*static_cast< const uint* >(mMaterialData));
  }
  CCubeMaterial GetMaterialByIndex(uint textureCount, uint index) const {
    const uchar* data = static_cast< const uchar* >(mMaterialData);
    data += (textureCount + 1) * sizeof(uint);
    const uint count = CBasics::SwapBytes(*reinterpret_cast< const uint* >(data));
    data += sizeof(uint);
    uint offset = 0;
    if (index != 0) {
      offset = CBasics::SwapBytes(reinterpret_cast< const uint* >(data)[index - 1]);
    }
    data += count * sizeof(uint);
    data += offset;
    return CCubeMaterial(data);
  }

  CCubeMaterial GetMaterialByIndex(uint index) const {
    const uint* data = static_cast< const uint* >(mMaterialData);
    const uint* offsets = data + CBasics::SwapBytes(data[0]) + 1;
    const uint count = CBasics::SwapBytes(offsets[0]);
    const uint* table = offsets + 1;
    uint offset = 0;
    if (index != 0) {
      offset = CBasics::SwapBytes(table[index - 1]);
    }
    return CCubeMaterial(reinterpret_cast< const uchar* >(table + count) + offset);
  }
#endif

private:
  int mVisorFlags;
  CTransform4f mWorldTransform;
  CAABox mWorldBounds;
  const void* mMaterialData;
  rstl::vector< TSurface > mSurfaces;
  const void* mPositions;
  const void* mNormals;
  const void* mColors;
  const void* mTexCoords;
  const void* mPackedTexCoords;
#if !RSTL_PRIME1
  const void* x74_;
  const void* mSurfaceGroups; // Guessed name.
#endif
};
#if RSTL_PRIME1
CHECK_SIZEOF(CMetroidModelInstance, 0x74)
#else
CHECK_SIZEOF(CMetroidModelInstance, 0x7c)
NESTED_CHECK_SIZEOF(CMetroidModelInstance, CSurfaceGroups, 4)
#endif


#endif // _CMETROIDMODELINSTANCE
