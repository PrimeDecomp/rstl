#ifndef _CAREAOCTTREE
#define _CAREAOCTTREE

#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "WorldFormat/CCollisionEdge.hpp"
#include "WorldFormat/CCollisionSurface.hpp"
#include "rstl/optional_object.hpp"
#include "ByteOrder.hpp"
#include "rstl/types.h"
#if TARGET_LITTLE_ENDIAN
#include "rstl/vector.hpp"
#endif
#include "WorldFormat/CCollisionPrimitiveData.hpp"

class CLine;
class CMaterialFilter;

// Prime stores the collision arrays in the tree itself; Echoes derives them from
// CCollisionPrimitiveData.
class CAreaOctTree
#if !RSTL_PRIME1
: public CCollisionPrimitiveData
#endif
{
public:
  struct SRayResult {
    CPlane mPlane;
    rstl::optional_object< CCollisionSurface > mSurface;
    float mT;

    SRayResult()
    : mPlane(
      0.f, 
#if RSTL_PRIME1
CUnitVector3f(1.f, 0.f, 0.f)
#else
      CVector3f::Right()
#endif
    )
    , mT(0.f)
    {}
  };

  class TriListReference {
  public:
    explicit TriListReference(const void* data) : mData(reinterpret_cast< const ushort* >(data)) {}
    explicit TriListReference(const ushort* data) : mData(data) {}
    const ushort GetSize() const { return BE_SWAP(mData[kTriangleCountOffset]); }
    const ushort GetAt(int index) const { return BE_SWAP(mData[index + kTriangleDataOffset]); }

  private:
    // Leaf records store bounds, then a ushort count followed by the triangle indices.
    enum {
      kTriangleCountOffset = sizeof(CAABox) / sizeof(ushort),
      kTriangleDataOffset = kTriangleCountOffset + 1,
    };

    const ushort* mData;
  };

  class Node {
  public:
    enum ETreeType { kTT_Invalid, kTT_Branch, kTT_Leaf };

    Node(const void* ptr, const CAABox& aabb, const CAreaOctTree& owner, ETreeType type)
    : mAabb(aabb)
    , mPtr(reinterpret_cast< const uchar* >(ptr))
    , mOwner(owner)
    , mNodeType(type) {}

    bool LineTest(const CLine& line, const CMaterialFilter& filter, float length) const;
    void LineTestEx(const CLine& line, const CMaterialFilter& filter, SRayResult& result,
                    float length) const;

    const CAreaOctTree& GetOwner() const { return mOwner; }
    const CAABox& GetBoundingBox() const { return mAabb; }
    ushort GetChildFlags() const {
      return CBasics::SwapBytes(*reinterpret_cast< const ushort* >(mPtr));
    }
    Node GetChild(int index) const;
    TriListReference GetTriangleArray() const;
    ETreeType GetChildType(int index) const {
      ushort flags = CBasics::SwapBytes(*reinterpret_cast< const ushort* >(mPtr));
      return ETreeType((flags >> (2 * index)) & 0x3);
    }
    ETreeType GetTreeType() const { return mNodeType; }

  private:
    CAABox mAabb;
    const uchar* mPtr;
    const CAreaOctTree& mOwner;
    ETreeType mNodeType;

    bool LineTestInternal(const CLine& line, const CMaterialFilter& filter, float lowT, float highT,
                          float maxT, const CVector3f& directionReciprocal) const;
    void LineTestExInternal(const CLine& line, const CMaterialFilter& filter, SRayResult& result,
                            float lowT, float highT, float maxT,
                            const CVector3f& directionReciprocal) const;
  };

#if RSTL_PRIME1
  CAreaOctTree(const CAABox& bounds, Node::ETreeType treeType, uchar* buffer, void* treeBuffer,
               uint materialCount, uint* materials, uchar* vertexMaterials, uchar* edgeMaterials,
               uchar* triMaterials, uint edgeCount, CCollisionEdge* edges, uint triCount,
               ushort* triangles, uint vertexCount, CVector3f* vertices);
#else
  CAreaOctTree(const CAABox& bounds, Node::ETreeType treeType, const uchar* buffer,
               const void* treeBuffer, int materialCount, int vertexCount, int edgeCount,
               int triangleCount, const u64* materials, const uchar* vertexMaterials,
               const uchar* edgeMaterials, const uchar* surfaceMaterials,
               const CCollisionEdge* edges, const ushort* surfaceIndices,
               const ushort* extraIndices, const CVector3f* vertices);
#endif
  static void MakeFromMemory(void* buffer, uint bufferLength, CAreaOctTree** treeOut, bool* valid);
#if RSTL_PRIME1
  CCollisionSurface GetMasterListTriangle(ushort index) const;
#endif
  Node GetRootNode() const {
    return Node(GetTreeMemory(), GetBoundingBox(), *this, GetTreeType());
  }
  const void* GetTreeMemory() const { return mTreeBuf; }
  const CAABox& GetBoundingBox() const { return mAabb; }
  Node::ETreeType GetTreeType() const { return mTreeType; }
#if RSTL_PRIME1

  uint GetNumVertices() const { return mVertexCount; }
  uint GetNumEdges() const { return mEdgeCount; }
  uint GetNumTriangles() const { return mTriangleCount; }
  const CVector3f& GetVert(int idx) const { return mVertices[idx]; }
  const CCollisionEdge& GetEdge(int idx) const { return mEdges[idx]; }
  uint GetVertMaterial(int idx) const { return mMaterials[mVertexMaterials[idx]]; }
  uint GetEdgeMaterial(int idx) const { return mMaterials[mEdgeMaterials[idx]]; }
  uint GetTriangleMaterial(int idx) const { return mMaterials[mSurfaceMaterials[idx]]; }
  void GetTriangleVertexIndices(ushort idx, ushort indicesOut[3]) const;
  const ushort* GetTriangleEdgeIndices(ushort idx) const;
#endif

private:
  CAABox mAabb;
  Node::ETreeType mTreeType;
  const uchar* mBuf;
  const void* mTreeBuf;
#if RSTL_PRIME1
  uint mMaterialCount;
  const uint* mMaterials;
  const uchar* mVertexMaterials;
  const uchar* mEdgeMaterials;
  const uchar* mSurfaceMaterials;
  uint mEdgeCount;
  const CCollisionEdge* mEdges;
  uint mTriangleCount;
  const ushort* mSurfaceIndices;
  uint mVertexCount;
  const CVector3f* mVertices;
#endif

#if TARGET_LITTLE_ENDIAN
  // TODO: maybe this is in CCollisionPrimitiveData for Echoes?
  rstl::vector< u64 > mNativeMaterials;
  rstl::vector< uchar > mNativeVertexMaterials;
  rstl::vector< uchar > mNativeEdgeMaterials;
  rstl::vector< uchar > mNativeTriangleMaterials;
  rstl::vector< CCollisionEdge > mNativeEdges;
  rstl::vector< ushort > mNativeSurfaceIndices;
  rstl::vector< ushort > mNativeExtraIndices; // For Echoes+
  rstl::vector< CVector3f > mNativeVertices;
#endif
};
#if RSTL_PRIME1
CHECK_SIZEOF(CAreaOctTree, 0x50)
#else
CHECK_CHILD_SIZEOF(CAreaOctTree, CCollisionPrimitiveData, 0x24)
#endif
NESTED_CHECK_SIZEOF(CAreaOctTree, Node, 0x24)
NESTED_CHECK_SIZEOF(CAreaOctTree, TriListReference, 0x4)
NESTED_CHECK_SIZEOF(CAreaOctTree, SRayResult, RSTL_PRIME1 ? 0x40 : 0x48)

#endif // _CAREAOCTTREE
