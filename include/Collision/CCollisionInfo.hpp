#ifndef _CCOLLISIONINFO
#define _CCOLLISIONINFO

#include "rstl/RstlVersions.h"
#include "rstl/construct.hpp"
#include "rstl/static_assert.hpp"
#include "rstl/types.h"

#include "Collision/CMaterialList.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TGameTypes.hpp"

class CAABox;

class CCollisionInfo {
public:
  // Original Wii export; the native invalid constructor ignores the tag.
  enum EInvalid {
    kI_Invalid,
    kI_Valid,
  };
  enum ESwapMaterials { kSM_Swap };

  CCollisionInfo(EInvalid valid = kI_Invalid);
  CCollisionInfo(const CVector3f& point, const CMaterialList& rightMaterial,
                 const CMaterialList& leftMaterial, const CVector3f& normal
#if !RSTL_PRIME1
                 ,
                 ushort objectId
#endif
  );
  CCollisionInfo(const CVector3f& point, const CMaterialList& rightMaterial,
                 const CMaterialList& leftMaterial, const CVector3f& leftNormal,
                 const CVector3f& rightNormal
#if !RSTL_PRIME1
                 ,
                 ushort objectId
#endif
  );
  CCollisionInfo(const CAABox& box, const CMaterialList& rightMaterial,
                 const CMaterialList& leftMaterial, const CVector3f& leftNormal,
                 const CVector3f& rightNormal
#if !RSTL_PRIME1
                 ,
                 ushort objectId
#endif
  );
  CCollisionInfo(const CCollisionInfo& other, ESwapMaterials)
  : mPoint(other.mPoint)
  , mExtentX(other.mExtentX)
  , mExtentY(other.mExtentY)
  , mExtentZ(other.mExtentZ)
#if RSTL_PRIME1
  , mValid(other.mValid)
  , mHasExtents(other.mHasExtents)
#endif
  , mMaterialLeft(other.mMaterialRight)
  , mMaterialRight(other.mMaterialLeft)
  , mNormalLeft(other.mNormalRight)
  , mNormalRight(other.mNormalLeft)
#if !RSTL_PRIME1
  , mObjectId(0xffff)
  , mValid(other.mValid)
  , mHasExtents(other.mHasExtents)
#endif
  {
  }

  CCollisionInfo GetSwapped(ESwapMaterials swap) const;

  bool IsValid() const { return mValid; }
  const CMaterialList& GetMaterialLeft() const { return mMaterialLeft; }
  const CMaterialList& GetMaterialRight() const { return mMaterialRight; }
  CVector3f GetExtreme() const;
  void Swap();
  const CVector3f& GetNormalLeft() const { return mNormalLeft; }
  const CVector3f& GetNormalRight() const { return mNormalRight; }
  const CVector3f& GetPoint() const { return mPoint; }
#if !RSTL_PRIME1
  bool HasExtents() const { return mHasExtents; }
  TUniqueId GetObjectId() const { return TUniqueId(mObjectId); }
  void SetObjectId(const TUniqueId& id) { mObjectId = id.value; }
#endif

  // TODO: hack for CollisionUtil::AccumulateCollisionInfo. Maybe it's a static function here?
  CMaterialList& MaterialLeft() { return mMaterialLeft; }
  CMaterialList& MaterialRight() { return mMaterialRight; }
  CVector3f& NormalLeft() { return mNormalLeft; }
  CVector3f& NormalRight() { return mNormalRight; }
  CVector3f& Point() { return mPoint; }

private:
  CVector3f mPoint;
  CVector3f mExtentX;
  CVector3f mExtentY;
  CVector3f mExtentZ;
#if RSTL_PRIME1
  bool mValid;
  bool mHasExtents;
#endif
  CMaterialList mMaterialLeft;
  CMaterialList mMaterialRight;
  CVector3f mNormalLeft;
  CVector3f mNormalRight;
#if !RSTL_PRIME1
  ushort mObjectId; // Packed TUniqueId storage; constructor API remains ushort.
  bool mValid : 1;
  bool mHasExtents : 1;
#endif
};
CHECK_SIZEOF(CCollisionInfo, 0x60)

#if !RSTL_PRIME1
namespace rstl {
RSTL_DECLARE_TRIVIALLY_DESTRUCTIBLE(CCollisionInfo)
RSTL_DECLARE_BITWISE_CONSTRUCTION(CCollisionInfo)
RSTL_DECLARE_OPTIONAL_VALUE_CONSTRUCTION(CCollisionInfo)
RSTL_DECLARE_RESERVED_VECTOR_PUSH_CONSTRUCTION(CCollisionInfo)
} // namespace rstl
#endif

#endif // _CCOLLISIONINFO
