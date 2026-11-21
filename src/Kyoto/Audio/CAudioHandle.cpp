#include "Kyoto/Audio/CAudioHandle.hpp"

#include "Kyoto/Audio/CAudioManager.hpp"
#include "Kyoto/Audio/CAudioVoice.hpp"
#include "Kyoto/Math/CVector3f.hpp"


uint CAudioHandle::mRefCount = 1;

CAudioHandle::CAudioHandle() : mID(0) {}

CAudioHandle::CAudioHandle(int voiceIndex) {
  ++mRefCount;
  if ((mRefCount & 0x7f) == 0) {
    mRefCount = 1;
  }
  mID = (mRefCount << 8) | (voiceIndex & 0x7f);
}

int CAudioHandle::GetVoiceIndex() const { return mID & 0x7f; }

void CAudioHandle::StopWithFade() {
  CAudioVoice* voice = CAudioManager::LookupVoice(*this);
  if (voice != nullptr) {
    voice->StopWithFade();
  }
}

bool CAudioHandle::IsActiveAndNotStopping() const {
  CAudioVoice* voice = CAudioManager::FindVoiceForHandle(*this);
  if (voice != nullptr) {
    return voice->HasAllocatedHandle() && !voice->IsStopping();
  }
  return false;
}

void CAudioHandle::SetPosition(const CVector3f& position) {
  SetPositionAndVelocity(position, CVector3f::Zero());
}

void CAudioHandle::SetPositionAndScaledVelocity(const CVector3f& position,
                                                const CVector3f& velocity, float scale) {
  CAudioVoice* voice = CAudioManager::LookupVoice(*this);
  if (voice != nullptr) {
    voice->SetPositionAndVelocity(position, scale * velocity);
  }
}

void CAudioHandle::SetPositionAndVelocity(const CVector3f& position, const CVector3f& velocity) {
  CAudioVoice* voice = CAudioManager::LookupVoice(*this);
  if (voice != nullptr) {
    voice->SetPositionAndVelocity(position, velocity);
  }
}

void CAudioHandle::SetPan(float pan) {
  CAudioVoice* voice = CAudioManager::LookupVoice(*this);
  if (voice != nullptr) {
    voice->SetPan(pan);
  }
}

void CAudioHandle::SetSurroundPan(float pan) {
  CAudioVoice* voice = CAudioManager::LookupVoice(*this);
  if (voice != nullptr) {
    voice->SetSpeakerMixSecondAxis(pan);
  }
}

void CAudioHandle::SetVolume(float volume) {
  CAudioVoice* voice = CAudioManager::LookupVoice(*this);
  if (voice != nullptr) {
    voice->SetVolume(volume);
  }
}

void CAudioHandle::SetPitchBend(float pitch) {
  CAudioVoice* voice = CAudioManager::LookupVoice(*this);
  if (voice != nullptr) {
    voice->SetPitchBend(pitch);
  }
}

float CAudioHandle::GetElapsedSeconds() const {
  CAudioVoice* voice = CAudioManager::LookupVoice(*this);
  if (voice != nullptr) {
    return voice->GetElapsedSeconds();
  }
  return 0.f;
}

void CAudioHandle::SetMinimumMaximumDistance(float minimum, float maximum) {
  CAudioVoice* voice = CAudioManager::LookupVoice(*this);
  if (voice != nullptr) {
    voice->SetMinimumMaximumDistance(minimum, maximum);
  }
}

bool CAudioHandle::IsLooping() const {
  CAudioVoice* voice = CAudioManager::LookupVoice(*this);
  if (voice != nullptr) {
    return voice->IsLooping();
  }
  return false;
}

bool CAudioHandle::IsExclusiveCharacterSound() const {
  CAudioVoice* voice = CAudioManager::LookupVoice(*this);
  if (voice != nullptr) {
    return voice->IsExclusiveCharacterSound();
  }
  return false;
}

float CAudioHandle::GetCharacterSoundPriority() const {
  CAudioVoice* voice = CAudioManager::LookupVoice(*this);
  if (voice != nullptr) {
    return voice->GetCharacterSoundPriority();
  }
  return 0.f;
}
