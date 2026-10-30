#ifndef _CSOUNDPOINODE
#define _CSOUNDPOINODE
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CPOINode.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Animation/CSegId.hpp"

class CSoundPOINode : public CPOINode {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CSoundPOINode(const rstl::string name = "", const EPOIType type = kPT_Sound,
                const CCharAnimTime& time = CCharAnimTime(), const int index = -1,
                const bool unique = false, const float weight = 1.f, const int charIdx = -1,
                const int flags = 0, const int sfxId = 0, const float fallOff = 0.f,
                const float maxDist = 0.f)
  : CPOINode(name, type, time, index, unique, weight, charIdx, flags)
  , mSfxId(sfxId)
  , mFalloff(fallOff)
  , mMaxDist(maxDist) {}

  CSoundPOINode(CInputStream& in)
  : CPOINode(in)
  , mSfxId(in.ReadInt32())
  , mFalloff(in.Get< float >())
  , mMaxDist(in.Get< float >()) {}
#else
  CSoundPOINode(uint nameHash = -1, EPOIType type = kPT_Sound,
                const CCharAnimTime& time = CCharAnimTime(), int index = -1,
                const bool unique = false, float weight = 1.f, int charIdx = -1, int flags = 0,
                int sfxId = 0, float fallOff = 0.f, float maxDist = 0.f,
                const CSegId& segId = CSegId(0), ushort pitchStart = 0, ushort pitchEnd = 0,
                float pitchDuration = 0.f);
  CSoundPOINode(CInputStream& in);
  ~CSoundPOINode() override;
#endif

  uint GetSoundId() const { return mSfxId; }
  float GetFallOff() const { return mFalloff; }
  float GetMaxDistance() const { return mMaxDist; }
#if !RSTL_PRIME1
  const CSegId& GetLocator() const { return mSegId; }
  const ushort GetPitchStart() const { return mPitchStart; }
  const ushort GetPitchEnd() const { return mPitchEnd; }
  float GetPitchDuration() const { return mPitchDuration; }
#endif

  static CSoundPOINode CopyNodeMinusStartTime(const CSoundPOINode& node,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                              const CCharAnimTime& startTime) {
    return CSoundPOINode(node.GetString(), node.GetPoiType(), node.GetTime() - startTime,
                         node.GetIndex(), node.GetSaveState(), node.GetWeight(),
                         node.GetCharacterIndex(), node.GetFlags(), node.GetSoundId(),
                         node.GetFallOff(), node.GetMaxDistance());
  }
#else
                                              const CCharAnimTime& startTime);
#endif

private:
  uint mSfxId;
  float mFalloff;
  float mMaxDist;
#if !RSTL_PRIME1
  CSegId mSegId;
  ushort mPitchStart;
  ushort mPitchEnd;
  float mPitchDuration;
#endif
};
#if !RSTL_PRIME1
CHECK_CHILD_SIZEOF(CSoundPOINode, CPOINode, 0x18)
#endif

#endif // _CSOUNDPOINODE
