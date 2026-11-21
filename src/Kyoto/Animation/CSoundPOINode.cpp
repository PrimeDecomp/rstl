#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CSoundPOINode.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CSoundPOINode::CSoundPOINode(uint nameHash, EPOIType type, const CCharAnimTime& time, int index,
#if RSTL_CORRUPTION
                             const bool unique, float weight, int charIdx, int flags, int unk2C,
                             const CSegId& segId, int sfxId, float fallOff, float maxDist,
                             ushort pitchStart, ushort pitchEnd, float pitchDuration)
: CPOINode(nameHash, type, time, index, unique, weight, charIdx, flags, unk2C, segId)
#else
                             const bool unique, float weight, int charIdx, int flags, int sfxId,
                             float fallOff, float maxDist, const CSegId& segId, ushort pitchStart,
                             ushort pitchEnd, float pitchDuration)
: CPOINode(nameHash, type, time, index, unique, weight, charIdx, flags)
#endif
, mSfxId(sfxId)
, mFalloff(fallOff)
, mMaxDist(maxDist)
#if !RSTL_CORRUPTION
, mSegId(segId)
#endif
, mPitchStart(pitchStart)
, mPitchEnd(pitchEnd)
, mPitchDuration(pitchDuration) {}

CSoundPOINode::CSoundPOINode(CInputStream& in)
: CPOINode(in)
, mSfxId(in.Get< int >())
, mFalloff(in.Get< float >())
, mMaxDist(in.Get< float >())
#if RSTL_CORRUPTION
, mPitchStart(in.Get< ushort >())
, mPitchEnd(in.Get< ushort >())
, mPitchDuration(in.Get< float >()) {}
#else
, mSegId(static_cast< uchar >(0))
, mPitchStart(0)
, mPitchEnd(0)
, mPitchDuration(0.f) {
  if (mVersion > skExtendedVersion) {
    mSegId = CSegId(in);
    mPitchStart = in.Get< ushort >();
    mPitchEnd = in.Get< ushort >();
    mPitchDuration = in.Get< float >();
  }
}
#endif

CSoundPOINode::~CSoundPOINode() {}

CSoundPOINode CSoundPOINode::CopyNodeMinusStartTime(const CSoundPOINode& node,
                                                    const CCharAnimTime& startTime) {
  return CSoundPOINode(node.GetNameHash(), node.GetPoiType(), node.GetTime() - startTime,
                       node.GetIndex(), node.GetSaveState(), node.GetWeight(),
#if RSTL_CORRUPTION
                       node.GetCharacterIndex(), node.GetFlags(), node.GetUnk2C(), node.GetSegId(),
                       node.GetSoundId(), node.GetFallOff(), node.GetMaxDistance(),
                       node.mPitchStart, node.mPitchEnd, node.mPitchDuration);
#else
                       node.GetCharacterIndex(), node.GetFlags(), node.GetSoundId(),
                       node.GetFallOff(), node.GetMaxDistance(), node.GetLocator(),
                       node.GetPitchStart(), node.GetPitchEnd(), node.GetPitchDuration());
#endif
}
