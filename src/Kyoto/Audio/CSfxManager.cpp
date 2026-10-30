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

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
float CSfxManager::mReverbAmount = 1.f;
float CSfxManager::mReverbScale = 0.1f;
CSfxManager::EAuxEffect CSfxManager::mCurrentAuxEffect = kAE_None;
CSfxManager::EAuxEffect CSfxManager::mRequestedAuxEffect = kAE_None;
CSfxManager::CSfxChannel CSfxManager::mChannels[4];

CSfxManager::ESfxChannels CSfxManager::mCurrentChannel = kSC_Default;
bool CSfxManager::mDoUpdate = false;

rstl::vector< short >* CSfxManager::mTranslationTable;
bool CSfxManager::mMuted = false;
rstl::auto_ptr< CToken > CSfxManager::mTranslationTableTok;

bool CSfxManager::mAuxProcessingEnabled = false;

rstl::reserved_vector< CSfxManager::CSfxEmitterWrapper, 64 > CSfxManager::mEmitterWrapperPool;
rstl::reserved_vector< CSfxManager::CSfxWrapper, 64 > CSfxManager::mWrapperPool;
SND_AUX_REVERBHI sReverbHi;
SND_AUX_CHORUS sChorus;
SND_AUX_REVERBSTD sReverbStd;
SND_AUX_DELAY sDelay;
SND_AUX_REVERBHI sPendingReverbHi;
SND_AUX_CHORUS sPendingChorus;
SND_AUX_REVERBSTD sPendingReverbStd;
SND_AUX_DELAY sPendingDelay;

const short CSfxManager::kMaxPriority = 255;
const short CSfxManager::kMedPriority = 127;
const ushort CSfxManager::kInternalInvalidSfxId = -1;

const int CSfxManager::kAllAreas = -1;
#else
CSfxManager::ESfxChannels CSfxManager::mCurrentChannel = kSC_Default;
bool CSfxManager::mDoUpdate = false;
int CSfxManager::mNextAreaFilterId = 0;
int CSfxManager::mNextFilterId = 0;
int CSfxManager::mNextAuxEffectId = 0;
rstl::vector< short >* CSfxManager::mpTranslationTable = nullptr;
bool CSfxManager::mMuted = false;
int CSfxManager::mAreaLowPassFrequency = 16000;
int CSfxManager::mLowPassFrequency = 16000;
int CSfxManager::mCurrentArea = -1;

CSfxManager::CSfxChannel CSfxManager::mChannels[4];
rstl::reserved_vector< CSfxManager::SLowPassFilter, 8 > CSfxManager::mAreaLowPassFilters;
rstl::reserved_vector< CSfxManager::SLowPassFilter, 8 > CSfxManager::mLowPassFilters;
rstl::auto_ptr< CToken > CSfxManager::mpTranslationTableToken;

rstl::reserved_vector< CSfxManager::CSfxEmitterWrapper, 64 > CSfxManager::mEmitterWrapperPool;
rstl::reserved_vector< CSfxManager::CSfxWrapper, 64 > CSfxManager::mWrapperPool;
rstl::reserved_vector< CSfxPitchBend, 8 > CSfxManager::mPitchBends;
rstl::reserved_vector< CAuxEffect, 10 > CSfxManager::mAuxEffects;
rstl::reserved_vector< CSfxManager::SAreaVolume, 10 >
    CSfxManager::mAreaVolumes(10, SAreaVolume(-1, uchar(0)));
CAuxEffectManager CSfxManager::mAuxEffectManager;
rstl::pair< int, bool > CSfxManager::mStudioState(-1, false);

const short CSfxManager::kMaxPriority = 255;
const short CSfxManager::kMedPriority = 127;
const ushort CSfxManager::kInternalInvalidSfxId = 0xffff;

const int CSfxManager::kAllAreas = -1;

CSfxManager::CSfxChannel::CSfxChannel() : mListeners(4, SListener()) {}

CSfxManager::SListener::SListener() : mActive(false) {}

CSfxManager::ESfxWrapperType CSfxManager::CSfxEmitterWrapper::GetType() const {
  return kSWT_Emitter;
}
#endif

CSfxManager::CBaseSfxWrapper::CBaseSfxWrapper(RSTL_PRIME1_CONST bool looped,
                                              RSTL_PRIME1_CONST short priority,
                                              RSTL_PRIME1_CONST CSfxHandle handle,
                                              const bool useAcoustics, RSTL_PRIME1_CONST int area)
: mTimeRemaining(15.f)
, mRank(0)
, mPriority(priority)
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
, mPitchBend(0x2000)
#endif
, mHandle(handle)
, mArea(area)
, mActive(true)
, mPlaying(false)
, mLooped(looped)
, mInArea(true)
, mReleased(false)
, mUseAcoustics(useAcoustics)
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
, mIgnoreAreaLowPass(false)
#endif
{
}

RSTL_PRIME1_CONST bool CSfxManager::CBaseSfxWrapper::Available() const { return mReleased; }

void CSfxManager::CBaseSfxWrapper::Release() {
  mReleased = true;
  mTimeRemaining = 15.f;
}

RSTL_PRIME1_CONST float CSfxManager::CBaseSfxWrapper::GetTimeRemaining() { return mTimeRemaining; }

void CSfxManager::CBaseSfxWrapper::SetTimeRemaining(float time) { mTimeRemaining = time; }

void CSfxManager::CBaseSfxWrapper::SetActive(bool active) { mActive = active; }

void CSfxManager::CBaseSfxWrapper::SetPlaying(bool playing) { mPlaying = playing; }

void CSfxManager::CBaseSfxWrapper::SetInArea(RSTL_PRIME1_CONST bool inArea) { mInArea = inArea; }

void CSfxManager::CBaseSfxWrapper::SetRank(RSTL_PRIME1_CONST short rank) { mRank = rank; }

RSTL_PRIME1_CONST bool CSfxManager::CBaseSfxWrapper::IsLooped() const { return mLooped; }

RSTL_PRIME1_CONST bool CSfxManager::CBaseSfxWrapper::IsInArea() const { return mInArea; }

bool CSfxManager::CBaseSfxWrapper::IsPlaying() const { return mPlaying; }

RSTL_PRIME1_CONST bool CSfxManager::CBaseSfxWrapper::IsActive() const { return mActive; }

RSTL_PRIME1_CONST bool CSfxManager::CBaseSfxWrapper::UseAcoustics() const { return mUseAcoustics; }

RSTL_PRIME1_CONST int CSfxManager::CBaseSfxWrapper::GetRank() const { return mRank; }

RSTL_PRIME1_CONST int CSfxManager::CBaseSfxWrapper::GetPriority() const { return mPriority; }

RSTL_PRIME1_CONST CSfxHandle CSfxManager::CBaseSfxWrapper::GetSfxHandle() const { return mHandle; }

RSTL_PRIME1_CONST int CSfxManager::CBaseSfxWrapper::GetArea() const { return mArea; }

// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
void CSfxManager::CBaseSfxWrapper::SetPitchBend(ushort pitch) { mPitchBend = pitch; }

ushort CSfxManager::CBaseSfxWrapper::GetPitchBend() const { return mPitchBend; }

bool CSfxManager::CBaseSfxWrapper::GetIgnoreAreaLowPass() const { return mIgnoreAreaLowPass; }

void CSfxManager::CBaseSfxWrapper::SetIgnoreAreaLowPass(bool ignore) {
  mIgnoreAreaLowPass = ignore;
}

#endif
CSfxManager::CSfxEmitterWrapper::CSfxEmitterWrapper(RSTL_PRIME1_CONST bool looped,
                                                    RSTL_PRIME1_CONST short priority,
                                                    CAudioSys::C3DEmitterParmData& emitter,
                                                    RSTL_PRIME1_CONST CSfxHandle handle,
                                                    const bool useAcoustics,
                                                    RSTL_PRIME1_CONST int area)
: CBaseSfxWrapper(looped, priority, handle, useAcoustics, area)
, mEmitterData(emitter)
, mEmitterHandle(SND_ID_ERROR)
, mReady(true)
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
, mUpdatePending(false)
#endif
{
}

void CSfxManager::CSfxEmitterWrapper::SetReverb(RSTL_PRIME1_CONST char reverb) {
  if (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      CSfxManager::IsAuxProcessingEnabled() &&
#endif
      UseAcoustics()) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    mPara.paraData.value7 = reverb;
#else
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

  if (CSfxManager::IsAuxProcessingEnabled() && UseAcoustics()) {
    mPara.paraData.value7 = CSfxManager::GetReverbAmount();
  } else {
    mPara.paraData.value7 = 0;
  }
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

  mEmitterHandle = CAudioSys::S3dAddEmitterParaEx(mEmitterData, GetSfxHandle().GetIndex() & 0xFF,
                                                  &mParameterInfo);
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
#if !RSTL_PRIME1
const CAudioSys::C3DEmitterParmData& CSfxManager::CSfxEmitterWrapper::GetEmitter() const {
  return mEmitterData;
}

#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
const SND_VOICEID CSfxManager::CSfxEmitterWrapper::GetHandle() const { return mEmitterHandle; }
#else
uint CSfxManager::CSfxEmitterWrapper::GetHandle() const { return mEmitterHandle; }
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
bool CSfxManager::CSfxEmitterWrapper::IsPlaying() const {
  if (!IsLooped()) {
    bool ret = false;
    if (CBaseSfxWrapper::IsPlaying() && CAudioSys::S3dCheckEmitter(mEmitterHandle)) {
      ret = true;
    }

    return ret;
  }

  return CBaseSfxWrapper::IsPlaying();
}
#else
bool CSfxManager::CSfxEmitterWrapper::IsPlaying() const {
  if (IsLooped()) {
    return CBaseSfxWrapper::IsPlaying();
  }
  return CBaseSfxWrapper::IsPlaying() && CAudioSys::S3dCheckEmitter(mEmitterHandle);
}
#endif

RSTL_PRIME1_CONST bool CSfxManager::CSfxEmitterWrapper::Ready() { return IsLooped() || mReady; }

short CSfxManager::CSfxEmitterWrapper::GetAudible(const CVector3f& position) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  float magSq = (mEmitterData.mPos - position).MagSquared();
  float maxDist = mEmitterData.mMaxDist * mEmitterData.mMaxDist;
  if (magSq < maxDist * 0.25f)
    return kSA_Aud3;
  else if (magSq < maxDist * 0.5f)
    return kSA_Aud2;
  else if (magSq < maxDist)
    return kSA_Aud1;
  return kSA_Aud0;
#else
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

RSTL_PRIME1_CONST SND_VOICEID CSfxManager::CSfxEmitterWrapper::GetVoice() const {
  return IsPlaying() ? CAudioSys::S3dEmitterVoiceID(mEmitterHandle) : SND_ID_ERROR;
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CSfxManager::CSfxEmitterWrapper::UpdateEmitterSilent() {
  mCachedMaxVol = mEmitterData.mMaxVol;

  CAudioSys::S3dUpdateEmitter(mEmitterHandle, mEmitterData.mPos, mEmitterData.mDir, 1);
}

void CSfxManager::CSfxEmitterWrapper::UpdateEmitter() {
  CAudioSys::S3dUpdateEmitter(mEmitterHandle, mEmitterData.mPos, mEmitterData.mDir, mCachedMaxVol);
}
#else
void CSfxManager::CSfxEmitterWrapper::UpdateEmitterSilent() {
  if (mEmitterData.mMaxVol != 1) {
    mCachedMaxVolume = mEmitterData.mMaxVol;
    mEmitterData.mMaxVol = 1;

    CAudioSys::S3dUpdateEmitter(mEmitterHandle, mEmitterData.mPos, mEmitterData.mDir, 1);
  }
}

void CSfxManager::CSfxEmitterWrapper::UpdateEmitter() { mUpdatePending = true; }
#endif

CSfxManager::CSfxWrapper::CSfxWrapper(RSTL_PRIME1_CONST bool looped,
                                      RSTL_PRIME1_CONST short priority,
                                      RSTL_PRIME1_CONST ushort sfxId, const short volume,
                                      RSTL_PRIME1_CONST short pan,
                                      RSTL_PRIME1_CONST CSfxHandle handle, const bool useAcoustics,
                                      RSTL_PRIME1_CONST int area)
: CBaseSfxWrapper(looped, priority, handle, useAcoustics, area)
, mSfxId(sfxId)
, mVoiceHandle(SND_ID_ERROR)
, mVolume(volume)
, mPan(pan)
, mReady(true) {}

void CSfxManager::CSfxWrapper::SetReverb(RSTL_PRIME1_CONST char reverb) {
  if (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      CSfxManager::IsAuxProcessingEnabled() &&
#endif
      UseAcoustics()) {
    CAudioSys::SfxCtrl(mVoiceHandle, SND_MIDICTRL_REVERB, reverb);
  }
}

void CSfxManager::CSfxWrapper::Play() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  mVoiceHandle = CAudioSys::SfxStart(mSfxId, mVolume, mPan, 0);
#else
  const uchar studio = UseAcoustics() ? GetStudio(GetArea()) : uchar(0);
  mVoiceHandle = CAudioSys::SfxStart(mSfxId, 127, mPan, studio);
  CAudioSys::SfxVolume(mVoiceHandle, mVolume);
#endif

  if (mVoiceHandle != SND_ID_ERROR) {
    if (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        CSfxManager::IsAuxProcessingEnabled() &&
#endif
        UseAcoustics()) {
      const ushort reverb = CSfxManager::GetReverbAmount();

      CAudioSys::SfxCtrl(mVoiceHandle, SND_MIDICTRL_REVERB, reverb);
    }

    SetPlaying(true);
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
    if (IsLowPassAreaFilterEnabled() && GetArea() != kAllAreas) {
      CAudioSys::SfxSetFilter(mVoiceHandle, 1, GetLowPassAreaFrequency());
    }
#endif
  }

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

RSTL_PRIME1_CONST bool CSfxManager::CSfxWrapper::Ready() { return IsLooped() || mReady; }

short CSfxManager::CSfxWrapper::GetAudible(const CVector3f&) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return kSA_Aud3;
#else
  return kSA_High;
#endif
}

RSTL_PRIME1_CONST SND_VOICEID CSfxManager::CSfxWrapper::GetVoice() const { return mVoiceHandle; }

void CSfxManager::CSfxWrapper::SetVolume(RSTL_PRIME1_CONST short volume) { mVolume = volume; }

void CSfxManager::CSfxWrapper::UpdateEmitterSilent() { CAudioSys::SfxVolume(mVoiceHandle, 1); }

void CSfxManager::CSfxWrapper::UpdateEmitter() { CAudioSys::SfxVolume(mVoiceHandle, mVolume); }

// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
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
  }
#else
  mAuxEffectManager.Shutdown();
  for (rstl::reserved_vector< CAuxEffect, 10 >::iterator it = mAuxEffects.begin();
       it != mAuxEffects.end(); ++it) {
    if (it->IsRegistered() && it->IsActive()) {
      it->Deactivate();
    }
  }
#endif
}

void CSfxManager::StopAndRemoveAllEmitters() {
  for (int i = 0; i < 4; ++i) {

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CSfxChannel* channel = &mChannels[i];
    for (int j = 0; j < channel->mSounds.size(); ++j) {
      if (channel->mSounds[j] != nullptr) {
        if (channel->mSounds[j]->IsPlaying()) {
          channel->mSounds[j]->Stop();
        }
        channel->mSounds[j]->Release();
        channel->mSounds[j] = nullptr;
      }
    }
#else
    CSfxChannel& channel = mChannels[i];
    for (int j = 0; j < channel.mSounds.size(); ++j) {
      if (channel.mSounds[j] != nullptr) {
        if (channel.mSounds[j]->IsPlaying()) {
          channel.mSounds[j]->Stop();
        }
        channel.mSounds[j]->Release();
        channel.mSounds[j] = nullptr;
      }
    }
#endif
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
}

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
}

CSfxHandle CSfxManager::AddEmitter(const SND_FXID id, const CVector3f& pos, const CVector3f& dir,
                                   bool useAcoustics, const bool looped, const short prio,
                                   const int areaId) {
  CAudioSys::C3DEmitterParmData emitterParm(150, 0.1f, 1, 127, 20);
  emitterParm.mPos = pos;
  emitterParm.mDir = dir;
  emitterParm.mSfxId = id;
  return AddEmitter(emitterParm, useAcoustics, prio, looped, areaId);
}

CSfxHandle CSfxManager::AddEmitter(const SND_FXID id, const CVector3f& pos, const CVector3f& dir,
                                   const uchar vol, bool useAcoustics, const bool looped,
                                   const short prio, const int areaId) {
  CAudioSys::C3DEmitterParmData emitterParm(150.f, 0.1f, 1, vol > 20 ? vol : 21, 20);
  emitterParm.mPos = pos;
  emitterParm.mDir = dir;
  emitterParm.mSfxId = id;

  return AddEmitter(emitterParm, useAcoustics, prio, looped, areaId);
}

CSfxHandle CSfxManager::AddEmitter(CAudioSys::C3DEmitterParmData& parmData, bool useAcoustics,
                                   const short prio, const bool looped, const int areaId) {
  if (!DEBUG_TOOLS_GET_BOOL(kDO_EnableAudio, true))

    return CSfxHandle::NullHandle();
#if NONMATCHING
  if (mMuted || parmData.mSfxId == 0xFFFF) {
#else
  if (mMuted || parmData.mSfxId == 0xFFFFFFFF || parmData.mSfxId == 0xFFFF) {
#endif

    return CSfxHandle::NullHandle();
  }

  CAudioSys::C3DEmitterParmData cpy = parmData;
  if (looped) {
    cpy.mFlags |= 6;
  }
  mDoUpdate = true;
  CSfxChannel& channel = mChannels[mCurrentChannel];
  CSfxHandle ret = LocateHandle(prio);
  if (ret) {
    CSfxEmitterWrapper wrapper(looped, prio, cpy, ret, useAcoustics, areaId);
    channel.mSounds[ret.GetIndex()] = AllocateCSfxEmitterWrapper(wrapper);
  }

  return ret;
}

void CSfxManager::UpdateEmitter(CSfxHandle handle, const CVector3f& pos, const CVector3f& dir,
                                const uchar maxVol) {
  CSfxChannel& chan = mChannels[mCurrentChannel];
  if (handle.GetIndex() < 0 || handle.GetIndex() >= chan.mSounds.size()) {
    return;
  }
  CSfxEmitterWrapper* wrapper = static_cast< CSfxEmitterWrapper* >(chan.mSounds[handle.GetIndex()]);
  if (!wrapper || handle != wrapper->GetSfxHandle() || !wrapper->IsPlaying()) {
    return;
  }
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
    chan.mSounds[ret.GetIndex()] = AllocateCSfxWrapper(wrapper);
  }
  return ret;
}

void CSfxManager::SfxStop(CSfxHandle handle) { StopSound(handle); }

void CSfxManager::SfxVolume(CSfxHandle handle, uchar volume) {
  CSfxChannel& chan = mChannels[mCurrentChannel];
  if (handle.GetIndex() < 0 || handle.GetIndex() >= chan.mSounds.size()) {
    return;
  }
  CSfxWrapper* wrapper = static_cast< CSfxWrapper* >(chan.mSounds[handle.GetIndex()]);
  if (wrapper == nullptr || handle != wrapper->GetSfxHandle()) {
    return;
  }
  wrapper->SetVolume(volume);
  if (wrapper->IsPlaying()) {
    CAudioSys::SfxVolume(wrapper->GetVoice(), volume);
  }
}

void CSfxManager::SfxSpan(CSfxHandle handle, uchar span) {
  CSfxChannel& chan = mChannels[mCurrentChannel];
  if (handle.GetIndex() < 0 || handle.GetIndex() >= chan.mSounds.size()) {
    return;
  }
  CSfxWrapper* wrapper = static_cast< CSfxWrapper* >(chan.mSounds[handle.GetIndex()]);
  if (wrapper == nullptr || handle != wrapper->GetSfxHandle()) {
    return;
  }
  if (wrapper->IsPlaying()) {
    CAudioSys::SfxSpan(wrapper->GetVoice(), span);
  }
}

void CSfxManager::KillAll(ESfxChannels channel) {
  CSfxChannel& chan = mChannels[channel];

  for (int i = 0; i < chan.mSounds.size(); ++i) {
    CBaseSfxWrapper* wrapper = chan.mSounds[i];
    if (wrapper != NULL && wrapper->IsPlaying()) {
      wrapper->Stop();
    }
    if (wrapper) {
      wrapper->Release();
    }
    chan.mSounds[i] = nullptr;
  }
}

void CSfxManager::StopSound(CSfxHandle handle) {
  CSfxChannel& chan = mChannels[mCurrentChannel];
  if (handle.GetIndex() < 0 || handle.GetIndex() >= chan.mSounds.size()) {
    return;
  }
  CBaseSfxWrapper* wrapper = chan.mSounds[handle.GetIndex()];
  if (wrapper == nullptr || handle != wrapper->GetSfxHandle()) {
    return;
  }
  mDoUpdate = true;
  if (wrapper->IsPlaying()) {
    wrapper->Stop();
  }
  wrapper->Release();
  chan.mSounds[handle.GetIndex()] = nullptr;
}

void CSfxManager::SetDuration(CSfxHandle handle, float duration) {
  CSfxChannel& chan = mChannels[mCurrentChannel];
  if (handle.GetIndex() < 0 || handle.GetIndex() >= chan.mSounds.size()) {
    return;
  }
  CBaseSfxWrapper* wrapper = chan.mSounds[handle.GetIndex()];
  if (wrapper == nullptr || handle != wrapper->GetSfxHandle()) {
    return;
  }
  wrapper->SetTimeRemaining(duration);
}

void CSfxManager::SetChannel(ESfxChannels channel) {
  if (channel == mCurrentChannel) {
    return;
  }
  if (mCurrentChannel != kSC_Invalid) {
    TurnOffChannel(mCurrentChannel);
  }
  TurnOnChannel(channel);
  mCurrentChannel = channel;
}

void CSfxManager::TurnOffChannel(ESfxChannels channel) {
  CSfxChannel& chan = mChannels[channel];
  for (int i = 0; i < chan.mSounds.size(); ++i) {
    if (!chan.mSounds[i]) {
      continue;
    }

    if (chan.mSounds[i]->IsLooped()) {
      chan.mSounds[i]->UpdateEmitterSilent();
    } else {
      chan.mSounds[i]->Stop();
    }
  }
  for (int i = 0; i < chan.mSounds.size(); ++i) {
    if (chan.mSounds[i] == nullptr || chan.mSounds[i]->IsLooped()) {
      continue;
    }
    chan.mSounds[i]->Release();
    chan.mSounds[i] = nullptr;
  }
}

void CSfxManager::TurnOnChannel(ESfxChannels channel) {
  mCurrentChannel = channel;
  CSfxChannel& chan = mChannels[channel];

  mDoUpdate = true;
  if (!chan.x44_) {
    return;
  }
  for (int i = 0; i < chan.mSounds.size(); ++i) {
    if (!chan.mSounds[i]) {
      continue;
    }
    chan.mSounds[i]->UpdateEmitter();
  }
}

CSfxHandle CSfxManager::LocateHandle(const short id) {
  CSfxChannel& chan = mChannels[mCurrentChannel];
  int i = 0;
  for (; i < chan.mSounds.size(); ++i) {
    if (chan.mSounds[i] != nullptr) {
      continue;
    }

    return CSfxHandle(i);
  }
  if (chan.mSounds.size() == chan.mSounds.capacity()) {
    return CSfxHandle::NullHandle();
  }
  chan.mSounds.push_back(nullptr);
  return CSfxHandle(chan.mSounds.size() - 1);
}

void CSfxManager::Update(float dt) {
  short i;
  ushort uVar21 = 0;
  CSfxChannel& chan = mChannels[mCurrentChannel];
  for (i = 0; i < chan.mSounds.size(); ++i) {
    if (!chan.mSounds[i] || chan.mSounds[i]->IsLooped()) {
      continue;
    }
    float remTime = chan.mSounds[i]->GetTimeRemaining();
    chan.mSounds[i]->SetTimeRemaining(remTime - dt);
    if (remTime < 0.f) {
      chan.mSounds[i]->Stop();

      mDoUpdate = true;
    }
  }
  ushort local_e0[70];

  if (mDoUpdate) {
    for (i = 0; i < chan.mSounds.size(); ++i) {
      if (!chan.mSounds[i]) {
        continue;
      }
      local_e0[uVar21++] = i;
      chan.mSounds[i]->SetRank(GetRank(chan.mSounds[i]));
    }
    for (i = 0; i < uVar21; i++) {
      bool done = true;
      for (int j = 0; j < (uVar21 - 1); j += 1) {
        if (chan.mSounds[local_e0[j]]->GetRank() < chan.mSounds[local_e0[j + 1]]->GetRank()) {
          done = false;
          rstl::swap(local_e0[j], local_e0[j + 1]);
        }
      }
      if (done) {
        break;
      }
    }
    for (i = 48; i < uVar21; ++i) {
      CBaseSfxWrapper* wrapper = chan.mSounds[local_e0[i]];
      if (!wrapper || !wrapper->IsPlaying()) {
        continue;
      }
      chan.mSounds[local_e0[i]]->Stop();
    }
    for (i = 0; i < uVar21; ++i) {
      if (chan.mSounds[local_e0[i]] == nullptr) {
        continue;
      }
      if (chan.mSounds[local_e0[i]]->IsPlaying() && !chan.mSounds[local_e0[i]]->IsInArea()) {
        chan.mSounds[local_e0[i]]->Stop();
      }
    }
  }

  CAudioSys::S3dFlushUnusedEmitters();
  if (mDoUpdate) {
    for (int iVar22 = 48, j = 0; j < uVar21 && iVar22 != 0; ++j) {
      if (!chan.mSounds[local_e0[j]]) {
        continue;
      }
      if (chan.mSounds[local_e0[j]]->IsPlaying()) {
        iVar22--;
      } else if (chan.mSounds[local_e0[j]]->Ready() && chan.mSounds[local_e0[j]]->IsInArea()) {
        chan.mSounds[local_e0[j]]->Play();
        iVar22--;
      }
    }
    mDoUpdate = false;
  }
  for (int i = 0; i < chan.mSounds.size(); ++i) {
    if (!chan.mSounds[i] || chan.mSounds[i]->IsPlaying() || chan.mSounds[i]->IsLooped()) {
      continue;
    }
    chan.mSounds[i]->Release();
    chan.mSounds[i] = nullptr;

    mDoUpdate = true;
  }

  if (mAuxProcessingEnabled && mReverbAmount < 1.f) {
    mReverbAmount = rstl::min_val(1.f, mReverbAmount + (dt / (mReverbScale)));
    ApplyReverb();
  } else if (!mAuxProcessingEnabled && mReverbAmount > 0.f) {
    mReverbAmount = rstl::max_val(0.f, mReverbAmount - (dt / (mReverbScale * 2.f)));
    ApplyReverb();

    if (mReverbAmount == 0.f) {
      DisableAuxCallbacks();
      EnableAuxCallbacks();
    }
  }

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
  CBaseSfxWrapper* wrapper = mChannels[mCurrentChannel].mSounds[handle.GetIndex()];
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
  if (handle.GetIndex() < 0 || handle.GetIndex() >= chan.mSounds.size()) {
    return false;
  }
  CBaseSfxWrapper* wrapper = chan.mSounds[handle.GetIndex()];
  if (wrapper == nullptr || handle != wrapper->GetSfxHandle() || !wrapper->IsPlaying()) {
    return false;
  }
  return wrapper->IsPlaying();
}

bool CSfxManager::IsHandleValid(CSfxHandle handle) {
  CSfxChannel& chan = mChannels[mCurrentChannel];
  if (handle.GetIndex() < 0 || handle.GetIndex() >= chan.mSounds.size()) {
    return false;
  }

  CBaseSfxWrapper* wrapper = chan.mSounds[handle.GetIndex()];
  if (wrapper == nullptr || handle != wrapper->GetSfxHandle()) {
    return false;
  }
  return true;
}

int CSfxManager::GetRank(CSfxManager::CBaseSfxWrapper* wrapper) {
  CSfxChannel& chan = mChannels[mCurrentChannel];
  if (!wrapper->IsInArea()) {
    return 0;
  }
  int rank = wrapper->GetPriority() >> 2;

  if (wrapper->IsPlaying()) {
    rank += 1;
  }
  if (wrapper->IsLooped()) {
    rank -= 2;
  }
  if (wrapper->Ready() && !wrapper->IsPlaying()) {
    rank += 3;
  }
  if (chan.x44_) {
    const int tmp = wrapper->GetAudible(chan.mListener.x0_);
    if (tmp == 0) {
      rank = 0;
    } else {
      rank += tmp * 2;
    }
  }

  return rank;
}

bool CSfxManager::LoadTranslationTable(CSimplePool* pool, const SObjectTag* tag) {
  if (!tag) {
    return false;
  }
  if (mTranslationTable) {
    delete mTranslationTable;
  }
  mTranslationTable = nullptr;
  mTranslationTableTok = rs_new CToken(pool->GetObj(*tag));
  mTranslationTableTok->Lock();

  return true;
}

ushort CSfxManager::TranslateSFXID(ushort id) {
  if (mTranslationTable == nullptr || id >= mTranslationTable->size()) {
    return -1;
  }

  short ret = (*mTranslationTable)[id];
  if (ret < 0) {
    return -1;
  }
  return CCast::ToUint16(ret);
}

void CSfxManager::SetActiveAreas(const rstl::reserved_vector< int, 10 >& areas) {
  CSfxChannel& chan = mChannels[mCurrentChannel];
  for (short i = 0; i < chan.mSounds.size(); ++i) {
    if (chan.mSounds[i] != nullptr) {
      int area = chan.mSounds[i]->GetArea();
      if (area == kAllAreas) {
        chan.mSounds[i]->SetInArea(true);

        continue;
      }

      bool inArea = false;
      for (rstl::reserved_vector< int, 10 >::const_iterator areaIt = areas.begin();
           areaIt != areas.end(); ++areaIt) {
        if ((*areaIt) == area) {
          inArea = true;
        }
      }

      mDoUpdate = true;
      chan.mSounds[i]->SetInArea(inArea);
    }
  }
}

CSfxManager::CSfxEmitterWrapper*
CSfxManager::AllocateCSfxEmitterWrapper(const CSfxEmitterWrapper& wrapper) {
  CSfxEmitterWrapper* ret = nullptr;

  for (int i = 0; i < mEmitterWrapperPool.size(); ++i) {
    if (mEmitterWrapperPool[i].Available()) {
      mEmitterWrapperPool[i] = wrapper;
      ret = &mEmitterWrapperPool[i];

      break;
    }
  }

  if (ret == nullptr && mEmitterWrapperPool.size() != mEmitterWrapperPool.capacity()) {
    mEmitterWrapperPool.push_back(wrapper);
    ret = &mEmitterWrapperPool.back();
  }
  return ret;
}
CSfxManager::CSfxWrapper* CSfxManager::AllocateCSfxWrapper(const CSfxWrapper& wrapper) {
  CSfxWrapper* ret = nullptr;

  for (int i = 0; i < mWrapperPool.size(); ++i) {
    if (mWrapperPool[i].Available()) {
      mWrapperPool[i] = wrapper;
      ret = &mWrapperPool[i];

      break;
    }
  }
  if (ret == nullptr && mWrapperPool.size() != mWrapperPool.capacity()) {
    mWrapperPool.push_back(wrapper);
    ret = &mWrapperPool.back();
  }
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
}

void CSfxManager::ApplyReverb() {
  CSfxChannel& chan = mChannels[mCurrentChannel];

  const short reverb = GetReverbAmount();
  for (int i = 0; i < chan.mSounds.size(); ++i) {
    if (chan.mSounds[i]) {
      chan.mSounds[i]->SetReverb(reverb);
    }
  }
}
void CSfxManager::PrepareReverbHiCallback(const SND_AUX_REVERBHI& info) {
  DisableAuxProcessing();
  mRequestedAuxEffect = kAE_ReverbHi;
  sPendingReverbHi = info;
  if (mReverbAmount == 0.f) {
    EnableAuxCallbacks();
  }
}
void CSfxManager::PrepareChorusCallback(const SND_AUX_CHORUS& info) {
  DisableAuxProcessing();
  mRequestedAuxEffect = kAE_Chorus;
  sPendingChorus = info;
  if (mReverbAmount == 0.f) {
    EnableAuxCallbacks();
  }
}
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
  }
}

void CSfxManager::EnableAuxCallbacks() {
  mReverbAmount = 0.f;
  SND_AUX_CALLBACK cb = nullptr;
  void* userA = nullptr;
  ApplyReverb();
  if (mCurrentAuxEffect != kAE_None) {
    DisableAuxCallbacks();
  }

  mCurrentAuxEffect = mRequestedAuxEffect;
  if (mCurrentAuxEffect == kAE_None) {
    return;
  }

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

    break;
  }
  sndSetAuxProcessingCallbacks(0, cb, userA, 0xFF, 0, nullptr, nullptr, 0xFF, 0);
  mAuxProcessingEnabled = true;
}
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
  }
  mCurrentAuxEffect = kAE_None;
}

const CFactoryFnReturn FAudioTranslationTableFactory(const SObjectTag& obj, CInputStream& in,
                                                     const CVParamTransfer& xfer) {
  return rs_new rstl::vector< short >(in);
}
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
}

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
}

CSfxHandle CSfxManager::AddEmitter(ushort id, const CVector3f& position, int area,
                                   bool useAcoustics, bool looped, short priority) {
  CAudioSys::C3DEmitterParmData params(150.f, 0.1f, 1, 127, 20);
  params.mPos = position;
  params.mDir = CVector3f::Zero();
  params.mSfxId = id;
  return AddEmitter(params, area, useAcoustics, looped, priority);
}

CSfxHandle CSfxManager::AddEmitter(ushort id, const CVector3f& position, uchar volume, int area,
                                   bool useAcoustics, bool looped, short priority) {
  CAudioSys::C3DEmitterParmData params(150.f, 0.1f, 1, volume > 20 ? volume : 21, 20);
  params.mPos = position;
  params.mDir = CVector3f::Zero();
  params.mSfxId = id;
  return AddEmitter(params, area, useAcoustics, looped, priority);
}

CSfxHandle CSfxManager::AddEmitter(CAudioSys::C3DEmitterParmData& params, int area,
                                   bool useAcoustics, bool looped, short priority) {
  if ((mMuted && !looped) || params.mSfxId == 0xffffffff ||
      params.mSfxId == kInternalInvalidSfxId) {
    return CSfxHandle::NullHandle();
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
    return CSfxHandle::NullHandle();
  }

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
  }
  mDoUpdate = true;
  CSfxChannel& channel = mChannels[mCurrentChannel];
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
}

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
}

void CSfxManager::SfxPan(CSfxHandle handle, uchar pan) {
  if (!handle) {
    return;
  }
  CSfxChannel& channel = mChannels[mCurrentChannel];
  const int index = handle.GetIndex();
  if (index < 0 || index >= channel.mSounds.size()) {
    return;
  }
  CBaseSfxWrapper* sound = channel.mSounds[index];
  if (sound == nullptr || handle != sound->GetSfxHandle()) {
    return;
  }
  if (!sound->IsPlaying()) {
    Update(0.f);
  }
  if (sound->IsPlaying()) {
    CAudioSys::SfxPan(sound->GetVoice(), pan);
  }
}

void CSfxManager::SfxSpan(CSfxHandle handle, uchar span) {
  if (!handle) {
    return;
  }
  CSfxChannel& channel = mChannels[mCurrentChannel];
  const int index = handle.GetIndex();
  if (index < 0 || index >= channel.mSounds.size()) {
    return;
  }
  CBaseSfxWrapper* sound = channel.mSounds[index];
  if (sound == nullptr || handle != sound->GetSfxHandle()) {
    return;
  }
  if (!sound->IsPlaying()) {
    Update(0.f);
  }
  if (sound->IsPlaying()) {
    CAudioSys::SfxSpan(sound->GetVoice(), span);
  }
}

void CSfxManager::KillAll(ESfxChannels channel) {
  CSfxChannel& sounds = mChannels[channel];
  for (int i = 0; i < sounds.mSounds.size(); ++i) {
    CBaseSfxWrapper* sound = sounds.mSounds[i];
    if (sound != nullptr && sound->IsPlaying()) {
      sound->Stop();
    }
    if (sound != nullptr) {
      sound->Release();
    }
    sounds.mSounds[i] = nullptr;
  }
  if (channel == kSC_Game) {
    mAuxEffectManager.Shutdown();
    mAuxEffectManager.Initialize();
  }
}

void CSfxManager::StopSound(ESfxChannels channel, CSfxHandle handle) {
  CSfxChannel& sounds = mChannels[channel];
  const int index = handle.GetIndex();
  if (index < 0 || index >= sounds.mSounds.size()) {
    if (channel != kSC_Game) {
      StopSound(kSC_Game, handle);
    }

    return;
  }
  CBaseSfxWrapper* sound = sounds.mSounds[index];
  if (sound == nullptr || handle != sound->GetSfxHandle()) {
    if (channel != kSC_Game) {
      StopSound(kSC_Game, handle);
    }

    return;
  }
  mDoUpdate = true;
  if (sound->IsPlaying()) {
    sound->Stop();
  }
  sound->Release();
  sounds.mSounds[handle.GetIndex()] = nullptr;
}

void CSfxManager::SetDuration(CSfxHandle handle, float duration) {
  CSfxChannel& channel = mChannels[mCurrentChannel];
  const int index = handle.GetIndex();
  if (index < 0 || index >= channel.mSounds.size()) {
    return;
  }
  CBaseSfxWrapper* sound = channel.mSounds[index];
  if (sound == nullptr || handle != sound->GetSfxHandle()) {
    return;
  }
  sound->SetTimeRemaining(duration);
}

void CSfxManager::SetChannel(ESfxChannels channel) {
  ESfxChannels current = mCurrentChannel;
  if (channel == current) {
    return;
  }
  if (channel == kSC_Default) {
    mAreaLowPassFilters.clear();
    mLowPassFilters.clear();
  }
  if (current != kSC_Invalid) {
    TurnOffChannel(current);
  }
  TurnOnChannel(channel);
  mCurrentChannel = channel;
}

CSfxManager::ESfxChannels CSfxManager::GetChannel() { return mCurrentChannel; }

void CSfxManager::TurnOffChannel(ESfxChannels channel) {
  CSfxChannel& sounds = mChannels[channel];
  for (int i = 0; i < sounds.mSounds.size(); ++i) {
    if (sounds.mSounds[i] != nullptr) {
      if (sounds.mSounds[i]->IsLooped()) {
        sounds.mSounds[i]->UpdateEmitterSilent();
      } else {
        sounds.mSounds[i]->Stop();
      }
    }
  }
  for (int i = 0; i < sounds.mSounds.size(); ++i) {
    if (sounds.mSounds[i] != nullptr && !sounds.mSounds[i]->IsLooped()) {
      sounds.mSounds[i]->Release();
      sounds.mSounds[i] = nullptr;
    }
  }
}

void CSfxManager::TurnOnChannel(ESfxChannels channel) {
  mCurrentChannel = channel;
  CSfxChannel& sounds = mChannels[channel];

  mDoUpdate = true;
  bool hasListener = false;
  for (int i = 0; i < 4; ++i) {
    if (sounds.mListeners[i].mActive) {
      hasListener = true;
      break;
    }
  }
  if (hasListener) {
    for (int j = 0; j < sounds.mSounds.size(); ++j) {
      if (sounds.mSounds[j] != nullptr) {
        sounds.mSounds[j]->UpdateEmitter();
      }
    }
  }
}

CSfxHandle CSfxManager::LocateHandle(int) {
  CSfxChannel& channel = mChannels[mCurrentChannel];
  for (int i = 0; i < channel.mSounds.size(); ++i) {
    if (channel.mSounds[i] == nullptr) {
      return CSfxHandle(i);
    }
  }
  if (channel.mSounds.size() == channel.mSounds.capacity()) {
    return CSfxHandle::NullHandle();
  }
  channel.mSounds.push_back(nullptr);
  return CSfxHandle(channel.mSounds.size() - 1);
}

void CSfxManager::Update(float dt) {
  short i;
  ushort count = 0;
  CSfxChannel& channel = mChannels[mCurrentChannel];
  for (i = 0; i < channel.mSounds.size(); ++i) {
    if (channel.mSounds[i] == nullptr || channel.mSounds[i]->IsLooped()) {
      continue;
    }
    const float remaining = channel.mSounds[i]->GetTimeRemaining();
    channel.mSounds[i]->SetTimeRemaining(remaining - dt);
    if (remaining < 0.f) {
      channel.mSounds[i]->Stop();

      mDoUpdate = true;
    }
  }
  ushort ranked[72];

  if (mDoUpdate) {
    for (i = 0; i < channel.mSounds.size(); ++i) {
      if (channel.mSounds[i] != nullptr) {
        ranked[count++] = i;
        channel.mSounds[i]->SetRank(GetRank(channel.mSounds[i]));
      }
    }
    for (i = 0; i < count; ++i) {
      bool done = true;
      for (int j = 0; j < count - 1; ++j) {
        if (channel.mSounds[ranked[j]]->GetRank() < channel.mSounds[ranked[j + 1]]->GetRank()) {
          done = false;
          rstl::swap(ranked[j], ranked[j + 1]);
        }
      }
      if (done) {
        break;
      }
    }
    for (i = 48; i < count; ++i) {
      if (channel.mSounds[ranked[i]] != nullptr && channel.mSounds[ranked[i]]->IsPlaying()) {
        channel.mSounds[ranked[i]]->Stop();
      }
    }
    for (i = 0; i < count; ++i) {
      if (channel.mSounds[ranked[i]] != nullptr && channel.mSounds[ranked[i]]->IsPlaying() &&
          !channel.mSounds[ranked[i]]->IsInArea()) {
        channel.mSounds[ranked[i]]->Stop();
      }
    }
  }

  CAudioSys::S3dFlushUnusedEmitters();
  if (mDoUpdate && !mMuted) {
    for (int available = 48, j = 0; j < count && available != 0; ++j) {
      if (channel.mSounds[ranked[j]] == nullptr) {
        continue;
      }
      if (channel.mSounds[ranked[j]]->IsPlaying()) {
        --available;
      } else if (channel.mSounds[ranked[j]]->Ready() && channel.mSounds[ranked[j]]->IsInArea()) {
        channel.mSounds[ranked[j]]->Play();
        --available;
      }
    }
    mDoUpdate = false;
  }
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
        continue;
      }
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

  mDoUpdate = true;
}

bool CSfxManager::IsPlaying(CSfxHandle handle) {
  if (!handle) {
    return false;
  }
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
}

bool CSfxManager::IsQueued(CSfxHandle handle) {
  if (!handle) {
    return false;
  }
  const int index = handle.GetIndex();
  CSfxChannel& channel = mChannels[mCurrentChannel];
  if (index < 0 || index >= channel.mSounds.size()) {
    return false;
  }
  CBaseSfxWrapper* sound = channel.mSounds[index];
  if (sound == nullptr || handle != sound->GetSfxHandle()) {
    return false;
  }
  return true;
}

int CSfxManager::GetRank(CBaseSfxWrapper* sound) {
  const CSfxChannel& channel = mChannels[mCurrentChannel];
  if (!sound->IsInArea()) {
    return 0;
  }
  int rank = sound->GetPriority() >> 2;
  if (sound->IsPlaying()) {
    ++rank;
  }
  if (sound->IsLooped()) {
    rank -= 2;
  }
  if (sound->Ready() && !sound->IsPlaying()) {
    rank += 3;
  }
  for (int i = 0; i < 4; ++i) {
    if (channel.mListeners[i].mActive) {
      const int audible = sound->GetAudible(channel.mListeners[i].mListener.mPosition);
      if (audible == kSA_Inaudible) {
        rank = 0;
      } else {
        rank += audible * 2;
      }
    }
  }

  return rank;
}

bool CSfxManager::LoadTranslationTable(CSimplePool* pool, const SObjectTag* tag) {
  if (tag == nullptr) {
    return false;
  }
  if (mpTranslationTable != nullptr) {
    delete mpTranslationTable;
  }
  mpTranslationTable = nullptr;
  mpTranslationTableToken = rs_new CToken(pool->GetObj(*tag));
  mpTranslationTableToken->Lock();

  return true;
}

ushort CSfxManager::TranslateSFXID(ushort id) {
  if (mpTranslationTable == nullptr || id >= mpTranslationTable->size()) {
    return kInternalInvalidSfxId;
  }
  short translated = (*mpTranslationTable)[id];
  if (translated < 0) {
    return kInternalInvalidSfxId;
  }
  return translated;
}

void CSfxManager::SetActiveAreas(const rstl::reserved_vector< int, 10 >& areas, int currentArea) {
  mCurrentArea = currentArea;
  CSfxChannel& channel = mChannels[mCurrentChannel];
  for (CAuxEffect* effect = mAuxEffects.begin(); effect != mAuxEffects.end(); ++effect) {
    if (!effect->IsRegistered() || !effect->IsActive() || effect->GetArea() == kAllAreas) {
      continue;
    }
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
      bool inArea = false;
      for (const int* area = areas.begin(); area != areas.end(); ++area) {
        if (*area == soundArea) {
          inArea = true;
        }
      }

      mDoUpdate = true;
      const bool active = inArea;
      channel.mSounds[i]->SetInArea(active);
    }
  }
}

CSfxManager::CSfxEmitterWrapper*
CSfxManager::AllocateCSfxEmitterWrapper(const CSfxEmitterWrapper& sound) {
  CSfxEmitterWrapper* result = nullptr;

  for (int i = 0; i < mEmitterWrapperPool.size(); ++i) {
    if (mEmitterWrapperPool[i].Available()) {
      mEmitterWrapperPool[i] = sound;
      result = &mEmitterWrapperPool[i];

      break;
    }
  }
  if (result == nullptr && mEmitterWrapperPool.size() != mEmitterWrapperPool.capacity()) {
    mEmitterWrapperPool.push_back(sound);
    result = &mEmitterWrapperPool.back();
  }
  return result;
}

CSfxManager::CSfxWrapper* CSfxManager::AllocateCSfxWrapper(const CSfxWrapper& sound) {
  CSfxWrapper* result = nullptr;

  for (int i = 0; i < mWrapperPool.size(); ++i) {
    if (mWrapperPool[i].Available()) {
      mWrapperPool[i] = sound;
      result = &mWrapperPool[i];

      break;
    }
  }
  if (result == nullptr && mWrapperPool.size() != mWrapperPool.capacity()) {
    mWrapperPool.push_back(sound);
    result = &mWrapperPool.back();
  }
  return result;
}

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
        }
      }
    }
    for (int i = 0; i < channel.mSounds.size(); ++i) {
      if (channel.mSounds[i] != nullptr && !channel.mSounds[i]->IsLooped()) {
        channel.mSounds[i]->Release();
        channel.mSounds[i] = nullptr;
      }
    }
  } else {
    for (int i = 0; i < channel.mSounds.size(); ++i) {
      if (channel.mSounds[i] != nullptr) {
        channel.mSounds[i]->UpdateEmitter();
      }
    }
  }
}

short CSfxManager::GetReverbAmount() { return 127; }

int CSfxManager::AddAuxEffect(int area, const SND_AUX_REVERBHI& params, uchar volume,
                              int priority) {
  return RegisterAuxEffect(CAuxEffect(params, area, volume, priority));
}

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
    return;
  }
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

      break;
    }
  }
  if (it != mAreaVolumes.end()) {
    return;
  }
  for (it = mAreaVolumes.begin(); it != mAreaVolumes.end(); ++it) {
    if (it->first == kAllAreas) {
      it->first = area;
      it->second = volume;
      return;
    }
  }
}

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
  return rs_new rstl::vector< short >(in);
}

CSfxManager::ESfxWrapperType CSfxManager::CSfxWrapper::GetType() const { return kSWT_Sound; }
#endif
