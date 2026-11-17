#include "WorldFormat/CAreaOctTree.hpp"

#include "rstl/RstlVersions.h"

#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#if RSTL_PRIME1 && TARGET_LITTLE_ENDIAN
#include "rstl/auto_ptr.hpp"
#endif

static CAABox BoxFromIndex(int index, const CVector3f& min, const CVector3f& center,
                           const CVector3f& max) {
  switch (index) {
  case 0:
    return CAABox(min, center);
  case 1:
    return CAABox(CVector3f(center.GetX(), min.GetY(), min.GetZ()),
                  CVector3f(max.GetX(), center.GetY(), center.GetZ()));
  case 2:
    return CAABox(CVector3f(min.GetX(), center.GetY(), min.GetZ()),
                  CVector3f(center.GetX(), max.GetY(), center.GetZ()));
  case 3:
    return CAABox(CVector3f(center.GetX(), center.GetY(), min.GetZ()),
                  CVector3f(max.GetX(), max.GetY(), center.GetZ()));
  case 4:
    return CAABox(CVector3f(min.GetX(), min.GetY(), center.GetZ()),
                  CVector3f(center.GetX(), center.GetY(), max.GetZ()));
  case 5:
    return CAABox(CVector3f(center.GetX(), min.GetY(), center.GetZ()),
                  CVector3f(max.GetX(), center.GetY(), max.GetZ()));
  case 6:
    return CAABox(CVector3f(min.GetX(), center.GetY(), center.GetZ()),
                  CVector3f(center.GetX(), max.GetY(), max.GetZ()));
  case 7:
    return CAABox(center, max);
  default:
    return CAABox(min, max);
  }
}

CAreaOctTree::Node CAreaOctTree::Node::GetChild(int index) const {
  ETreeType type = GetChildType(index);
  const uint* offsets = reinterpret_cast< const uint* >(mPtr + sizeof(uint));
  const void* node = mPtr + 9 * sizeof(uint) + CBasics::SwapBytes(offsets[index]);
  if (type == kTT_Leaf) {
#if RSTL_PRIME1 && TARGET_LITTLE_ENDIAN
    CMemoryInStream in(node, sizeof(CAABox));
    CAABox bounds(in);
#else
    CAABox bounds = *reinterpret_cast< const CAABox* >(node);
#endif
    return Node(node, bounds, GetOwner(), type);
  }
  const CVector3f center = 0.5f * (mAabb.GetMinPoint() + mAabb.GetMaxPoint());
  CAABox bounds = BoxFromIndex(index, mAabb.GetMinPoint(), center, mAabb.GetMaxPoint());
  return Node(node, bounds, GetOwner(), type);
}

CAreaOctTree::TriListReference CAreaOctTree::Node::GetTriangleArray() const {
#if RSTL_PRIME1 && NONMATCHING
  static const ushort skDeadArray[sizeof(CAABox) / sizeof(ushort) + 1] = {0};
#else
  static const ushort skDeadArray[2] = {0, 0};
#endif
  if (GetTreeType() != kTT_Leaf) {
    return TriListReference(skDeadArray);
  }
  return TriListReference(mPtr);
}

#if RSTL_PRIME1
CAreaOctTree::CAreaOctTree(const CAABox& bounds, Node::ETreeType treeType, uchar* buffer,
                           void* treeBuffer, uint materialCount, uint* materials,
                           uchar* vertexMaterials, uchar* edgeMaterials, uchar* triMaterials,
                           uint edgeCount, CCollisionEdge* edges, uint triCount, ushort* triangles,
                           uint vertexCount, CVector3f* vertices)
: mAabb(bounds)
, mTreeType(treeType)
, mBuf(buffer)
, mTreeBuf(treeBuffer)
, mMatCount(materialCount)
, mMaterials(materials)
, mVertMats(vertexMaterials)
, mEdgeMats(edgeMaterials)
, mPolyMats(triMaterials)
, mEdgeCount(edgeCount)
, mEdges(edges)
, mPolyCount(triCount)
, mPolyEdges(triangles)
, mVertCount(vertexCount)
, mVerts(vertices) {}
#else
CAreaOctTree::CAreaOctTree(const CAABox& bounds, Node::ETreeType treeType, const uchar* buffer,
                           const void* treeBuffer, int materialCount, int vertexCount,
                           int edgeCount, int triangleCount, const u64* materials,
                           const uchar* vertexMaterials, const uchar* edgeMaterials,
                           const uchar* surfaceMaterials, const CCollisionEdge* edges,
                           const ushort* surfaceIndices, const ushort* extraIndices,
                           const CVector3f* vertices)
: CCollisionPrimitiveData(materialCount, vertexCount, edgeCount, triangleCount, materials,
                          vertexMaterials, edgeMaterials, surfaceMaterials, edges, surfaceIndices,
                          extraIndices, vertices, false)
, mAabb(bounds)
, mTreeType(treeType)
, mBuf(buffer)
, mTreeBuf(treeBuffer) {}
#endif

#if RSTL_PRIME1 && TARGET_LITTLE_ENDIAN
template < typename T >
static void ReadCollisionArray(rstl::vector< T >& out, CInputStream& in) {
  const uint count = in.ReadLong();
  out.reserve(count);
  for (uint i = 0; i < count; ++i) {
    out.push_back(in.Get< T >());
  }
}
#endif

void CAreaOctTree::MakeFromMemory(void* buffer, const uint bufferLength, CAreaOctTree** treeOut,
                                  bool* valid) {
  CMemoryInStream in(buffer, bufferLength, CMemoryInStream::kOS_NotOwned);
#if RSTL_PRIME1
  in.ReadLong();
  in.ReadLong();
  CAABox bounds(in);
  Node::ETreeType treeType = static_cast< Node::ETreeType >(in.ReadLong());
  uint treeSize = in.ReadLong();
#else
  in.ReadInt32();
  in.ReadInt32();
  CAABox bounds(in);
  const int treeTypeValue = in.ReadInt32();
  uint treeSize = in.ReadInt32();
  Node::ETreeType treeType = static_cast< Node::ETreeType >(treeTypeValue);
#endif

  uchar* treeBuffer = static_cast< uchar* >(buffer) + in.GetReadPosition();
#if RSTL_PRIME1 && TARGET_LITTLE_ENDIAN
  // The accessors expose native references and arrays. Decode these separately;
  // the packed resource does not guarantee alignment for every array element.
  CMemoryInStream data(treeBuffer + treeSize, bufferLength - in.GetReadPosition() - treeSize);
  rstl::auto_ptr< CAreaOctTree > tree(
      rs_new CAreaOctTree(bounds, treeType, static_cast< uchar* >(buffer), treeBuffer, 0, nullptr,
                          nullptr, nullptr, nullptr, 0, nullptr, 0, nullptr, 0, nullptr));
  ReadCollisionArray(tree->mNativeMaterials, data);
  ReadCollisionArray(tree->mNativeVertexMaterials, data);
  ReadCollisionArray(tree->mNativeEdgeMaterials, data);
  ReadCollisionArray(tree->mNativeTriangleMaterials, data);
  ReadCollisionArray(tree->mNativeEdges, data);
  ReadCollisionArray(tree->mNativeTriangleEdges, data);
  ReadCollisionArray(tree->mNativeVertices, data);

  tree->mMatCount = tree->mNativeMaterials.size();
  tree->mMaterials = tree->mNativeMaterials.data();
  tree->mVertMats = tree->mNativeVertexMaterials.data();
  tree->mEdgeMats = tree->mNativeEdgeMaterials.data();
  tree->mPolyMats = tree->mNativeTriangleMaterials.data();
  tree->mEdgeCount = tree->mNativeEdges.size();
  tree->mEdges = tree->mNativeEdges.data();
  tree->mPolyCount = tree->mNativeTriangleEdges.size() / 3;
  tree->mPolyEdges = tree->mNativeTriangleEdges.data();
  tree->mVertCount = tree->mNativeVertices.size();
  tree->mVerts = tree->mNativeVertices.data();
  *treeOut = tree.release();
#else
  uint* materialHeader = reinterpret_cast< uint* >(treeBuffer + treeSize);
  uint materialCount = *materialHeader;
#if RSTL_PRIME1
  uint* materials = materialHeader + 1;
  uint* vertexMaterialHeader = materials + materialCount;
#else
  u64* materials = reinterpret_cast< u64* >(materialHeader + 1);
  uint* vertexMaterialHeader = reinterpret_cast< uint* >(materials + materialCount);
#endif
  uchar* vertexMaterials = reinterpret_cast< uchar* >(vertexMaterialHeader + 1);
  uint* edgeMaterialHeader = reinterpret_cast< uint* >(vertexMaterials + *vertexMaterialHeader);
  uchar* edgeMaterials = reinterpret_cast< uchar* >(edgeMaterialHeader + 1);
  uint* surfaceMaterialHeader = reinterpret_cast< uint* >(edgeMaterials + *edgeMaterialHeader);
  uchar* surfaceMaterials = reinterpret_cast< uchar* >(surfaceMaterialHeader + 1);

  uint* edgeHeader = reinterpret_cast< uint* >(surfaceMaterials + *surfaceMaterialHeader);
  uint edgeCount = *edgeHeader;
  CCollisionEdge* edges = reinterpret_cast< CCollisionEdge* >(edgeHeader + 1);
  uint* surfaceHeader = reinterpret_cast< uint* >(edges + edgeCount);
  uint triangleCount = *surfaceHeader / 3;
  ushort* surfaceIndices = reinterpret_cast< ushort* >(surfaceHeader + 1);
#if RSTL_PRIME1
  uint* vertexHeader = reinterpret_cast< uint* >(surfaceIndices + triangleCount * 3);
#else
  uint* extraIndexHeader = reinterpret_cast< uint* >(surfaceIndices + triangleCount * 3);
  ushort* extraIndices = reinterpret_cast< ushort* >(extraIndexHeader + 1);
  uint* vertexHeader = reinterpret_cast< uint* >(extraIndices + triangleCount * 3);
#endif
  uint vertexCount = *vertexHeader;
  CVector3f* vertices = reinterpret_cast< CVector3f* >(vertexHeader + 1);

  *treeOut = rs_new CAreaOctTree(bounds, treeType, static_cast< uchar* >(buffer), treeBuffer,
                                 materialCount,
#if RSTL_PRIME1
                                 materials, vertexMaterials, edgeMaterials, surfaceMaterials,
                                 edgeCount, edges, triangleCount, surfaceIndices, vertexCount,
                                 vertices);
#else
                                 vertexCount, edgeCount, triangleCount, materials,
                                 vertexMaterials, edgeMaterials, surfaceMaterials, edges,
                                 surfaceIndices, extraIndices, vertices);
#endif
#endif
  *valid = true;
}

#if RSTL_PRIME1

CCollisionSurface CAreaOctTree::GetMasterListTriangle(ushort idx) const {
  int start = idx * 3;
  ushort edgeIndex0 = mPolyEdges[start];
  ushort edgeIndex1 = mPolyEdges[start + 1];
  const CCollisionEdge& edge0 = mEdges[edgeIndex0];
  const CCollisionEdge& edge1 = mEdges[edgeIndex1];
  ushort vert2 = edge1.GetVertIndex1() != edge0.GetVertIndex1() &&
                         edge1.GetVertIndex1() != edge0.GetVertIndex2()
                     ? edge1.GetVertIndex1()
                     : edge1.GetVertIndex2();
  uint material = mMaterials[mPolyMats[idx]];
  if ((material & 0x2000000) != 0) {
    const CVector3f& thirdVertex = mVerts[vert2];
    return CCollisionSurface(mVerts[edge0.GetVertIndex2()], mVerts[edge0.GetVertIndex1()],
                             thirdVertex, material);
  }
  const CVector3f& thirdVertex = mVerts[vert2];
  return CCollisionSurface(mVerts[edge0.GetVertIndex1()], mVerts[edge0.GetVertIndex2()],
                           thirdVertex, material);
}

void CAreaOctTree::GetTriangleVertexIndices(ushort idx, ushort indicesOut[3]) const {
  int start = idx * 3;
  ushort edgeIndex0 = mPolyEdges[start];
  ushort edgeIndex1 = mPolyEdges[start + 1];
  const CCollisionEdge& edge0 = mEdges[edgeIndex0];
  const CCollisionEdge& edge1 = mEdges[edgeIndex1];
  indicesOut[2] = edge1.GetVertIndex1() != edge0.GetVertIndex1() &&
                          edge1.GetVertIndex1() != edge0.GetVertIndex2()
                      ? edge1.GetVertIndex1()
                      : edge1.GetVertIndex2();
  uint material = mMaterials[mPolyMats[idx]];
  if ((material & 0x2000000) != 0) {
    indicesOut[0] = edge0.GetVertIndex2();
    indicesOut[1] = edge0.GetVertIndex1();
  } else {
    indicesOut[0] = edge0.GetVertIndex1();
    indicesOut[1] = edge0.GetVertIndex2();
  }
}

const ushort* CAreaOctTree::GetTriangleEdgeIndices(ushort idx) const {
  return &mPolyEdges[idx * 3];
}
#endif
