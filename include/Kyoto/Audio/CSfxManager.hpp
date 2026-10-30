#ifndef _CSFXMANAGER
#define _CSFXMANAGER
#include "rstl/RstlVersions.h"

#include "rstl/optional_object.hpp"
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
#include "Kyoto/Audio/CAuxEffectManager.hpp"
#include "Kyoto/Audio/CSfxPitchBend.hpp"
#endif

#include "types.h"

#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Audio/CSfxHandle.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/pair.hpp"

#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

#include "musyx/musyx.h"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "Kyoto/CFactoryFnReturn.hpp"
#else
struct SObjectTag;
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
class CVector3f;
#else
class CToken;
class CSimplePool;
class CSfxPitchBend;
struct SFlangerAuxParameters;
struct SBitcrusherAuxParameters;
struct SPhaserAuxParameters;
#endif

class CSfxManager {
public:
  // Guessed names for the balanced Wii audio-suppression API.
  static void PushMute();
  static void PopMute();

  enum ESfxChannels {
    kSC_Invalid = -1,
    kSC_Default = 0,
    kSC_Game,
    kSC_PauseScreen,
#if !RSTL_PRIME1
    kSC_SoftPaused // Guessed name: selected by CStateManager while gameplay is soft-paused.
#endif
  };
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  enum ESfxAudibility {
    kSA_Aud0,
    kSA_Aud1,
    kSA_Aud2,
    kSA_Aud3,
  };

  enum EAuxEffect {
    kAE_None = -1,
    kAE_ReverbHi = 0,
    kAE_Chorus,
    kAE_ReverbStd,
    kAE_Delay,
  };
#else
  enum ESfxWrapperType { kSWT_Sound, kSWT_Emitter }; // Guessed name
  enum ESfxAudibility { kSA_Inaudible, kSA_Low, kSA_Medium, kSA_High };
#endif

  class CBaseSfxWrapper {
  public:
    CBaseSfxWrapper(RSTL_PRIME1_CONST bool looped, RSTL_PRIME1_CONST short priority,
                    RSTL_PRIME1_CONST CSfxHandle handle, RSTL_PRIME1_CONST bool useAcoustics,
                    RSTL_PRIME1_CONST int area);

    virtual ~CBaseSfxWrapper() = 0;
    virtual void SetActive(bool active);
    virtual void SetPlaying(bool playing);
    virtual void SetRank(RSTL_PRIME1_CONST short rank);
    virtual void SetInArea(bool inArea);
    virtual RSTL_PRIME1_CONST bool IsLooped() const;

    virtual bool IsPlaying() const;
    virtual RSTL_PRIME1_CONST bool IsActive() const;
    virtual RSTL_PRIME1_CONST bool IsInArea() const;
    virtual RSTL_PRIME1_CONST bool UseAcoustics() const;
    virtual RSTL_PRIME1_CONST int GetRank() const;
    virtual RSTL_PRIME1_CONST int GetPriority() const;
    virtual RSTL_PRIME1_CONST int GetArea() const;
    virtual RSTL_PRIME1_CONST CSfxHandle GetSfxHandle() const;
#if !RSTL_PRIME1
    virtual ESfxWrapperType GetType() const = 0; // Guessed name
#endif

    virtual void Play() = 0;
    virtual void Stop() = 0;
    virtual RSTL_PRIME1_CONST bool Ready() = 0;
    virtual short GetAudible(const CVector3f& position) = 0;
    virtual RSTL_PRIME1_CONST SND_VOICEID GetVoice() const = 0;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    virtual const SND_FXID GetSfxId() = 0;
#else
    virtual ushort GetSfxId() = 0;
#endif

    virtual void UpdateEmitterSilent() = 0;
    virtual void UpdateEmitter() = 0;
    virtual void SetReverb(RSTL_PRIME1_CONST char reverb) = 0;
    RSTL_PRIME1_CONST bool Available() const;

    void Release();
    RSTL_PRIME1_CONST float GetTimeRemaining();
    void SetTimeRemaining(float time);
#if !RSTL_PRIME1
    ushort GetPitchBend() const;
    void SetPitchBend(ushort pitch);
    bool GetIgnoreAreaLowPass() const;
    void SetIgnoreAreaLowPass(bool ignore);
#endif

  private:
    float mTimeRemaining;
    short mRank;
    short mPriority;
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
    ushort mPitchBend;
#endif

    CSfxHandle mHandle;
    int mArea;
    bool mActive : 1;
    bool mPlaying : 1;
    bool mLooped : 1;
    bool mInArea : 1;
    bool mReleased : 1;
    bool mUseAcoustics : 1;
#if !RSTL_PRIME1
    bool mIgnoreAreaLowPass : 1; // Guessed name
#endif
  };

  class CSfxEmitterWrapper : public CBaseSfxWrapper {
  public:
    CSfxEmitterWrapper(RSTL_PRIME1_CONST bool looped, short priority,
                       CAudioSys::C3DEmitterParmData& emitter, RSTL_PRIME1_CONST CSfxHandle handle,
                       RSTL_PRIME1_CONST bool useAcoustics, RSTL_PRIME1_CONST int area);

    // CBaseSfxWrapper
    ~CSfxEmitterWrapper() {}
    bool IsPlaying() const override;
#if !RSTL_PRIME1
    ESfxWrapperType GetType() const override;
#endif

    void Play() override;
    void Stop() override;
    RSTL_PRIME1_CONST bool Ready() override;
    short GetAudible(const CVector3f& position) override;
    RSTL_PRIME1_CONST SND_VOICEID GetVoice() const override;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const SND_FXID GetSfxId() override;
#else
    ushort GetSfxId() override;
#endif

    void UpdateEmitterSilent() override;
    void UpdateEmitter() override;
    void SetReverb(RSTL_PRIME1_CONST char reverb) override;

    CAudioSys::C3DEmitterParmData& GetEmitter();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const SND_VOICEID GetHandle() const;
#else
    const CAudioSys::C3DEmitterParmData& GetEmitter() const;
    uint GetHandle() const;
    bool IsSilent() const; // Guessed name
    bool IsUpdatePending() const { return mUpdatePending; } // Guessed name
    uchar GetCachedMaxVolume() const { return mCachedMaxVolume; } // Guessed name
    void ClearUpdatePending() { mUpdatePending = false; } // Guessed name
#endif

  private:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    SND_PARAMETER mPara;
#else
    SND_PARAMETER mParameters[3];
#endif

    SND_PARAMETER_INFO mParameterInfo;
    CAudioSys::C3DEmitterParmData mEmitterData;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    SND_VOICEID mEmitterHandle;
    bool mReady;
    char mCachedMaxVol;
#else
    uint mEmitterHandle;
    uchar mCachedMaxVolume;
    bool mReady : 1;
    bool mUpdatePending : 1; // Restores the cached volume in Update.
#endif
  };

  class CSfxWrapper : public CBaseSfxWrapper {
  public:
    CSfxWrapper(RSTL_PRIME1_CONST bool looped, RSTL_PRIME1_CONST short priority,
                RSTL_PRIME1_CONST ushort sfxId, RSTL_PRIME1_CONST short volume,
                RSTL_PRIME1_CONST short pan, RSTL_PRIME1_CONST CSfxHandle handle,
                RSTL_PRIME1_CONST bool useAcoustics, RSTL_PRIME1_CONST int area);

    // CBaseSfxWrapper
    ~CSfxWrapper() {}

    bool IsPlaying() const override;
#if !RSTL_PRIME1
    ESfxWrapperType GetType() const override;
#endif

    void Play() override;
    void Stop() override;
    RSTL_PRIME1_CONST bool Ready() override;
    short GetAudible(const CVector3f& position) override;
    RSTL_PRIME1_CONST SND_VOICEID GetVoice() const override;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const SND_FXID GetSfxId() override;
#else
    ushort GetSfxId() override;
#endif

    void UpdateEmitterSilent() override;
    void UpdateEmitter() override;
    void SetReverb(RSTL_PRIME1_CONST char reverb) override;
    void SetVolume(short volume);

  private:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    SND_FXID mSfxId;
#else
    ushort mSfxId;
#endif

    SND_VOICEID mVoiceHandle;
    short mVolume;
    short mPan;
    bool mReady;
  };
  class CSfxListener {
  public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    explicit CSfxListener(const CVector3f pos = CVector3f::Zero(),
                          const CVector3f dir = CVector3f::Zero(),
                          const CVector3f vec1 = CVector3f::Zero(),
                          const CVector3f vec2 = CVector3f::Zero(), const float f1 = 0.f,
                          const float f2 = 0.f, const float f3 = 0.f, const uint w1 = 0,
#if RSTL_VERSION >= RSTL_GM8P_00
                          uchar maxVolume = 0)
#else
                          const uchar maxVolume = 0)
#endif
    : x0_(pos)
    , xc_(dir)
    , x18_(vec1)
    , x24_(vec2)
    , x30_(f1)
    , x34_(f2)
    , x38_(f3)
    , x3c_(w1)
    , x40_(maxVolume) {}
#else
    CSfxListener(CVector3f position = CVector3f::Zero(), CVector3f direction = CVector3f::Zero(),
                 CVector3f heading = CVector3f::Zero(), CVector3f up = CVector3f::Zero(),
                 float frontSur = 0.f, float backSur = 0.f, float soundSpeed = 0.f, uint flags = 0,
                 uchar maxVolume = 0);
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CVector3f x0_;
    CVector3f xc_;
    CVector3f x18_;
    CVector3f x24_;
    float x30_;
    float x34_;
    float x38_;
    int x3c_;
    char x40_;
#else
    CVector3f mPosition;
    CVector3f mDirection;
    CVector3f mHeading;
    CVector3f mUp;
    float mFrontSur;
    float mBackSur;
    float mSoundSpeed;
    uint mFlags;
    uchar mMaxVolume;
#endif
  };

// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  struct SListener { // Guessed name
    SListener();
    CSfxListener mListener;
    bool mActive;
  };
#endif

  class CSfxChannel {
  public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CSfxChannel() : x44_(false) {}
    CSfxListener mListener;
    bool x44_;
#else
    CSfxChannel();
    rstl::reserved_vector< SListener, 4 > mListeners;
#endif
    rstl::reserved_vector< CBaseSfxWrapper*, 72 > mSounds;
  };

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static void Update(float dt);
  static void RemoveEmitter(CSfxHandle handle);
  static void UpdateEmitter(CSfxHandle handle, const CVector3f& pos, const CVector3f& dir,
                            const uchar maxVol);
  static void UpdateListener(const CVector3f& pos, const CVector3f& dir, const CVector3f&,
                             const CVector3f&, const uchar);
#else
  struct SLowPassFilter { // Guessed name
    SLowPassFilter(int frequency, float duration, int id)
    : mFrequency(frequency), mTimeRemaining(duration), mId(id), mTimed(duration > 0.f) {}
    int mFrequency;
    float mTimeRemaining;
    int mId;
    bool mTimed : 1;
  };
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static const short kMaxPriority;           // 0xFF
  static const short kMedPriority;           // 0x7F
  static const ushort kInternalInvalidSfxId; // 0xFFFF
  static const int kAllAreas;                // 0xFFFFFFFF
  static CSfxHandle AddEmitter(const SND_FXID id, const CVector3f& pos, const CVector3f& dir,
                               const bool useAcoustics = false, const bool looped = false,
                               const short prio = kMedPriority, const int areaId = kAllAreas);
  static CSfxHandle AddEmitter(const SND_FXID id, const CVector3f& pos, const CVector3f& dir,
                               const uchar vol, const bool useAcoustics = false,
                               const bool looped = false, const short prio = kMedPriority,
                               const int areaId = kAllAreas);
  static CSfxHandle AddEmitter(CAudioSys::C3DEmitterParmData& parmData,
                               bool useAcoustics = false, const short prio = kMedPriority,
                               const bool looped = false, const int areaId = kAllAreas);
#else
  typedef rstl::pair< int, uchar > SAreaVolume; // Guessed name
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static void AddListener(ESfxChannels channel, const CVector3f& pos, const CVector3f& dir,
                          const CVector3f& vec1, const CVector3f& vec2, float f1, float f2,
                          float f3, uint w1, const uchar maxVolume);
#else
  static const short kMaxPriority;
  static const short kMedPriority;
  static const ushort kInternalInvalidSfxId;
  static const int kAllAreas;
#endif

#if !RSTL_PRIME1
  static void Initialize(); // Guessed name
#endif

  static void Shutdown();
  static void StopAndRemoveAllEmitters();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static bool LoadTranslationTable(CSimplePool* pool, const SObjectTag* tag);
  static ushort TranslateSFXID(ushort);
  static void SetActiveAreas(const rstl::reserved_vector< int, 10 >& areas);

  static void PitchBend(CSfxHandle handle, int pitch);
  static void SetDuration(CSfxHandle handle, float duration);
  static const short GetReverbAmount();

  static CSfxHandle SfxStart(const ushort id, const short vol = 127, const short pan = 64,
                             bool useAcoustics = false, const short prio = kMedPriority,
                             const bool looped = false, const int areaId = kAllAreas);
#else
  static void Update(float dt);
  static CSfxHandle SfxStart(ushort id, short volume, short pan, int area = kAllAreas,
                             bool useAcoustics = false, bool looped = false,
                             const short priority = kMedPriority);
  static CSfxHandle AddEmitter(const ushort id, const CVector3f& position, int area = kAllAreas,
                               bool useAcoustics = false, bool looped = false,
                               const short priority = kMedPriority);
  static CSfxHandle AddEmitter(const ushort id, const CVector3f& position, uchar volume, int area,
                               bool useAcoustics, bool looped, const short priority);
  static CSfxHandle AddEmitter(CAudioSys::C3DEmitterParmData& params, int area = kAllAreas,
                               bool useAcoustics = false, bool looped = false,
                               const short priority = kMedPriority);
  static void RemoveEmitter(CSfxHandle handle);
  static void UpdateEmitter(CSfxHandle handle, const CVector3f& position,
                            const CVector3f& direction, uchar maxVolume);
#endif

  static void SfxStop(CSfxHandle handle);
  static void SfxStop(ESfxChannels channel, CSfxHandle handle);
#if !RSTL_PRIME1
  static void StopSound(ESfxChannels channel, CSfxHandle handle);
#endif

  static void SfxVolume(CSfxHandle handle, uchar volume);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static void SfxSpan(CSfxHandle, uchar);
#else
  static void SfxPan(CSfxHandle handle, uchar pan);
  static void SfxSpan(CSfxHandle handle, uchar span);
  static void PitchBend(CSfxHandle handle, int pitch);
  static void AddPitchBend(const CSfxPitchBend& pitchBend); // Guessed name
  static void SetDuration(CSfxHandle handle, float duration);
  static bool IsPlaying(CSfxHandle handle);
  static bool IsQueued(CSfxHandle handle);
  static void SetMuted(bool muted);
  static void SetChannel(ESfxChannels channel);
  static ESfxChannels GetChannel();
  static void KillAll(ESfxChannels channel);
  static void TurnOnChannel(ESfxChannels channel);
  static void TurnOffChannel(ESfxChannels channel);
  static void AddListener(ESfxChannels channel, const CVector3f& position,
                          const CVector3f& direction, const CVector3f& heading, const CVector3f& up,
                          float frontSur, float backSur, float soundSpeed, uint flags,
                          const uchar maxVolume, int listener);
  static void UpdateListener(const CVector3f& position, const CVector3f& direction,
                             const CVector3f& heading, const CVector3f& up, uchar maxVolume,
                             int listener);
  static void SetActiveAreas(const rstl::reserved_vector< int, 10 >& areas, int currentArea);
  static ushort TranslateSFXID(ushort id);
  static bool LoadTranslationTable(CSimplePool* pool, const SObjectTag* tag);
  static CSfxHandle LocateHandle(int priority);
  static int GetRank(CBaseSfxWrapper* sound);
  static CSfxWrapper* AllocateCSfxWrapper(const CSfxWrapper& sound);
  static CSfxEmitterWrapper* AllocateCSfxEmitterWrapper(const CSfxEmitterWrapper& sound);
  static short GetReverbAmount();
  static uchar GetStudio(int area);                                 // Guessed name
  static void SetAreaVolume(int area, uchar volume);                // Guessed name
  static uchar GetAreaVolume(int area);                             // Guessed name
  static void SetIgnoreAreaLowPass(CSfxHandle handle, bool ignore); // Guessed name
  static int AddLowPassAreaFilter(int frequency, float duration);
  static void RemoveLowPassAreaFilter(int id);
  static void UpdateLowPassAreaFilters(float dt);             // Guessed name
  static int GetLowPassAreaFrequency();                       // Guessed name
  static bool IsLowPassAreaFilterEnabled();                   // Guessed name
  static int AddLowPassFilter(int frequency, float duration); // Guessed name
  static void RemoveLowPassFilter(int id);                    // Guessed name
  static void UpdateLowPassFilters(float dt);                 // Guessed name
  static int GetLowPassFrequency();                           // Guessed name
  static bool IsLowPassEnabled();                             // Guessed name
  static bool ShouldApplyLowPass(CBaseSfxWrapper* sound);     // Guessed name
  static int GetLowPassFrequency(CBaseSfxWrapper* sound);     // Guessed name
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static bool IsPlaying(CSfxHandle handle);
  static void StopSound(CSfxHandle handle);
#else
  // Guessed names; each registration returns an effect ID (zero is invalid).
  static void RemoveAuxEffect(int id);
  static int AddAuxEffect(int area, const SND_AUX_REVERBHI& params, uchar volume, int priority);
  static int AddAuxEffect(int area, const SND_AUX_CHORUS& params, uchar volume, int priority);
  static int AddAuxEffect(int area, const SND_AUX_REVERBSTD& params, uchar volume, int priority);
  static int AddAuxEffect(int area, const SND_AUX_DELAY& params, uchar volume, int priority);
  static int AddAuxEffect(int area, const SFlangerAuxParameters& params, uchar volume,
                          int priority);
  static int AddAuxEffect(int area, const SBitcrusherAuxParameters& params, uchar volume,
                          int priority);
  static int AddAuxEffect(int area, const SPhaserAuxParameters& params, uchar volume, int priority);
  static int AddAuxEffect(int area, const SFilteredDelayAuxParameters& params, uchar volume,
                          int priority);
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static void SetChannel(ESfxChannels);
  static void KillAll(ESfxChannels);
  static void TurnOnChannel(ESfxChannels);
  static void TurnOffChannel(ESfxChannels);
  static CSfxHandle LocateHandle(const short id);

  static CSfxEmitterWrapper* AllocateCSfxEmitterWrapper(const CSfxEmitterWrapper& wrapper);
  static CSfxWrapper* AllocateCSfxWrapper(const CSfxWrapper& wrapper);
  static const bool IsAuxProcessingEnabled();
  static void ApplyReverb();
  static void DisableAuxCallbacks();
  static void EnableAuxCallbacks();
  static void PrepareDelayCallback(const SND_AUX_DELAY& info);
  static void PrepareReverbStdCallback(const SND_AUX_REVERBSTD& info);
  static void PrepareChorusCallback(const SND_AUX_CHORUS& info);
  static void PrepareReverbHiCallback(const SND_AUX_REVERBHI& info);
  static void DisableAuxProcessing();
#else
private:
  static void UpdatePitchBends(float dt);
  static int RegisterAuxEffect(const CAuxEffect& effect);
  static CAuxEffectManager mAuxEffectManager;
  static rstl::reserved_vector< CAuxEffect, 10 > mAuxEffects;
  static rstl::reserved_vector< CSfxPitchBend, 8 > mPitchBends;
  static int mNextAuxEffectId;
  static rstl::pair< int, bool > mStudioState;
#endif

  static CSfxChannel mChannels[4];
  static ESfxChannels mCurrentChannel;
  static bool mDoUpdate;
#if RSTL_PRIME1
  static EAuxEffect mCurrentAuxEffect;
  static EAuxEffect mRequestedAuxEffect;
  static float mReverbAmount;
  static float mReverbScale;
#endif
  static rstl::vector< short >* mTranslationTable;

  static bool mMuted;
  static rstl::auto_ptr< CToken > mTranslationTableTok;
#if RSTL_PRIME1
  static bool mAuxProcessingEnabled;
#endif

  static rstl::reserved_vector< CSfxEmitterWrapper, 64 > mEmitterWrapperPool;
  static rstl::reserved_vector< CSfxWrapper, 64 > mWrapperPool;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static bool IsHandleValid(CSfxHandle handle);
  static int GetRank(CBaseSfxWrapper* wrapper);
#else
  static rstl::reserved_vector< SLowPassFilter, 8 > mAreaLowPassFilters;
  static rstl::reserved_vector< SLowPassFilter, 8 > mLowPassFilters;
  static int mNextAreaFilterId;
  static int mNextFilterId;
  static int mAreaLowPassFrequency;
  static int mLowPassFrequency;
  static rstl::reserved_vector< SAreaVolume, 10 > mAreaVolumes;
  static int mCurrentArea;
#endif
};

inline CSfxManager::CBaseSfxWrapper::~CBaseSfxWrapper() {}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
const CFactoryFnReturn FAudioTranslationTableFactory(const SObjectTag& obj, CInputStream& in,
                                                     const CVParamTransfer& xfer);
#else
NESTED_CHECK_SIZEOF(CSfxManager, CBaseSfxWrapper, 0x1c)
NESTED_CHECK_SIZEOF(CSfxManager, CSfxWrapper, 0x2c)
NESTED_CHECK_SIZEOF(CSfxManager, CSfxEmitterWrapper, 0x64)
NESTED_CHECK_SIZEOF(CSfxManager, CSfxListener, 0x44)
NESTED_CHECK_SIZEOF(CSfxManager, SListener, 0x48)
NESTED_CHECK_SIZEOF(CSfxManager, CSfxChannel, 0x248)
NESTED_CHECK_SIZEOF(CSfxManager, SLowPassFilter, 0x10)
#endif

#endif // _CSFXMANAGER
