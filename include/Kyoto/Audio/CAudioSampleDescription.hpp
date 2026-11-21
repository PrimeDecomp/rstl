#ifndef _CAUDIOSAMPLEDESCRIPTION
#define _CAUDIOSAMPLEDESCRIPTION

#include "rstl/static_assert.hpp"
#include "Kyoto/Audio/CAudioSample.hpp"
#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"


class CAudioSampleDescription { // Guessed name
public:
  CAudioSampleDescription(CInputStream& in, IObjectStore& pool);
  ~CAudioSampleDescription() {}
  rstl::reserved_vector< CVector3f, 5 >
  BuildDistanceRolloffPoints(float minimum,
                             float maximum) const;                                   // Guessed name
  float GetFadeOutSeconds() const;                                                   // Guessed name
  void DecrementActiveVoiceCount();                                                  // Guessed name
  void IncrementActiveVoiceCount();                                                  // Guessed name
  float GetMaximumDistance() const;                                                  // Guessed name
  float GetMinimumDistance() const;                                                  // Guessed name
  bool UsesPositionAndVelocity() const;                                              // Guessed name
  float EvaluateSpeakerMixSecondAxisCurve(float time) const;                         // Guessed name
  float EvaluatePanCurve(float time) const;                                          // Guessed name
  float EvaluatePitchCurve(float time) const;                                        // Guessed name
  float EvaluateVolumeCurve(float time) const;                                       // Guessed name
  bool IsSampleLoaded() const;                                                       // Guessed name
  CAudioChannel CreateSpatialChannel(const CVector3f& position, float volume) const; // Guessed name
  CAudioChannel CreatePannedChannel(float volume, float pan) const;                  // Guessed name
  float GetVolume() const { return mVolume; }                                        // Guessed name
  int GetPitch() const { return mPitch; }                                            // Guessed name
  int GetPitchVariation() const { return mPitchVariation; }                          // Guessed name
  int GetPitchBendRange() const { return mPitchBendRange; }                          // Guessed name
  float GetProbability() const { return mProbability; }                              // Guessed name
  // Guessed name
  const rstl::vector< rstl::pair< float, float > >& GetTriggers() const { return mTriggers; }

private:
  TLockedToken< CAudioSample > mSample;
  float mRolloff;
  int mMaximumVoices;
  int x14_;
  float mMinimumDistance;
  float mMaximumDistance;
  float mVolume;
  int mFadeOutMilliseconds;
  int mPitch;
  int mPitchVariation;
  int mPitchBendRange;
  float mProbability;
  uint mUseVelocity : 1;
  uint x38_1_ : 1;
  uint x38_2_ : 1;
  uint x38_3_ : 1;
  uint x38_4_ : 1;
  uint x38_5_ : 1;
  int mActiveVoiceCount;
  CMayaSpline mVolumeCurve;
  CMayaSpline mPitchCurve;
  CMayaSpline mPanCurve;
  CMayaSpline mSurroundPanCurve;
  rstl::vector< rstl::pair< float, float > > mTriggers;
};
CHECK_SIZEOF(CAudioSampleDescription, 0x160)


#endif // _CAUDIOSAMPLEDESCRIPTION
