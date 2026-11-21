#ifndef _CASSETSOUNDPOINODE
#define _CASSETSOUNDPOINODE

#include "rstl/static_assert.hpp"
#include "rstl/types.h"

#include "Kyoto/Animation/CPOINode.hpp"
#include "Kyoto/Audio/CSoundEvaluator.hpp"
#include "Kyoto/CAssetId.hpp"

#include "rstl/rc_ptr.hpp"



class CInputStream;

// Guessed name
class CAssetSoundPOINode : public CPOINode {
public:
  CAssetSoundPOINode(
      uint nameHash = -1, EPOIType type = kPT_Sound, const CCharAnimTime& time = CCharAnimTime(),
      int index = -1, bool unique = false, float weight = 1.f, int charIdx = -1, int flags = 0,
      int unk2C = CPOINode::skDefaultUnk2C, const CSegId& segId = CSegId(),
      CAssetId soundAsset = kInvalidAssetId, float valueA = 0.f, float valueB = 0.f,
      const rstl::rc_ptr< CSoundEvaluator >& evaluatorA = rstl::rc_ptr< CSoundEvaluator >(nullptr),
      const rstl::rc_ptr< CSoundEvaluator >& evaluatorB = rstl::rc_ptr< CSoundEvaluator >(nullptr));
  explicit CAssetSoundPOINode(CInputStream& in);
  ~CAssetSoundPOINode() override;

  CAssetId GetSoundAsset() const { return mSoundAsset; } // Guessed name
  float GetValueA() const { return mValueA; }            // Guessed name
  float GetValueB() const { return mValueB; }            // Guessed name
  // Guessed names. Native playback uses the second evaluator for volume and the first for pitch.
  const rstl::rc_ptr< CSoundEvaluator >& GetVolumeEvaluator() const { return mEvaluatorB; }
  const rstl::rc_ptr< CSoundEvaluator >& GetPitchEvaluator() const { return mEvaluatorA; }

  static CAssetSoundPOINode CopyNodeMinusStartTime(const CAssetSoundPOINode& node,
                                                   const CCharAnimTime& startTime);

private:
  bool mFlag;                                  // Guessed name
  CAssetId mSoundAsset;                        // Guessed name
  float mValueA;                               // Guessed name
  float mValueB;                               // Guessed name
  rstl::rc_ptr< CSoundEvaluator > mEvaluatorA; // Guessed name
  rstl::rc_ptr< CSoundEvaluator > mEvaluatorB; // Guessed name
};
CHECK_SIZEOF(CAssetSoundPOINode, 0x58)


#endif // _CASSETSOUNDPOINODE
