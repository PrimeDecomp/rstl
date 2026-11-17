#include "Collision/CCollisionInfo.hpp"

#include "rstl/RstlVersions.h"

#include "Kyoto/Math/CAABox.hpp"

#include "rstl/algorithm.hpp"

CCollisionInfo::CCollisionInfo(const CVector3f& point, const CMaterialList& rightMaterial,
                               const CMaterialList& leftMaterial, const CVector3f& normal
#if !RSTL_PRIME1
                               ,
                               const ushort objectId
#endif
                               )
: mPoint(point)
, mExtentX(CVector3f::Zero())
, mExtentY(CVector3f::Zero())
, mExtentZ(CVector3f::Zero())
#if RSTL_PRIME1
, mValid(true)
, mHasExtents(false)
#endif
, mMaterialLeft(leftMaterial)
, mMaterialRight(rightMaterial)
, mNormalLeft(normal)
, mNormalRight(-normal)
#if !RSTL_PRIME1
, mObjectId(objectId)
, mValid(true)
, mHasExtents(false)
#endif
{
}

CCollisionInfo::CCollisionInfo(const CVector3f& point, const CMaterialList& rightMaterial,
                               const CMaterialList& leftMaterial, const CVector3f& leftNormal,
                               const CVector3f& rightNormal
#if !RSTL_PRIME1
                               ,
                               const ushort objectId
#endif
                               )
: mPoint(point)
, mExtentX(CVector3f::Zero())
, mExtentY(CVector3f::Zero())
, mExtentZ(CVector3f::Zero())
#if RSTL_PRIME1
, mValid(true)
, mHasExtents(false)
#endif
, mMaterialLeft(leftMaterial)
, mMaterialRight(rightMaterial)
, mNormalLeft(leftNormal)
, mNormalRight(rightNormal)
#if !RSTL_PRIME1
, mObjectId(objectId)
, mValid(true)
, mHasExtents(false)
#endif
{
}

CCollisionInfo::CCollisionInfo(const CAABox& box, const CMaterialList& rightMaterial,
                               const CMaterialList& leftMaterial, const CVector3f& leftNormal,
                               const CVector3f& rightNormal
#if !RSTL_PRIME1
                               ,
                               const ushort objectId
#endif
                               )
: mPoint(box.GetMinPoint())
, mExtentX(box.GetMaxPoint().GetX() - box.GetMinPoint().GetX(), 0.f, 0.f)
, mExtentY(0.f, box.GetMaxPoint().GetY() - box.GetMinPoint().GetY(), 0.f)
, mExtentZ(0.f, 0.f, box.GetMaxPoint().GetZ() - box.GetMinPoint().GetZ())
#if RSTL_PRIME1
, mValid(true)
, mHasExtents(true)
#endif
, mMaterialLeft(leftMaterial)
, mMaterialRight(rightMaterial)
, mNormalLeft(leftNormal)
, mNormalRight(rightNormal)
#if !RSTL_PRIME1
, mObjectId(objectId)
, mValid(true)
, mHasExtents(true)
#endif
{
}

CCollisionInfo CCollisionInfo::GetSwapped(ESwapMaterials swap) const {
  return CCollisionInfo(*this, swap);
}

CCollisionInfo::CCollisionInfo(EInvalid)
: mPoint(0.f, 0.f, 0.f)
, mExtentX(0.f, 0.f, 0.f)
, mExtentY(0.f, 0.f, 0.f)
, mExtentZ(0.f, 0.f, 0.f)
#if RSTL_PRIME1
, mValid(false)
, mHasExtents(false)
#endif
, mMaterialLeft(CMaterialList())
, mMaterialRight(CMaterialList())
, mNormalLeft(0.f, 0.f, 0.f)
, mNormalRight(0.f, 0.f, 0.f)
#if !RSTL_PRIME1
, mObjectId(0xffff)
, mValid(false)
, mHasExtents(false)
#endif
{
}

CVector3f CCollisionInfo::GetExtreme() const { return mPoint + mExtentX + mExtentY + mExtentZ; }

void CCollisionInfo::Swap() {
  mNormalLeft = -mNormalLeft;
  mNormalRight = -mNormalRight;
  rstl::swap(mMaterialLeft, mMaterialRight);
}
