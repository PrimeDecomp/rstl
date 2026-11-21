#ifndef _CASSETPOINODE
#define _CASSETPOINODE

#include "rstl/static_assert.hpp"
#include "rstl/types.h"

#include "Kyoto/Animation/CPOINode.hpp"
#include "Kyoto/CAssetId.hpp"



// Guessed name
class CAssetPOINode : public CPOINode {
public:
  static const uchar skDefaultFlag; // Guessed name

  CAssetPOINode(uint nameHash = -1, EPOIType type = kPT_EmptyInt32,
                const CCharAnimTime& time = CCharAnimTime(), int index = -1,
                const bool unique = false, float weight = 1.f, int charIdx = -1, int flags = 0,
                int unk2C = CPOINode::skDefaultUnk2C, const CSegId& segId = CSegId(),
                int eventType = -1, int eventValue = -1, CAssetId asset = kInvalidAssetId)
  : CPOINode(nameHash, type, time, index, unique, weight, charIdx, flags, unk2C, segId)
  , mFlag(skDefaultFlag)
  , mEventType(eventType)
  , mEventValue(eventValue)
  , mAsset(asset) {}
  CAssetPOINode(CInputStream& in);
  ~CAssetPOINode() override {}

  int GetEventType() const { return mEventType; }
  int GetEventValue() const { return mEventValue; }
  CAssetId GetAssetId() const { return mAsset; }

  static CAssetPOINode CopyNodeMinusStartTime(const CAssetPOINode& node,
                                              const CCharAnimTime& startTime);

private:
  uchar mFlag;    // Guessed name
  int mEventType;  // Guessed name
  int mEventValue; // Guessed name
  CAssetId mAsset; // Guessed name
};
CHECK_SIZEOF(CAssetPOINode, 0x48)


#endif // _CASSETPOINODE
