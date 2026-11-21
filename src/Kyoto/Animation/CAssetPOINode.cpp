#include "Kyoto/Animation/CAssetPOINode.hpp"

#include "Kyoto/Streams/CInputStream.hpp"



const uchar CAssetPOINode::skDefaultFlag = 0;

CAssetPOINode::CAssetPOINode(CInputStream& in)
: CPOINode(in)
, mFlag(in.ReadUint8())
, mEventType(in.ReadInt32())
, mEventValue(in.ReadInt32())
, mAsset(in.Get< CAssetId >()) {}

CAssetPOINode CAssetPOINode::CopyNodeMinusStartTime(const CAssetPOINode& node,
                                                    const CCharAnimTime& startTime) {
  return CAssetPOINode(node.GetNameHash(), node.GetPoiType(), node.GetTime() - startTime,
                       node.GetIndex(), node.GetSaveState(), node.GetWeight(),
                       node.GetCharacterIndex(), node.GetFlags(), node.GetUnk2C(), node.GetSegId(),
                       node.GetEventType(), node.GetEventValue(), node.GetAssetId());
}
