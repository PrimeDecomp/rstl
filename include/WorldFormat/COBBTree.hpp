#ifndef _COBBTREE
#define _COBBTREE

#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"
#include "rstl/types.h"
#include "Collision/COBBox.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "WorldFormat/CCollisionEdge.hpp"
#include "WorldFormat/CCollisionPrimitiveData.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/vector.hpp"

class CInputStream;
class CAABox;
class CTransform4f;
class CCollisionSurface;

// Prime keeps the collision arrays in the tree and exposes them through accessors; Echoes derives
// from CCollisionPrimitiveData, which points at SIndexData's vectors.
class COBBTree
#if !RSTL_PRIME1
: public CCollisionPrimitiveData
#endif
{
public:
#if !RSTL_PRIME1
  enum EPreBuiltTrees {
    kPBT_UnitCube,
    kPBT_UnitSphereLow,
    kPBT_UnitSphereMedium,
    kPBT_UnitSphereHigh,
  };
#endif

  class CSimpleAllocator {
  public:
    explicit CSimpleAllocator(uint size);
    ~CSimpleAllocator();
    void* Alloc(size_t size);
    uint GetPoolMemSize() const { return mSize; }

  private:
    char* mBuffer;
    uint mSize;
    uint mOffset;
  };

  class CLeafData {
  public:
    CLeafData() {}
    explicit CLeafData(const rstl::vector< ushort >& surfaces);
    explicit CLeafData(CInputStream& in);
    const rstl::vector< ushort >& GetSurfaceVector() const { return mSurfaces; }
    uint GetMemoryUsage() const;

  private:
    rstl::vector< ushort > mSurfaces;
  };

  class CNode {
  public:
    CNode(const CTransform4f& xf, const CVector3f& extents, const CNode* left, const CNode* right,
          const CLeafData* leaf);
    explicit CNode(CInputStream& in);
    ~CNode();

    const COBBox& GetOBB() const { return mObb; }
    bool IsLeaf() const { return mIsLeaf; }
#if RSTL_PRIME1
    bool HasLeftNode() const { return mLeft != nullptr; }
    bool HasRightNode() const { return mRight != nullptr; }
    const CNode& GetLeftNode() const { return *mLeft; }
    const CNode& GetRightNode() const { return *mRight; }
    const CLeafData& GetLeafData() const { return *mLeaf; }
#else
    const CNode* GetLeftNode() const { return mLeft; }
    const CNode* GetRightNode() const { return mRight; }
    const CLeafData* GetLeafData() const { return mLeaf; }
#endif
    void SetHit(bool hit) const { mHit = hit; }
    bool WasHit() const { return mHit; }
    uint GetMemoryUsage() const;

    static void SetAllocator(CSimpleAllocator* allocator);
    void* operator new(size_t size, const char* file, int line);
#if defined(TARGET_PC)
    void* operator new(size_t size) { return operator new(size, "COBBTree", 0); }
#endif
    void operator delete(void* ptr, size_t size);
#if RSTL_PRIME1
    void operator delete(void* ptr, const char* file, int line);
#endif

  private:
    COBBox mObb;
    bool mIsLeaf;
    const CNode* mLeft;
    const CNode* mRight;
    const CLeafData* mLeaf;
    mutable bool mHit;

    static CSimpleAllocator* spAllocator;
  };

  struct SIndexData {
    SIndexData() {}
    explicit SIndexData(CInputStream& in);

#if RSTL_PRIME1
    rstl::vector< uint > mMaterials;
#else
    rstl::vector< u64 > mMaterials;
#endif
    rstl::vector< uchar > mVertMaterials;
    rstl::vector< uchar > mEdgeMaterials;
    rstl::vector< uchar > mSurfaceMaterials;
    rstl::vector< CCollisionEdge > mEdges;
    rstl::vector< ushort > mSurfaceIndices;
#if !RSTL_PRIME1
    rstl::vector< ushort > x60_; // Additional serialized index array; role unresolved.
#endif
    rstl::vector< CVector3f > mVertices;
  };

  explicit COBBTree(CInputStream& in);
  COBBTree(const SIndexData& indexData, const CNode* root);
  ~COBBTree();

#if RSTL_PRIME1
  const CNode& GetRoot() const { return *mRoot; }
  bool HasRoot() const { return mRoot != nullptr; }
  int GetNumVerts() const { return mIndexData.mVertices.size(); }

  CCollisionSurface GetSurface(ushort idx) const;
  CCollisionSurface GetTransformedSurface(const ushort idx, const CTransform4f& xf) const;
  const ushort* GetTriangleEdgeIndices(ushort idx) const;
  void GetTriangleVertexIndices(ushort idx, ushort* out) const;

  const CCollisionEdge& GetEdge(int idx) const { return mIndexData.mEdges[idx]; }
  const CVector3f& GetVert(int idx) const { return mIndexData.mVertices[idx]; }
  uint GetVertMaterial(int idx) const {
    return mIndexData.mMaterials[mIndexData.mVertMaterials[idx]];
  }
  uint GetEdgeMaterial(int idx) const {
    return mIndexData.mMaterials[mIndexData.mEdgeMaterials[idx]];
  }

  int NumSurfaceMaterials() const { return mIndexData.mSurfaceMaterials.size(); }
#else
  const CNode* GetRoot() const { return mRoot; }
  const SIndexData& GetIndexData() const { return mIndexData; }
#endif
  const CNode& GetRootNode() const { return *mRoot; }
  CAABox CalculateLocalAABox() const;

  static rstl::auto_ptr< COBBTree > BuildOrientedBoundingBoxTree(const CVector3f& extent,
                                                                 const CVector3f& center);
#if !RSTL_PRIME1
  // Guessed name; enum values follow the four resource registrations.
  static void SetPrebuiltTree(COBBTree* tree, EPreBuiltTrees which);
  static COBBTree* GetPrebuiltTree(EPreBuiltTrees which);
#endif

private:
  uint mMagic;
  uint mVersion;
  uint mMemsize;
  CSimpleAllocator mAllocator;
  SIndexData mIndexData;
  const CNode* mRoot;

#if !RSTL_PRIME1
  static COBBTree* sPrebuiltTrees[4];
#endif
};
#if RSTL_PRIME1
CHECK_SIZEOF(COBBTree, 0x8c)
#else
CHECK_CHILD_SIZEOF(COBBTree, CCollisionPrimitiveData, 0x9c)
#endif
NESTED_CHECK_SIZEOF(COBBTree, CSimpleAllocator, 0xc)
NESTED_CHECK_SIZEOF(COBBTree, CLeafData, 0x10)
NESTED_CHECK_SIZEOF(COBBTree, CNode, 0x50)
NESTED_CHECK_SIZEOF(COBBTree, SIndexData, RSTL_PRIME1 ? 0x70 : 0x80)

#endif // _COBBTREE
