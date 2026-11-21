#include "Kyoto/Audio/CAudioVoice.hpp"

#include "Kyoto/Audio/CAudioManager.hpp"
#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "math.h"


static const float kZeroVolume = 0.f;

CAudioVoice::CChildVoice::CChildVoice(const CAudioChannel& channel,
                                      CAudioSampleDescription& description, bool panned,
                                      float duration)
: mChannel(channel)
, mDescription(description)
, mRolloffPoints()
, mDuration(duration)
, mElapsed(0.f)
, mFadeElapsed(0.f)
, mVolume(1.f)
, mPitchBend(0.f)
, mPan(0.f)
, mSurroundPan(0.f)
, mPanned(panned)
, mPendingUnpause(true)
, mPaused(false)
, mStopping(false) {
  CRandom16& random = CAudioManager::GetRandom();
  mPitch = random.Range(description.GetPitch() - description.GetPitchVariation(),
                        description.GetPitch() + description.GetPitchVariation());
  mDescription.IncrementActiveVoiceCount();
  SetMinimumMaximumDistance(mDescription.GetMinimumDistance(), mDescription.GetMaximumDistance());
}

CAudioVoice::CChildVoice::CChildVoice(const CChildVoice& other)
: mChannel(other.mChannel)
, mDescription(other.mDescription)
, mRolloffPoints()
, mDuration(other.mDuration)
, mElapsed(other.mElapsed)
, mFadeElapsed(other.mFadeElapsed)
, mPitch(other.mPitch)
, mVolume(other.mVolume)
, mPitchBend(other.mPitchBend)
, mPan(other.mPan)
, mSurroundPan(other.mSurroundPan)
, mPanned(other.mPanned)
, mPendingUnpause(other.mPendingUnpause)
, mPaused(other.mPaused)
, mStopping(other.mStopping) {
  mDescription.IncrementActiveVoiceCount();
  SetMinimumMaximumDistance(mDescription.GetMinimumDistance(), mDescription.GetMaximumDistance());
}

CAudioVoice::CChildVoice::~CChildVoice() {
  if (mChannel) {
    mDescription.DecrementActiveVoiceCount();
  }
}

int CAudioVoice::CChildVoice::CalculatePlaybackFrequency(float pitchBend) const {
  pitchBend = CMath::Clamp(-1.f, pitchBend, 1.f);
  const float factor = pow(2.0, (mPitch + pitchBend * mDescription.GetPitchBendRange()) / 1200.f);
  const int baseFrequency = mChannel.GetSampleBaseFrequency();
  return baseFrequency * factor;
}

void CAudioVoice::CChildVoice::Update(float dt, float voiceElapsed) {
  if (mPaused) {
    return;
  }
  if (mDuration != 0.f && mElapsed >= mDuration) {
    StopWithFade();
  }
  if (mPanned) {
    const float pan = mPan + mDescription.EvaluatePanCurve(mElapsed);
    const float surroundPan =
        mSurroundPan + mDescription.EvaluateSpeakerMixSecondAxisCurve(mElapsed);
    mChannel.SetSpeakerMixCoordinates(pan, surroundPan);
  }
  float fade = 1.f;
  if (mStopping) {
    mFadeElapsed += dt;
    fade = 1.f - mFadeElapsed / mDescription.GetFadeOutSeconds();
  }
  if (fade <= 0.f) {
    Stop();
  } else {
    const float volume = CMath::Clamp(
        0.f,
        fade * (mVolume * (mDescription.EvaluateVolumeCurve(mElapsed) * mDescription.GetVolume())),
        1.f);
    mChannel.SetVolume(volume);
    const int frequency =
        CalculatePlaybackFrequency(mPitchBend + mDescription.EvaluatePitchCurve(mElapsed));
    mChannel.SetPlaybackFrequency(frequency);
  }
  if (mPendingUnpause) {
    mChannel.SetPaused(false);
    mPendingUnpause = false;
  }
  mElapsed += dt;
}

void CAudioVoice::CChildVoice::StopWithFade() {
  if (!close_enough(0.f, mDescription.GetFadeOutSeconds())) {
    mStopping = true;
  } else {
    Stop();
  }
}

void CAudioVoice::CChildVoice::SetPitchBend(float pitch) {
  mPitchBend = CMath::Clamp(-1.f, pitch, 1.f);
}

void CAudioVoice::CChildVoice::SetVolume(float volume) {
  mVolume = volume;
  if (close_enough(volume, 0.f)) {
    mChannel.SetVolume(kZeroVolume);
  }
}

void CAudioVoice::CChildVoice::SetPan(float pan) { mPan = CMath::Clamp(-1.f, pan, 1.f); }

void CAudioVoice::CChildVoice::SetSpeakerMixSecondAxis(float pan) {
  mSurroundPan = CMath::Clamp(-1.f, pan, 1.f);
}

bool CAudioVoice::CChildVoice::IsPlaying() const { return mChannel.IsPlaying(); }

void CAudioVoice::CChildVoice::SetPositionAndVelocity(const CVector3f& position,
                                                      const CVector3f& velocity) {
  if (mDescription.UsesPositionAndVelocity()) {
    mChannel.SetPositionAndVelocity(position, velocity);
  } else {
    mChannel.SetPosition(position);
  }
}

void CAudioVoice::CChildVoice::SetPaused(bool paused) {
  mPaused = paused;
  mChannel.SetPaused(paused);
}

void CAudioVoice::CChildVoice::SetMinimumMaximumDistance(float minimum, float maximum) {
  mRolloffPoints = mDescription.BuildDistanceRolloffPoints(minimum, maximum);
  mChannel.SetDistanceRolloffPoints(reinterpret_cast< FMOD_VECTOR* >(mRolloffPoints.data()),
                                    mRolloffPoints.size());
}

float CAudioVoice::CChildVoice::GetAudibility() const { return mChannel.GetAudibility(); }

const CAudioSampleDescription& CAudioVoice::CChildVoice::GetSampleDescription() const {
  return mDescription;
}

void CAudioVoice::CChildVoice::SetMuted(bool muted) { mChannel.SetMuted(muted); }

void CAudioVoice::CChildVoice::Stop() {
  if (mPendingUnpause) {
    mChannel.SetMuted(true);
    mChannel.SetPaused(false);
    mPendingUnpause = false;
  }
  mChannel.Stop();
  mDescription.DecrementActiveVoiceCount();
}

CAudioVoice::CAudioVoice()
: mSound()
, mHandle()
, mElapsed(-1.f)
, mVolume(-1.f)
, mPitchBend(-1.f)
, mPan(-1.f)
, mSurroundPan(-1.f)
, mMinimumDistance(-1.f)
, mMaximumDistance(-1.f)
, mPosition(CVector3f::Zero())
, mVelocity(CVector3f::Zero())
, mAreaId(-1)
, mContext(-1)
, mChildren()
, mStopping(false) {}

void CAudioVoice::SetHandle(const CAudioHandle& handle) { mHandle = handle; }

void CAudioVoice::PlaySpatial(const CToken& sound, const CVector3f& position, float volume,
                              int areaId) {
  mPosition = position;
  mVelocity = CVector3f::Zero();
  Play(sound, volume, 0.f, areaId, true);
}

void CAudioVoice::Play(const CToken& sound, float volume, float pan, int areaId, bool spatial) {
  if (mSound) {
    mSound.clear();
  }
  mChildren.clear();
  mSound = rstl::optional_object< TLockedToken< CAudioSoundEffect > >(
      TLockedToken< CAudioSoundEffect >(sound));
  mSpatial = spatial;
  mVolume = volume;
  mPan = pan;
  mAreaId = areaId;
  mContext = CAudioManager::GetVoiceContext();
  mPaused = false;
  mStopping = false;
  mElapsed = 0.f;
  mPitchBend = 0.f;
  mSurroundPan = 0.f;
  mMinimumDistance = -1.f;
  mMaximumDistance = -1.f;
  x50_ = 0.f;
  if (CAudioManager::GetDebugOption(1)) {
    RS_DEBUGGER_PRINT("Playing Sound %s\n", GetSoundName().data());
  }
  if (mSound && (*mSound)->GetDuration() == 0.f) {
    rstl::vector< CAudioSampleDescription >& descriptions = (*mSound)->GetDescriptions();
    float totalWeight = 0.f;
    for (rstl::vector< CAudioSampleDescription >::iterator it = descriptions.begin();
         it != descriptions.end(); ++it) {
      if ((*mSound)->GetLastSelectedDescriptionIndex() !=
          rstl::distance(descriptions.begin(), it)) {
        totalWeight += it->GetProbability();
      }
    }
    float remainingWeight = totalWeight * CAudioManager::GetRandom().Float();
    int selected = 0;
    for (rstl::vector< CAudioSampleDescription >::iterator it = descriptions.begin();
         it != descriptions.end(); ++it) {
      if ((*mSound)->GetLastSelectedDescriptionIndex() !=
          rstl::distance(descriptions.begin(), it)) {
        remainingWeight -= it->GetProbability();
        if (remainingWeight <= 0.f) {
          selected = rstl::distance(descriptions.begin(), it);
          break;
        }
      }
    }
    if (descriptions.size() > 2) {
      (*mSound)->SetLastSelectedDescriptionIndex(selected);
    }
    if (mSpatial) {
      CAudioChannel channel = descriptions[selected].CreateSpatialChannel(mPosition, mVolume);
      if (channel) {
        mChildren.push_back(CChildVoice(channel, descriptions[selected], false, 0.f));
        mChildren.back().SetVolume(mVolume);
        mChildren.back().SetPitchBend(mPitchBend);
      }
    } else {
      CAudioChannel channel = descriptions[selected].CreatePannedChannel(mVolume, mPan);
      if (channel) {
        mChildren.push_back(CChildVoice(channel, descriptions[selected], true, 0.f));
        mChildren.back().SetVolume(mVolume);
        mChildren.back().SetPitchBend(mPitchBend);
        mChildren.back().SetSpeakerMixSecondAxis(mSurroundPan);
        mChildren.back().SetPan(mPan);
      }
    }
  }
}

void CAudioVoice::ReleaseSound() {
  mSound.clear();
  mHandle.Clear();
}

CAudioHandle CAudioVoice::GetHandle() const { return mHandle; }

bool CAudioVoice::HasAllocatedHandle() const { return mSound && mHandle; }

void CAudioVoice::Update(float dt) {
  if (!mSound) {
    return;
  }
  if (!mHandle) {
    mSound.clear();
    return;
  }
  if (!mPaused) {
    const float previousElapsed = mElapsed;
    mElapsed += dt;
    if (mSound && mElapsed <= (*mSound)->GetDuration() && !mStopping) {
      rstl::vector< CAudioSampleDescription >& descriptions = (*mSound)->GetDescriptions();
      for (rstl::vector< CAudioSampleDescription >::iterator it = descriptions.begin();
           it != descriptions.end(); ++it) {
        const rstl::vector< rstl::pair< float, float > >& triggers = it->GetTriggers();
        for (rstl::vector< rstl::pair< float, float > >::const_iterator trigger = triggers.begin();
             trigger != triggers.end(); ++trigger) {
          if (trigger->first >= mElapsed) {
            break;
          }
          if (trigger->first >= previousElapsed) {
            CRandom16& random = CAudioManager::GetRandom();
            if (random.Float() <= it->GetProbability()) {
              if (mSpatial) {
                CAudioChannel channel = it->CreateSpatialChannel(mPosition, mVolume);
                if (channel) {
                  mChildren.push_back(CChildVoice(channel, *it, false, 0.f));
                  mChildren.back().SetVolume(mVolume);
                  mChildren.back().SetPitchBend(mPitchBend);
                }
              } else {
                CAudioChannel channel = it->CreatePannedChannel(mVolume, mPan);
                if (channel) {
                  mChildren.push_back(CChildVoice(channel, *it, true, 0.f));
                  mChildren.back().SetVolume(mVolume);
                  mChildren.back().SetPitchBend(mPitchBend);
                  mChildren.back().SetSpeakerMixSecondAxis(mSurroundPan);
                  mChildren.back().SetPan(mPan);
                  if (mMinimumDistance != -1.f && mMaximumDistance != -1.f) {
                    mChildren.back().SetMinimumMaximumDistance(mMinimumDistance, mMaximumDistance);
                  }
                }
              }
            }
          }
        }
      }
    }
    rstl::list< CChildVoice >::iterator it = mChildren.begin();
    const rstl::list< CChildVoice >::iterator end = mChildren.end();
    for (; it != end;) {
      if (it->IsPlaying()) {
        it->Update(dt, mElapsed);
        ++it;
      } else {
        it = mChildren.erase(it);
      }
    }
  }
  if (mSound && mElapsed > 15.f + (*mSound)->GetDuration() && !IsLooping()) {
    StopWithFade();
  }
  if (mSound && mElapsed > (*mSound)->GetDuration() && mChildren.empty()) {
    ReleaseSound();
  }
}

void CAudioVoice::StopWithFade() {
  mStopping = true;
  for (rstl::list< CChildVoice >::iterator it = mChildren.begin(); it != mChildren.end(); ++it) {
    it->StopWithFade();
  }
}

void CAudioVoice::SetVolume(float volume) {
  mVolume = volume;
  for (rstl::list< CChildVoice >::iterator it = mChildren.begin(); it != mChildren.end(); ++it) {
    it->SetVolume(volume);
  }
}

void CAudioVoice::SetPan(float pan) {
  mPan = pan;
  for (rstl::list< CChildVoice >::iterator it = mChildren.begin(); it != mChildren.end(); ++it) {
    it->SetPan(pan);
  }
}

void CAudioVoice::SetPitchBend(float pitch) {
  mPitchBend = pitch;
  for (rstl::list< CChildVoice >::iterator it = mChildren.begin(); it != mChildren.end(); ++it) {
    it->SetPitchBend(pitch);
  }
}

void CAudioVoice::SetSpeakerMixSecondAxis(float pan) {
  mSurroundPan = pan;
  for (rstl::list< CChildVoice >::iterator it = mChildren.begin(); it != mChildren.end(); ++it) {
    it->SetSpeakerMixSecondAxis(pan);
  }
}

void CAudioVoice::SetPositionAndVelocity(const CVector3f& position, const CVector3f& velocity) {
  mPosition = position;
  mVelocity = velocity;
  for (rstl::list< CChildVoice >::iterator it = mChildren.begin(); it != mChildren.end(); ++it) {
    it->SetPositionAndVelocity(position, velocity);
  }
}

float CAudioVoice::GetElapsedSeconds() const { return mElapsed; }

bool CAudioVoice::IsLooping() const { return mSound ? (*mSound)->IsLooping() : false; }

bool CAudioVoice::IsExclusiveCharacterSound() const {
  return mSound ? (*mSound)->IsExclusiveCharacterSound() : false;
}

float CAudioVoice::GetCharacterSoundPriority() const {
  return mSound ? (*mSound)->GetCharacterSoundPriority() : 0.f;
}

void CAudioVoice::SetPaused(bool paused) {
  mPaused = paused;
  for (rstl::list< CChildVoice >::iterator it = mChildren.begin(); it != mChildren.end(); ++it) {
    it->SetPaused(paused);
  }
}

bool CAudioVoice::IsSpatial() const { return mSpatial; }

const CVector3f& CAudioVoice::GetPosition() const { return mPosition; }

void CAudioVoice::SetMinimumMaximumDistance(float minimum, float maximum) {
  if (mSpatial) {
    mMinimumDistance = minimum;
    mMaximumDistance = maximum;
    for (rstl::list< CChildVoice >::iterator it = mChildren.begin(); it != mChildren.end(); ++it) {
      it->SetMinimumMaximumDistance(minimum, maximum);
    }
  }
}

rstl::string CAudioVoice::GetSoundName() {
  if (mSound) {
    return (*mSound)->GetName();
  }
  return rstl::string_l("unknown");
}

float CAudioVoice::GetMaximumChildAudibility() {
  float audibility = 0.f;
  for (rstl::list< CChildVoice >::const_iterator it = mChildren.begin(); it != mChildren.end();
       ++it) {
    audibility = CMath::Max(it->GetAudibility(), audibility);
  }
  return audibility;
}

rstl::pair< float, float > CAudioVoice::GetMinimumMaximumDistance() {
  float minimum = mMinimumDistance;
  if (minimum < 0.f) {
    minimum = 1000.f;
    for (rstl::list< CChildVoice >::const_iterator it = mChildren.begin(); it != mChildren.end();
         ++it) {
      minimum = CMath::Min(it->GetSampleDescription().GetMinimumDistance(), minimum);
    }
  }
  float maximum = mMaximumDistance;
  if (maximum < 0.f) {
    for (rstl::list< CChildVoice >::const_iterator it = mChildren.begin(); it != mChildren.end();
         ++it) {
      maximum = CMath::Max(it->GetSampleDescription().GetMaximumDistance(), maximum);
    }
  }
  return rstl::pair< float, float >(minimum, maximum);
}

int CAudioVoice::GetAreaId() const { return mAreaId; }

bool CAudioVoice::IsMuted() const { return mMuted; }

void CAudioVoice::SetMuted(bool muted) {
  mMuted = muted;
  for (rstl::list< CChildVoice >::iterator it = mChildren.begin(); it != mChildren.end(); ++it) {
    it->SetMuted(muted);
  }
}

int CAudioVoice::GetContext() const { return mContext; }

bool CAudioVoice::IsStopping() const { return mStopping; }
