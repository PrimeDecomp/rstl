#include "WorldFormat/CAreaBspTree.hpp"

#include "rstl/RstlVersions.h"

#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

CAreaBspTree::CAreaBspTree(CInputStream& in, const CTransform4f& transform) {
#if !RSTL_PRIME1
  in.ReadInt32();
  in.ReadInt16();
  in.ReadInt16();
#else
  in.ReadLong();
  in.ReadShort();
  in.ReadShort();
#endif
}
