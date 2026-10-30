#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CBoolPOINode.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CBoolPOINode::CBoolPOINode(CInputStream& in) : CPOINode(in), mVal(in.ReadBool()) {}

CBoolPOINode CBoolPOINode::CopyNodeMinusStartTime(const CBoolPOINode& node,
                                                  const CCharAnimTime& startTime) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return CBoolPOINode(node.GetString(), node.GetPoiType(), node.GetTime() - startTime,
#else
  return CBoolPOINode(node.GetNameHash(), node.GetPoiType(), node.GetTime() - startTime,
#endif

                      node.GetIndex(), node.GetSaveState(), node.GetWeight(),
                      node.GetCharacterIndex(), node.GetFlags(), node.GetValue());
}

