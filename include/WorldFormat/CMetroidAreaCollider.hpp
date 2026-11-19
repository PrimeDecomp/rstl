#ifndef _CMETROIDAREACOLLIDER
#define _CMETROIDAREACOLLIDER

#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"
#if RSTL_PRIME1
#include "rstl/types.h"
#else
#include "Kyoto/TGameTypes.hpp"
#endif

#include "WorldFormat/CAreaOctTree.hpp"

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CMaterialList.hpp"

#include "Kyoto/Math/CAABox.hpp"
#if RSTL_PRIME1
#include "Kyoto/Math/CLineSeg.hpp"
#endif
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CSphere.hpp"
#include "Kyoto/Math/CVector3d.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "rstl/reserved_vector.hpp"

class CCollisionCache;
class CCollisionCacheWriter;

class CAABoxAreaCache {
public:
  friend class CMetroidAreaCollider;

  CAABoxAreaCache(const CAABox& aabb, const CPlane* pl, const CMaterialFilter& filter,
                  const CMaterialList& material, CCollisionInfoList& collisionList);

private:
  const CAABox& mAabb;
  const CPlane* mPlanes;
  const CMaterialFilter& mFilter;
  const CMaterialList& mMaterial;
  CCollisionInfoList& mCollisionList;
  CVector3f mCenter;
  CVector3f mHalfExtent;
};

CHECK_SIZEOF(CAABoxAreaCache, 0x2c)

class CBooleanAABoxAreaCache {
public:
  friend class CMetroidAreaCollider;

  CBooleanAABoxAreaCache(const CAABox& aabb, const CMaterialFilter& filter);

private:
  const CAABox& mAabb;
  const CMaterialFilter& mFilter;
  CVector3f mCenter;
  CVector3f mHalfExtent;
};

CHECK_SIZEOF(CBooleanAABoxAreaCache, 0x20)

class CSphereAreaCache {
public:
  friend class CMetroidAreaCollider;

  CSphereAreaCache(const CAABox& aabb, const CSphere& sphere, const CMaterialFilter& filter,
                   const CMaterialList& material, CCollisionInfoList& collisionList)
  : mAabb(aabb)
  , mSphere(sphere)
  , mFilter(filter)
  , mMaterial(material)
  , mCollisionList(collisionList) {}

private:
  const CAABox& mAabb;
  const CSphere& mSphere;
  const CMaterialFilter& mFilter;
  const CMaterialList& mMaterial;
  CCollisionInfoList& mCollisionList;
};

CHECK_SIZEOF(CSphereAreaCache, 0x14)

class CBooleanSphereAreaCache {
public:
  friend class CMetroidAreaCollider;

  CBooleanSphereAreaCache(const CAABox& aabb, const CSphere& sphere, const CMaterialFilter& filter)
  : mAabb(aabb), mSphere(sphere), mFilter(filter) {}

private:
  const CAABox& mAabb;
  const CSphere& mSphere;
  const CMaterialFilter& mFilter;
};

CHECK_SIZEOF(CBooleanSphereAreaCache, 0xc)

class CMetroidAreaCollider {
public:
  struct SBoxEdge {
#if RSTL_PRIME1
    CLineSeg mSeg;
#endif
    CVector3d mStart;
    CVector3d mEnd;
    CVector3d mDelta;
    CVector3d mCoDir;
    double mDirCoDirDot;
#if RSTL_PRIME1
    SBoxEdge(const CAABox& aabb, int idx, const CVector3f& dir);
#else
    uint mDominantAxis; // Guessed name
    SBoxEdge();
#endif
  };

  class CMovingAABoxComponents {
  public:
    CMovingAABoxComponents(const CAABox& aabb, const CVector3f& dir);

  private:
    friend class CMetroidAreaCollider;
    friend class CCollidableOBBTree;
    rstl::reserved_vector< SBoxEdge, 12 > mEdges;
    rstl::reserved_vector< uint, 8 > mVertIdxs;
    CAABox mAabb;
  };

  class COctreeLeafCache {
  public:
#if RSTL_PRIME1
    COctreeLeafCache(const CAreaOctTree& octTree);
#else
    COctreeLeafCache(const CAreaOctTree& octTree, int areaId);
#endif
    void AddLeaf(const CAreaOctTree::Node& node);
    const CAreaOctTree::Node& GetLeaf(int i) const { return mNodeCache[i]; }
    int GetNumLeaves() const { return mNodeCache.size(); }
    bool HasCacheOverflowed() const { return mOverflow; }
#if !RSTL_PRIME1
    TAreaId GetAreaId() const { return mAreaId; } // Guessed name
#endif
    const CAreaOctTree& GetOctTree() const { return mOctTree; }
    rstl::reserved_vector< CAreaOctTree::Node, 64 >::const_iterator begin() const {
      return mNodeCache.begin();
    }
    rstl::reserved_vector< CAreaOctTree::Node, 64 >::const_iterator end() const {
      return mNodeCache.end();
    }

  private:
    friend class CMetroidAreaCollider;
#if !RSTL_PRIME1
    TAreaId mAreaId;
#endif
    const CAreaOctTree& mOctTree;
    rstl::reserved_vector< CAreaOctTree::Node, 64 > mNodeCache;
    bool mOverflow : 1;
  };

  static bool ConvexPolyCollision(const CPlane* planes, const CVector3f* verts, CAABox& aabb);
  static void BuildOctreeLeafCache(const CAreaOctTree::Node& node, const CAABox& aabb,
                                   COctreeLeafCache& leafCache);
  static bool AABoxCollisionCheckBoolean(const CAreaOctTree& octTree, const CAABox& aabb,
                                         const CMaterialFilter& filter);
  static bool SphereCollisionCheckBoolean(const CAreaOctTree& octTree, const CAABox& aabb,
                                          const CSphere& sphere, const CMaterialFilter& filter);
  static bool AABoxCollisionCheckBoolean_Cached(const COctreeLeafCache& leafCache,
                                                const CAABox& aabb, const CMaterialFilter& filter);
  static bool SphereCollisionCheckBoolean_Cached(const COctreeLeafCache& leafCache,
                                                 const CAABox& aabb, const CSphere& sphere,
                                                 const CMaterialFilter& filter);
  static bool AABoxCollisionCheck(const CAreaOctTree& octTree, const CAABox& aabb,
                                  const CMaterialFilter& filter, const CMaterialList& matList,
                                  CCollisionInfoList& list);
  static bool SphereCollisionCheck(const CAreaOctTree& octTree, const CAABox& aabb,
                                   const CSphere& sphere, const CMaterialList& matList,
                                   const CMaterialFilter& filter, CCollisionInfoList& list);
  static bool AABoxCollisionCheck_Cached(const COctreeLeafCache& leafCache, const CAABox& aabb,
                                         const CMaterialFilter& filter,
                                         const CMaterialList& matList, CCollisionInfoList& list);
  static bool SphereCollisionCheck_Cached(const COctreeLeafCache& leafCache, const CAABox& aabb,
                                          const CSphere& sphere, const CMaterialList& matList,
                                          const CMaterialFilter& filter, CCollisionInfoList& list);
  static bool MovingAABoxCollisionCheck_Cached(const COctreeLeafCache& leafCache,
                                               const CAABox& aabb, const CMaterialFilter& filter,
                                               const CMaterialList& matList, CVector3f dir, float d,
                                               CCollisionInfo& infoOut, double& dOut);
  static bool MovingSphereCollisionCheck_Cached(const COctreeLeafCache& leafCache,
                                                const CAABox& aabb, const CSphere& sphere,
                                                const CMaterialFilter& filter,
                                                const CMaterialList& matList, CVector3f dir,
                                                float d, CCollisionInfo& infoOut, double& dOut);

#if !RSTL_PRIME1
  // Guessed names for Echoes's packed-cache construction.
  static void BuildCollisionCache(const CAreaOctTree::Node& node, CCollisionCache& cache);
  static void CacheAllNodes(const CAreaOctTree::Node& node, CCollisionCacheWriter& writer);
  static void CacheNodes(const CAreaOctTree::Node& node, CCollisionCacheWriter& writer,
                         const CVector3f& center, const CVector3f& halfExtent);
  // Guessed name; the buffers are borrowed, not owned.
  static void SetDuplicatePrimitiveBuffers(uchar* vertices, ushort vertexCount, uchar* edges,
                                           ushort edgeCount, uchar* triangles,
                                           ushort triangleCount);
  static bool AABoxCollisionCheckBoolean_Cached(CCollisionCache& cache, const CAABox& aabb,
                                                const CMaterialFilter& filter);
  static bool SphereCollisionCheckBoolean_Cached(CCollisionCache& cache, const CAABox& aabb,
                                                 const CSphere& sphere,
                                                 const CMaterialFilter& filter);
  static bool AABoxCollisionCheck_Cached(CCollisionCache& cache, const CAABox& aabb,
                                         const CMaterialFilter& filter,
                                         const CMaterialList& matList, CCollisionInfoList& list);
  static bool SphereCollisionCheck_Cached(CCollisionCache& cache, const CAABox& aabb,
                                          const CSphere& sphere, const CMaterialList& matList,
                                          const CMaterialFilter& filter, CCollisionInfoList& list);
  static bool MovingAABoxCollisionCheck_Cached(CCollisionCache& cache, const CAABox& aabb,
                                               const CMaterialFilter& filter,
                                               const CMaterialList& matList, CVector3f dir, float d,
                                               CCollisionInfo& infoOut, double& dOut);

#endif
  static void ResetInternalCounters();
#if RSTL_PRIME1
  static ushort GetDupPrimitiveCheckCount() { return sDupPrimitiveCheckCount; }
  static ushort& DupVertexListValue(uint idx) { return sDupVertexList[idx]; }
  static ushort& DupEdgeListValue(uint idx) { return sDupEdgeList[idx]; }
  static ushort& DupTriangleListValue(uint idx) { return sDupTriangleList[idx]; }
#else
  static uchar GetDupPrimitiveCheckCount() { return sDupPrimitiveCheckCount; }
  static uchar& DupVertexListValue(uint idx) { return spDupVertexList[idx]; }
  static uchar& DupEdgeListValue(uint idx) { return spDupEdgeList[idx]; }
  static uchar& DupTriangleListValue(uint idx) { return spDupTriangleList[idx]; }
#endif

private:
  friend class CCollidableOBBTree;
#if RSTL_PRIME1
  static ushort sDupPrimitiveCheckCount;
  static ushort sDupVertexList[0x2800];
  static ushort sDupEdgeList[0x6000];
  static ushort sDupTriangleList[0x4000];
#else
  static uchar sDupPrimitiveCheckCount;
  static uchar* spDupVertexList;
  static uchar* spDupEdgeList;
  static uchar* spDupTriangleList;
  static ushort sDupVertexCount;
  static ushort sDupEdgeCount;
  static ushort sDupTriangleCount;
#endif
  static bool AABoxCollisionCheck_Internal(const CAreaOctTree::Node&, CAABoxAreaCache&);
  static bool AABoxCollisionCheckBoolean_Internal(const CAreaOctTree::Node&,
                                                  const CBooleanAABoxAreaCache&);
  static bool SphereCollisionCheck_Internal(const CAreaOctTree::Node&, CSphereAreaCache&);
  static bool SphereCollisionCheckBoolean_Internal(const CAreaOctTree::Node&,
                                                   const CBooleanSphereAreaCache&);
  static bool MovingAABoxCollisionCheck_BoxVertexTri(const CCollisionSurface&, const CAABox&,
                                                     const rstl::reserved_vector< uint, 8 >&,
                                                     CVector3f, double&, CVector3f&, CVector3f&);
  static bool MovingAABoxCollisionCheck_TriVertexBox(const CVector3f&, const CAABox&, CVector3f,
                                                     double&, CVector3f&, CVector3f&);
  static bool MovingAABoxCollisionCheck_Edge(const CVector3f&, const CVector3f&,
                                             const rstl::reserved_vector< SBoxEdge, 12 >&,
#if RSTL_PRIME1
                                             CVector3f, double&, CVector3f&, CVector3f&);
#else
                                             const CVector3f&, double&, CVector3f&, CVector3f&);
#endif
};

#if RSTL_PRIME1
NESTED_CHECK_SIZEOF(CMetroidAreaCollider, SBoxEdge, 0x90)
NESTED_CHECK_SIZEOF(CMetroidAreaCollider, CMovingAABoxComponents, 0x700)
NESTED_CHECK_SIZEOF(CMetroidAreaCollider, COctreeLeafCache, 0x90c)
#else
NESTED_CHECK_SIZEOF(CMetroidAreaCollider, SBoxEdge, 0x70)

namespace rstl {
RSTL_DECLARE_TRIVIALLY_DESTRUCTIBLE(CMetroidAreaCollider::SBoxEdge)
RSTL_DECLARE_BITWISE_CONSTRUCTION(CMetroidAreaCollider::SBoxEdge)
RSTL_DECLARE_DIRECT_CONSTRUCTION(CMetroidAreaCollider::SBoxEdge)
} // namespace rstl

NESTED_CHECK_SIZEOF(CMetroidAreaCollider, CMovingAABoxComponents, 0x580)
NESTED_CHECK_SIZEOF(CMetroidAreaCollider, COctreeLeafCache, 0x910)
#endif

class CAreaCollisionCache {
public:
  CAreaCollisionCache(const CAABox& aabb);

  void ClearCache();
  const CAABox& GetCacheBounds() const { return mAabb; }
  void SetCacheBounds(const CAABox& aabb);
  void AddOctreeLeafCache(const CMetroidAreaCollider::COctreeLeafCache& leafCache);
  uint GetNumCaches() const { return mLeafCaches.size(); }
  const CMetroidAreaCollider::COctreeLeafCache& GetOctreeLeafCache(int idx) {
    return mLeafCaches[idx];
  }
  const CMetroidAreaCollider::COctreeLeafCache& GetOctreeLeafCache(int idx) const {
    return mLeafCaches[idx];
  }
  bool HasCacheOverflowed() const { return mLeafOverflow; }
  rstl::reserved_vector< CMetroidAreaCollider::COctreeLeafCache, 3 >::const_iterator begin() const {
    return mLeafCaches.begin();
  }
  rstl::reserved_vector< CMetroidAreaCollider::COctreeLeafCache, 3 >::const_iterator end() const {
    return mLeafCaches.end();
  }

private:
  CAABox mAabb;
  rstl::reserved_vector< CMetroidAreaCollider::COctreeLeafCache, 3 > mLeafCaches;
  bool mLeafOverflow : 1;
  bool mCacheOverflow : 1;
};
#if RSTL_PRIME1
CHECK_SIZEOF(CAreaCollisionCache, 0x1b44)
#else
CHECK_SIZEOF(CAreaCollisionCache, 0x1b50)
#endif

#endif // _CMETROIDAREACOLLIDER
