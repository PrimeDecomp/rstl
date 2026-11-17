#ifndef _CCOLLISIONSURFACE
#define _CCOLLISIONSURFACE

#include "rstl/RstlVersions.h"
#include "rstl/construct.hpp"
#include "rstl/static_assert.hpp"
#include "rstl/types.h"

#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CVector3f.hpp"

class CCollisionSurface {
public:
#if RSTL_COLLISION_SURFACE_WIDE_FLAGS
  typedef u64 TFlags;
#else
  typedef uint TFlags;
#endif

#if RSTL_COLLISION_SURFACE_UNIT_NORMAL
  typedef CUnitVector3f TNormal;
#else
  typedef CVector3f TNormal;
#endif

#if RSTL_KYOTO_CONSTRUCTION_POLICIES
  CCollisionSurface() {}
#endif
  CCollisionSurface(const CVector3f& a, const CVector3f& b, const CVector3f& c, TFlags flags);

  TNormal GetNormal() const;
  CPlane GetPlane() const;
  CPlane GetEdgePlane(int edge) const; // Guessed name
  bool IsDegenerate() const;           // Guessed name

  TFlags GetSurfaceFlags() const { return mFlags; }
  const CVector3f& GetVert(int index) const { return (&mA)[index]; }
  const CVector3f& GetPoint(int index) const { return (&mA)[index]; }

private:
  CVector3f mA;
  CVector3f mB;
  CVector3f mC;
  TFlags mFlags;
};
#if RSTL_COLLISION_SURFACE_WIDE_FLAGS
CHECK_SIZEOF(CCollisionSurface, 0x30)
#else
CHECK_SIZEOF(CCollisionSurface, 0x28)
#endif

#if !RSTL_PRIME1
inline CCollisionSurface::CCollisionSurface(const CVector3f& a, const CVector3f& b,
                                            const CVector3f& c, TFlags flags)
: mA(a), mB(b), mC(c), mFlags(flags) {}
#endif

#if RSTL_KYOTO_CONSTRUCTION_POLICIES
namespace rstl {
RSTL_DECLARE_TRIVIALLY_DESTRUCTIBLE(CCollisionSurface)
RSTL_DECLARE_BITWISE_CONSTRUCTION(CCollisionSurface)
RSTL_DECLARE_VECTOR_CONSTRUCTION(CCollisionSurface)
RSTL_DECLARE_VECTOR_RESERVE_CONSTRUCTION(CCollisionSurface)
} // namespace rstl
#endif

#endif // _CCOLLISIONSURFACE
