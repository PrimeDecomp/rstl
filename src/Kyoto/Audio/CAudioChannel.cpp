#include "rstl/RstlVersions.h"
#if RSTL_KYOTO_FMOD_AUDIO
#include "Kyoto/Audio/CAudioChannel.hpp"

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Math/CMath.hpp"


// Guessed name
static float CalculateSpeakerMixCornerGain(float x, float y, float cornerX, float cornerY);

static float CalculateSpeakerMixCornerGain(float x, float y, float cornerX, float cornerY) {
  return ((1.f + cornerX * x) + (1.f + cornerY * y)) / 2.f;
}

CAudioChannel::CAudioChannel(FMOD::Channel* channel) : mChannel(channel) {
  float audibility;
  channel->getAudibility(&audibility);
}

void CAudioChannel::SetPosition(const CVector3f& position) {
  SetPositionAndVelocity(position, CVector3f::Zero());
}

void CAudioChannel::SetPositionAndVelocity(const CVector3f& position, const CVector3f& velocity) {
  FMOD_VECTOR fmodPosition = {0.f, 0.f, 0.f};
  FMOD_VECTOR fmodVelocity = {0.f, 0.f, 0.f};
  fmodPosition.x = position.GetX();
  fmodPosition.y = position.GetY();
  fmodPosition.z = position.GetZ();
  fmodVelocity.x = velocity.GetX();
  fmodVelocity.y = velocity.GetY();
  fmodVelocity.z = velocity.GetZ();
  mChannel->set3DAttributes(&fmodPosition, &fmodVelocity);
}

void CAudioChannel::SetPaused(bool paused) { mChannel->setPaused(paused); }

void CAudioChannel::SetVolume(const float& volume) { mChannel->setVolume(volume); }

int CAudioChannel::GetSampleBaseFrequency() const {
  FMOD::Sound* sound;
  if (mChannel->getCurrentSound(&sound) == FMOD_OK) {
    float frequency;
    if (sound->getDefaults(&frequency, nullptr, nullptr, nullptr) == FMOD_OK) {
      return frequency;
    }
  }
  return 0;
}

void CAudioChannel::SetPlaybackFrequency(int frequency) { mChannel->setFrequency(frequency); }

void CAudioChannel::Stop() {
  mChannel->stop();
  mChannel = nullptr;
}

bool CAudioChannel::IsPlaying() const {
  if (mChannel == nullptr) {
    return false;
  }
  bool playing;
  mChannel->isPlaying(&playing);
  return playing;
}

void CAudioChannel::SetSpeakerMixCoordinates(float pan, float surroundPan) {
  pan = CMath::Clamp(-1.f, pan, 1.f);
  surroundPan = CMath::Clamp(-1.f, surroundPan, 1.f);
  const float frontLeft = CalculateSpeakerMixCornerGain(pan, surroundPan, -1.f, 1.f);
  const float frontRight = CalculateSpeakerMixCornerGain(pan, surroundPan, 1.f, 1.f);
  const float backLeft = CalculateSpeakerMixCornerGain(pan, surroundPan, -1.f, -1.f);
  const float backRight = CalculateSpeakerMixCornerGain(pan, surroundPan, 1.f, -1.f);
  mChannel->setSpeakerMix(frontLeft, frontRight, 0.f, 0.f, backLeft, backRight, 0.f, 0.f);
}

void CAudioChannel::SetMinimumMaximumDistance(float minimum, float maximum) {
  FMOD_VECTOR position, velocity;
  if (mChannel->get3DAttributes(&position, &velocity) == FMOD_OK) {
    mChannel->set3DMinMaxDistance(minimum, maximum);
    position.x += 0.01f;
    mChannel->set3DAttributes(&position, &velocity);
    position.x -= 0.01f;
    mChannel->set3DAttributes(&position, &velocity);
  }
}

float CAudioChannel::GetAudibility() const {
  float audibility = 0.f;
  mChannel->getAudibility(&audibility);
  return audibility;
}

void CAudioChannel::SetMuted(bool muted) { mChannel->setMute(muted); }

void CAudioChannel::SetDistanceRolloffPoints(FMOD_VECTOR* points, int count) {
  FMOD_VECTOR* const rolloffPoints = points;
  FMOD_VECTOR position, velocity;
  if (mChannel->get3DAttributes(&position, &velocity) == FMOD_OK) {
    mChannel->set3DCustomRolloff(rolloffPoints, count);
    position.x += 0.01f;
    mChannel->set3DAttributes(&position, &velocity);
    position.x -= 0.01f;
    mChannel->set3DAttributes(&position, &velocity);
  }
}
#endif
