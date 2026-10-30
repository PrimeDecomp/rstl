#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CBoolPOINode.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CBoolPOINode::CBoolPOINode(CInputStream& in) : CPOINode(in), mVal(in.ReadBool()) {}

CBoolPOINode CBoolPOINode::CopyNodeMinusStartTime(const CBoolPOINode& node,
                                                  const CCharAnimTime& startTime) {
  return CBoolPOINode(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      node.GetString(),
#else
      node.GetNameHash(),
#endif
      node.GetPoiType(), node.GetTime() - startTime, node.GetIndex(), node.GetSaveState(),
      node.GetWeight(), node.GetCharacterIndex(), node.GetFlags(), node.GetValue());
}

