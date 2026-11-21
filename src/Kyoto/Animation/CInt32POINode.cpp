#include "rstl/RstlVersions.h"
#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

#if RSTL_CORRUPTION


CInt32POINode::CInt32POINode(CInputStream& in) : CPOINode(in), mVal(in.Get< int >()) {}
#else
CInt32POINode::CInt32POINode(CInputStream& in)
: CPOINode(in)
#if RSTL_PRIME1
, mVal(in.ReadLong())
#else
, mVal(in.Get< int >())
#endif
, mLctrName(in) {}
#endif

CInt32POINode CInt32POINode::CopyNodeMinusStartTime(const CInt32POINode& node,
                                                    const CCharAnimTime& startTime) {
#if RSTL_CORRUPTION
  return CInt32POINode(node.GetNameHash(), node.GetPoiType(), node.GetTime() - startTime,
                       node.GetIndex(), node.GetSaveState(), node.GetWeight(),
                       node.GetCharacterIndex(), node.GetFlags(), node.GetUnk2C(), node.GetSegId(),
                       node.GetValue());
#else
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
#endif
}
