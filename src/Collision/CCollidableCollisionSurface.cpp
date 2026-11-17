#include "rstl/RstlVersions.h"
#include "Collision/CCollidableCollisionSurface.hpp"

int CCollidableCollisionSurface::sTableIndex = -1;

#if !RSTL_COLLISION_SURFACE_TYPE_FIRST
void CCollidableCollisionSurface::SetStaticTableIndex(uint index) { sTableIndex = index; }

#endif
CCollisionPrimitive::Type CCollidableCollisionSurface::GetType() {
  return CCollisionPrimitive::Type(SetStaticTableIndex, "CCollidableCollisionSurface");
}

#if RSTL_COLLISION_SURFACE_TYPE_FIRST
void CCollidableCollisionSurface::SetStaticTableIndex(uint index) { sTableIndex = index; }
#endif
