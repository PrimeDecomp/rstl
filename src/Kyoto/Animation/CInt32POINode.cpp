#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

CInt32POINode::CInt32POINode(CInputStream& in)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
: CPOINode(in), mVal(in.ReadInt32()), mLctrName(in) {}
#else
: CPOINode(in), mVal(in.Get< int >()), mLctrName(in) {}
#endif

CInt32POINode CInt32POINode::CopyNodeMinusStartTime(const CInt32POINode& node,
                                                    const CCharAnimTime& startTime) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return CInt32POINode(node.GetString(), node.GetPoiType(), node.GetTime() - startTime, node.GetIndex(),
                       node.GetSaveState(), node.GetWeight(), node.GetCharacterIndex(), node.GetFlags(),
                       node.GetValue(), node.GetLocatorName());
#else
  return CInt32POINode(node.GetNameHash(), node.GetPoiType(), node.GetTime() - startTime,
                       node.GetIndex(), node.GetSaveState(), node.GetWeight(),
                       node.GetCharacterIndex(), node.GetFlags(), node.GetValue(),
                       node.GetLocatorName());
#endif

}

