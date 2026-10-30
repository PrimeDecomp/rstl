#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

CInt32POINode::CInt32POINode(CInputStream& in)
: CPOINode(in)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mVal(in.ReadInt32())
#else
, mVal(in.Get< int >())
#endif
, mLctrName(in) {}

CInt32POINode CInt32POINode::CopyNodeMinusStartTime(const CInt32POINode& node,
                                                    const CCharAnimTime& startTime) {
  return CInt32POINode(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      node.GetString(),
#else
      node.GetNameHash(),
#endif
      node.GetPoiType(), node.GetTime() - startTime, node.GetIndex(), node.GetSaveState(),
      node.GetWeight(), node.GetCharacterIndex(), node.GetFlags(), node.GetValue(),
      node.GetLocatorName());
}

