#ifndef _CCOLLIDABLEOBBTREEGROUP
#define _CCOLLIDABLEOBBTREEGROUP

#include "rstl/RstlVersions.h"

#include "rstl/static_assert.hpp"
#include "Collision/CCollisionPrimitive.hpp"
#include "WorldFormat/COBBTreeGroup.hpp"
#include "rstl/auto_ptr.hpp"

#include "Kyoto/CFactoryFnReturn.hpp"

class CCollisionCache;
class CInputStream;
class CVParamTransfer;

class CCollidableOBBTreeGroup : public CCollisionPrimitive {
public:
  CCollidableOBBTreeGroup(COBBTreeGroup* container, const CMaterialList& material);
#if !RSTL_PRIME1
  CCollidableOBBTreeGroup(const COBBTreeGroup* container, const CMaterialList& material);
#endif

  // CCollisionPrimitive
  uint GetTableIndex() const override;
  CAABox CalculateAABox(const CTransform4f& xf) const override;
  CAABox CalculateLocalAABox() const override;
  FourCC GetPrimType() const override;
#if RSTL_PRIME1
  ~CCollidableOBBTreeGroup() override {}
#else
  ~CCollidableOBBTreeGroup() override;
#endif
  CRayCastResult CastRayInternal(const CInternalRayCastStructure& rayCast) const override;

  const COBBTreeGroup* GetContainer() const { return mContainer; }
  COBBTree* GetOBBTree(int index) const;

  static Type GetType();
  static void SetStaticTableIndex(uint index);
  static bool SphereCollide(const CInternalCollisionStructure& collision, CCollisionInfoList& list);
  static bool SphereCollideBoolean(const CInternalCollisionStructure& collision);
  static bool CollideMovingSphere(const CInternalCollisionStructure& collision,
                                  const CVector3f& direction, double& distance,
                                  CCollisionInfo& info);
  static bool AABoxCollide(const CInternalCollisionStructure& collision, CCollisionInfoList& list);
  static bool AABoxCollideBoolean(const CInternalCollisionStructure& collision);
  static bool CollideMovingAABox(const CInternalCollisionStructure& collision,
                                 const CVector3f& direction, double& distance,
                                 CCollisionInfo& info);

#if !RSTL_PRIME1
  // Guessed name for the Echoes collision-cache addition.
  void CacheTree(CCollisionCache& cache, const CTransform4f& xf, ushort ownerId,
                 u64 material) const;
#endif

private:
#if RSTL_PRIME1
  COBBTreeGroup* mContainer;
#else
  rstl::auto_ptr< const COBBTreeGroup > mOwnedContainer;
  const COBBTreeGroup* mContainer;
#endif
  static uint sTableIndex;
};
#if RSTL_PRIME1
CHECK_SIZEOF(CCollidableOBBTreeGroup, 0x18)
#else
CHECK_CHILD_SIZEOF(CCollidableOBBTreeGroup, CCollisionPrimitive, 0x10)
#endif

#if RSTL_PRIME1
const CFactoryFnReturn FCollidableOBBTreeGroupFactory(const SObjectTag& tag, CInputStream& in,
                                                      const CVParamTransfer& xfer);
#else
CFactoryFnReturn FCollidableOBBTreeGroupFactory(const SObjectTag& tag, CInputStream& in,
                                                const CVParamTransfer& xfer);
#endif

#endif // _CCOLLIDABLEOBBTREEGROUP
