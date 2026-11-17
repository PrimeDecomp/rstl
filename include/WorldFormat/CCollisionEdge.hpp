#ifndef _CCOLLISIONEDGE
#define _CCOLLISIONEDGE

#include "rstl/RstlVersions.h"
#include "rstl/construct.hpp"
#include "rstl/static_assert.hpp"
#include "rstl/types.h"

#include "Kyoto/Streams/CInputStream.hpp"

class CCollisionEdge {
public:
#if RSTL_COLLISION_EDGE_INT_INDICES
  typedef int TIndex;
#else
  typedef ushort TIndex;
#endif

  CCollisionEdge(RSTL_KYOTO_SCALAR_PARAM_CONST ushort index1,
                 RSTL_KYOTO_SCALAR_PARAM_CONST ushort index2);
  explicit CCollisionEdge(CInputStream& in);

  RSTL_PRIME1_CONST TIndex GetVertIndex1() const { return mIndex1; }
  RSTL_PRIME1_CONST TIndex GetVertIndex2() const { return mIndex2; }

private:
  ushort mIndex1;
  ushort mIndex2;
};
CHECK_SIZEOF(CCollisionEdge, 4)

#if RSTL_PRIME1
inline CCollisionEdge::CCollisionEdge(RSTL_KYOTO_SCALAR_PARAM_CONST ushort index1,
                                      RSTL_KYOTO_SCALAR_PARAM_CONST ushort index2)
: mIndex1(index1), mIndex2(index2) {}
inline CCollisionEdge::CCollisionEdge(CInputStream& in) {
  mIndex1 = in.Get< ushort >();
  mIndex2 = in.Get< ushort >();
}
#endif

#if RSTL_KYOTO_CONSTRUCTION_POLICIES
namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CCollisionEdge)
} // namespace rstl
#endif

#endif // _CCOLLISIONEDGE
