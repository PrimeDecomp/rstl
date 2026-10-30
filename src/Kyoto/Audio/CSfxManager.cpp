#include "rstl/RstlVersions.h"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/CToken.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/TToken.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
// TODO: verify that this Prime/Echoes difference is real
#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "dolphin/types.h"
#include "musyx/musyx.h"
#endif

#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"
// TODO: verify that this Prime/Echoes difference is real
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"
#include "types.h"
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "Prime/MetroidPrime/CGameDebug.hpp"
#else
#include "math.h"
#endif

#if RSTL_PRIME1
float CSfxManager::mReverbAmount = 1.f;
float CSfxManager::mReverbScale = 0.1f;
CSfxManager::EAuxEffect CSfxManager::mCurrentAuxEffect = kAE_None;
CSfxManager::EAuxEffect CSfxManager::mRequestedAuxEffect = kAE_None;
CSfxManager::CSfxChannel CSfxManager::mChannels[4];
#endif

CSfxManager::ESfxChannels CSfxManager::mCurrentChannel = kSC_Default;
bool CSfxManager::mDoUpdate = false;
#if !RSTL_PRIME1
int CSfxManager::mNextAreaFilterId = 0;
int CSfxManager::mNextFilterId = 0;
int CSfxManager::mNextAuxEffectId = 0;
rstl::vector< short >* CSfxManager::mpTranslationTable = nullptr;
bool CSfxManager::mMuted = false;
int CSfxManager::mAreaLowPassFrequency = 16000;
int CSfxManager::mLowPassFrequency = 16000;
int CSfxManager::mCurrentArea = -1;
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
rstl::vector< short >* CSfxManager::mTranslationTable;
bool CSfxManager::mMuted = false;
rstl::auto_ptr< CToken > CSfxManager::mTranslationTableTok;

bool CSfxManager::mAuxProcessingEnabled = false;
#else
CSfxManager::CSfxChannel CSfxManager::mChannels[4];
rstl::reserved_vector< CSfxManager::SLowPassFilter, 8 > CSfxManager::mAreaLowPassFilters;
rstl::reserved_vector< CSfxManager::SLowPassFilter, 8 > CSfxManager::mLowPassFilters;
rstl::auto_ptr< CToken > CSfxManager::mpTranslationTableToken;
#endif

rstl::reserved_vector< CSfxManager::CSfxEmitterWrapper, 64 > CSfxManager::mEmitterWrapperPool;
rstl::reserved_vector< CSfxManager::CSfxWrapper, 64 > CSfxManager::mWrapperPool;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
SND_AUX_REVERBHI sReverbHi;
SND_AUX_CHORUS sChorus;
SND_AUX_REVERBSTD sReverbStd;
SND_AUX_DELAY sDelay;
SND_AUX_REVERBHI sPendingReverbHi;
SND_AUX_CHORUS sPendingChorus;
SND_AUX_REVERBSTD sPendingReverbStd;
SND_AUX_DELAY sPendingDelay;
#else
rstl::reserved_vector< CSfxPitchBend, 8 > CSfxManager::mPitchBends;
rstl::reserved_vector< CAuxEffect, 10 > CSfxManager::mAuxEffects;
rstl::reserved_vector< CSfxManager::SAreaVolume, 10 >
    CSfxManager::mAreaVolumes(10, SAreaVolume(-1, uchar(0)));
CAuxEffectManager CSfxManager::mAuxEffectManager;
rstl::pair< int, bool > CSfxManager::mStudioState(-1, false);
#endif

const short CSfxManager::kMaxPriority = 255;
const short CSfxManager::kMedPriority = 127;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
const ushort CSfxManager::kInternalInvalidSfxId = -1;
#else
const ushort CSfxManager::kInternalInvalidSfxId = 0xffff;
#endif

const int CSfxManager::kAllAreas = -1;

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CSfxManager::CBaseSfxWrapper::CBaseSfxWrapper(const bool looped, const short prio,
                                              const CSfxHandle handle, const bool useAcoustics,
                                              const int area)
#else
CSfxManager::CSfxChannel::CSfxChannel() : mListeners(4, SListener()) {}

CSfxManager::SListener::SListener() : mActive(false) {}

CSfxManager::ESfxWrapperType CSfxManager::CSfxEmitterWrapper::GetType() const {
  return kSWT_Emitter;
}

CSfxManager::CBaseSfxWrapper::CBaseSfxWrapper(bool looped, short priority, CSfxHandle handle,
                                              const bool useAcoustics, int area)
#endif

: mTimeRemaining(15.f)
, mRank(0)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mPrio(prio)
#else
, mPriority(priority)
, mPitchBend(0x2000)
#endif

, mHandle(handle)
, mArea(area)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mIsActive(true)
, mIsPlaying(false)
#else
, mActive(true)
, mPlaying(false)
#endif

, mLooped(looped)
, mInArea(true)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mIsReleased(false)
, mUseAcoustics(useAcoustics) {}
#else
, mReleased(false)
, mUseAcoustics(useAcoustics)
, mIgnoreAreaLowPass(false) {}
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
const bool CSfxManager::CBaseSfxWrapper::Available() const { return mIsReleased; }
#else
bool CSfxManager::CBaseSfxWrapper::Available() const { return mReleased; }
#endif

void CSfxManager::CBaseSfxWrapper::Release() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mIsReleased = true;
#else
  mReleased = true;
#endif

  mTimeRemaining = 15.f;
}

// TODO: verify that this Prime/Echoes difference is real
RSTL_PRIME1_CONST float CSfxManager::CBaseSfxWrapper::GetTimeRemaining() { return mTimeRemaining; }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CSfxManager::CBaseSfxWrapper::SetTimeRemaining(float t) { mTimeRemaining = t; }
#else
void CSfxManager::CBaseSfxWrapper::SetTimeRemaining(float time) { mTimeRemaining = time; }
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CSfxManager::CBaseSfxWrapper::SetActive(bool v) { mIsActive = v; }
#else
void CSfxManager::CBaseSfxWrapper::SetActive(bool active) { mActive = active; }
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CSfxManager::CBaseSfxWrapper::SetPlaying(bool v) { mIsPlaying = v; }
#else
void CSfxManager::CBaseSfxWrapper::SetPlaying(bool playing) { mPlaying = playing; }
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CSfxManager::CBaseSfxWrapper::SetInArea(const bool v) { mInArea = v; }
#else
void CSfxManager::CBaseSfxWrapper::SetInArea(bool inArea) { mInArea = inArea; }
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CSfxManager::CBaseSfxWrapper::SetRank(const short v) { mRank = v; }
#else
void CSfxManager::CBaseSfxWrapper::SetRank(short rank) { mRank = rank; }
#endif

// TODO: verify that this Prime/Echoes difference is real
RSTL_PRIME1_CONST bool CSfxManager::CBaseSfxWrapper::IsLooped() const { return mLooped; }

// TODO: verify that this Prime/Echoes difference is real
RSTL_PRIME1_CONST bool CSfxManager::CBaseSfxWrapper::IsInArea() const { return mInArea; }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
bool CSfxManager::CBaseSfxWrapper::IsPlaying() const { return mIsPlaying; }
#else
bool CSfxManager::CBaseSfxWrapper::IsPlaying() const { return mPlaying; }
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
const bool CSfxManager::CBaseSfxWrapper::IsActive() const { return mIsActive; }
#else
bool CSfxManager::CBaseSfxWrapper::IsActive() const { return mActive; }
#endif

// TODO: verify that this Prime/Echoes difference is real
RSTL_PRIME1_CONST bool CSfxManager::CBaseSfxWrapper::UseAcoustics() const { return mUseAcoustics; }

// TODO: verify that this Prime/Echoes difference is real
RSTL_PRIME1_CONST int CSfxManager::CBaseSfxWrapper::GetRank() const { return mRank; }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
const int CSfxManager::CBaseSfxWrapper::GetPriority() const { return mPrio; }
#else
int CSfxManager::CBaseSfxWrapper::GetPriority() const { return mPriority; }
#endif

// TODO: verify that this Prime/Echoes difference is real
RSTL_PRIME1_CONST CSfxHandle CSfxManager::CBaseSfxWrapper::GetSfxHandle() const { return mHandle; }

// TODO: verify that this Prime/Echoes difference is real
RSTL_PRIME1_CONST int CSfxManager::CBaseSfxWrapper::GetArea() const { return mArea; }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CSfxManager::CSfxEmitterWrapper::CSfxEmitterWrapper(const bool looped, const short prio,
                                                    CAudioSys::C3DEmitterParmData& emitterData,
                                                    const CSfxHandle handle,
                                                    const bool useAcoustics, const int area)
: CBaseSfxWrapper(looped, prio, handle, useAcoustics, area)
, mEmitterData(emitterData)
#else
void CSfxManager::CBaseSfxWrapper::SetPitchBend(ushort pitch) { mPitchBend = pitch; }

ushort CSfxManager::CBaseSfxWrapper::GetPitchBend() const { return mPitchBend; }

bool CSfxManager::CBaseSfxWrapper::GetIgnoreAreaLowPass() const { return mIgnoreAreaLowPass; }

void CSfxManager::CBaseSfxWrapper::SetIgnoreAreaLowPass(bool ignore) {
  mIgnoreAreaLowPass = ignore;
}

CSfxManager::CSfxEmitterWrapper::CSfxEmitterWrapper(bool looped, short priority,
                                                    CAudioSys::C3DEmitterParmData& emitter,
                                                    CSfxHandle handle, const bool useAcoustics,
                                                    int area)
: CBaseSfxWrapper(looped, priority, handle, useAcoustics, area)
, mEmitterData(emitter)
#endif

, mEmitterHandle(SND_ID_ERROR)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mReady(true) {}
#else
, mReady(true)
, mUpdatePending(false) {}
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CSfxManager::CSfxEmitterWrapper::SetReverb(const char rev) {
  if (CSfxManager::IsAuxProcessingEnabled() && UseAcoustics()) {
    mPara.paraData.value7 = rev;
#else
void CSfxManager::CSfxEmitterWrapper::SetReverb(char reverb) {
  if (UseAcoustics()) {
    mParameters[mParameterInfo.numPara - 1].paraData.value7 = reverb;
#endif

  }
}

void CSfxManager::CSfxEmitterWrapper::Play() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mParameterInfo.numPara = 1;
  mParameterInfo.paraArray = &mPara;
  mPara.ctrl = SND_MIDICTRL_REVERB;
#else
  mParameterInfo.numPara = 0;
  mParameterInfo.paraArray = mParameters;
  mEmitterData.mStudio = UseAcoustics() ? GetStudio(GetArea()) : uchar(0);
  mParameters[mParameterInfo.numPara].ctrl = SND_MIDICTRL_REVERB;
  if (UseAcoustics()) {
    mParameters[mParameterInfo.numPara].paraData.value7 = GetReverbAmount();
  } else {
    mParameters[mParameterInfo.numPara].paraData.value7 = 0;
  }
  ++mParameterInfo.numPara;
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (CSfxManager::IsAuxProcessingEnabled() && UseAcoustics()) {
    mPara.paraData.value7 = CSfxManager::GetReverbAmount();
  } else {
    mPara.paraData.value7 = 0;
  }

  mEmitterHandle = CAudioSys::S3dAddEmitterParaEx(
      mEmitterData, GetSfxHandle().GetIndex() & 0xFF, &mParameterInfo);
#else
  mEmitterHandle = CAudioSys::S3dAddEmitterParaEx(mEmitterData, GetSfxHandle().GetIndex() & 0xff,
                                                  &mParameterInfo);
#endif

  if (mEmitterHandle != SND_ID_ERROR) {
    SetPlaying(true);
  }
  mReady = false;
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
const SND_FXID CSfxManager::CSfxEmitterWrapper::GetSfxId() { return mEmitterData.mSfxId; }
#else
ushort CSfxManager::CSfxEmitterWrapper::GetSfxId() { return mEmitterData.mSfxId; }

bool CSfxManager::CSfxEmitterWrapper::IsSilent() const { return GetEmitter().mMaxVol == 1; }
#endif

void CSfxManager::CSfxEmitterWrapper::Stop() {
  if (mEmitterHandle != SND_ID_ERROR) {
    CAudioSys::S3dRemoveEmitter(mEmitterHandle);
    SetPlaying(false);
    mEmitterHandle = SND_ID_ERROR;
  }
}

CAudioSys::C3DEmitterParmData& CSfxManager::CSfxEmitterWrapper::GetEmitter() {
  return mEmitterData;
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
const SND_VOICEID CSfxManager::CSfxEmitterWrapper::GetHandle() const { return mEmitterHandle; }
#else
const CAudioSys::C3DEmitterParmData& CSfxManager::CSfxEmitterWrapper::GetEmitter() const {
  return mEmitterData;
}

uint CSfxManager::CSfxEmitterWrapper::GetHandle() const { return mEmitterHandle; }
#endif

bool CSfxManager::CSfxEmitterWrapper::IsPlaying() const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (!IsLooped()) {
    bool ret = false;
    if (CBaseSfxWrapper::IsPlaying() && CAudioSys::S3dCheckEmitter(mEmitterHandle)) {
      ret = true;
#else
  if (IsLooped()) {
    return CBaseSfxWrapper::IsPlaying();
  }
  return CBaseSfxWrapper::IsPlaying() && CAudioSys::S3dCheckEmitter(mEmitterHandle);
#endif

    }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return ret;
#else
bool CSfxManager::CSfxEmitterWrapper::Ready() { return IsLooped() || mReady; }

short CSfxManager::CSfxEmitterWrapper::GetAudible(const CVector3f& position) {
  const float distanceSquared = (mEmitterData.mPos - position).MagSquared();
  const float maxDistanceSquared = mEmitterData.mMaxDist * mEmitterData.mMaxDist;
  if (distanceSquared < maxDistanceSquared * 0.25f) {
    return kSA_High;
  }
  if (distanceSquared < maxDistanceSquared * 0.5f) {
    return kSA_Medium;
  }
  return static_cast< ESfxAudibility >(distanceSquared < maxDistanceSquared);
#endif

  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return CBaseSfxWrapper::IsPlaying();
}

const bool CSfxManager::CSfxEmitterWrapper::Ready() { return IsLooped() || mReady; }

short CSfxManager::CSfxEmitterWrapper::GetAudible(const CVector3f& vec) {
  float magSq = (mEmitterData.mPos - vec).MagSquared();
  float maxDist = mEmitterData.mMaxDist * mEmitterData.mMaxDist;
  if (magSq < maxDist * 0.25f)
    return kSA_Aud3;
  else if (magSq < maxDist * 0.5f)
    return kSA_Aud2;
  else if (magSq < maxDist)
    return kSA_Aud1;
  return kSA_Aud0;
}

const SND_VOICEID CSfxManager::CSfxEmitterWrapper::GetVoice() const {
#else
SND_VOICEID CSfxManager::CSfxEmitterWrapper::GetVoice() const {
#endif

  return IsPlaying() ? CAudioSys::S3dEmitterVoiceID(mEmitterHandle) : SND_ID_ERROR;
}

void CSfxManager::CSfxEmitterWrapper::UpdateEmitterSilent() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mCachedMaxVol = mEmitterData.mMaxVol;
#else
  if (mEmitterData.mMaxVol != 1) {
    mCachedMaxVolume = mEmitterData.mMaxVol;
    mEmitterData.mMaxVol = 1;
#endif

  CAudioSys::S3dUpdateEmitter(mEmitterHandle, mEmitterData.mPos, mEmitterData.mDir, 1);
}
#if RSTL_PRIME1
void CSfxManager::CSfxEmitterWrapper::UpdateEmitter() {
  CAudioSys::S3dUpdateEmitter(mEmitterHandle, mEmitterData.mPos, mEmitterData.mDir,
                              mCachedMaxVol);
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CSfxManager::CSfxWrapper::CSfxWrapper(const bool looped, const short prio, const ushort sfxId,
                                      const short vol, const short pan, const CSfxHandle handle,
                                      const bool useAcoustics, const int area)
: CBaseSfxWrapper(looped, prio, handle, useAcoustics, area)
#else
void CSfxManager::CSfxEmitterWrapper::UpdateEmitter() { mUpdatePending = true; }

CSfxManager::CSfxWrapper::CSfxWrapper(bool looped, short priority, ushort sfxId, const short volume,
                                      short pan, CSfxHandle handle, const bool useAcoustics,
                                      int area)
: CBaseSfxWrapper(looped, priority, handle, useAcoustics, area)
#endif

, mSfxId(sfxId)
, mVoiceHandle(SND_ID_ERROR)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mVol(vol)
#else
, mVolume(volume)
#endif

, mPan(pan)
, mReady(true) {}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CSfxManager::CSfxWrapper::SetReverb(const char rev) {
  if (CSfxManager::IsAuxProcessingEnabled() && UseAcoustics()) {
    CAudioSys::SfxCtrl(mVoiceHandle, SND_MIDICTRL_REVERB, rev);
#else
void CSfxManager::CSfxWrapper::SetReverb(char reverb) {
  if (UseAcoustics()) {
    CAudioSys::SfxCtrl(mVoiceHandle, SND_MIDICTRL_REVERB, reverb);
#endif

  }
}

void CSfxManager::CSfxWrapper::Play() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mVoiceHandle = CAudioSys::SfxStart(mSfxId, mVol, mPan, 0);
#else
  const uchar studio = UseAcoustics() ? GetStudio(GetArea()) : uchar(0);
  mVoiceHandle = CAudioSys::SfxStart(mSfxId, 127, mPan, studio);
  CAudioSys::SfxVolume(mVoiceHandle, mVolume);
#endif

  if (mVoiceHandle != SND_ID_ERROR) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    if (CSfxManager::IsAuxProcessingEnabled() && UseAcoustics()) {
      const ushort reverb = CSfxManager::GetReverbAmount();
#else
    if (UseAcoustics()) {
      const ushort reverb = GetReverbAmount();
#endif

      CAudioSys::SfxCtrl(mVoiceHandle, SND_MIDICTRL_REVERB, reverb);
    }

    SetPlaying(true);
#if !RSTL_PRIME1
    if (IsLowPassAreaFilterEnabled() && GetArea() != kAllAreas) {
      CAudioSys::SfxSetFilter(mVoiceHandle, 1, GetLowPassAreaFrequency());
#endif

  }
#if !RSTL_PRIME1
  }
#endif

  mReady = false;
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
const SND_FXID CSfxManager::CSfxWrapper::GetSfxId() { return mSfxId; }
#else
ushort CSfxManager::CSfxWrapper::GetSfxId() { return mSfxId; }
#endif

void CSfxManager::CSfxWrapper::Stop() {
  if (mVoiceHandle != SND_ID_ERROR) {
    CAudioSys::SfxStop(mVoiceHandle);
    SetPlaying(false);
    mVoiceHandle = SND_ID_ERROR;
  }
}

bool CSfxManager::CSfxWrapper::IsPlaying() const {
  return CBaseSfxWrapper::IsPlaying() && CAudioSys::SfxCheck(mVoiceHandle) != SND_ID_ERROR;
}

// TODO: verify that this Prime/Echoes difference is real
RSTL_PRIME1_CONST bool CSfxManager::CSfxWrapper::Ready() { return IsLooped() || mReady; }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
short CSfxManager::CSfxWrapper::GetAudible(const CVector3f&) { return kSA_Aud3; }
#else
short CSfxManager::CSfxWrapper::GetAudible(const CVector3f&) { return kSA_High; }
#endif

// TODO: verify that this Prime/Echoes difference is real
RSTL_PRIME1_CONST SND_VOICEID CSfxManager::CSfxWrapper::GetVoice() const { return mVoiceHandle; }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CSfxManager::CSfxWrapper::SetVolume(const short vol) { mVol = vol; }
#else
void CSfxManager::CSfxWrapper::SetVolume(short volume) { mVolume = volume; }
#endif

void CSfxManager::CSfxWrapper::UpdateEmitterSilent() { CAudioSys::SfxVolume(mVoiceHandle, 1); }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CSfxManager::CSfxWrapper::UpdateEmitter() { CAudioSys::SfxVolume(mVoiceHandle, mVol); }
#else
void CSfxManager::CSfxWrapper::UpdateEmitter() { CAudioSys::SfxVolume(mVoiceHandle, mVolume); }

void CSfxManager::Initialize() {
  mChannels[kSC_Game].mSounds.push_back(nullptr);
  mAuxEffectManager.Initialize();
}
#endif

void CSfxManager::Shutdown() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  delete mTranslationTable;
  mTranslationTable = nullptr;
#else
  delete mpTranslationTable;
  mpTranslationTable = nullptr;
#endif

  StopAndRemoveAllEmitters();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (mCurrentAuxEffect != kAE_None) {
    DisableAuxCallbacks();
#else
  mAuxEffectManager.Shutdown();
  for (rstl::reserved_vector< CAuxEffect, 10 >::iterator it = mAuxEffects.begin();
       it != mAuxEffects.end(); ++it) {
    if (it->IsRegistered() && it->IsActive()) {
      it->Deactivate();
    }
#endif

  }
}

void CSfxManager::StopAndRemoveAllEmitters() {
  for (int i = 0; i < 4; ++i) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CSfxChannel* channel = &mChannels[i];
    for (int j = 0; j < channel->x48_.size(); ++j) {
      if (channel->x48_[j] != nullptr) {
        if (channel->x48_[j]->IsPlaying()) {
          channel->x48_[j]->Stop();
#else
    CSfxChannel& channel = mChannels[i];
    for (int j = 0; j < channel.mSounds.size(); ++j) {
      if (channel.mSounds[j] != nullptr) {
        if (channel.mSounds[j]->IsPlaying()) {
          channel.mSounds[j]->Stop();
#endif

        }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        channel->x48_[j]->Release();
        channel->x48_[j] = nullptr;
#else
        channel.mSounds[j]->Release();
        channel.mSounds[j] = nullptr;
#endif

      }
    }
  }
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CSfxManager::AddListener(ESfxChannels channel, const CVector3f& pos, const CVector3f& dir,
                              const CVector3f& vec1, const CVector3f& vec2, const float f1,
                              const float f2, const float f3, const uint w1,
                              const uchar maxVolume) {
  CSfxChannel* chan = &mChannels[channel];
  chan->mListener = CSfxListener(pos, dir, vec1, vec2, f1, f2, f3, w1, maxVolume);
  chan->x44_ = true;
  CAudioSys::S3dAddListener(pos, dir, vec1, vec2, f1, f2, f3, w1, maxVolume);
#else
CSfxManager::CSfxListener::CSfxListener(CVector3f position, CVector3f direction, CVector3f heading,
                                        CVector3f up, float frontSur, float backSur,
                                        float soundSpeed, uint flags, const uchar maxVolume)
: mPosition(position)
, mDirection(direction)
, mHeading(heading)
, mUp(up)
, mFrontSur(frontSur)
, mBackSur(backSur)
, mSoundSpeed(soundSpeed)
, mFlags(flags)
, mMaxVolume(maxVolume) {}

void CSfxManager::AddListener(ESfxChannels channel, const CVector3f& position,
                              const CVector3f& direction, const CVector3f& heading,
                              const CVector3f& up, float frontSur, float backSur, float soundSpeed,
                              uint flags, uchar maxVolume, int listener) {
  SListener& entry = mChannels[channel].mListeners[listener];
  entry.mListener = CSfxListener(position, direction, heading, up, frontSur, backSur, soundSpeed,
                                 flags, maxVolume);
  entry.mActive = true;
  CAudioSys::S3dAddListener(position, direction, heading, up, frontSur, backSur, soundSpeed, flags,
                            maxVolume);
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CSfxManager::UpdateListener(const CVector3f& pos, const CVector3f& dir, const CVector3f& vec1,
                                 const CVector3f& vec2, const uchar maxVolume) {
  CSfxChannel* chan = &mChannels[mCurrentChannel];
  chan->mListener.x0_ = pos;
  chan->mListener.xc_ = dir;
  chan->mListener.x18_ = vec1;
  chan->mListener.x24_ = vec2;
  chan->mListener.x40_ = maxVolume;
  chan->x44_ = true;
  CAudioSys::S3dUpdateListener(pos, dir, vec1, vec2, maxVolume);
#else
void CSfxManager::UpdateListener(const CVector3f& position, const CVector3f& direction,
                                 const CVector3f& heading, const CVector3f& up,
                                 const uchar maxVolume, int listener) {
  SListener& entry = mChannels[mCurrentChannel].mListeners[listener];
  entry.mListener.mPosition = position;
  entry.mListener.mDirection = direction;
  entry.mListener.mHeading = heading;
  entry.mListener.mUp = up;
  entry.mListener.mMaxVolume = maxVolume;
  entry.mActive = true;
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CSfxHandle CSfxManager::AddEmitter(const SND_FXID id, const CVector3f& pos, const CVector3f& dir,
                                   bool useAcoustics, const bool looped, const short prio,
                                   const int areaId) {
  CAudioSys::C3DEmitterParmData emitterParm(150, 0.1f, 1, 127, 20);
  emitterParm.mPos = pos;
  emitterParm.mDir = dir;
  emitterParm.mSfxId = id;
  return AddEmitter(emitterParm, useAcoustics, prio, looped, areaId);
#else
CSfxHandle CSfxManager::AddEmitter(ushort id, const CVector3f& position, int area,
                                   bool useAcoustics, bool looped, short priority) {
  CAudioSys::C3DEmitterParmData params(150.f, 0.1f, 1, 127, 20);
  params.mPos = position;
  params.mDir = CVector3f::Zero();
  params.mSfxId = id;
  return AddEmitter(params, area, useAcoustics, looped, priority);
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CSfxHandle CSfxManager::AddEmitter(const SND_FXID id, const CVector3f& pos, const CVector3f& dir,
                                   const uchar vol, bool useAcoustics, const bool looped,
                                   const short prio, const int areaId) {
  CAudioSys::C3DEmitterParmData emitterParm(150.f, 0.1f, 1, vol > 20 ? vol : 21, 20);
  emitterParm.mPos = pos;
  emitterParm.mDir = dir;
  emitterParm.mSfxId = id;

  return AddEmitter(emitterParm, useAcoustics, prio, looped, areaId);
#else
CSfxHandle CSfxManager::AddEmitter(ushort id, const CVector3f& position, uchar volume, int area,
                                   bool useAcoustics, bool looped, short priority) {
  CAudioSys::C3DEmitterParmData params(150.f, 0.1f, 1, volume > 20 ? volume : 21, 20);
  params.mPos = position;
  params.mDir = CVector3f::Zero();
  params.mSfxId = id;
  return AddEmitter(params, area, useAcoustics, looped, priority);
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CSfxHandle CSfxManager::AddEmitter(CAudioSys::C3DEmitterParmData& parmData, bool useAcoustics,
                                   const short prio, const bool looped, const int areaId) {
  if (!DEBUG_TOOLS_GET_BOOL(kDO_EnableAudio, true))
#else
CSfxHandle CSfxManager::AddEmitter(CAudioSys::C3DEmitterParmData& params, int area,
                                   bool useAcoustics, bool looped, short priority) {
  if ((mMuted && !looped) || params.mSfxId == 0xffffffff ||
      params.mSfxId == kInternalInvalidSfxId) {
#endif

    return CSfxHandle::NullHandle();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if NONMATCHING
  if (mMuted || parmData.mSfxId == 0xFFFF) {
#else
  if (mMuted || parmData.mSfxId == 0xFFFFFFFF || parmData.mSfxId == 0xFFFF) {
#endif
#else
  }
  CAudioSys::C3DEmitterParmData emitter(params);
  if (looped) {
    emitter.mFlags |= 6;
  }
  emitter.mSfxId = TranslateSFXID(params.mSfxId);
  const uchar areaVolume = GetAreaVolume(area);
  const uchar maxVol = emitter.mMaxVol;
  emitter.mMaxVol =
      areaVolume == 127 ? maxVol : uchar(areaVolume * (maxVol > 127 ? 127 : maxVol) / 127);
  if (emitter.mSfxId == kInternalInvalidSfxId) {
#endif

    return CSfxHandle::NullHandle();
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CAudioSys::C3DEmitterParmData cpy = parmData;
  if (looped) {
    cpy.mFlags |= 6;
#else
  mDoUpdate = true;
  CSfxChannel& channel = mChannels[mCurrentChannel];
  const CSfxHandle handle = LocateHandle(priority);
  if (handle) {
    CSfxEmitterWrapper* sound = AllocateCSfxEmitterWrapper(
        CSfxEmitterWrapper(looped, priority, emitter, handle, useAcoustics, area));
    if (mMuted) {
      sound->UpdateEmitterSilent();
    }
    channel.mSounds[handle.GetIndex()] = sound;
  }
  return handle;
}

// Guessed helper: applies the area volume scale to a sound volume.
static inline uchar ScaleVolumeForArea(uchar volume, int area) {
  const uchar areaVolume = CSfxManager::GetAreaVolume(area);
  if (areaVolume == 127) {
    return volume;
  }
  return areaVolume * (volume > 127 ? 127 : volume) / 127;
}

void CSfxManager::UpdateEmitter(CSfxHandle handle, const CVector3f& position,
                                const CVector3f& direction, uchar maxVolume) {
  CSfxChannel& channel = mChannels[mCurrentChannel];
  const int index = handle.GetIndex();
  if (index < 0 || index >= channel.mSounds.size()) {
    return;
  }
  CBaseSfxWrapper* base = channel.mSounds[index];
  if (base == nullptr || handle != base->GetSfxHandle() || !base->IsPlaying()) {
    return;
  }
  CSfxEmitterWrapper* sound = static_cast< CSfxEmitterWrapper* >(base);
  mDoUpdate = true;
  if (sound->IsSilent()) {
    sound->GetEmitter().mPos = position;
    sound->GetEmitter().mDir = direction;
  } else {
    sound->GetEmitter().mPos = position;
    sound->GetEmitter().mDir = direction;
    sound->GetEmitter().mMaxVol =
        rstl::max_val< uchar >(2, ScaleVolumeForArea(maxVolume, sound->GetArea()));
  }
}

void CSfxManager::RemoveEmitter(CSfxHandle handle) { StopSound(mCurrentChannel, handle); }

CSfxHandle CSfxManager::SfxStart(ushort id, short volume, short pan, int area, bool useAcoustics,
                                 bool looped, short priority) {
  if ((mMuted && !looped) || id == 0xffffffff || id == kInternalInvalidSfxId) {
    return CSfxHandle::NullHandle();
#endif

  }
  mDoUpdate = true;
  CSfxChannel& channel = mChannels[mCurrentChannel];
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CSfxHandle ret = LocateHandle(prio);
  if (ret) {
    CSfxEmitterWrapper wrapper(looped, prio, cpy, ret, useAcoustics, areaId);
    channel.x48_[ret.GetIndex()] = AllocateCSfxEmitterWrapper(wrapper);
#else
  const CSfxHandle handle = LocateHandle(priority);
  if (handle) {
    id = TranslateSFXID(id);
    if (id == kInternalInvalidSfxId) {
      return CSfxHandle::NullHandle();
    }
    channel.mSounds[handle.GetIndex()] = AllocateCSfxWrapper(CSfxWrapper(
        looped, priority, id, ScaleVolumeForArea(volume, area), pan, handle, useAcoustics, area));
  }
  return handle;
#endif

  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return ret;
#else
void CSfxManager::SfxStop(CSfxHandle handle) { StopSound(mCurrentChannel, handle); }

void CSfxManager::SfxStop(ESfxChannels channel, CSfxHandle handle) { StopSound(channel, handle); }

void CSfxManager::SfxVolume(CSfxHandle handle, uchar volume) {
  CSfxChannel& channel = mChannels[mCurrentChannel];
  const int index = handle.GetIndex();
  if (index < 0 || index >= channel.mSounds.size()) {
    return;
  }
  CBaseSfxWrapper* base = channel.mSounds[index];
  if (base == nullptr || handle != base->GetSfxHandle()) {
    return;
  }
  CSfxWrapper* sound = static_cast< CSfxWrapper* >(base);
  const uchar scaled = ScaleVolumeForArea(volume, sound->GetArea());
  uchar clamped = scaled < 1 ? uchar(1) : (scaled > 127 ? uchar(127) : scaled);
  sound->SetVolume(clamped);
  if (!mMuted && sound->IsPlaying()) {
    CAudioSys::SfxVolume(sound->GetVoice(), clamped);
  }
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CSfxManager::UpdateEmitter(CSfxHandle handle, const CVector3f& pos, const CVector3f& dir,
                                const uchar maxVol) {
  CSfxChannel& chan = mChannels[mCurrentChannel];
  if (handle.GetIndex() < 0 || handle.GetIndex() >= chan.x48_.size()) {
#else
void CSfxManager::SfxPan(CSfxHandle handle, uchar pan) {
  if (!handle) {
#endif

    return;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CSfxEmitterWrapper* wrapper = static_cast< CSfxEmitterWrapper* >(chan.x48_[handle.GetIndex()]);
  if (!wrapper || handle != wrapper->GetSfxHandle() || !wrapper->IsPlaying()) {
#else
  CSfxChannel& channel = mChannels[mCurrentChannel];
  const int index = handle.GetIndex();
  if (index < 0 || index >= channel.mSounds.size()) {
#endif

    return;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mDoUpdate = true;
  wrapper->GetEmitter().mPos = pos;
  wrapper->GetEmitter().mDir = dir;
  wrapper->GetEmitter().mMaxVol = maxVol;
  CAudioSys::S3dUpdateEmitter(wrapper->GetHandle(), pos, dir, maxVol);
}

void CSfxManager::RemoveEmitter(CSfxHandle handle) { StopSound(handle); }

CSfxHandle CSfxManager::SfxStart(const ushort id, const short vol, const short pan,
                                 bool useAcoustics, const short prio, const bool looped,
                                 const int areaId) {
  if (!DEBUG_TOOLS_GET_BOOL(kDO_EnableAudio, true))
    return CSfxHandle::NullHandle();
#if NONMATCHING
  if (mMuted || id == 0xFFFF) {
#else
  if (mMuted || id == 0xFFFFFFFF || id == 0xFFFF) {
#endif
    return CSfxHandle::NullHandle();
  }
  mDoUpdate = true;
  CSfxChannel& chan = mChannels[mCurrentChannel];
  CSfxHandle ret = LocateHandle(prio);
  if (ret) {
    CSfxWrapper wrapper(looped, prio, id, vol, pan, ret, useAcoustics, areaId);
    chan.x48_[ret.GetIndex()] = AllocateCSfxWrapper(wrapper);
  }
  return ret;
}

void CSfxManager::SfxStop(CSfxHandle handle) { StopSound(handle); }

void CSfxManager::SfxVolume(CSfxHandle handle, uchar volume) {
  CSfxChannel& chan = mChannels[mCurrentChannel];
  if (handle.GetIndex() < 0 || handle.GetIndex() >= chan.x48_.size()) {
#else
  CBaseSfxWrapper* sound = channel.mSounds[index];
  if (sound == nullptr || handle != sound->GetSfxHandle()) {
#endif

    return;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CSfxWrapper* wrapper = static_cast< CSfxWrapper* >(chan.x48_[handle.GetIndex()]);
  if (wrapper == nullptr || handle != wrapper->GetSfxHandle()) {
    return;
#else
  if (!sound->IsPlaying()) {
    Update(0.f);
#endif

  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  wrapper->SetVolume(volume);
  if (wrapper->IsPlaying()) {
    CAudioSys::SfxVolume(wrapper->GetVoice(), volume);
#else
  if (sound->IsPlaying()) {
    CAudioSys::SfxPan(sound->GetVoice(), pan);
#endif

  }
}

void CSfxManager::SfxSpan(CSfxHandle handle, uchar span) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CSfxChannel& chan = mChannels[mCurrentChannel];
  if (handle.GetIndex() < 0 || handle.GetIndex() >= chan.x48_.size()) {
#else
  if (!handle) {
#endif

    return;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CSfxWrapper* wrapper = static_cast< CSfxWrapper* >(chan.x48_[handle.GetIndex()]);
  if (wrapper == nullptr || handle != wrapper->GetSfxHandle()) {
#else
  CSfxChannel& channel = mChannels[mCurrentChannel];
  const int index = handle.GetIndex();
  if (index < 0 || index >= channel.mSounds.size()) {
#endif

    return;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (wrapper->IsPlaying()) {
    CAudioSys::SfxSpan(wrapper->GetVoice(), span);
#else
  CBaseSfxWrapper* sound = channel.mSounds[index];
  if (sound == nullptr || handle != sound->GetSfxHandle()) {
    return;
  }
  if (!sound->IsPlaying()) {
    Update(0.f);
  }
  if (sound->IsPlaying()) {
    CAudioSys::SfxSpan(sound->GetVoice(), span);
#endif

  }
}

void CSfxManager::KillAll(ESfxChannels channel) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CSfxChannel& chan = mChannels[channel];

  for (int i = 0; i < chan.x48_.size(); ++i) {
    CBaseSfxWrapper* wrapper = chan.x48_[i];
    if (wrapper != NULL && wrapper->IsPlaying()) {
      wrapper->Stop();
#else
  CSfxChannel& sounds = mChannels[channel];
  for (int i = 0; i < sounds.mSounds.size(); ++i) {
    CBaseSfxWrapper* sound = sounds.mSounds[i];
    if (sound != nullptr && sound->IsPlaying()) {
      sound->Stop();
#endif

    }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    if (wrapper) {
      wrapper->Release();
#else
    if (sound != nullptr) {
      sound->Release();
#endif

    }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    chan.x48_[i] = nullptr;
#else
    sounds.mSounds[i] = nullptr;
  }
  if (channel == kSC_Game) {
    mAuxEffectManager.Shutdown();
    mAuxEffectManager.Initialize();
#endif

  }
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CSfxManager::StopSound(CSfxHandle handle) {
  CSfxChannel& chan = mChannels[mCurrentChannel];
  if (handle.GetIndex() < 0 || handle.GetIndex() >= chan.x48_.size()) {
#else
void CSfxManager::StopSound(ESfxChannels channel, CSfxHandle handle) {
  CSfxChannel& sounds = mChannels[channel];
  const int index = handle.GetIndex();
  if (index < 0 || index >= sounds.mSounds.size()) {
    if (channel != kSC_Game) {
      StopSound(kSC_Game, handle);
    }
#endif

    return;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CBaseSfxWrapper* wrapper = chan.x48_[handle.GetIndex()];
  if (wrapper == nullptr || handle != wrapper->GetSfxHandle()) {
#else
  CBaseSfxWrapper* sound = sounds.mSounds[index];
  if (sound == nullptr || handle != sound->GetSfxHandle()) {
    if (channel != kSC_Game) {
      StopSound(kSC_Game, handle);
    }
#endif

    return;
  }
  mDoUpdate = true;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (wrapper->IsPlaying()) {
    wrapper->Stop();
#else
  if (sound->IsPlaying()) {
    sound->Stop();
#endif

  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  wrapper->Release();
  chan.x48_[handle.GetIndex()] = nullptr;
#else
  sound->Release();
  sounds.mSounds[handle.GetIndex()] = nullptr;
#endif

}

void CSfxManager::SetDuration(CSfxHandle handle, float duration) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CSfxChannel& chan = mChannels[mCurrentChannel];
  if (handle.GetIndex() < 0 || handle.GetIndex() >= chan.x48_.size()) {
#else
  CSfxChannel& channel = mChannels[mCurrentChannel];
  const int index = handle.GetIndex();
  if (index < 0 || index >= channel.mSounds.size()) {
#endif

    return;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CBaseSfxWrapper* wrapper = chan.x48_[handle.GetIndex()];
  if (wrapper == nullptr || handle != wrapper->GetSfxHandle()) {
#else
  CBaseSfxWrapper* sound = channel.mSounds[index];
  if (sound == nullptr || handle != sound->GetSfxHandle()) {
#endif

    return;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  wrapper->SetTimeRemaining(duration);
#else
  sound->SetTimeRemaining(duration);
#endif

}

void CSfxManager::SetChannel(ESfxChannels channel) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (channel == mCurrentChannel) {
#else
  ESfxChannels current = mCurrentChannel;
  if (channel == current) {
#endif

    return;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (mCurrentChannel != kSC_Invalid) {
    TurnOffChannel(mCurrentChannel);
#else
  if (channel == kSC_Default) {
    mAreaLowPassFilters.clear();
    mLowPassFilters.clear();
  }
  if (current != kSC_Invalid) {
    TurnOffChannel(current);
#endif

  }
  TurnOnChannel(channel);
  mCurrentChannel = channel;
}

#if !RSTL_PRIME1
CSfxManager::ESfxChannels CSfxManager::GetChannel() { return mCurrentChannel; }
#endif

void CSfxManager::TurnOffChannel(ESfxChannels channel) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CSfxChannel& chan = mChannels[channel];
  for (int i = 0; i < chan.x48_.size(); ++i) {
    if (!chan.x48_[i]) {
      continue;
    }

    if (chan.x48_[i]->IsLooped()) {
      chan.x48_[i]->UpdateEmitterSilent();
#else
  CSfxChannel& sounds = mChannels[channel];
  for (int i = 0; i < sounds.mSounds.size(); ++i) {
    if (sounds.mSounds[i] != nullptr) {
      if (sounds.mSounds[i]->IsLooped()) {
        sounds.mSounds[i]->UpdateEmitterSilent();
#endif

    } else {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      chan.x48_[i]->Stop();
#else
        sounds.mSounds[i]->Stop();
#endif

    }
  }
#if RSTL_PRIME1
  for (int i = 0; i < chan.x48_.size(); ++i) {
    if (chan.x48_[i] == nullptr || chan.x48_[i]->IsLooped()) {
      continue;
#endif

    }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    chan.x48_[i]->Release();
    chan.x48_[i] = nullptr;
#else
  for (int i = 0; i < sounds.mSounds.size(); ++i) {
    if (sounds.mSounds[i] != nullptr && !sounds.mSounds[i]->IsLooped()) {
      sounds.mSounds[i]->Release();
      sounds.mSounds[i] = nullptr;
    }
#endif

  }
}

void CSfxManager::TurnOnChannel(ESfxChannels channel) {
  mCurrentChannel = channel;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CSfxChannel& chan = mChannels[channel];
#else
  CSfxChannel& sounds = mChannels[channel];
#endif

  mDoUpdate = true;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (!chan.x44_) {
    return;
#else
  bool hasListener = false;
  for (int i = 0; i < 4; ++i) {
    if (sounds.mListeners[i].mActive) {
      hasListener = true;
      break;
#endif

  }
#if RSTL_PRIME1
  for (int i = 0; i < chan.x48_.size(); ++i) {
    if (!chan.x48_[i]) {
      continue;
#endif

    }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    chan.x48_[i]->UpdateEmitter();
#else
  if (hasListener) {
    for (int j = 0; j < sounds.mSounds.size(); ++j) {
      if (sounds.mSounds[j] != nullptr) {
        sounds.mSounds[j]->UpdateEmitter();
      }
    }
#endif

  }
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CSfxHandle CSfxManager::LocateHandle(const short id) {
  CSfxChannel& chan = mChannels[mCurrentChannel];
  int i = 0;
  for (; i < chan.x48_.size(); ++i) {
    if (chan.x48_[i] != nullptr) {
      continue;
    }
#else
CSfxHandle CSfxManager::LocateHandle(int) {
  CSfxChannel& channel = mChannels[mCurrentChannel];
  for (int i = 0; i < channel.mSounds.size(); ++i) {
    if (channel.mSounds[i] == nullptr) {
#endif

    return CSfxHandle(i);
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (chan.x48_.size() == chan.x48_.capacity()) {
#else
  }
  if (channel.mSounds.size() == channel.mSounds.capacity()) {
#endif

    return CSfxHandle::NullHandle();
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  chan.x48_.push_back(nullptr);
  return CSfxHandle(chan.x48_.size() - 1);
#else
  channel.mSounds.push_back(nullptr);
  return CSfxHandle(channel.mSounds.size() - 1);
#endif

}

void CSfxManager::Update(float dt) {
  short i;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  ushort uVar21 = 0;
  CSfxChannel& chan = mChannels[mCurrentChannel];
  for (i = 0; i < chan.x48_.size(); ++i) {
    if (!chan.x48_[i] || chan.x48_[i]->IsLooped()) {
#else
  ushort count = 0;
  CSfxChannel& channel = mChannels[mCurrentChannel];
  for (i = 0; i < channel.mSounds.size(); ++i) {
    if (channel.mSounds[i] == nullptr || channel.mSounds[i]->IsLooped()) {
#endif

      continue;
    }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    float remTime = chan.x48_[i]->GetTimeRemaining();
    chan.x48_[i]->SetTimeRemaining(remTime - dt);
    if (remTime < 0.f) {
      chan.x48_[i]->Stop();
#else
    const float remaining = channel.mSounds[i]->GetTimeRemaining();
    channel.mSounds[i]->SetTimeRemaining(remaining - dt);
    if (remaining < 0.f) {
      channel.mSounds[i]->Stop();
#endif

      mDoUpdate = true;
    }
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  ushort local_e0[70];
#else
  ushort ranked[72];
#endif

  if (mDoUpdate) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    for (i = 0; i < chan.x48_.size(); ++i) {
      if (!chan.x48_[i]) {
        continue;
#else
    for (i = 0; i < channel.mSounds.size(); ++i) {
      if (channel.mSounds[i] != nullptr) {
        ranked[count++] = i;
        channel.mSounds[i]->SetRank(GetRank(channel.mSounds[i]));
#endif

      }
#if RSTL_PRIME1
      local_e0[uVar21++] = i;
      chan.x48_[i]->SetRank(GetRank(chan.x48_[i]));
#endif

    }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    for (i = 0; i < uVar21; i++) {
#else
    for (i = 0; i < count; ++i) {
#endif

      bool done = true;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      for (int j = 0; j < (uVar21 - 1); j += 1) {
        if (chan.x48_[local_e0[j]]->GetRank() < chan.x48_[local_e0[j + 1]]->GetRank()) {
#else
      for (int j = 0; j < count - 1; ++j) {
        if (channel.mSounds[ranked[j]]->GetRank() < channel.mSounds[ranked[j + 1]]->GetRank()) {
#endif

          done = false;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
          rstl::swap(local_e0[j], local_e0[j + 1]);
#else
          rstl::swap(ranked[j], ranked[j + 1]);
#endif

        }
      }
      if (done) {
        break;
      }
    }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    for (i = 48; i < uVar21; ++i) {
      CBaseSfxWrapper* wrapper = chan.x48_[local_e0[i]];
      if (!wrapper || !wrapper->IsPlaying()) {
        continue;
#else
    for (i = 48; i < count; ++i) {
      if (channel.mSounds[ranked[i]] != nullptr && channel.mSounds[ranked[i]]->IsPlaying()) {
        channel.mSounds[ranked[i]]->Stop();
#endif

      }
#if RSTL_PRIME1
      chan.x48_[local_e0[i]]->Stop();
#endif

    }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    for (i = 0; i < uVar21; ++i) {
      if (chan.x48_[local_e0[i]] == nullptr) {
        continue;
      }
      if (chan.x48_[local_e0[i]]->IsPlaying() && !chan.x48_[local_e0[i]]->IsInArea()) {
        chan.x48_[local_e0[i]]->Stop();
#else
    for (i = 0; i < count; ++i) {
      if (channel.mSounds[ranked[i]] != nullptr && channel.mSounds[ranked[i]]->IsPlaying() &&
          !channel.mSounds[ranked[i]]->IsInArea()) {
        channel.mSounds[ranked[i]]->Stop();
#endif

      }
    }
  }

  CAudioSys::S3dFlushUnusedEmitters();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (mDoUpdate) {
    for (int iVar22 = 48, j = 0; j < uVar21 && iVar22 != 0; ++j) {
      if (!chan.x48_[local_e0[j]]) {
#else
  if (mDoUpdate && !mMuted) {
    for (int available = 48, j = 0; j < count && available != 0; ++j) {
      if (channel.mSounds[ranked[j]] == nullptr) {
#endif

        continue;
      }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      if (chan.x48_[local_e0[j]]->IsPlaying()) {
        iVar22--;
      } else if (chan.x48_[local_e0[j]]->Ready() && chan.x48_[local_e0[j]]->IsInArea()) {
        chan.x48_[local_e0[j]]->Play();
        iVar22--;
#else
      if (channel.mSounds[ranked[j]]->IsPlaying()) {
        --available;
      } else if (channel.mSounds[ranked[j]]->Ready() && channel.mSounds[ranked[j]]->IsInArea()) {
        channel.mSounds[ranked[j]]->Play();
        --available;
#endif

      }
    }
    mDoUpdate = false;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  for (int i = 0; i < chan.x48_.size(); ++i) {
    if (!chan.x48_[i] || chan.x48_[i]->IsPlaying() || chan.x48_[i]->IsLooped()) {
#else
  for (int j = 0; j < channel.mSounds.size(); ++j) {
    if (channel.mSounds[j] != nullptr && !channel.mSounds[j]->IsPlaying() &&
        !channel.mSounds[j]->IsLooped()) {
      channel.mSounds[j]->Release();
      channel.mSounds[j] = nullptr;
      mDoUpdate = true;
    }
  }
  if (mpTranslationTableToken.get() && mpTranslationTableToken->IsLocked() &&
      mpTranslationTableToken->IsLoaded()) {
    if (mpTranslationTable == nullptr) {
      TToken< rstl::vector< short > > token(*mpTranslationTableToken);
      mpTranslationTable = rs_new rstl::vector< short >(*token.GetT());
    }
    mpTranslationTableToken = nullptr;
  }
  if (!close_enough(dt, 0.f)) {
    UpdatePitchBends(dt);
  }
  int primary = -1;
  for (int j = 0; j < 4; ++j) {
    if (channel.mListeners[j].mActive) {
      primary = j;
      const CSfxListener& listener = channel.mListeners[j].mListener;
      CAudioSys::S3dUpdateListener(listener.mPosition, listener.mDirection, listener.mHeading,
                                   listener.mUp, listener.mMaxVolume);
      break;
    }
  }
  if (primary != -1) {
    rstl::reserved_vector< CVector3f, 4 > rightVectors;
    for (int j = primary; j < 4; ++j) {
      if (channel.mListeners[j].mActive) {
        const CSfxListener& listener = channel.mListeners[j].mListener;
        rightVectors.push_back(CVector3f::Cross(listener.mHeading, listener.mUp).AsNormalized());
      } else {
        rightVectors.push_back(CVector3f::Right());
      }
    }
    const CSfxListener& listener = channel.mListeners[primary].mListener;
    const CVector3f& primaryHeading = listener.mHeading;
    const CVector3f& primaryUp = listener.mUp;
    const CVector3f& primaryRight = rightVectors[primary];
    for (int j = 0; j < channel.mSounds.size(); ++j) {
      CBaseSfxWrapper* sound = channel.mSounds[j];
      if (sound == nullptr || sound->GetType() != kSWT_Emitter) {
#endif

      continue;
    }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    chan.x48_[i]->Release();
    chan.x48_[i] = nullptr;
#else
      CSfxEmitterWrapper* emitter = static_cast< CSfxEmitterWrapper* >(sound);
      if (!emitter->IsPlaying()) {
        continue;
      }
      if (emitter->IsSilent() && !emitter->IsUpdatePending()) {
        continue;
      }
      const CVector3f& position = emitter->GetEmitter().mPos;
      const CVector3f& direction = emitter->GetEmitter().mDir;
      uchar volume = emitter->GetEmitter().mMaxVol;
      if (emitter->IsUpdatePending()) {
        volume = emitter->GetCachedMaxVolume();
        emitter->ClearUpdatePending();
        emitter->GetEmitter().mMaxVol = volume;
      }
      int closest = -1;
      float distance = FLT_MAX;
      for (int k = primary; k < 4; ++k) {
        if (channel.mListeners[k].mActive) {
          const float candidate =
              (channel.mListeners[k].mListener.mPosition - position).MagSquared();
          if (candidate < distance) {
            distance = candidate;
            closest = k;
          }
        }
      }
      if (closest != primary && closest != -1) {
        const CSfxListener& other = channel.mListeners[closest].mListener;
        const CVector3f& otherHeading = other.mHeading;
        const CVector3f& otherUp = other.mUp;
        const CVector3f& otherRight = rightVectors[closest];
        const CVector3f relative = position - other.mPosition;
        const CVector3f transformedPosition =
            listener.mPosition + CVector3f::Dot(otherHeading, relative) * primaryHeading +
            CVector3f::Dot(otherRight, relative) * primaryRight +
            CVector3f::Dot(otherUp, relative) * primaryUp;
        const CVector3f transformedDirection =
            direction.IsNonZero() ? CVector3f::Dot(otherHeading, direction) * primaryHeading +
                                        CVector3f::Dot(otherRight, direction) * primaryRight +
                                        CVector3f::Dot(otherUp, direction) * primaryUp
                                  : CVector3f::Zero();
        CAudioSys::S3dUpdateEmitter(emitter->GetHandle(), transformedPosition, transformedDirection,
                                    volume);
      } else {
        CAudioSys::S3dUpdateEmitter(emitter->GetHandle(), position, direction, volume);
      }
    }
  }
  UpdateLowPassAreaFilters(dt);
  UpdateLowPassFilters(dt);
  if (mCurrentChannel == kSC_Game) {
    CSfxChannel& game = mChannels[kSC_Game];
    for (int j = 0; j < game.mSounds.size(); ++j) {
      if (game.mSounds[j] != nullptr && game.mSounds[j]->IsPlaying()) {
        const int area = game.mSounds[j]->GetArea();
        const bool acoustics = game.mSounds[j]->UseAcoustics();
        if (area != kAllAreas || acoustics) {
          const uint lowPass = ShouldApplyLowPass(game.mSounds[j]) ? 1 : 0;
          CAudioSys::SfxSetFilter(game.mSounds[j]->GetVoice(), lowPass,
                                  GetLowPassFrequency(game.mSounds[j]));
        }
        CAudioSys::SfxPitchBend(game.mSounds[j]->GetVoice(), game.mSounds[j]->GetPitchBend());
      }
    }
  }
  mAuxEffectManager.Cleanup();
}

void CSfxManager::PitchBend(CSfxHandle handle, int pitch) {
  CSfxChannel& channel = mChannels[mCurrentChannel];
  if (!handle) {
    return;
  }
  CBaseSfxWrapper* sound = channel.mSounds[handle.GetIndex()];
  if (sound == nullptr || handle != sound->GetSfxHandle()) {
    return;
  }
  sound->SetPitchBend(pitch);
#endif

    mDoUpdate = true;
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (mAuxProcessingEnabled && mReverbAmount < 1.f) {
    mReverbAmount = rstl::min_val(1.f, mReverbAmount + (dt / (mReverbScale)));
    ApplyReverb();
  } else if (!mAuxProcessingEnabled && mReverbAmount > 0.f) {
    mReverbAmount = rstl::max_val(0.f, mReverbAmount - (dt / (mReverbScale * 2.f)));
    ApplyReverb();

    if (mReverbAmount == 0.f) {
      DisableAuxCallbacks();
      EnableAuxCallbacks();
#else
bool CSfxManager::IsPlaying(CSfxHandle handle) {
  if (!handle) {
    return false;
#endif

    }
#if !RSTL_PRIME1
  const int index = handle.GetIndex();
  CSfxChannel& channel = mChannels[mCurrentChannel];
  if (index < 0 || index >= channel.mSounds.size()) {
    return false;
  }
  CBaseSfxWrapper* sound = channel.mSounds[index];
  if (sound == nullptr || handle != sound->GetSfxHandle() || !sound->IsPlaying()) {
    return false;
  }
  return sound->IsPlaying();
#endif

  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (mTranslationTableTok.get() && mTranslationTableTok->IsLocked() &&
      mTranslationTableTok->IsLoaded()) {
    if (mTranslationTable == nullptr) {
      TToken< rstl::vector< short > > tmp(*mTranslationTableTok);
      mTranslationTable = rs_new rstl::vector< short >(*tmp.GetT());
      int i = 0;
      for (rstl::vector< short >::iterator it = tmp.GetT()->begin(); it != tmp.GetT()->end();
           ++it) {
        if (*it > i) {
          i = *it;
        }
      }
    }
    mTranslationTableTok = nullptr;
  }
}
void CSfxManager::PitchBend(CSfxHandle handle, int pitch) {
  CBaseSfxWrapper* wrapper = mChannels[mCurrentChannel].x48_[handle.GetIndex()];
  if (wrapper == nullptr || handle != wrapper->GetSfxHandle()) {
    return;
  }

  if (!wrapper->IsPlaying()) {
    Update(0.f);
  }

  if (wrapper->IsPlaying()) {
    mDoUpdate = true;
    const SND_VOICEID handle = wrapper->GetVoice();
    CAudioSys::SfxPitchBend(handle, pitch);
  }
}

bool CSfxManager::IsPlaying(CSfxHandle handle) {
  CSfxChannel& chan = mChannels[mCurrentChannel];
  if (handle.GetIndex() < 0 || handle.GetIndex() >= chan.x48_.size()) {
#else
bool CSfxManager::IsQueued(CSfxHandle handle) {
  if (!handle) {
#endif

    return false;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CBaseSfxWrapper* wrapper = chan.x48_[handle.GetIndex()];
  if (wrapper == nullptr || handle != wrapper->GetSfxHandle() || !wrapper->IsPlaying()) {
#else
  const int index = handle.GetIndex();
  CSfxChannel& channel = mChannels[mCurrentChannel];
  if (index < 0 || index >= channel.mSounds.size()) {
#endif

    return false;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return wrapper->IsPlaying();
}

bool CSfxManager::IsHandleValid(CSfxHandle handle) {
  CSfxChannel& chan = mChannels[mCurrentChannel];
  if (handle.GetIndex() < 0 || handle.GetIndex() >= chan.x48_.size()) {
    return false;
  }

  CBaseSfxWrapper* wrapper = chan.x48_[handle.GetIndex()];
  if (wrapper == nullptr || handle != wrapper->GetSfxHandle()) {
#else
  CBaseSfxWrapper* sound = channel.mSounds[index];
  if (sound == nullptr || handle != sound->GetSfxHandle()) {
#endif

    return false;
  }
  return true;
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
int CSfxManager::GetRank(CSfxManager::CBaseSfxWrapper* wrapper) {
  CSfxChannel& chan = mChannels[mCurrentChannel];
  if (!wrapper->IsInArea()) {
#else
int CSfxManager::GetRank(CBaseSfxWrapper* sound) {
  const CSfxChannel& channel = mChannels[mCurrentChannel];
  if (!sound->IsInArea()) {
#endif

    return 0;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int rank = wrapper->GetPriority() >> 2;

  if (wrapper->IsPlaying()) {
    rank += 1;
#else
  int rank = sound->GetPriority() >> 2;
  if (sound->IsPlaying()) {
    ++rank;
#endif

  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (wrapper->IsLooped()) {
#else
  if (sound->IsLooped()) {
#endif

    rank -= 2;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (wrapper->Ready() && !wrapper->IsPlaying()) {
#else
  if (sound->Ready() && !sound->IsPlaying()) {
#endif

    rank += 3;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (chan.x44_) {
    const int tmp = wrapper->GetAudible(chan.mListener.x0_);
    if (tmp == 0) {
#else
  for (int i = 0; i < 4; ++i) {
    if (channel.mListeners[i].mActive) {
      const int audible = sound->GetAudible(channel.mListeners[i].mListener.mPosition);
      if (audible == kSA_Inaudible) {
#endif

      rank = 0;
    } else {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      rank += tmp * 2;
#else
        rank += audible * 2;
#endif

    }
  }
#if !RSTL_PRIME1
  }
#endif

  return rank;
}

bool CSfxManager::LoadTranslationTable(CSimplePool* pool, const SObjectTag* tag) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (!tag) {
#else
  if (tag == nullptr) {
#endif

    return false;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (mTranslationTable) {
    delete mTranslationTable;
#else
  if (mpTranslationTable != nullptr) {
    delete mpTranslationTable;
#endif

  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mTranslationTable = nullptr;
  mTranslationTableTok = rs_new CToken(pool->GetObj(*tag));
  mTranslationTableTok->Lock();
#else
  mpTranslationTable = nullptr;
  mpTranslationTableToken = rs_new CToken(pool->GetObj(*tag));
  mpTranslationTableToken->Lock();
#endif

  return true;
}

ushort CSfxManager::TranslateSFXID(ushort id) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (mTranslationTable == nullptr || id >= mTranslationTable->size()) {
    return -1;
#else
  if (mpTranslationTable == nullptr || id >= mpTranslationTable->size()) {
    return kInternalInvalidSfxId;
  }
  short translated = (*mpTranslationTable)[id];
  if (translated < 0) {
    return kInternalInvalidSfxId;
  }
  return translated;
#endif

  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  short ret = (*mTranslationTable)[id];
  if (ret < 0) {
    return -1;
  }
  return CCast::ToUint16(ret);
}

void CSfxManager::SetActiveAreas(const rstl::reserved_vector< int, 10 >& areas) {
  CSfxChannel& chan = mChannels[mCurrentChannel];
  for (short i = 0; i < chan.x48_.size(); ++i) {
    if (chan.x48_[i] != nullptr) {
      int area = chan.x48_[i]->GetArea();
      if (area == kAllAreas) {
        chan.x48_[i]->SetInArea(true);
#else
void CSfxManager::SetActiveAreas(const rstl::reserved_vector< int, 10 >& areas, int currentArea) {
  mCurrentArea = currentArea;
  CSfxChannel& channel = mChannels[mCurrentChannel];
  for (CAuxEffect* effect = mAuxEffects.begin(); effect != mAuxEffects.end(); ++effect) {
    if (!effect->IsRegistered() || !effect->IsActive() || effect->GetArea() == kAllAreas) {
#endif

        continue;
      }
#if !RSTL_PRIME1
    const int* area = areas.begin();
    for (; area != areas.end(); ++area) {
      if (*area == effect->GetArea()) {
        break;
      }
    }
    if (area == areas.end()) {
      effect->Deactivate();
      mAuxEffectManager.RemoveEffect(effect->GetProcessingId());
      SetAreaVolume(effect->GetArea(), 127);
    }
  }
  for (SAreaVolume* volume = mAreaVolumes.begin(); volume != mAreaVolumes.end(); ++volume) {
    if (volume->first == kAllAreas) {
      continue;
    }
    const int* area = areas.begin();
    for (; area != areas.end(); ++area) {
      if (volume->first == *area) {
        break;
      }
    }
    if (area == areas.end()) {
      volume->first = kAllAreas;
    }
  }
  if (currentArea != mStudioState.first) {
    mStudioState.first = currentArea;
    mStudioState.second = (uchar)!mStudioState.second;
  }
  for (const int* area = areas.begin(); area != areas.end(); ++area) {
    int priority = -1;
    CAuxEffect* best = mAuxEffects.end();
    for (CAuxEffect* effect = mAuxEffects.begin(); effect != mAuxEffects.end(); ++effect) {
      if (effect->IsRegistered() && effect->GetArea() == *area &&
          effect->GetPriority() > priority) {
        best = effect;
        priority = effect->GetPriority();
      }
    }
    if (best == mAuxEffects.end() || best->IsActive()) {
      continue;
    }
    for (CAuxEffect* effect = mAuxEffects.begin(); effect != mAuxEffects.end(); ++effect) {
      if (effect->IsRegistered() && effect->GetArea() == *area) {
        mAuxEffectManager.RemoveEffect(effect->GetProcessingId());
        effect->SetActive(false);
      }
    }
    bool bus = mStudioState.second;
    if (best->GetArea() != mStudioState.first) {
      bus = mStudioState.second ? false : true;
    }
    best->SetProcessingId(
        mAuxEffectManager.AddEffect(bus, *best, CAuxEffectManager::kEC_Parallel, true));
    best->SetActive(true);
    best->SetBusIndex(bus);
    SetAreaVolume(best->GetArea(), best->GetVolume());
    break;
  }
  for (int i = 0; i < channel.mSounds.size(); ++i) {
    if (channel.mSounds[i] == nullptr) {
      continue;
    }
    const int soundArea = channel.mSounds[i]->GetArea();
    if (soundArea == kAllAreas) {
      channel.mSounds[i]->SetInArea(true);
    } else {
#endif

      bool inArea = false;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      for (rstl::reserved_vector< int, 10 >::const_iterator areaIt = areas.begin();
           areaIt != areas.end(); ++areaIt) {
        if ((*areaIt) == area) {
#else
      for (const int* area = areas.begin(); area != areas.end(); ++area) {
        if (*area == soundArea) {
#endif

          inArea = true;
        }
      }

      mDoUpdate = true;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      chan.x48_[i]->SetInArea(inArea);
#else
      const bool active = inArea;
      channel.mSounds[i]->SetInArea(active);
#endif

    }
  }
}

CSfxManager::CSfxEmitterWrapper*
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CSfxManager::AllocateCSfxEmitterWrapper(const CSfxEmitterWrapper& wrapper) {
  CSfxEmitterWrapper* ret = nullptr;
#else
CSfxManager::AllocateCSfxEmitterWrapper(const CSfxEmitterWrapper& sound) {
  CSfxEmitterWrapper* result = nullptr;
#endif

  for (int i = 0; i < mEmitterWrapperPool.size(); ++i) {
    if (mEmitterWrapperPool[i].Available()) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      mEmitterWrapperPool[i] = wrapper;
      ret = &mEmitterWrapperPool[i];
#else
      mEmitterWrapperPool[i] = sound;
      result = &mEmitterWrapperPool[i];
#endif

      break;
    }
  }
#if !RSTL_PRIME1
  if (result == nullptr && mEmitterWrapperPool.size() != mEmitterWrapperPool.capacity()) {
    mEmitterWrapperPool.push_back(sound);
    result = &mEmitterWrapperPool.back();
  }
  return result;
}
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (ret == nullptr && mEmitterWrapperPool.size() != mEmitterWrapperPool.capacity()) {
    mEmitterWrapperPool.push_back(wrapper);
    ret = &mEmitterWrapperPool.back();
  }
  return ret;
}
CSfxManager::CSfxWrapper* CSfxManager::AllocateCSfxWrapper(const CSfxWrapper& wrapper) {
  CSfxWrapper* ret = nullptr;
#else
CSfxManager::CSfxWrapper* CSfxManager::AllocateCSfxWrapper(const CSfxWrapper& sound) {
  CSfxWrapper* result = nullptr;
#endif

  for (int i = 0; i < mWrapperPool.size(); ++i) {
    if (mWrapperPool[i].Available()) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      mWrapperPool[i] = wrapper;
      ret = &mWrapperPool[i];
#else
      mWrapperPool[i] = sound;
      result = &mWrapperPool[i];
#endif

      break;
    }
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (ret == nullptr && mWrapperPool.size() != mWrapperPool.capacity()) {
    mWrapperPool.push_back(wrapper);
    ret = &mWrapperPool.back();
#else
  if (result == nullptr && mWrapperPool.size() != mWrapperPool.capacity()) {
    mWrapperPool.push_back(sound);
    result = &mWrapperPool.back();
#endif

  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return ret;
}
void CSfxManager::DisableAuxProcessing() {
  mRequestedAuxEffect = kAE_None;
  mAuxProcessingEnabled = false;
}
const bool CSfxManager::IsAuxProcessingEnabled() { return mAuxProcessingEnabled; }
const short CSfxManager::GetReverbAmount() {
  const short v = CCast::FtoS(mReverbAmount * 127.f);
  return v;
#else
  return result;
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CSfxManager::ApplyReverb() {
  CSfxChannel& chan = mChannels[mCurrentChannel];

  const short reverb = GetReverbAmount();
  for (int i = 0; i < chan.x48_.size(); ++i) {
    if (chan.x48_[i]) {
      chan.x48_[i]->SetReverb(reverb);
#else
void CSfxManager::SetMuted(bool muted) {
  CSfxChannel& channel = mChannels[mCurrentChannel];
  mDoUpdate = true;
  mMuted = muted;
  if (muted) {
    for (int i = 0; i < channel.mSounds.size(); ++i) {
      if (channel.mSounds[i] != nullptr) {
        if (channel.mSounds[i]->IsLooped()) {
          channel.mSounds[i]->UpdateEmitterSilent();
        } else {
          channel.mSounds[i]->Stop();
#endif

    }
  }
}
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CSfxManager::PrepareReverbHiCallback(const SND_AUX_REVERBHI& info) {
  DisableAuxProcessing();
  mRequestedAuxEffect = kAE_ReverbHi;
  sPendingReverbHi = info;
  if (mReverbAmount == 0.f) {
    EnableAuxCallbacks();
#else
    for (int i = 0; i < channel.mSounds.size(); ++i) {
      if (channel.mSounds[i] != nullptr && !channel.mSounds[i]->IsLooped()) {
        channel.mSounds[i]->Release();
        channel.mSounds[i] = nullptr;
#endif

  }
}
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CSfxManager::PrepareChorusCallback(const SND_AUX_CHORUS& info) {
  DisableAuxProcessing();
  mRequestedAuxEffect = kAE_Chorus;
  sPendingChorus = info;
  if (mReverbAmount == 0.f) {
    EnableAuxCallbacks();
#else
  } else {
    for (int i = 0; i < channel.mSounds.size(); ++i) {
      if (channel.mSounds[i] != nullptr) {
        channel.mSounds[i]->UpdateEmitter();
#endif

  }
}
#if RSTL_PRIME1
void CSfxManager::PrepareReverbStdCallback(const SND_AUX_REVERBSTD& info) {
  DisableAuxProcessing();
  mRequestedAuxEffect = kAE_ReverbStd;
  sPendingReverbStd = info;
  if (mReverbAmount == 0.f) {
    EnableAuxCallbacks();
  }
}
void CSfxManager::PrepareDelayCallback(const SND_AUX_DELAY& info) {
  DisableAuxProcessing();
  mRequestedAuxEffect = kAE_Delay;
  sPendingDelay = info;
  if (mReverbAmount == 0.f) {
    EnableAuxCallbacks();
#endif

  }
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CSfxManager::EnableAuxCallbacks() {
  mReverbAmount = 0.f;
  SND_AUX_CALLBACK cb = nullptr;
  void* userA = nullptr;
  ApplyReverb();
  if (mCurrentAuxEffect != kAE_None) {
    DisableAuxCallbacks();
#else
short CSfxManager::GetReverbAmount() { return 127; }

int CSfxManager::AddAuxEffect(int area, const SND_AUX_REVERBHI& params, uchar volume,
                              int priority) {
  return RegisterAuxEffect(CAuxEffect(params, area, volume, priority));
#endif

  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mCurrentAuxEffect = mRequestedAuxEffect;
  if (mCurrentAuxEffect == kAE_None) {
#else
int CSfxManager::AddAuxEffect(int area, const SND_AUX_CHORUS& params, uchar volume, int priority) {
  return RegisterAuxEffect(CAuxEffect(params, area, volume, priority));
}

int CSfxManager::AddAuxEffect(int area, const SND_AUX_REVERBSTD& params, uchar volume,
                              int priority) {
  return RegisterAuxEffect(CAuxEffect(params, area, volume, priority));
}

int CSfxManager::AddAuxEffect(int area, const SND_AUX_DELAY& params, uchar volume, int priority) {
  return RegisterAuxEffect(CAuxEffect(params, area, volume, priority));
}

int CSfxManager::AddAuxEffect(int area, const SFlangerAuxParameters& params, uchar volume,
                              int priority) {
  return RegisterAuxEffect(CAuxEffect(params, area, volume, priority));
}

int CSfxManager::AddAuxEffect(int area, const SBitcrusherAuxParameters& params, uchar volume,
                              int priority) {
  return RegisterAuxEffect(CAuxEffect(params, area, volume, priority));
}

int CSfxManager::AddAuxEffect(int area, const SPhaserAuxParameters& params, uchar volume,
                              int priority) {
  return RegisterAuxEffect(CAuxEffect(params, area, volume, priority));
}

int CSfxManager::AddAuxEffect(int area, const SFilteredDelayAuxParameters& params, uchar volume,
                              int priority) {
  return RegisterAuxEffect(CAuxEffect(params, area, volume, priority));
}

int CSfxManager::RegisterAuxEffect(const CAuxEffect& effect) {
  if (++mNextAuxEffectId == 0) {
    ++mNextAuxEffectId;
  }
  CAuxEffect* slot = nullptr;
  for (CAuxEffect* candidate = mAuxEffects.begin(); candidate != mAuxEffects.end(); ++candidate) {
    if (!candidate->IsRegistered()) {
      slot = candidate;
      break;
    }
  }
  if (slot == nullptr) {
    if (mAuxEffects.size() >= mAuxEffects.capacity()) {
      return 0;
    }
    mAuxEffects.push_back(effect);
    slot = &mAuxEffects.back();
  }
  *slot = effect;
  slot->Register();
  slot->SetActive(false);
  slot->SetId(mNextAuxEffectId);
  if (slot->GetArea() == kAllAreas) {
    slot->SetProcessingId(
        mAuxEffectManager.AddEffect(2, *slot, CAuxEffectManager::kEC_Serial, true));
    slot->SetActive(true);
  }
  return mNextAuxEffectId;
}

void CSfxManager::RemoveAuxEffect(int id) {
  for (CAuxEffect* effect = mAuxEffects.begin(); effect != mAuxEffects.end(); ++effect) {
    if (effect->IsRegistered() && effect->GetId() == id) {
      if (effect->IsActive()) {
        mAuxEffectManager.RemoveEffect(effect->GetProcessingId());
        SetAreaVolume(effect->GetArea(), 127);
      }
      effect->Unregister();
    }
  }
}

uchar CSfxManager::GetStudio(int area) {
  static const uchar studios[] = {1, 2};
  if (area == kAllAreas || area == mStudioState.first) {
    return studios[mStudioState.second];
  }
  return studios[mStudioState.second ? 0 : 1];
}

void CSfxManager::UpdatePitchBends(float dt) {
  if (mCurrentChannel != kSC_Game) {
#endif

    return;
  }
#if !RSTL_PRIME1
  for (CSfxPitchBend* bend = mPitchBends.begin(); bend != mPitchBends.end();) {
    bend->Update(dt);
    PitchBend(bend->GetHandle(), bend->GetPitch());
    const bool queued = IsQueued(bend->GetHandle());
    if (bend->IsFinished() || !queued) {
      bend = mPitchBends.erase(bend);
    } else {
      ++bend;
    }
  }
}
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  switch (mCurrentAuxEffect) {
  case kAE_ReverbHi:
    sReverbHi = sPendingReverbHi;
    userA = &sReverbHi;
    sndAuxCallbackPrepareReverbHI(&sReverbHi);
    cb = &sndAuxCallbackReverbHI;
    break;
  case kAE_Chorus:
    sChorus = sPendingChorus;
    userA = &sChorus;
    sndAuxCallbackPrepareChorus(&sChorus);
    cb = sndAuxCallbackChorus;
    break;
  case kAE_ReverbStd:
    sReverbStd = sPendingReverbStd;
    userA = &sReverbStd;
    sndAuxCallbackPrepareReverbSTD(&sReverbStd);
    cb = sndAuxCallbackReverbSTD;
    break;
  case kAE_Delay:
    sDelay = sPendingDelay;
    userA = &sDelay;
    sndAuxCallbackPrepareDelay(&sDelay);
    cb = sndAuxCallbackDelay;
    break;
  default:
#else
void CSfxManager::AddPitchBend(const CSfxPitchBend& pitchBend) {
  if (mPitchBends.size() < mPitchBends.capacity()) {
    mPitchBends.push_back(pitchBend);
  }
}

int CSfxManager::AddLowPassAreaFilter(int frequency, float duration) {
  if (mAreaLowPassFilters.size() < mAreaLowPassFilters.capacity()) {
    if (++mNextAreaFilterId == 0) {
      mNextAreaFilterId = 1;
    }
    mAreaLowPassFilters.push_back(SLowPassFilter(frequency, duration, mNextAreaFilterId));
    return mNextAreaFilterId;
  }
  return 0;
}

void CSfxManager::UpdateLowPassAreaFilters(float dt) {
  mAreaLowPassFrequency = 44100;
  bool haveFrequency = false;
  for (SLowPassFilter* it = mAreaLowPassFilters.begin(); it != mAreaLowPassFilters.end();) {
    it->mTimeRemaining -= dt;
    const bool active = !it->mTimed || it->mTimeRemaining > 0.f;
    if (!active) {
      it = mAreaLowPassFilters.erase(it);
      continue;
    }
    if (!haveFrequency || it->mFrequency < mAreaLowPassFrequency) {
      mAreaLowPassFrequency = it->mFrequency;
      haveFrequency = true;
    }
    ++it;
  }
}

void CSfxManager::RemoveLowPassAreaFilter(int id) {
  if (id == 0) {
    return;
  }
  for (SLowPassFilter* it = mAreaLowPassFilters.begin(); it != mAreaLowPassFilters.end(); ++it) {
    if (id == it->mId) {
      mAreaLowPassFilters.erase(it);
      return;
    }
  }
}

bool CSfxManager::IsLowPassAreaFilterEnabled() {
  return mAreaLowPassFilters.size() > 0 && mCurrentChannel == kSC_Game;
}

int CSfxManager::GetLowPassAreaFrequency() { return mAreaLowPassFrequency; }

int CSfxManager::AddLowPassFilter(int frequency, float duration) {
  if (mLowPassFilters.size() < mLowPassFilters.capacity()) {
    if (++mNextFilterId == 0) {
      mNextFilterId = 1;
    }
    mLowPassFilters.push_back(SLowPassFilter(frequency, duration, mNextFilterId));
    return mNextFilterId;
  }
  return 0;
}

void CSfxManager::UpdateLowPassFilters(float dt) {
  mLowPassFrequency = 44100;
  bool haveFrequency = false;
  for (SLowPassFilter* it = mLowPassFilters.begin(); it != mLowPassFilters.end();) {
    it->mTimeRemaining -= dt;
    const bool active = !it->mTimed || it->mTimeRemaining > 0.f;
    if (!active) {
      it = mLowPassFilters.erase(it);
      continue;
    }
    if (!haveFrequency || it->mFrequency < mLowPassFrequency) {
      mLowPassFrequency = it->mFrequency;
      haveFrequency = true;
    }
    ++it;
  }
}

void CSfxManager::RemoveLowPassFilter(int id) {
  if (id == 0) {
    return;
  }
  for (SLowPassFilter* it = mLowPassFilters.begin(); it != mLowPassFilters.end(); ++it) {
    if (id == it->mId) {
      mLowPassFilters.erase(it);
      return;
    }
  }
}

bool CSfxManager::IsLowPassEnabled() {
  return mLowPassFilters.size() > 0 && mCurrentChannel == kSC_Game;
}

int CSfxManager::GetLowPassFrequency() { return mLowPassFrequency; }

bool CSfxManager::ShouldApplyLowPass(CBaseSfxWrapper* sound) {
  if (sound->GetArea() != kAllAreas && IsLowPassAreaFilterEnabled() &&
      !sound->GetIgnoreAreaLowPass()) {
    return true;
  }
  return sound->UseAcoustics() && IsLowPassEnabled();
}

int CSfxManager::GetLowPassFrequency(CBaseSfxWrapper* sound) {
  int frequency = mAreaLowPassFrequency;
  if (sound->GetArea() != kAllAreas && IsLowPassAreaFilterEnabled() &&
      !sound->GetIgnoreAreaLowPass()) {
    frequency = GetLowPassAreaFrequency();
  }
  if (sound->UseAcoustics() && IsLowPassEnabled()) {
    frequency = rstl::min_val(frequency, GetLowPassFrequency());
  }
  return frequency;
}

uchar CSfxManager::GetAreaVolume(int area) {
  if (area != kAllAreas) {
    for (SAreaVolume* it = mAreaVolumes.begin(); it != mAreaVolumes.end(); ++it) {
      if (area == it->first) {
        return it->second;
      }
    }
  }
  return 127;
}

void CSfxManager::SetAreaVolume(int area, uchar volume) {
  SAreaVolume* it;
  for (it = mAreaVolumes.begin(); it != mAreaVolumes.end(); ++it) {
    if (area == it->first) {
      it->second = volume;
#endif

    break;
  }
#if RSTL_PRIME1
  sndSetAuxProcessingCallbacks(0, cb, userA, 0xFF, 0, nullptr, nullptr, 0xFF, 0);
  mAuxProcessingEnabled = true;
#endif

}
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CSfxManager::DisableAuxCallbacks() {
  sndSetAuxProcessingCallbacks(0, nullptr, nullptr, 0xFF, 0, nullptr, 0, 0xFF, 0);
  switch (mCurrentAuxEffect) {
  case kAE_ReverbHi:
    sndAuxCallbackShutdownReverbHI(&sReverbHi);
    break;
  case kAE_Chorus:
    sndAuxCallbackShutdownChorus(&sChorus);
    break;
  case kAE_ReverbStd:
    sndAuxCallbackShutdownReverbSTD(&sReverbStd);
    break;
  case kAE_Delay:
    sndAuxCallbackShutdownDelay(&sDelay);
    break;
  default:
    break;
#else
  if (it != mAreaVolumes.end()) {
    return;
#endif

  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mCurrentAuxEffect = kAE_None;
#else
  for (it = mAreaVolumes.begin(); it != mAreaVolumes.end(); ++it) {
    if (it->first == kAllAreas) {
      it->first = area;
      it->second = volume;
      return;
    }
  }
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
const CFactoryFnReturn FAudioTranslationTableFactory(const SObjectTag& obj, CInputStream& in,
                                               const CVParamTransfer& xfer) {
#else
void CSfxManager::SetIgnoreAreaLowPass(CSfxHandle handle, bool ignore) {
  CSfxChannel& channel = mChannels[mCurrentChannel];
  const int index = handle.GetIndex();
  if (index < 0 || index >= channel.mSounds.size()) {
    return;
  }
  CBaseSfxWrapper* sound = channel.mSounds[index];
  if (sound == nullptr || handle != sound->GetSfxHandle()) {
    return;
  }
  sound->SetIgnoreAreaLowPass(ignore);
}

CFactoryFnReturn FAudioTranslationTableFactory(const SObjectTag&, CInputStream& in,
                                               const CVParamTransfer&) {
#endif

  return rs_new rstl::vector< short >(in);
}

#if !RSTL_PRIME1
CSfxManager::ESfxWrapperType CSfxManager::CSfxWrapper::GetType() const { return kSWT_Sound; }
#endif

