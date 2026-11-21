#ifndef _CCHARACTERAUDIOMANAGER
#define _CCHARACTERAUDIOMANAGER

#include "rstl/static_assert.hpp"
#include "rstl/types.h"

#include "Kyoto/CAssetId.hpp"
#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Audio/CAudioHandle.hpp"
#include "Kyoto/Audio/CSoundEvaluator.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/listpool.hpp"
#include "rstl/pair.hpp"
#include "rstl/vector.hpp"



class IObjectStore;
class CAnimData;
class CAssetIdList;
class CAssetSoundPOINode;
class CAudioSoundEffect;
class CTransform4f;

// Guessed name. Per-character CAUD playback state embedded in CAnimData at 0x80.
class CCharacterAudioManager {
public:
  CCharacterAudioManager(IObjectStore* store, const CAssetIdList& preloads);
  ~CCharacterAudioManager();

  bool HasLoopingSound(CAssetId sound, int index) const; // Guessed name
  void UpdateActiveSounds(float dt, const CAnimData& animData,
                          const CTransform4f& worldTransform); // Guessed name
  void StopSoundsByIndex(int index);        // Guessed name
  void StopLoopingSoundsByIndex(int index); // Guessed name
  void StopSoundByAssetAndIndex(CAssetId sound, int index); // Guessed name
  void StartSound(const CAssetSoundPOINode& poi, const CTransform4f& worldTransform,
                  const CAnimData& animData, int areaId, int index); // Guessed name

private:
  // Guessed names throughout this native record.
  class SActiveSound {
  public:
    SActiveSound(const CAssetSoundPOINode& poi, const CTransform4f& worldTransform,
                 const CAnimData& animData, int areaId,
                 const TLockedToken< CAudioSoundEffect >& sound, int index);
    bool IsPlaying() const;
    void Stop();
    void Update(float dt, const CAnimData& animData, const CTransform4f& worldTransform);
    int GetIndex() const { return mIndex; }
    bool IsExclusive() const { return mExclusive; }
    bool IsLooping() const { return mHandle.IsLooping(); }
    CAssetId GetSoundAsset() const { return mSoundAsset; }

    CAssetId mSoundAsset;
    int mIndex;
    rstl::rc_ptr< CSoundEvaluator > mVolumeEvaluator;
    rstl::rc_ptr< CSoundEvaluator > mPitchEvaluator;
    CSegId mLocator;
    CAudioHandle mHandle;
    int mAreaId;
    float mPriority;
    bool mExclusive;
  };
  typedef char SActiveSoundSizeCheck[sizeof(SActiveSound) == 0x30 ? 1 : -1];

  typedef rstl::listpool< SActiveSound, 8 > CActiveSounds;
  typedef rstl::pair< CAssetId, TLockedToken< CAudioSoundEffect > > CPreloadedSound;

  IObjectStore* mStore; // Guessed name
  CActiveSounds mActiveSounds; // Guessed name
  float mExclusivePriority; // Guessed name
  CAssetId mExclusiveSound; // Guessed name
  rstl::vector< CPreloadedSound > mPreloads; // Guessed name
};
CHECK_SIZEOF(CCharacterAudioManager, 0x40)


#endif // _CCHARACTERAUDIOMANAGER
