#ifndef _CCOLLIDABLEAABOXSPHERE
#define _CCOLLIDABLEAABOXSPHERE

#include "rstl/RstlVersions.h"
#include "rstl/static_assert.hpp"
#include "Collision/CCollidableAABox.hpp"
#include "Collision/CCollidableSphere.hpp"
#include "Collision/CCollisionPrimitive.hpp"

// Guessed name, correlated with Prime's ABSH compound primitive.
class CCollidableAABoxSphere : public CCollisionPrimitive {
public:
  const CCollidableAABox& GetCollidableAABox() const { return mAabb; }
  const CCollidableSphere& GetCollidableSphere() const { return mSphere; }

private:
  CCollidableAABox mAabb;
  CCollidableSphere mSphere;
};
CHECK_CHILD_SIZEOF(CCollidableAABoxSphere, CCollisionPrimitive, 0x48)

#endif // _CCOLLIDABLEAABOXSPHERE
