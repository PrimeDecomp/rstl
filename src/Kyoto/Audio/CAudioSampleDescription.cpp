#include "Kyoto/Audio/CAudioSampleDescription.hpp"

#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"


static CVector2f sDistanceRolloffProfiles[3][5] = {
    {CVector2f(0.f, 1.f), CVector2f(0.9f, 0.97f), CVector2f(0.95f, 0.95f), CVector2f(0.97f, 0.9f),
     CVector2f(1.f, 0.f)},
    {CVector2f(0.f, 1.f), CVector2f(0.3f, 0.7f), CVector2f(0.5f, 0.5f), CVector2f(0.7f, 0.3f),
     CVector2f(1.f, 0.f)},
    {CVector2f(0.f, 1.f), CVector2f(0.03f, 0.1f), CVector2f(0.05f, 0.05f), CVector2f(0.1f, 0.03f),
     CVector2f(1.f, 0.f)}};

CAudioSampleDescription::CAudioSampleDescription(CInputStream& in, IObjectStore& pool)
: mSample(pool.GetObj(SObjectTag('CSMP', CAssetId(in))))
, mRolloff(in.ReadFloat())
, mMaximumVoices(in.ReadInt32())
, x14_(in.ReadInt32())
, mMinimumDistance(in.ReadFloat())
, mMaximumDistance(in.ReadFloat())
, mVolume(in.ReadFloat())
, mFadeOutMilliseconds(in.ReadInt32())
, mPitch(in.ReadInt32())
, mPitchVariation(in.ReadInt32())
, mPitchBendRange(in.ReadInt32())
, mProbability(in.ReadFloat())
, mUseVelocity(in.ReadBool())
, x38_1_(in.ReadBool())
, x38_2_(in.ReadBool())
, x38_3_(in.ReadBool())
, x38_4_(in.ReadBool())
, x38_5_(in.ReadBool())
, mActiveVoiceCount(0)
, mVolumeCurve(in)
, mPitchCurve(in)
, mPanCurve(in)
, mSurroundPanCurve(in)
, mTriggers(in) {}

CAudioChannel CAudioSampleDescription::CreatePannedChannel(float volume, float pan) const {
  if (mMaximumVoices != 0 && mActiveVoiceCount >= mMaximumVoices) {
    return CAudioChannel(nullptr);
  }
  if (IsSampleLoaded()) {
    CAudioChannel channel = mSample->CreatePannedChannel(true);
    channel.SetVolume(volume * (mVolume * EvaluateVolumeCurve(0.f)));
    channel.SetSpeakerMixCoordinates(pan + EvaluatePanCurve(0.f),
                                     EvaluateSpeakerMixSecondAxisCurve(0.f));
    return channel;
  }
  return CAudioChannel(nullptr);
}

CAudioChannel CAudioSampleDescription::CreateSpatialChannel(const CVector3f& position,
                                                            float volume) const {
  if (mMaximumVoices != 0 && mActiveVoiceCount >= mMaximumVoices) {
    return CAudioChannel(nullptr);
  }
  if (IsSampleLoaded()) {
    CAudioChannel channel =
        mSample->CreateSpatialChannel(position, mMinimumDistance, mMaximumDistance, true);
    rstl::reserved_vector< CVector3f, 5 > points =
        BuildDistanceRolloffPoints(mMinimumDistance, mMaximumDistance);
    channel.SetVolume(volume * (mVolume * EvaluateVolumeCurve(0.f)));
    return channel;
  }
  return CAudioChannel(nullptr);
}

bool CAudioSampleDescription::IsSampleLoaded() const { return mSample.IsLoaded(); }

float CAudioSampleDescription::EvaluateVolumeCurve(float time) const {
  if (mVolumeCurve.GetKnotCount() == 0) {
    return 1.f;
  }
  return mVolumeCurve.EvaluateAt(time);
}

float CAudioSampleDescription::EvaluatePitchCurve(float time) const {
  return mPitchCurve.EvaluateAt(time);
}

float CAudioSampleDescription::EvaluatePanCurve(float time) const {
  return mPanCurve.EvaluateAt(time);
}

float CAudioSampleDescription::EvaluateSpeakerMixSecondAxisCurve(float time) const {
  return mSurroundPanCurve.EvaluateAt(time);
}

bool CAudioSampleDescription::UsesPositionAndVelocity() const { return mUseVelocity; }

float CAudioSampleDescription::GetMinimumDistance() const { return mMinimumDistance; }

float CAudioSampleDescription::GetMaximumDistance() const { return mMaximumDistance; }

void CAudioSampleDescription::IncrementActiveVoiceCount() { ++mActiveVoiceCount; }

void CAudioSampleDescription::DecrementActiveVoiceCount() { --mActiveVoiceCount; }

float CAudioSampleDescription::GetFadeOutSeconds() const { return mFadeOutMilliseconds / 1000.f; }

rstl::reserved_vector< CVector3f, 5 >
CAudioSampleDescription::BuildDistanceRolloffPoints(float minimum, float maximum) const {
  const CVector2f* lower =
      mRolloff < 0.f ? sDistanceRolloffProfiles[0] : sDistanceRolloffProfiles[1];
  const CVector2f* upper =
      mRolloff < 0.f ? sDistanceRolloffProfiles[1] : sDistanceRolloffProfiles[2];
  const float factor = mRolloff < 0.f ? 1.f + mRolloff : mRolloff;
  rstl::reserved_vector< CVector3f, 5 > points;
  for (int i = 0; i < 5; ++i) {
    const CVector2f& upperPoint = upper[i];
    const CVector2f& point = CVector2f::Lerp(lower[i], upperPoint, factor);
    const float gain = point.GetY();
    points.push_back(CVector3f((maximum - minimum) * point.GetX() + minimum, gain, 0.f));
  }
  return points;
}
