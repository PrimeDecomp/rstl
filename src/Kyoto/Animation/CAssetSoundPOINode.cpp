#include "Kyoto/Animation/CAssetSoundPOINode.hpp"

#include "Kyoto/Audio/CSoundEvaluator.hpp"
#include "Kyoto/Streams/CInputStream.hpp"



CAssetSoundPOINode::CAssetSoundPOINode(uint nameHash, EPOIType type, const CCharAnimTime& time,
                                       int index, const bool unique, float weight, int charIdx, int flags,
                                       int unk2C, const CSegId& segId, CAssetId soundAsset,
                                       float valueA, float valueB,
                                       const rstl::rc_ptr< CSoundEvaluator >& evaluatorA,
                                       const rstl::rc_ptr< CSoundEvaluator >& evaluatorB)
: CPOINode(nameHash, type, time, index, unique, weight, charIdx, flags, unk2C, segId)
, mFlag(false)
, mSoundAsset(soundAsset)
, mValueA(valueA)
, mValueB(valueB)
, mEvaluatorA(evaluatorA)
, mEvaluatorB(evaluatorB) {}

// The native stream constructor leaves mFlag untouched.
CAssetSoundPOINode::CAssetSoundPOINode(CInputStream& in)
: CPOINode(in)
, mSoundAsset(in.Get< CAssetId >())
, mValueA(in.ReadFloat())
, mValueB(in.ReadFloat())
, mEvaluatorA(CSoundEvaluatorFactory::GetSoundEvaluator(in))
, mEvaluatorB(CSoundEvaluatorFactory::GetSoundEvaluator(in)) {}

CAssetSoundPOINode::~CAssetSoundPOINode() {}

CAssetSoundPOINode CAssetSoundPOINode::CopyNodeMinusStartTime(const CAssetSoundPOINode& node,
                                                              const CCharAnimTime& startTime) {
  return CAssetSoundPOINode(node.GetNameHash(), node.GetPoiType(), node.GetTime() - startTime,
                            node.GetIndex(), node.GetSaveState(), node.GetWeight(),
                            node.GetCharacterIndex(), node.GetFlags(), node.GetUnk2C(),
                            node.GetSegId(), node.GetSoundAsset(), node.GetValueA(),
                            node.GetValueB(), node.mEvaluatorA, node.mEvaluatorB);
}
