#include "rstl/RstlVersions.h"
#include "WorldFormat/CCollidableOBBTree.hpp"

#include "Collision/CCollisionInfo.hpp"
#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CInternalRayCastStructure.hpp"
#include "Collision/CMRay.hpp"
#include "Collision/COBBox.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Math/CSphere.hpp"
#if !RSTL_PRIME1
#include "WorldFormat/CCollisionCache.hpp"
#endif
#include "WorldFormat/CCollisionSurface.hpp"

#include "rstl/locked_cache_allocator_impl.hpp"
#include <math.h>

#if !RSTL_PRIME1
static CPlane TransformPlane(const CPlane& plane, const CTransform4f& xf);
#endif

uint CCollidableOBBTree::sTableIndex = -1;

#if RSTL_PRIME1
CCollidableOBBTree::CCollidableOBBTree(COBBTree* tree, const CMaterialList& list)
: CCollisionPrimitive(list), mTree(tree), mTries(0), mMisses(0), mHits(0) {}
#else
CCollidableOBBTree::CCollidableOBBTree(COBBTree* tree, const CMaterialList& material)
: CCollisionPrimitive(material), mTree(tree), mTries(0), mMisses(0), mHits(0) {}

CCollidableOBBTree::~CCollidableOBBTree() {}
#endif

CAABox CCollidableOBBTree::CalculateAABox(const CTransform4f& xf) const {
  COBBox obb = COBBox::FromAABox(GetOBBTree().CalculateLocalAABox(), xf);
  return obb.CalculateAABox(CTransform4f::Identity());
}

CAABox CCollidableOBBTree::CalculateLocalAABox() const {
  return GetOBBTree().CalculateLocalAABox();
}

FourCC CCollidableOBBTree::GetPrimType() const { return 'OBBT'; }

bool CCollidableOBBTree::AABoxCollision(const COBBTree::CNode& node, const CTransform4f& xf,
#if RSTL_PRIME1
                                        const CAABox& aabb, const COBBox& obb,
#else
                                        const CAABox& box, const COBBox& obb,
#endif
                                        const CMaterialList& material,
                                        const CMaterialFilter& filter, const CPlane* planes,
                                        CCollisionInfoList& infoList) const {
#if RSTL_PRIME1
  bool ret = false;

  mTries += 1;
#else
  bool hit = false;
  ++mTries;
#endif
  if (obb.OBBIntersectsBox(node.GetOBB())) {
    node.SetHit(true);
    if (node.IsLeaf()) {
#if RSTL_PRIME1
      if (AABoxCollideWithLeaf(node.GetLeafData(), xf, aabb, material, filter, planes, infoList))
        ret = true;
    } else {
      if (AABoxCollision(node.GetLeftNode(), xf, aabb, obb, material, filter, planes, infoList))
        ret = true;
      if (AABoxCollision(node.GetRightNode(), xf, aabb, obb, material, filter, planes, infoList))
        ret = true;
#else
      if (AABoxCollideWithLeaf(*node.GetLeafData(), xf, box, material, filter, planes, infoList)) {
        hit = true;
#endif
      }
    } else {
#if RSTL_PRIME1
    mMisses += 1;
#else
      if (node.GetLeftNode() &&
          AABoxCollision(*node.GetLeftNode(), xf, box, obb, material, filter, planes, infoList)) {
        hit = true;
#endif
      }
#if RSTL_PRIME1

  return ret;
#else
      if (node.GetRightNode() &&
          AABoxCollision(*node.GetRightNode(), xf, box, obb, material, filter, planes, infoList)) {
        hit = true;
      }
    }
  } else {
    ++mMisses;
  }
  return hit;
#endif
}

bool CCollidableOBBTree::AABoxCollideWithLeaf(const COBBTree::CLeafData& leaf,
                                              const CTransform4f& xf, const CAABox& aabb,
                                              const CMaterialList& material,
                                              const CMaterialFilter& filter, const CPlane* planes,
                                              CCollisionInfoList& infoList) const {
  CVector3f center = aabb.GetCenterPoint();
#if RSTL_PRIME1
  CVector3f extent = aabb.GetHalfExtent();
#else
  CVector3f extent = (aabb.GetMaxPoint() - aabb.GetMinPoint()) * 0.5f;
#endif

  int surfCount = leaf.GetSurfaceVector().size();
  bool ret = false;
  for (int i = 0; i < surfCount; ++i) {
#if RSTL_PRIME1
    CCollisionSurface surf = GetOBBTree().GetTransformedSurface(leaf.GetSurfaceVector()[i], xf);
    const CMaterialList& baseMat = GetMaterial();
    CMaterialList triMat(static_cast< u64 >(surf.GetSurfaceFlags()) | baseMat.GetValue());
#else
    CCollisionSurface surf = GetOBBTree().GetTriangle(leaf.GetSurfaceVector()[i], &xf);
    CMaterialList triMat(surf.GetSurfaceFlags());
#endif
    if (filter.Passes(triMat) && CollisionUtil::TriBoxOverlap(center, extent, surf.GetVert(0),
                                                              surf.GetVert(1), surf.GetVert(2))) {
      mHits += 1;
      CAABox newAABB = CAABox::MakeMaxInvertedBox();
      if (CMetroidAreaCollider::ConvexPolyCollision(planes, &surf.GetVert(0), newAABB)) {
        CPlane plane = surf.GetPlane();
#if RSTL_PRIME1
        infoList.Add(
            CCollisionInfo(newAABB, triMat, material, plane.GetNormal(), -plane.GetNormal()),
            false);
#else
        CCollisionInfo info(newAABB, material,
                            CMaterialList(triMat.GetValue() | GetMaterial().GetValue()),
                            plane.GetNormal(), -plane.GetNormal(), -1);
        infoList.Add(info);
#endif
        ret = true;
      }
    }
  }

  return ret;
}

bool CCollidableOBBTree::SphereCollision(const COBBTree::CNode& node, const CTransform4f& xf,
                                         const CSphere& sphere, const COBBox& obb,
                                         const CMaterialList& material,
                                         const CMaterialFilter& filter,
                                         CCollisionInfoList& infoList) const {
#if RSTL_PRIME1
  bool ret = false;
  mTries += 1;
#else
  bool hit = false;
  ++mTries;
#endif
  if (obb.OBBIntersectsBox(node.GetOBB())) {
    node.SetHit(true);
    if (node.IsLeaf()) {
#if RSTL_PRIME1
      if (SphereCollideWithLeaf(node.GetLeafData(), xf, sphere, material, filter, infoList))
        ret = true;
    } else {
      if (SphereCollision(node.GetLeftNode(), xf, sphere, obb, material, filter, infoList))
        ret = true;
      if (SphereCollision(node.GetRightNode(), xf, sphere, obb, material, filter, infoList))
        ret = true;
#else
      if (SphereCollideWithLeaf(*node.GetLeafData(), xf, sphere, material, filter, infoList)) {
        hit = true;
#endif
      }
    } else {
#if RSTL_PRIME1
    mMisses += 1;
#else
      if (node.GetLeftNode() &&
          SphereCollision(*node.GetLeftNode(), xf, sphere, obb, material, filter, infoList)) {
        hit = true;
#endif
      }
#if RSTL_PRIME1
  return ret;
#else
      if (node.GetRightNode() &&
          SphereCollision(*node.GetRightNode(), xf, sphere, obb, material, filter, infoList)) {
        hit = true;
      }
    }
  } else {
    ++mMisses;
  }
  return hit;
#endif
}

bool CCollidableOBBTree::SphereCollideWithLeaf(const COBBTree::CLeafData& leaf,
                                               const CTransform4f& xf, const CSphere& sphere,
                                               const CMaterialList& material,
                                               const CMaterialFilter& filter,
                                               CCollisionInfoList& infoList) const {
  bool ret = false;
  CVector3f point = CVector3f::Zero();
  CVector3f normal = CVector3f::Zero();

  int surfCount = leaf.GetSurfaceVector().size();
  for (int i = 0; i < surfCount; ++i) {
#if RSTL_PRIME1
    CCollisionSurface surf = GetOBBTree().GetTransformedSurface(leaf.GetSurfaceVector()[i], xf);
    const CMaterialList& baseMat = GetMaterial();
    CMaterialList triMat(static_cast< u64 >(surf.GetSurfaceFlags()) | baseMat.GetValue());
#else
    CCollisionSurface surf = GetOBBTree().GetTriangle(leaf.GetSurfaceVector()[i], &xf);
    CMaterialList triMat(surf.GetSurfaceFlags());
#endif
    if (filter.Passes(triMat)) {
      mHits += 1;
      if (CollisionUtil::TriSphereIntersection(sphere, surf.GetVert(0), surf.GetVert(1),
                                               surf.GetVert(2), point, normal)) {
#if RSTL_PRIME1
        infoList.Add(CCollisionInfo(point, material, triMat, normal), false);
#else
        infoList.Add(CCollisionInfo(point, material,
                                    CMaterialList(triMat.GetValue() | GetMaterial().GetValue()),
                                    normal, -1));
#endif
        ret = true;
      }
    }
  }

  return ret;
}

bool CCollidableOBBTree::AABoxCollisionBoolean(const COBBTree::CNode& node, const CTransform4f& xf,
                                               const CAABox& aabb, const COBBox& obb,
                                               const CMaterialFilter& filter) const {
  CVector3f center = aabb.GetCenterPoint();
#if RSTL_PRIME1
  CVector3f extent = aabb.GetHalfExtent();
#else
  CVector3f extent = (aabb.GetMaxPoint() - aabb.GetMinPoint()) * 0.5f;
#endif

  mTries += 1;
  if (obb.OBBIntersectsBox(node.GetOBB())) {
    node.SetHit(true);
    if (node.IsLeaf()) {
#if RSTL_PRIME1
      const COBBTree::CLeafData& leaf = node.GetLeafData();
#else
      const COBBTree::CLeafData& leaf = *node.GetLeafData();
#endif
      int surfCount = leaf.GetSurfaceVector().size();
      for (int i = 0; i < surfCount; ++i) {
#if RSTL_PRIME1
        CCollisionSurface surf = GetOBBTree().GetTransformedSurface(leaf.GetSurfaceVector()[i], xf);
        const CMaterialList& baseMat = GetMaterial();
        CMaterialList triMat(static_cast< u64 >(surf.GetSurfaceFlags()) | baseMat.GetValue());
#else
        CCollisionSurface surf = GetOBBTree().GetTriangle(leaf.GetSurfaceVector()[i], &xf);
        CMaterialList triMat(surf.GetSurfaceFlags());
#endif
        if (filter.Passes(triMat) &&
            CollisionUtil::TriBoxOverlap(center, extent, surf.GetVert(0), surf.GetVert(1),
                                         surf.GetVert(2))) {
          return true;
        }
      }
    } else {
#if RSTL_PRIME1
      if (AABoxCollisionBoolean(node.GetLeftNode(), xf, aabb, obb, filter))
#else
      if (node.GetLeftNode() && AABoxCollisionBoolean(*node.GetLeftNode(), xf, aabb, obb, filter))
#endif
        return true;
#if RSTL_PRIME1
      if (AABoxCollisionBoolean(node.GetRightNode(), xf, aabb, obb, filter))
#else
      if (node.GetRightNode() && AABoxCollisionBoolean(*node.GetRightNode(), xf, aabb, obb, filter))
#endif
        return true;
    }
  } else {
    mMisses += 1;
  }

  return false;
}

bool CCollidableOBBTree::SphereCollisionBoolean(const COBBTree::CNode& node, const CTransform4f& xf,
                                                const CSphere& sphere, const COBBox& obb,
                                                const CMaterialFilter& filter) const {
  mTries += 1;
  if (obb.OBBIntersectsBox(node.GetOBB())) {
    node.SetHit(true);
    if (node.IsLeaf()) {
#if RSTL_PRIME1
      const COBBTree::CLeafData& leaf = node.GetLeafData();
#else
      const COBBTree::CLeafData& leaf = *node.GetLeafData();
#endif
      int surfCount = leaf.GetSurfaceVector().size();
      for (int i = 0; i < surfCount; ++i) {
#if RSTL_PRIME1
        CCollisionSurface surf = GetOBBTree().GetTransformedSurface(leaf.GetSurfaceVector()[i], xf);
        const CMaterialList& baseMat = GetMaterial();
        CMaterialList triMat(static_cast< u64 >(surf.GetSurfaceFlags()) | baseMat.GetValue());
#else
        CCollisionSurface surf = GetOBBTree().GetTriangle(leaf.GetSurfaceVector()[i], &xf);
        CMaterialList triMat(surf.GetSurfaceFlags());
#endif
        if (filter.Passes(triMat) &&
            CollisionUtil::TriSphereOverlap(sphere, surf.GetVert(0), surf.GetVert(1),
                                            surf.GetVert(2))) {
          return true;
        }
      }
    } else {
#if RSTL_PRIME1
      if (SphereCollisionBoolean(node.GetLeftNode(), xf, sphere, obb, filter))
#else
      if (node.GetLeftNode() &&
          SphereCollisionBoolean(*node.GetLeftNode(), xf, sphere, obb, filter))
#endif
        return true;
#if RSTL_PRIME1
      if (SphereCollisionBoolean(node.GetRightNode(), xf, sphere, obb, filter))
#else
      if (node.GetRightNode() &&
          SphereCollisionBoolean(*node.GetRightNode(), xf, sphere, obb, filter))
#endif
        return true;
    }
  } else {
    mMisses += 1;
  }

  return false;
}

bool CCollidableOBBTree::AABoxCollisionMoving(
#if RSTL_PRIME1
    const COBBTree::CNode& node, const CTransform4f& xf, const CAABox& aabb, const COBBox& obb,
#else
    const COBBTree::CNode& node, const CTransform4f& xf, const CAABox& box, const COBBox& obb,
#endif
    const CMaterialList& material, const CMaterialFilter& filter,
#if RSTL_PRIME1
    const CMetroidAreaCollider::CMovingAABoxComponents& components, const CVector3f& dir,
    double& dOut, CCollisionInfo& info) const {
  bool ret = false;
  mTries += 1;
#else
    const CMetroidAreaCollider::CMovingAABoxComponents& components, const CVector3f& direction,
    double& time, CCollisionInfo& info) const {
  bool hit = false;
  ++mTries;
#endif
  if (obb.OBBIntersectsBox(node.GetOBB())) {
    node.SetHit(true);
    if (node.IsLeaf()) {
#if RSTL_PRIME1
      if (AABoxCollideWithLeafMoving(node.GetLeafData(), xf, aabb, material, filter, components,
                                     dir, dOut, info))
        ret = true;
    } else {
      if (AABoxCollisionMoving(node.GetLeftNode(), xf, aabb, obb, material, filter, components, dir,
                               dOut, info))
        ret = true;
      if (AABoxCollisionMoving(node.GetRightNode(), xf, aabb, obb, material, filter, components,
                               dir, dOut, info))
        ret = true;
#else
      if (AABoxCollideWithLeafMoving(*node.GetLeafData(), xf, box, material, filter, components,
                                     direction, time, info)) {
        hit = true;
#endif
      }
    } else {
#if RSTL_PRIME1
    mMisses += 1;
#else
      if (node.GetLeftNode() && AABoxCollisionMoving(*node.GetLeftNode(), xf, box, obb, material,
                                                     filter, components, direction, time, info)) {
        hit = true;
#endif
      }
#if RSTL_PRIME1
  return ret;
#else
      if (node.GetRightNode() && AABoxCollisionMoving(*node.GetRightNode(), xf, box, obb, material,
                                                      filter, components, direction, time, info)) {
        hit = true;
      }
    }
  } else {
    ++mMisses;
  }
  return hit;
#endif
}

bool CCollidableOBBTree::AABoxCollideWithLeafMoving(
    const COBBTree::CLeafData& leaf, const CTransform4f& xf, const CAABox& aabb,
    const CMaterialList& material, const CMaterialFilter& filter,
    const CMetroidAreaCollider::CMovingAABoxComponents& components, const CVector3f& dir,
    double& dOut, CCollisionInfo& info) const {
  CVector3f normal(CVector3f::Zero());
  CVector3f point(CVector3f::Zero());

  CAABox movedAABB = components.mAabb;
  CVector3f moveVec = static_cast< float >(dOut) * dir;
  movedAABB.AccumulateBounds(aabb.GetMaxPoint() + moveVec);
  movedAABB.AccumulateBounds(aabb.GetMinPoint() + moveVec);

  CVector3f center = movedAABB.GetCenterPoint();
  bool ret = false;
#if RSTL_PRIME1
  CVector3f extent = movedAABB.GetHalfExtent();
#else
  CVector3f extent = (movedAABB.GetMaxPoint() - movedAABB.GetMinPoint()) * 0.5f;
#endif

  int surfCount = leaf.GetSurfaceVector().size();
  for (int i = 0; i < surfCount; ++i) {
    int triIdx = leaf.GetSurfaceVector()[i];
#if RSTL_PRIME1
    CCollisionSurface surf = GetOBBTree().GetTransformedSurface(triIdx, xf);
    const CMaterialList& baseMat = GetMaterial();
    CMaterialList triMat(static_cast< u64 >(surf.GetSurfaceFlags()) | baseMat.GetValue());
#else
    CCollisionSurface surf = GetOBBTree().GetTriangle(triIdx, &xf);
    CMaterialList triMat(surf.GetSurfaceFlags());
#endif
    if (filter.Passes(triMat)) {
      if (CollisionUtil::TriBoxOverlap(center, extent, surf.GetVert(0), surf.GetVert(1),
                                       surf.GetVert(2))) {
        mHits += 1;

        ushort vertIndices[3];
        GetOBBTree().GetTriangleVertexIndices(triIdx, vertIndices);

        double d = dOut;
        if (CMetroidAreaCollider::MovingAABoxCollisionCheck_BoxVertexTri(
                surf, aabb, components.mVertIdxs, dir, d, normal, point) &&
            d < dOut) {
#if RSTL_PRIME1
          info = CCollisionInfo(point, material, triMat, normal);
#else
          info = CCollisionInfo(point, material,
                                CMaterialList(triMat.GetValue() | GetMaterial().GetValue()), normal,
                                -1);
#endif
          ret = true;
          dOut = d;
        }

        for (int k = 0; k < 3; ++k) {
          uint vertIdx = vertIndices[k];
#if RSTL_PRIME1
          if (CMetroidAreaCollider::DupVertexListValue(vertIdx) !=
#else
          u64 vertMatVal = GetOBBTree().GetVertMaterial(vertIdx);
          if (!(vertMatVal & (u64(1) << kMT_NoEdgeCollision)) &&
              CMetroidAreaCollider::DupVertexListValue(vertIdx) !=
#endif
                  CMetroidAreaCollider::GetDupPrimitiveCheckCount()) {
            CMetroidAreaCollider::DupVertexListValue(vertIdx) =
                CMetroidAreaCollider::GetDupPrimitiveCheckCount();
            if (movedAABB.PointInside(surf.GetVert(k))) {
              d = dOut;
              if (CMetroidAreaCollider::MovingAABoxCollisionCheck_TriVertexBox(
                      surf.GetVert(k), aabb, dir, d, normal, point) &&
                  d < dOut) {
                info = CCollisionInfo(point, material,
#if RSTL_PRIME1
                                      CMaterialList(GetOBBTree().GetVertMaterial(vertIdx)), normal);
#else
                                      CMaterialList(vertMatVal | GetMaterial().GetValue()), normal,
                                      -1);
#endif
                ret = true;
                dOut = d;
              }
            }
          }
        }

        const ushort* edgeIndices = GetOBBTree().GetTriangleEdgeIndices(triIdx);
        for (int k = 0; k < 3; ++k) {
          uint edgeIdx = edgeIndices[k];
          if (CMetroidAreaCollider::DupEdgeListValue(edgeIdx) !=
              CMetroidAreaCollider::GetDupPrimitiveCheckCount()) {
            CMetroidAreaCollider::DupEdgeListValue(edgeIdx) =
                CMetroidAreaCollider::GetDupPrimitiveCheckCount();
#if RSTL_PRIME1
            uint edgeMatVal = GetOBBTree().GetEdgeMaterial(edgeIdx);
            if (!(edgeMatVal & (1u << kMT_NoEdgeCollision))) {
#else
            u64 edgeMatVal = GetOBBTree().GetEdgeMaterial(edgeIdx);
            if (!(edgeMatVal & (u64(1) << kMT_NoEdgeCollision))) {
#endif
              int nextVert = k == 2 ? 0 : k + 1;
              d = dOut;
              if (CMetroidAreaCollider::MovingAABoxCollisionCheck_Edge(
                      surf.GetVert(k), surf.GetVert(nextVert), components.mEdges, dir, d, normal,
                      point) &&
                  d < dOut) {
#if RSTL_PRIME1
                info = CCollisionInfo(point, material, CMaterialList(edgeMatVal), normal);
#else
                info = CCollisionInfo(point, material,
                                      CMaterialList(edgeMatVal | GetMaterial().GetValue()), normal,
                                      -1);
#endif
                ret = true;
                dOut = d;
              }
            }
          }
        }
      } else {
        const ushort* edgeIndices = GetOBBTree().GetTriangleEdgeIndices(triIdx);
        CMetroidAreaCollider::DupEdgeListValue(edgeIndices[0]) =
            CMetroidAreaCollider::GetDupPrimitiveCheckCount();
        CMetroidAreaCollider::DupEdgeListValue(edgeIndices[1]) =
            CMetroidAreaCollider::GetDupPrimitiveCheckCount();
        CMetroidAreaCollider::DupEdgeListValue(edgeIndices[2]) =
            CMetroidAreaCollider::GetDupPrimitiveCheckCount();

        ushort vertIndices[3];
        GetOBBTree().GetTriangleVertexIndices(triIdx, vertIndices);
        CMetroidAreaCollider::DupVertexListValue(vertIndices[0]) =
            CMetroidAreaCollider::GetDupPrimitiveCheckCount();
        CMetroidAreaCollider::DupVertexListValue(vertIndices[1]) =
            CMetroidAreaCollider::GetDupPrimitiveCheckCount();
        CMetroidAreaCollider::DupVertexListValue(vertIndices[2]) =
            CMetroidAreaCollider::GetDupPrimitiveCheckCount();
      }
    }
  }

  return ret;
}

#if !RSTL_PRIME1
bool CCollidableOBBTree::CacheTree(CCollisionCacheWriter& writer, const COBBTree::CNode& node,
                                   const CTransform4f& xf, const CVector3f& center,
                                   const CVector3f& halfExtent, const COBBox& obb) const {
  ++mTries;
  if (obb.OBBIntersectsBox(node.GetOBB())) {
    node.SetHit(true);
    if (node.IsLeaf()) {
      writer.BeginLeaf(CAABox::MakeMaxInvertedBox());
      const COBBTree::CLeafData& leaf = *node.GetLeafData();
      int count = leaf.GetSurfaceVector().size();
      writer.ReserveTriangles(count);
      for (int i = 0; i < count; ++i) {
        int index = leaf.GetSurfaceVector()[i];
        CCollisionSurface surface = GetOBBTree().GetTriangle(index, &xf);
        if (CollisionUtil::TriBoxOverlap(center, halfExtent, surface.GetVert(0), surface.GetVert(1),
                                         surface.GetVert(2))) {
          writer.AddTriangle(surface, index);
        }
      }
    } else {
      if (node.GetLeftNode()) {
        CacheTree(writer, *node.GetLeftNode(), xf, center, halfExtent, obb);
      }
      if (node.GetRightNode()) {
        CacheTree(writer, *node.GetRightNode(), xf, center, halfExtent, obb);
      }
    }
  } else {
    ++mMisses;
  }
  return false;
}

void CCollidableOBBTree::CacheSphere(CCollisionCache& cache, const CTransform4f& xf, ushort ownerId,
                                     u64 material) {
  float scale = xf.GetRight().Magnitude();
  COBBTree* tree;
  if (scale < 2.f) {
    tree = COBBTree::GetPrebuiltTree(COBBTree::kPBT_UnitSphereLow);
  } else if (scale < 5.f) {
    tree = COBBTree::GetPrebuiltTree(COBBTree::kPBT_UnitSphereMedium);
  } else {
    tree = COBBTree::GetPrebuiltTree(COBBTree::kPBT_UnitSphereHigh);
  }

  CCollidableOBBTree primitive(tree, CMaterialList(material));
  float invScale = 1.f / scale;
  CVector3f center = cache.GetBounds().GetCenterPoint();
  COBBox obb(CTransform4f::Translate((center - xf.GetTranslation()) * invScale),
             (cache.GetBounds().GetMaxPoint() - center) * invScale);
  CVector3f worldCenter = cache.GetBounds().GetCenterPoint();
  CVector3f halfExtent = (cache.GetBounds().GetMaxPoint() - cache.GetBounds().GetMinPoint()) * 0.5f;
  CCollisionCacheWriter writer(cache);
  writer.BeginGeometry(*tree, &xf, ownerId, material);
  primitive.CacheTree(writer, *tree->GetRoot(), xf, worldCenter, halfExtent, obb);
}

void CCollidableOBBTree::CacheAABox(CCollisionCache& cache, const CTransform4f& xf, ushort ownerId,
                                    u64 material) {
  COBBTree* tree = COBBTree::GetPrebuiltTree(COBBTree::kPBT_UnitCube);
  CCollisionCacheWriter writer(cache);
  writer.BeginGeometry(*tree, &xf, ownerId, material);
  writer.ReserveTriangles(tree->GetTriangleCount());
  CVector3f center = cache.GetBounds().GetCenterPoint();
  CVector3f halfExtent = (cache.GetBounds().GetMaxPoint() - cache.GetBounds().GetMinPoint()) * 0.5f;
  for (ushort i = 0; i < tree->GetTriangleCount(); ++i) {
    CCollisionSurface surface = tree->GetTriangle(i, &xf);
    if (CollisionUtil::TriBoxOverlap(center, halfExtent, surface.GetVert(0), surface.GetVert(1),
                                     surface.GetVert(2))) {
      writer.AddTriangle(surface, i);
    }
  }
}

#endif
bool CCollidableOBBTree::SphereCollisionMoving(const COBBTree::CNode& node, const CTransform4f& xf,
                                               const CSphere& sphere, const COBBox& obb,
                                               const CMaterialList& material,
#if RSTL_PRIME1
                                               const CMaterialFilter& filter, const CVector3f& dir,
                                               double& dOut, CCollisionInfo& info) const {
  bool ret = false;
  mTries += 1;
#else
                                               const CMaterialFilter& filter,
                                               const CVector3f& direction, double& time,
                                               CCollisionInfo& info) const {
  bool hit = false;
  ++mTries;
#endif
  if (obb.OBBIntersectsBox(node.GetOBB())) {
    node.SetHit(true);
    if (node.IsLeaf()) {
#if RSTL_PRIME1
      if (SphereCollideWithLeafMoving(node.GetLeafData(), xf, sphere, material, filter, dir, dOut,
                                      info))
        ret = true;
    } else {
      if (SphereCollisionMoving(node.GetLeftNode(), xf, sphere, obb, material, filter, dir, dOut,
                                info))
        ret = true;
      if (SphereCollisionMoving(node.GetRightNode(), xf, sphere, obb, material, filter, dir, dOut,
                                info))
        ret = true;
#else
      if (SphereCollideWithLeafMoving(*node.GetLeafData(), xf, sphere, material, filter, direction,
                                      time, info)) {
        hit = true;
#endif
      }
    } else {
#if RSTL_PRIME1
    mMisses += 1;
#else
      if (node.GetLeftNode() && SphereCollisionMoving(*node.GetLeftNode(), xf, sphere, obb,
                                                      material, filter, direction, time, info)) {
        hit = true;
#endif
      }
#if RSTL_PRIME1
  return ret;
#else
      if (node.GetRightNode() && SphereCollisionMoving(*node.GetRightNode(), xf, sphere, obb,
                                                       material, filter, direction, time, info)) {
        hit = true;
      }
    }
  } else {
    ++mMisses;
  }
  return hit;
#endif
}

static inline CVector3f TriangleEdgeNormal(const CVector3f& normal, const CVector3f& edge) {
#if RSTL_PRIME1
  const float nX = normal.GetX();
  const float nY = normal.GetY();
  const float nZ = normal.GetZ();
  const float eX = edge.GetX();
  const float eY = edge.GetY();
  const float eZ = edge.GetZ();
  return CVector3f(nY * eZ - eY * nZ, nZ * eX - eZ * nX, nX * eY - eX * nY);
#else
  return CVector3f::Cross(normal, edge);
#endif
}

bool CCollidableOBBTree::SphereCollideWithLeafMoving(const COBBTree::CLeafData& leaf,
                                                     const CTransform4f& xf, const CSphere& sphere,
                                                     const CMaterialList& material,
                                                     const CMaterialFilter& filter,
                                                     const CVector3f& dir, double& dOut,
                                                     CCollisionInfo& info) const {
  static int mod3[4] = {0, 1, 2, 0};

  float radius = sphere.GetRadius();
  CVector3f radiusVec(radius, radius, radius);
  CAABox aabb(sphere.GetCenter() - radiusVec, sphere.GetCenter() + radiusVec);

#if RSTL_PRIME1
  CVector3f moveVec = CCast::ToReal32(dOut) * dir;
#else
  CVector3f moveVec = static_cast< float >(dOut) * dir;
#endif
  CAABox moveAABB = aabb;
  moveAABB.AccumulateBounds(moveAABB.GetMaxPoint() + moveVec);
  moveAABB.AccumulateBounds(aabb.GetMinPoint() + moveVec);

  CVector3f boxCenter = moveAABB.GetCenterPoint();
  bool ret = false;
#if RSTL_PRIME1
  CVector3f extent = moveAABB.GetHalfExtent();
#else
  CVector3f extent = (moveAABB.GetMaxPoint() - moveAABB.GetMinPoint()) * 0.5f;
#endif

  int surfCount = leaf.GetSurfaceVector().size();
  for (int i = 0; i < surfCount; ++i) {
#if RSTL_PRIME1
    int triIdx = leaf.GetSurfaceVector()[i];
    CCollisionSurface surf = GetOBBTree().GetTransformedSurface(triIdx, xf);
    const CMaterialList& baseMat = GetMaterial();
    CMaterialList triMat(static_cast< u64 >(surf.GetSurfaceFlags()) | baseMat.GetValue());
#else
    ushort triIdx = leaf.GetSurfaceVector()[i];
    CCollisionSurface surf = GetOBBTree().GetTriangle(triIdx, &xf);
    CMaterialList triMat(surf.GetSurfaceFlags());
#endif
    if (filter.Passes(triMat)) {
      if (CollisionUtil::TriBoxOverlap(boxCenter, extent, surf.GetVert(0), surf.GetVert(1),
                                       surf.GetVert(2))) {
        mHits += 1;

        CVector3f surfNormal = surf.GetNormal();
        CVector3f toMovedSphere = sphere.GetCenter() + moveVec - surf.GetVert(0);
        if (!(CVector3f::Dot(toMovedSphere, surfNormal) > sphere.GetRadius())) {
          double mag =
              sphere.GetRadius() - CVector3f::Dot(sphere.GetCenter() - surf.GetVert(0), surfNormal);
          mag /= CVector3f::Dot(dir, surfNormal);

          CVector3f intersectPoint = sphere.GetCenter() + static_cast< float >(mag) * dir;
          bool outsideEdges[3];
          outsideEdges[0] =
              CVector3f::Dot(intersectPoint - surf.GetVert(0),
                             TriangleEdgeNormal(surfNormal, surf.GetVert(1) - surf.GetVert(0))) <
              0.f;
          outsideEdges[1] =
              CVector3f::Dot(intersectPoint - surf.GetVert(1),
                             TriangleEdgeNormal(surfNormal, surf.GetVert(2) - surf.GetVert(1))) <
              0.f;
          outsideEdges[2] =
              CVector3f::Dot(intersectPoint - surf.GetVert(2),
                             TriangleEdgeNormal(surfNormal, surf.GetVert(0) - surf.GetVert(2))) <
              0.f;

          if (mag >= 0.0 && !outsideEdges[0] && !outsideEdges[1] && !outsideEdges[2] &&
              mag < dOut) {
#if RSTL_PRIME1
            const CVector3f& collisionPoint = intersectPoint - sphere.GetRadius() * surfNormal;
            info = CCollisionInfo(collisionPoint, material, triMat, surfNormal);
#else
            info = CCollisionInfo(intersectPoint - sphere.GetRadius() * surfNormal, material,
                                  CMaterialList(triMat.GetValue() | GetMaterial().GetValue()),
                                  surfNormal, -1);
#endif
            ret = true;
            dOut = mag;
          }

          const CVector3f& vts2 = sphere.GetCenter() - surf.GetVert(0);
          bool intersects = CVector3f::Dot(vts2, surfNormal) <= sphere.GetRadius();
          bool testVert[3] = {true, true, true};
          const ushort* edgeIndices = GetOBBTree().GetTriangleEdgeIndices(triIdx);
          for (int k = 0; k < 3; ++k) {
            if (intersects || outsideEdges[k]) {
              uint edgeIdx = edgeIndices[k];
              if (CMetroidAreaCollider::DupEdgeListValue(edgeIdx) !=
                  CMetroidAreaCollider::GetDupPrimitiveCheckCount()) {
                CMetroidAreaCollider::DupEdgeListValue(edgeIdx) =
                    CMetroidAreaCollider::GetDupPrimitiveCheckCount();
#if RSTL_PRIME1
                uint edgeMatVal = GetOBBTree().GetEdgeMaterial(edgeIdx);
                if (!(edgeMatVal & (1u << kMT_NoEdgeCollision))) {
#else
                u64 edgeMatVal = GetOBBTree().GetEdgeMaterial(edgeIdx);
                if (!(edgeMatVal & (u64(1) << kMT_NoEdgeCollision))) {
#endif
                  CVector3f edgeVec = surf.GetVert(mod3[k + 1]) - surf.GetVert(k);
                  float edgeVecMag = edgeVec.Magnitude();
                  edgeVec *= 1.f / edgeVecMag;

                  CVector3f vertToSphere = sphere.GetCenter() - surf.GetVert(k);
                  float vtsDotEdge = CVector3f::Dot(vertToSphere, edgeVec);
                  float dirDotEdge = CVector3f::Dot(dir, edgeVec);
                  CVector3f vtsRej = vertToSphere - vtsDotEdge * edgeVec;
                  CVector3f edgeRej = dir - dirDotEdge * edgeVec;
                  float edgeRejMagSq = edgeRej.MagSquared();

                  if (edgeRejMagSq > 0.f) {
                    float b = 2.f * CVector3f::Dot(vtsRej, edgeRej);
                    float discriminant =
                        b * b - 4.f * edgeRejMagSq *
                                    (vtsRej.MagSquared() - sphere.GetRadius() * sphere.GetRadius());
                    if (discriminant >= 0.f) {
                      double inverse = 0.5 / edgeRejMagSq;
#if RSTL_PRIME1
                      double mag2 = inverse * (-b - sqrt(discriminant));
#else
                      double mag2 = inverse * (-static_cast< double >(b) - sqrt(discriminant));
#endif
                      if (mag2 >= 0.0) {
                        double t = mag2 * dirDotEdge + vtsDotEdge;
                        if (t >= 0.0 && t <= edgeVecMag && mag2 < dOut) {
                          CVector3f point = surf.GetVert(k) + static_cast< float >(t) * edgeVec;
                          CVector3f normal =
                              (sphere.GetCenter() + static_cast< float >(mag2) * dir - point)
                                  .AsNormalized();
#if RSTL_PRIME1
                          info = CCollisionInfo(point, material, CMaterialList(edgeMatVal), normal);
#else
                          info = CCollisionInfo(
                              point, material, CMaterialList(edgeMatVal | GetMaterial().GetValue()),
                              normal, -1);
#endif
                          dOut = mag2;
                          ret = true;
                          testVert[k] = false;
                          testVert[mod3[k + 1]] = false;
                        } else if (t < -sphere.GetRadius() && dirDotEdge <= 0.f) {
                          testVert[k] = false;
                        } else if (t > edgeVecMag + sphere.GetRadius() && dirDotEdge >= 0.f) {
                          testVert[mod3[k + 1]] = false;
                        }
                      }
                    } else {
                      testVert[k] = false;
                      testVert[mod3[k + 1]] = false;
                    }
                  }
                }
              }
            }
          }

          ushort vertIndices[3];
          GetOBBTree().GetTriangleVertexIndices(triIdx, vertIndices);
          for (int k = 0; k < 3; ++k) {
            uint vertIdx = vertIndices[k];
            if (testVert[k]) {
#if RSTL_PRIME1
              if (CMetroidAreaCollider::DupVertexListValue(vertIdx) !=
#else
              u64 vertMatVal = GetOBBTree().GetVertMaterial(vertIdx);
              if (!(vertMatVal & (u64(1) << kMT_NoEdgeCollision)) &&
                  CMetroidAreaCollider::DupVertexListValue(vertIdx) !=
#endif
                      CMetroidAreaCollider::GetDupPrimitiveCheckCount()) {
                CMetroidAreaCollider::DupVertexListValue(vertIdx) =
                    CMetroidAreaCollider::GetDupPrimitiveCheckCount();
                double d = dOut;
                if (CollisionUtil::RaySphereIntersection_Double(
                        CSphere(surf.GetVert(k), sphere.GetRadius()), sphere.GetCenter(), dir, d) &&
                    d >= 0.0) {
#if RSTL_PRIME1
                  float dF = CCast::ToReal32(d);
#else
                  float dF = static_cast< float >(d);
#endif
                  CVector3f normal =
                      (sphere.GetCenter() + dF * dir - surf.GetVert(k)).AsNormalized();
#if RSTL_PRIME1
                  info =
                      CCollisionInfo(surf.GetVert(k), material,
                                     CMaterialList(GetOBBTree().GetVertMaterial(vertIdx)), normal);
#else
                  info = CCollisionInfo(surf.GetVert(k), material,
                                        CMaterialList(vertMatVal | GetMaterial().GetValue()),
                                        normal, -1);
#endif
                  dOut = d;
                  ret = true;
                }
              }
            } else {
              CMetroidAreaCollider::DupVertexListValue(vertIdx) =
                  CMetroidAreaCollider::GetDupPrimitiveCheckCount();
            }
          }
        }
      } else {
        const ushort* edgeIndices = GetOBBTree().GetTriangleEdgeIndices(triIdx);
        CMetroidAreaCollider::DupEdgeListValue(edgeIndices[0]) =
            CMetroidAreaCollider::GetDupPrimitiveCheckCount();
        CMetroidAreaCollider::DupEdgeListValue(edgeIndices[1]) =
            CMetroidAreaCollider::GetDupPrimitiveCheckCount();
        CMetroidAreaCollider::DupEdgeListValue(edgeIndices[2]) =
            CMetroidAreaCollider::GetDupPrimitiveCheckCount();

        ushort vertIndices[3];
        GetOBBTree().GetTriangleVertexIndices(triIdx, vertIndices);
        CMetroidAreaCollider::DupVertexListValue(vertIndices[0]) =
            CMetroidAreaCollider::GetDupPrimitiveCheckCount();
        CMetroidAreaCollider::DupVertexListValue(vertIndices[1]) =
            CMetroidAreaCollider::GetDupPrimitiveCheckCount();
        CMetroidAreaCollider::DupVertexListValue(vertIndices[2]) =
            CMetroidAreaCollider::GetDupPrimitiveCheckCount();
      }
    }
  }

  return ret;
}

CRayCastResult CCollidableOBBTree::CastRayInternal(const CInternalRayCastStructure& rayCast) const {
  return LineIntersectsTree(rayCast.GetRay(), rayCast.GetFilter(), rayCast.GetMaxTime(),
                            rayCast.GetTransform());
}

#if RSTL_PRIME1
static CPlane TransformPlane(const CPlane& pl, const CTransform4f& xf) {
  CVector3f transformed = xf * (pl.GetNormal() * pl.GetConstant());
  CUnitVector3f normal(xf.Rotate(pl.GetNormal()), CUnitVector3f::kN_No);
  return CPlane(transformed, normal);
#else
static CPlane TransformPlane(const CPlane& plane, const CTransform4f& xf) {
  const CVector3f point = xf * (plane.GetNormal() * plane.GetConstant());
  const CUnitVector3f normal(xf.Rotate(plane.GetNormal()), CUnitVector3f::kN_No);
  return CPlane(point, normal);
#endif
}
#if !RSTL_PRIME1

inline CRayCastInfo::CRayCastInfo(const CMRay& ray, const CMaterialFilter& filter, float magnitude)
: mRay(ray)
, mFilter(filter)
, mMagnitude(magnitude)
, mPlane(CVector3f::Zero(), CUnitVector3f(CVector3f(0.f, 0.f, 1.f), CUnitVector3f::kN_Yes))
, mMaterial() {}
#endif

CRayCastResult CCollidableOBBTree::LineIntersectsTree(const CMRay& ray,
                                                      const CMaterialFilter& filter, float maxTime,
                                                      const CTransform4f& xf) const {
#if RSTL_PRIME1
  CMRay useRay = ray.GetInvUnscaledTransformRay(xf);
  CRayCastInfo info(useRay, filter, maxTime);
  if (LineIntersectsOBBTree(&GetOBBTree().GetRoot(), info)) {
    CPlane xfPlane = TransformPlane(info.GetPlane(), xf);
#else
  CMRay localRay = ray.GetInvUnscaledTransformRay(xf);
  CRayCastInfo info(localRay, filter, maxTime);
  if (LineIntersectsOBBTree(GetOBBTree().GetRoot(), info)) {
    const CPlane plane = TransformPlane(info.GetPlane(), xf);
#endif
    return CRayCastResult(info.GetMagnitude(),
#if RSTL_PRIME1
                          ray.GetStart() + info.GetMagnitude() * ray.GetDirection(), xfPlane,
#else
                          ray.GetStart() + info.GetMagnitude() * ray.GetDirection(), plane,
#endif
                          info.GetMaterial());
  }
#if RSTL_PRIME1
  return CRayCastResult();
#else
  return CRayCastResult::MakeInvalid();
#endif
}

bool CCollidableOBBTree::LineIntersectsOBBTree(const COBBTree::CNode* node,
                                               CRayCastInfo& info) const {
#if RSTL_PRIME1
  float t;
#else
  if (!node) {
    return false;
  }

#endif
  bool ret = false;
#if RSTL_PRIME1

  mTries += 1;
  if (node->GetOBB().LineIntersectsBox(info.GetRay(), t) && t < info.GetMagnitude()) {
#else
  ++mTries;
  float time;
  if (node->GetOBB().LineIntersectsBox(info.GetRay(), time) && time < info.GetMagnitude()) {
#endif
    if (node->IsLeaf() == true) {
#if RSTL_PRIME1
      if (LineIntersectsLeaf(node->GetLeafData(), info) == true)
#else
      if (LineIntersectsLeaf(*node->GetLeafData(), info) == true)
#endif
        ret = true;
#if RSTL_PRIME1
    } else {
      if (LineIntersectsOBBTree(&node->GetLeftNode(), &node->GetRightNode(), info) == true)
#else
    } else if (LineIntersectsOBBTree(node->GetLeftNode(), node->GetRightNode(), info) == true) {
#endif
      ret = true;
    }
    node->SetHit(true);
  } else {
#if RSTL_PRIME1
    mMisses += 1;
#else
    ++mMisses;
#endif
  }
#if RSTL_PRIME1

#endif
  return ret;
}

bool CCollidableOBBTree::LineIntersectsOBBTree(const COBBTree::CNode* n0, const COBBTree::CNode* n1,
                                               CRayCastInfo& info) const {
  bool ret = false;
  float t0, t1;
  bool intersects0 = false;

  mTries += 2;

#if RSTL_PRIME1
  if (n0->GetOBB().LineIntersectsBox(info.GetRay(), t0) == true && t0 < info.GetMagnitude())
#else
  if (n0 && n0->GetOBB().LineIntersectsBox(info.GetRay(), t0) == true && t0 < info.GetMagnitude())
#endif
    intersects0 = true;

  bool intersects1 = false;
#if RSTL_PRIME1
  if (n1->GetOBB().LineIntersectsBox(info.GetRay(), t1) == true && t1 < info.GetMagnitude())
#else
  if (n1 && n1->GetOBB().LineIntersectsBox(info.GetRay(), t1) == true && t1 < info.GetMagnitude())
#endif
    intersects1 = true;

  if (intersects0 && intersects1) {
    if (t0 < t1) {
      if ((n0->IsLeaf() == true
#if RSTL_PRIME1
               ? LineIntersectsLeaf(n0->GetLeafData(), info)
               : LineIntersectsOBBTree(&n0->GetLeftNode(), &n0->GetRightNode(), info)) == true) {
#else
               ? LineIntersectsLeaf(*n0->GetLeafData(), info)
               : LineIntersectsOBBTree(n0->GetLeftNode(), n0->GetRightNode(), info)) == true) {
#endif
        if (info.GetMagnitude() < t1)
          return true;
        ret = true;
      }
      if (n1->IsLeaf()) {
#if RSTL_PRIME1
        if (LineIntersectsLeaf(n1->GetLeafData(), info))
#else
        if (LineIntersectsLeaf(*n1->GetLeafData(), info))
#endif
          ret = true;
      } else {
#if RSTL_PRIME1
        if (LineIntersectsOBBTree(&n1->GetLeftNode(), &n1->GetRightNode(), info) == true)
#else
        if (LineIntersectsOBBTree(n1->GetLeftNode(), n1->GetRightNode(), info) == true)
#endif
          ret = true;
      }
    } else {
      if ((n1->IsLeaf() == true
#if RSTL_PRIME1
               ? LineIntersectsLeaf(n1->GetLeafData(), info)
               : LineIntersectsOBBTree(&n1->GetLeftNode(), &n1->GetRightNode(), info)) == true) {
#else
               ? LineIntersectsLeaf(*n1->GetLeafData(), info)
               : LineIntersectsOBBTree(n1->GetLeftNode(), n1->GetRightNode(), info)) == true) {
#endif
        if (info.GetMagnitude() < t0)
          return true;
        ret = true;
      }
      if (n0->IsLeaf()) {
#if RSTL_PRIME1
        if (LineIntersectsLeaf(n0->GetLeafData(), info))
#else
        if (LineIntersectsLeaf(*n0->GetLeafData(), info))
#endif
          ret = true;
      } else {
#if RSTL_PRIME1
        if (LineIntersectsOBBTree(&n0->GetLeftNode(), &n0->GetRightNode(), info) == true)
#else
        if (LineIntersectsOBBTree(n0->GetLeftNode(), n0->GetRightNode(), info) == true)
#endif
          ret = true;
      }
    }
  } else {
    if (intersects0) {
      if (n0->IsLeaf() == true) {
#if RSTL_PRIME1
        if (LineIntersectsLeaf(n0->GetLeafData(), info))
#else
        if (LineIntersectsLeaf(*n0->GetLeafData(), info))
#endif
          return true;
      } else {
#if RSTL_PRIME1
        if (LineIntersectsOBBTree(&n0->GetLeftNode(), &n0->GetRightNode(), info) == true)
#else
        if (LineIntersectsOBBTree(n0->GetLeftNode(), n0->GetRightNode(), info) == true)
#endif
          return true;
      }
    }
    if (intersects1) {
      if (n1->IsLeaf() == true) {
#if RSTL_PRIME1
        if (LineIntersectsLeaf(n1->GetLeafData(), info))
#else
        if (LineIntersectsLeaf(*n1->GetLeafData(), info))
#endif
          return true;
      } else {
#if RSTL_PRIME1
        if (LineIntersectsOBBTree(&n1->GetLeftNode(), &n1->GetRightNode(), info) == true)
#else
        if (LineIntersectsOBBTree(n1->GetLeftNode(), n1->GetRightNode(), info) == true)
#endif
          return true;
      }
    }
  }

  return ret;
}

bool CCollidableOBBTree::LineIntersectsLeaf(const COBBTree::CLeafData& leaf,
                                            CRayCastInfo& info) const {
  ushort intersectIdx = 0;
  bool ret = false;
  int surfCount = leaf.GetSurfaceVector().size();
#if RSTL_PRIME1
  const CMaterialFilter& filter = info.mFilter;
#else
  const CMaterialFilter& filter = info.GetMaterialFilter();
#endif
  for (ushort i = 0; i < surfCount; ++i) {
#if RSTL_PRIME1
    const CCollisionSurface& surface = GetOBBTree().GetSurface(leaf.GetSurfaceVector()[i]);
    const CMaterialList& baseMat = GetMaterial();
    CMaterialList matList(static_cast< u64 >(surface.GetSurfaceFlags()) | baseMat.GetValue());
#else
    const CCollisionSurface& surface = GetOBBTree().GetTriangle(leaf.GetSurfaceVector()[i]);
    CMaterialList matList(surface.GetSurfaceFlags());
#endif
    if (filter.Passes(matList)) {
      if (CollisionUtil::RayTriangleIntersection(info.GetRay().GetStart(),
                                                 info.GetRay().GetDirection(), &surface.GetVert(0),
                                                 info.Magnitude())) {
        intersectIdx = i;
        ret = true;
      }
    }
  }

  if (ret) {
#if RSTL_PRIME1
    const CCollisionSurface& surf = GetOBBTree().GetSurface(leaf.GetSurfaceVector()[intersectIdx]);
#else
    const CCollisionSurface& surf = GetOBBTree().GetTriangle(leaf.GetSurfaceVector()[intersectIdx]);
#endif
    info.Plane() = surf.GetPlane();
    info.Material() = CMaterialList(surf.GetSurfaceFlags());
  }

  return ret;
}

uint CCollidableOBBTree::GetTableIndex() const { return sTableIndex; }
