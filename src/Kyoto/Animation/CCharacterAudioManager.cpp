#include "Kyoto/Animation/CCharacterAudioManager.hpp"

#include "Kyoto/CAssetIdList.hpp"
#include "Kyoto/Animation/CAssetSoundPOINode.hpp"
#include "Kyoto/Audio/CAudioSoundEffect.hpp"
#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Corruption/MetroidPrime/CAnimData.hpp"



CCharacterAudioManager::SActiveSound::SActiveSound(
    const CAssetSoundPOINode& poi, const CTransform4f& worldTransform, const CAnimData& animData,
    int areaId, const TLockedToken< CAudioSoundEffect >& sound, int index)
: mSoundAsset(poi.GetSoundAsset())
, mIndex(index)
, mVolumeEvaluator(poi.GetVolumeEvaluator())
, mPitchEvaluator(poi.GetPitchEvaluator())
, mLocator(poi.GetSegId())
, mAreaId(areaId)
, mPriority(-3.402823466e38f)
, mExclusive(false) {
  const int flags = poi.GetFlags();
  const CTransform4f transform = worldTransform * animData.GetLocatorTransform(mLocator, true, nullptr);
  if ((flags & 0x40000000) > 0) {
    mHandle = sound->PlayPanned(areaId, 1.f, 0.f);
  } else {
    mHandle = sound->PlaySpatial(areaId, transform.GetTranslation(), 1.f);
  }
  mExclusive = mHandle.IsExclusiveCharacterSound();
  mPriority = mHandle.GetCharacterSoundPriority();
}

void CCharacterAudioManager::SActiveSound::Update(float dt, const CAnimData& animData,
                                                const CTransform4f& worldTransform) {
  const float time = mHandle.GetElapsedSeconds();
  if (mVolumeEvaluator) {
    mHandle.SetVolume(mVolumeEvaluator->Evaluate(time));
  }
  if (mPitchEvaluator) {
    mHandle.SetPitchBend(mPitchEvaluator->Evaluate(time));
  }
  const CTransform4f transform = worldTransform * animData.GetLocatorTransform(mLocator, true, nullptr);
  mHandle.SetPosition(transform.GetTranslation());
}

void CCharacterAudioManager::SActiveSound::Stop() { mHandle.StopWithFade(); }

bool CCharacterAudioManager::SActiveSound::IsPlaying() const {
  return mHandle && mHandle.IsActiveAndNotStopping();
}

CCharacterAudioManager::CCharacterAudioManager(IObjectStore* store, const CAssetIdList& preloads)
: mStore(store), mExclusivePriority(-3.402823466e38f), mExclusiveSound(kInvalidAssetId) {
  const rstl::vector< CAssetId >& ids = preloads.GetAssetIds();
  mPreloads.reserve(ids.size());
  for (rstl::vector< CAssetId >::const_iterator it = ids.begin(); it != ids.end(); ++it) {
    const CAssetId id = *it;
    TLockedToken< CAudioSoundEffect > sound = mStore->GetObj(SObjectTag('CAUD', id));
    mPreloads.push_back(CPreloadedSound(id, sound));
  }
}

CCharacterAudioManager::~CCharacterAudioManager() { StopLoopingSoundsByIndex(-1); }

void CCharacterAudioManager::StartSound(const CAssetSoundPOINode& poi,
                                       const CTransform4f& worldTransform,
                                       const CAnimData& animData, int areaId, int index) {
  bool canPlay = true;
  TLockedToken< CAudioSoundEffect > sound = mStore->GetObj(SObjectTag('CAUD', poi.GetSoundAsset()));
  if (sound->IsExclusiveCharacterSound()) {
    const float priority = sound->GetCharacterSoundPriority();
    if (priority > mExclusivePriority) {
      if (mExclusivePriority >= 0.f && mExclusiveSound != kInvalidAssetId) {
        StopSoundByAssetAndIndex(mExclusiveSound, -1);
      }
      mExclusivePriority = priority;
      mExclusiveSound = poi.GetSoundAsset();
    } else {
      canPlay = false;
    }
  }
  if (canPlay) {
    mActiveSounds.insert(SActiveSound(poi, worldTransform, animData, areaId, sound, index));
  }
}

void CCharacterAudioManager::StopSoundByAssetAndIndex(CAssetId sound, int index) {
  CActiveSounds::iterator it = mActiveSounds.begin();
  const CActiveSounds::const_iterator end = mActiveSounds.end();
  for (; it != end; ++it) {
    if (it->GetSoundAsset() == sound && (index == it->GetIndex() || index == -1)) {
      if (it->IsExclusive()) {
        mExclusivePriority = -3.402823466e38f;
        mExclusiveSound = kInvalidAssetId;
      }
      it->Stop();
      mActiveSounds.erase(it);
      break;
    }
  }
}

void CCharacterAudioManager::StopLoopingSoundsByIndex(int index) {
  CActiveSounds::iterator it = mActiveSounds.begin();
  const CActiveSounds::const_iterator end = mActiveSounds.end();
  while (it != end) {
    if (it->IsLooping() && (index == it->GetIndex() || index == -1)) {
      if (it->IsExclusive()) {
        mExclusivePriority = -3.402823466e38f;
        mExclusiveSound = kInvalidAssetId;
      }
      it->Stop();
      it = mActiveSounds.erase(it);
    } else {
      ++it;
    }
  }
}

void CCharacterAudioManager::StopSoundsByIndex(int index) {
  CActiveSounds::iterator it = mActiveSounds.begin();
  const CActiveSounds::const_iterator end = mActiveSounds.end();
  while (it != end) {
    if (index == it->GetIndex() || index == -1) {
      if (it->IsExclusive()) {
        mExclusivePriority = -3.402823466e38f;
        mExclusiveSound = kInvalidAssetId;
      }
      it->Stop();
      it = mActiveSounds.erase(it);
    }
    // G2MEAB does not advance past entries with a different index here.
  }
}

void CCharacterAudioManager::UpdateActiveSounds(float dt, const CAnimData& animData,
                                               const CTransform4f& worldTransform) {
  CActiveSounds::iterator it = mActiveSounds.begin();
  while (it != mActiveSounds.end()) {
    if (it->IsPlaying()) {
      it->Update(dt, animData, worldTransform);
      ++it;
    } else {
      if (it->IsExclusive()) {
        mExclusivePriority = -3.402823466e38f;
        mExclusiveSound = kInvalidAssetId;
      }
      it = mActiveSounds.erase(it);
    }
  }
}

bool CCharacterAudioManager::HasLoopingSound(CAssetId sound, int index) const {
  CActiveSounds::const_iterator it = mActiveSounds.begin();
  const CActiveSounds::const_iterator end = mActiveSounds.end();
  for (; it != end; ++it) {
    if (sound == it->GetSoundAsset() && it->IsLooping() &&
        (index == it->GetIndex() || index == -1)) {
      return true;
    }
  }
  return false;
}
