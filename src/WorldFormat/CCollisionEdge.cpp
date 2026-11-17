#include "WorldFormat/CCollisionEdge.hpp"

#if !RSTL_PRIME1
CCollisionEdge::CCollisionEdge(RSTL_KYOTO_SCALAR_PARAM_CONST ushort index1,
                               RSTL_KYOTO_SCALAR_PARAM_CONST ushort index2)
: mIndex1(index1), mIndex2(index2) {}

CCollisionEdge::CCollisionEdge(CInputStream& in)
: mIndex1(in.Get< ushort >()), mIndex2(in.Get< ushort >()) {}
#endif
