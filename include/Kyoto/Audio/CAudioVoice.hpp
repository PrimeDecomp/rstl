#ifndef _CAUDIOVOICE
#define _CAUDIOVOICE

#include "rstl/static_assert.hpp"
#include "Kyoto/Audio/CAudioHandle.hpp"
#include "Kyoto/Audio/CAudioSoundEffect.hpp"
#include "rstl/list.hpp"
#include "rstl/optional_object.hpp"


class CAudioVoice { // Guessed name
public:
  class CChildVoice { // Guessed name
  public:
    CChildVoice(const CAudioChannel& channel, CAudioSampleDescription& description, bool panned,
                float duration);
    CChildVoice(const CChildVoice& other);
    ~CChildVoice();
    void Stop();                                                  // Guessed name
    void SetMuted(bool muted);                                    // Guessed name
    const CAudioSampleDescription& GetSampleDescription() const;  // Guessed name
    float GetAudibility() const;                                  // Guessed name
    void SetMinimumMaximumDistance(float minimum, float maximum); // Guessed name
    void SetPaused(bool paused);                                  // Guessed name
    void SetPositionAndVelocity(const CVector3f& position,
                                const CVector3f& velocity); // Guessed name
    bool IsPlaying() const;                                 // Guessed name
    void SetSpeakerMixSecondAxis(float pan);                // Guessed name
    void SetPan(float pan);                                 // Guessed name
    void SetVolume(float volume);                           // Guessed name
    void SetPitchBend(float pitch);                         // Guessed name
    void StopWithFade();                                    // Guessed name
    void Update(float dt, float voiceElapsed);              // Guessed name
    int CalculatePlaybackFrequency(float pitchBend) const;  // Guessed name

  private:
    CAudioChannel mChannel;
    CAudioSampleDescription& mDescription;
    rstl::reserved_vector< CVector3f, 5 > mRolloffPoints;
    float mDuration;
    float mElapsed;
    float mFadeElapsed;
    int mPitch;
    float mVolume;
    float mPitchBend;
    float mPan;
    float mSurroundPan;
    bool mPanned : 1;
    bool mPendingUnpause : 1;
    bool mPaused : 1;
    bool mStopping : 1;
  };

  CAudioVoice();
  bool IsStopping() const;                                                           // Guessed name
  int GetContext() const;                                                            // Guessed name
  void SetMuted(bool muted);                                                         // Guessed name
  bool IsMuted() const;                                                              // Guessed name
  int GetAreaId() const;                                                             // Guessed name
  rstl::pair< float, float > GetMinimumMaximumDistance();                            // Guessed name
  float GetMaximumChildAudibility();                                                 // Guessed name
  rstl::string GetSoundName();                                                       // Guessed name
  void SetMinimumMaximumDistance(float minimum, float maximum);                      // Guessed name
  const CVector3f& GetPosition() const;                                              // Guessed name
  bool IsSpatial() const;                                                            // Guessed name
  void SetPaused(bool paused);                                                       // Guessed name
  float GetCharacterSoundPriority() const;                                           // Guessed name
  bool IsExclusiveCharacterSound() const;                                            // Guessed name
  bool IsLooping() const;                                                            // Guessed name
  float GetElapsedSeconds() const;                                                   // Guessed name
  void SetPositionAndVelocity(const CVector3f& position, const CVector3f& velocity); // Guessed name
  void SetSpeakerMixSecondAxis(float pan);                                           // Guessed name
  void SetPitchBend(float pitch);                                                    // Guessed name
  void SetPan(float pan);                                                            // Guessed name
  void SetVolume(float volume);                                                      // Guessed name
  void StopWithFade();                                                               // Guessed name
  void Update(float dt);                                                             // Guessed name
  bool HasAllocatedHandle() const;                                                   // Guessed name
  CAudioHandle GetHandle() const;                                                    // Guessed name
  void ReleaseSound();                                                               // Guessed name
  void Play(const CToken& sound, float volume, float pan, int areaId, bool spatial); // Guessed name
  // Guessed name
  void PlaySpatial(const CToken& sound, const CVector3f& position, float volume, int areaId);
  void SetHandle(const CAudioHandle& handle); // Guessed name

private:
  rstl::optional_object< TLockedToken< CAudioSoundEffect > > mSound;
  CAudioHandle mHandle;
  float mElapsed;
  float mVolume;
  float mPitchBend;
  float mPan;
  float mSurroundPan;
  float mMinimumDistance;
  float mMaximumDistance;
  CVector3f mPosition;
  CVector3f mVelocity;
  int mAreaId;
  int mContext;
  float x50_;
  rstl::list< CChildVoice > mChildren;
  bool mSpatial : 1;
  bool mPaused : 1;
  bool mMuted : 1;
  bool mStopping : 1;
};
CHECK_SIZEOF(CAudioVoice, 0x70)
NESTED_CHECK_SIZEOF(CAudioVoice, CChildVoice, 0x6c)


#endif // _CAUDIOVOICE
