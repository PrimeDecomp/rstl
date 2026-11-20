#include "Kyoto/Audio/CAudioSys.hpp"
#include "rstl/macros.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Audio/CAudioGroupSet.hpp"
#include "rstl/RstlVersions.h"
#include "rstl/math.hpp"

#include "dolphin/ai.h"
#include "dolphin/ar.h"
#include "dolphin/dtk.h"
#include "dolphin/os.h"
#include "dolphin/types.h"
#include "musyx/musyx.h"

#include "string.h"
#if defined(TARGET_PC)
#include "Metaforce/Audio.hpp"
#endif

extern "C" bool sndFXSetFilter(SND_VOICEID vid, u32 filter, u32 param);

#if !RSTL_KYOTO_MOVIE_OWNS_VOLUME_TABLE
#include "CAudioVolumeTable.inc"
#endif
bool CAudioSys::mInitialized = false;
bool CAudioSys::mIsListenerActive = false;
bool CAudioSys::mVerbose = false;

uchar CAudioSys::mMaxNumEmitters = 0;
rstl::map< rstl::string, rstl::ncrc_ptr< CAudioGroupSet > >* CAudioSys::mpGroupSetDB = nullptr;
rstl::map< uint, rstl::string >* CAudioSys::mpGroupSetResNameDB = nullptr;

rstl::map< rstl::string, rstl::ncrc_ptr< CAudioSys::CTrkData > >* CAudioSys::mpDVDTrackDB = nullptr;
rstl::vector< CAudioSys::CEmitterData >* CAudioSys::mpEmitterDB = nullptr;
SND_LISTENER* CAudioSys::mpListener = nullptr;
uint CAudioSys::mUnusedEmitterHandle = 0;
CAudioSys::ESurroundModes CAudioSys::mSurroundMode = CAudioSys::kSM_Mono;
#if !defined(TARGET_PC)
int CAudioSys::mMaxAramUsage = 0;
int CAudioSys::mCurrentAramUsage = 0;
#endif

const uchar CAudioSys::kMaxVolume = 0x7f;
const uchar CAudioSys::kEmitterMedPriority = 0x7f;
bool CAudioSys::mProLogic2 = true;

short CAudioSys::mVolumeScale = 0x7f;
short CAudioSys::mDefaultVolumeScale = 0x7f;

bool CAudioSys::mAICallbackEnabled = true;
void* CAudioSys::mAICallback = nullptr;
void* CAudioSys::mpSampleDataUploadBuffer = nullptr;

const rstl::string CAudioSys::mpDefaultInvalidString(rstl::string_l("NULL"));

#if RSTL_PRIME1
static const uchar kProLogic2Studios = 1;
#else
static const uchar kProLogic2Studios = 3;
#endif

void* DoMalloc(size_t len) { return rs_new char[len]; }

void DoFree(void* ptr) {
  if (!ptr) {
    return;
  }
  delete[] reinterpret_cast< char* >(ptr);
}

CAudioSys::CAudioSys(const uchar numVoices, const uchar numMusic, const uchar numSfx,
                     const uchar maxNumEmitters, const uint aramSize) {
  mInitialized = true;
  SND_HOOKS hooks = {DoMalloc, DoFree};
  AIInit(nullptr);

  sndSetHooks(&hooks);
#if defined(TARGET_PC)
  metaforce::InitializeAudio(numVoices, numMusic, numSfx, 0);//mProLogic2 ? 1 : 0);
#else
  if (mProLogic2) {
    sndInit(numVoices, numMusic, numSfx, kProLogic2Studios, 1, aramSize);
  } else {
    sndInit(numVoices, numMusic, numSfx, kProLogic2Studios, 0, aramSize);
  }
#endif
#if !RSTL_PRIME1
  for (int i = 1; i < 3; ++i) {
    if (mProLogic2) {
      sndActivateStudioEx(i, false, SND_STUDIO_TYPE_DPL2);
    } else {
      sndActivateStudioEx(i, false, SND_STUDIO_TYPE_STD);
    }
  }

  SND_STUDIO_INPUT input1 = {0, 0x7f, 0, 1};
  sndAddStudioInput(0, &input1);
  SND_STUDIO_INPUT input2 = {0, 0x7f, 0, 2};
  sndAddStudioInput(0, &input2);
#endif

  DTKInit();

#if RSTL_PRIME1
  mpGroupSetDB = rs_new rstl::map< rstl::string, rstl::ncrc_ptr< CAudioGroupSet > >();
  mpGroupSetResNameDB = rs_new rstl::map< uint, rstl::string >();
#endif

  mpDVDTrackDB = rs_new rstl::map< rstl::string, rstl::ncrc_ptr< CTrkData > >();
#if !RSTL_PRIME1 || RSTL_VERSION >= RSTL_GM8E_02
  mpEmitterDB = rs_new rstl::vector< CEmitterData >(maxNumEmitters, CEmitterData(),
                                                    rstl::rmemory_allocator());
#else
  mpEmitterDB = rs_new rstl::vector< CEmitterData >(maxNumEmitters, CEmitterData());
#endif

  mpListener = rs_new SND_LISTENER;
  mIsListenerActive = false;
  mMaxNumEmitters = maxNumEmitters;

  if (OSGetSoundMode() == OS_SOUND_MODE_MONO) {
    sndOutputMode(SND_OUTPUTMODE_MONO);
    mSurroundMode = kSM_Mono;
  } else {
    sndOutputMode(SND_OUTPUTMODE_SURROUND);
    mSurroundMode = kSM_Surround;
  }

#if !defined(TARGET_PC)
  mMaxAramUsage = aramSize;
#endif
}

CAudioSys::~CAudioSys() {
  S3dFlushAllEmitters();
  S3dRemoveListener();
  sndQuit();
#if RSTL_PRIME1
  delete mpGroupSetDB;
  mpGroupSetDB = nullptr;
  delete mpGroupSetResNameDB;
  mpGroupSetResNameDB = nullptr;
#endif

  delete mpDVDTrackDB;
  mpDVDTrackDB = nullptr;

  delete mpEmitterDB;
  mpEmitterDB = nullptr;
  delete mpListener;
  mpListener = nullptr;
  mInitialized = false;
}

void CAudioSys::SysSetVolume(const uchar volume, const ushort time, const uchar group) {
  sndVolume(volume, time, group);
}

void CAudioSys::SysSetSfxVolume(const uchar volume, const ushort time, const uchar music,
                                const uchar fx) {
  sndMasterVolume(volume, time, music, fx);
}

#if RSTL_PRIME1
bool CAudioSys::SysLoadGroupSet(CSimplePool* pool, const uint id) {
  const rstl::string& name = SysGetGroupSetName(id);
  rstl::rc_ptr< CAudioGroupSet > existing = FindGroupSet(name);
  if (!existing) {
    TLockedToken< CAudioGrpSetLoc > token(pool->GetObj(SObjectTag('AGSC', id)));
    rstl::ncrc_ptr< CAudioGroupSet > group(rs_new CAudioGroupSet(token));
#if !defined(TARGET_PC)
    int aramUsage = mCurrentAramUsage + group->AramUsage();
    if (aramUsage > mMaxAramUsage) {
      return true;
    }
    mCurrentAramUsage = aramUsage;
#endif
    const rstl::string& groupName = group->GetName();
    mpGroupSetDB->insert(
        rstl::pair< rstl::string, rstl::ncrc_ptr< CAudioGroupSet > >(groupName, group));
    mpGroupSetResNameDB->insert(rstl::pair< uint, rstl::string >(id, groupName));
    return false;
  }
  existing->Reload();
  return true;
}

bool CAudioSys::SysLoadGroupSet(const CToken& token, const rstl::string& name, const uint id) {
  rstl::rc_ptr< CAudioGroupSet > existing = FindGroupSet(name);
  if (!existing) {
    rstl::ncrc_ptr< CAudioGroupSet > group(
        rs_new CAudioGroupSet(TLockedToken< CAudioGrpSetLoc >(token)));
#if !defined(TARGET_PC)
    int aramUsage = mCurrentAramUsage + group->AramUsage();
    if (aramUsage > mMaxAramUsage) {
      return true;
    }
    mCurrentAramUsage = aramUsage;
#endif
    const rstl::string& groupName = group->GetName();
    mpGroupSetDB->insert(
        rstl::pair< rstl::string, rstl::ncrc_ptr< CAudioGroupSet > >(groupName, group));
    mpGroupSetResNameDB->insert(rstl::pair< uint, rstl::string >(id, groupName));
    return false;
  }
  existing->Reload();
  return true;
}

bool CAudioSys::SysIsGroupSetLoaded(const rstl::string& name) { return FindGroupSet(name); }

bool CAudioSys::SysUnloadSampleData(const rstl::string& name) {
  rstl::ncrc_ptr< CAudioGroupSet > groupSet = FindGroupSet(name);
  CAudioGroupSet* ptr = groupSet.GetPtr();
  if (ptr) {
    ptr->FreeSampleBuffer();
    return true;
  }
  return false;
}

bool CAudioSys::SysUnloadGroupSet(const rstl::string& name) {
  rstl::rc_ptr< CAudioGroupSet > group = FindGroupSet(name);
  if (group) {
    AUTO(it, mpGroupSetResNameDB->begin());
    while (it != mpGroupSetResNameDB->end()) {
      if (it->second == name) {
        it = mpGroupSetResNameDB->erase(it);
      } else {
        ++it;
      }
    }
#if !defined(TARGET_PC)
    mCurrentAramUsage -= group->AramUsage();
#endif
    mpGroupSetDB->erase(name);
    return true;
  }
  return false;
}

void* CAudioSys::SampleDataUploadCallback(u32 address, u32 bytes) {
  volatile bool busy = true;
  while (busy) {
    busy = ARGetDMAStatus() != 0;
  }
  memcpy(mpSampleDataUploadBuffer, reinterpret_cast< void* >(address), bytes);
  DCFlushRange(mpSampleDataUploadBuffer, bytes);
  return mpSampleDataUploadBuffer;
}

bool CAudioSys::SysPushGroupIntoARAM(const rstl::string& name, const uchar groupId) {
  rstl::ncrc_ptr< CAudioGroupSet > groupSet = FindGroupSet(name);
  CAudioGroupSet* group = groupSet.GetPtr();
  if (group) {
    void* project = group->GetProjBuffer();
    void* samples = group->GetSampleBuffer();
    void* sampleDir = group->GetSDirBuffer();
    void* pool = group->GetPoolBuffer();
#if defined(TARGET_PC)
    return sndPushGroup(project, groupId, samples, sampleDir, pool);
#else
    uchar buffer[0x1020];
    mpSampleDataUploadBuffer =
        reinterpret_cast< void* >((reinterpret_cast< uintptr_t >(buffer) + 31) & ~31);
    sndSetSampleDataUploadCallback(SampleDataUploadCallback, 0x1000);
    const bool result = sndPushGroup(project, groupId, samples, sampleDir, pool);
    sndSetSampleDataUploadCallback(nullptr, 0);
    return result;
#endif
  }
  return false;
}

void CAudioSys::SysPopGroupFromARAM() { sndPopGroup(); }

const rstl::string& CAudioSys::SysGetGroupSetName(const uint id) {
  rstl::map< uint, rstl::string >::const_iterator it = mpGroupSetResNameDB->find(id);
  rstl::map< uint, rstl::string >::const_iterator end = mpGroupSetResNameDB->end();
  if (it != end) {
    return it->second;
  }
  return mpDefaultInvalidString;
}

rstl::ncrc_ptr< CAudioGroupSet > CAudioSys::FindGroupSet(const rstl::string& name) {
  if (mpGroupSetDB->size() <= 0) {
    return rstl::ncrc_ptr< CAudioGroupSet >();
  }
  rstl::map< rstl::string, rstl::ncrc_ptr< CAudioGroupSet > >::const_iterator it(
      mpGroupSetDB->find(name));
  rstl::map< rstl::string, rstl::ncrc_ptr< CAudioGroupSet > >::const_iterator end(
      mpGroupSetDB->end());
  if (it != end) {
    return it->second;
  }
  return rstl::ncrc_ptr< CAudioGroupSet >();
}
#endif

SND_VOICEID CAudioSys::SfxStart(const SND_FXID sfxId, const uchar vol, const uchar pan,
                                const uchar prio) {
  const uchar scaledVol = (mVolumeScale * (vol > 0x7f ? 0x7f : vol)) / 0x7f;
  return sndFXStartEx(sfxId, scaledVol, pan, prio);
}

void CAudioSys::SfxStop(const SND_VOICEID handle) { sndFXKeyOff(handle); }

SND_VOICEID CAudioSys::SfxCheck(const SND_VOICEID handle) { return sndFXCheck(handle); }

void CAudioSys::SfxPan(const SND_VOICEID handle, const uchar pan) { sndFXPanning(handle, pan); }

void CAudioSys::SfxSpan(const SND_VOICEID handle, const uchar span) {
  sndFXSurroundPanning(handle, span);
}

void CAudioSys::SfxVolume(const SND_VOICEID handle, const uchar vol) { sndFXVolume(handle, vol); }

void CAudioSys::SfxPitchBend(const SND_VOICEID handle, const ushort pitch) {
  sndFXPitchBend(handle, pitch);
}

void CAudioSys::SfxCtrl(const SND_VOICEID handle, uchar ctrl, uchar val) {
  sndFXCtrl(handle, ctrl, val);
}

#if !RSTL_PRIME1
bool CAudioSys::SfxSetFilter(const SND_VOICEID handle, const uint filter, const uint param) {
  sndFXSetFilter(handle, filter, param);
  return true;
}
#endif

int CAudioSys::TrkQueueTrack(const rstl::string& name, void (*callback)(u32),
                             const uint eventMask) {
  while (TrkGetState() == kTPS_Busy) {
  }

  rstl::ncrc_ptr< CTrkData > trk = FindTrack(name);
  if (!trk.GetPtr()) {
    CTrkData* data = rs_new CTrkData(name);
    rstl::ncrc_ptr< CTrkData > newTrk(data);
    mpDVDTrackDB->insert(rstl::pair< rstl::string, rstl::ncrc_ptr< CTrkData > >(name, newTrk));
    return DTKQueueTrack(newTrk->GetFileName(), newTrk->GetTrack(), eventMask, callback);
  }

  if (!trk->GetIsTrackInUse()) {
    trk->SetIsTrackInUse(true);
    return DTKQueueTrack(trk->GetFileName(), trk->GetTrack(), eventMask, callback);
  }
  return 5000;
}

void CAudioSys::TrkRemoveTrack(const rstl::string& name) {
#if RSTL_PRIME1
  mpDVDTrackDB->erase(name);
#else
  rstl::map< rstl::string, rstl::ncrc_ptr< CTrkData > >::iterator it = mpDVDTrackDB->find(name);
  if (it != mpDVDTrackDB->end()) {
    mpDVDTrackDB->erase(it);
  }
#endif
}

#if RSTL_PRIME1
static void track_flusher_callback() {}
#endif

void CAudioSys::TrkFlushTracks() {
  if (mpDVDTrackDB->size() > 0) {
    DTKFlushTracks(nullptr);
    mpDVDTrackDB->clear();
  }
}

void CAudioSys::TrkSetSampleRate(const ETRKSampleRate rate) { DTKSetSampleRate(rate); }

void CAudioSys::TrkSetRepeatMode(const ETRKRepeatMode mode) { DTKSetRepeatMode(mode); }

void CAudioSys::TrkSetState(const ETRKPlayState state) { DTKSetState(state); }

ETRKPlayState CAudioSys::TrkGetState() { return ETRKPlayState(DTKGetState()); }

void CAudioSys::TrkSetVolume(const uchar left, const uchar right) { DTKSetVolume(left, right); }

void CAudioSys::TrkNextTrack() { DTKNextTrack(); }

rstl::ncrc_ptr< CAudioSys::CTrkData > CAudioSys::FindTrack(const rstl::string& name) {
  if (mpDVDTrackDB->size() <= 0) {
    return rstl::ncrc_ptr< CTrkData >();
  }
  rstl::map< rstl::string, rstl::ncrc_ptr< CTrkData > >::const_iterator it(
      mpDVDTrackDB->find(name));
  rstl::map< rstl::string, rstl::ncrc_ptr< CTrkData > >::const_iterator end(mpDVDTrackDB->end());
  if (it != end) {
    return it->second;
  }
  return rstl::ncrc_ptr< CTrkData >();
}

void CAudioSys::S3dAddListener(const CVector3f& pos, const CVector3f& dir, const CVector3f& heading,
                               const CVector3f& up, const float frontSur, const float backSur,
                               const float soundSpeed, const uint flags, const uchar volume) {
  if (mIsListenerActive) {
    S3dRemoveListener();
  }

  SND_FVECTOR _pos;
  _pos.x = pos.GetX();
  _pos.y = pos.GetY();
  _pos.z = pos.GetZ();
  SND_FVECTOR _dir;
  _dir.x = dir.GetX();
  _dir.y = dir.GetY();
  _dir.z = dir.GetZ();
  SND_FVECTOR _heading;
  _heading.x = heading.GetX();
  _heading.y = heading.GetY();
  _heading.z = heading.GetZ();
  SND_FVECTOR _up;
  _up.x = up.GetX();
  _up.y = up.GetY();
  _up.z = up.GetZ();
  mIsListenerActive = true;
  sndAddListener(mpListener, &_pos, &_dir, &_heading, &_up, frontSur, backSur, soundSpeed, flags,
                 volume, nullptr);
}

bool CAudioSys::S3dUpdateListener(const CVector3f& pos, const CVector3f& dir,
                                  const CVector3f& heading, const CVector3f& up,
                                  const uchar volume) {
  if (mIsListenerActive) {
    SND_FVECTOR _pos;
    _pos.x = pos.GetX();
    _pos.y = pos.GetY();
    _pos.z = pos.GetZ();
    SND_FVECTOR _dir;
    _dir.x = dir.GetX();
    _dir.y = dir.GetY();
    _dir.z = dir.GetZ();
    SND_FVECTOR _heading;
    _heading.x = heading.GetX();
    _heading.y = heading.GetY();
    _heading.z = heading.GetZ();
    SND_FVECTOR _up;
    _up.x = up.GetX();
    _up.y = up.GetY();
    _up.z = up.GetZ();
    return sndUpdateListener(mpListener, &_pos, &_dir, &_heading, &_up, volume, nullptr);
  }
  return false;
}

bool CAudioSys::S3dRemoveListener() {
  if (mIsListenerActive) {
    mIsListenerActive = false;
    return sndRemoveListener(mpListener);
  }
  return false;
}

uint CAudioSys::S3dAddEmitterParaEx(const C3DEmitterParmData& params, ushort groupId,
                                    SND_PARAMETER_INFO* paraInfo) {
  uint handle = mUnusedEmitterHandle;
  if (handle == -1) {
    uint lowerHandle = S3dFindLowerPriorityHandle(params.mPrio);
    if (lowerHandle == -1) {
      return -1;
    }
    handle = lowerHandle;
  }

  CEmitterData& data = (*mpEmitterDB)[handle];

  SND_FVECTOR _pos;
  _pos.x = params.mPos.GetX();
  _pos.y = params.mPos.GetY();
  _pos.z = params.mPos.GetZ();
  SND_FVECTOR _dir;
  _dir.x = params.mDir.GetX();
  _dir.y = params.mDir.GetY();
  _dir.z = params.mDir.GetZ();

#if RSTL_PRIME1
  const uchar scaledMaxVol =
      (mVolumeScale * (params.mMaxVol > 0x7f ? 0x7f : params.mMaxVol)) / 0x7f;
  const char maxVol = scaledMaxVol;
  const uchar minVol = (mVolumeScale * (params.mMinVol > 0x7f ? 0x7f : params.mMinVol)) / 0x7f;

  sndAddEmitterParaEx(&data.mEmitter, &_pos, &_dir, params.mMaxDist, params.mDistComp,
                      params.mFlags, params.mSfxId, groupId, maxVol, minVol, nullptr, paraInfo);
#else
  const char maxVol =
      static_cast< uchar >((mVolumeScale * (params.mMaxVol > 0x7f ? 0x7f : params.mMaxVol)) / 0x7f);
  const char minVol =
      static_cast< uchar >((mVolumeScale * (params.mMinVol > 0x7f ? 0x7f : params.mMinVol)) / 0x7f);
  if (params.mStudio == 0) {
    sndAddEmitterParaEx(&data.mEmitter, &_pos, &_dir, params.mMaxDist, params.mDistComp,
                        params.mFlags, params.mSfxId, groupId, maxVol, minVol, nullptr, paraInfo);
  } else {
    sndAddEmitter2StudioParaEx(&data.mEmitter, &_pos, &_dir, params.mMaxDist, params.mDistComp,
                               params.mFlags, params.mSfxId, groupId, maxVol, minVol,
                               params.mStudio, paraInfo);
  }
#endif

  data.mUsed = true;
  data.mImportant = params.mImportant;
  data.mPrio = params.mPrio;
  mUnusedEmitterHandle = S3dFindUnusedHandle();
  return handle;
}

bool CAudioSys::S3dUpdateEmitter(const uint handle, const CVector3f& pos, const CVector3f& dir,
                                 const uchar maxVol) {
  if (handle == -1) {
    return false;
  }

  CEmitterData& data = (*mpEmitterDB)[handle];

  SND_FVECTOR _pos;
  _pos.x = pos.GetX();
  _pos.y = pos.GetY();
  _pos.z = pos.GetZ();
  SND_FVECTOR _dir;
  _dir.x = dir.GetX();
  _dir.y = dir.GetY();
  _dir.z = dir.GetZ();
  return sndUpdateEmitter(&data.mEmitter, &_pos, &_dir, maxVol, nullptr);
}
bool CAudioSys::S3dRemoveEmitter(const uint handle) {
  if (handle == -1) {
    return false;
  }

  CEmitterData& data = (*mpEmitterDB)[handle];
  if (data.mUsed) {
    data.mUsed = false;
    mUnusedEmitterHandle = handle;
    return sndRemoveEmitter(&data.mEmitter);
  }

  return true;
}

void CAudioSys::S3dFlushAllEmitters() {
  rstl::vector< CEmitterData >::iterator iter = mpEmitterDB->begin();
  for (; iter != mpEmitterDB->end(); ++iter) {
    if (!iter->mUsed) {
      continue;
    }

    iter->mUsed = false;
    sndRemoveEmitter(&iter->mEmitter);
  }
  mUnusedEmitterHandle = 0;
}

void CAudioSys::S3dFlushUnusedEmitters() {
  rstl::vector< CEmitterData >::iterator iter = mpEmitterDB->begin();
  for (; iter != mpEmitterDB->end(); ++iter) {
    if (!iter->mUsed || sndCheckEmitter(&iter->mEmitter) || iter->mImportant) {
      continue;
    }

    iter->mUsed = false;
    sndRemoveEmitter(&iter->mEmitter);
  }
}

bool CAudioSys::S3dCheckEmitter(const uint handle) {
  if (handle == -1) {
    return false;
  }

  CEmitterData& data = (*mpEmitterDB)[handle];
  if (data.mUsed) {
    return sndCheckEmitter(&data.mEmitter);
  }

  return false;
}

uint CAudioSys::S3dEmitterVoiceID(const uint handle) {
  if (handle == -1) {
    return 0;
  }

  CEmitterData& data = (*mpEmitterDB)[handle];
  if (data.mUsed) {
    return sndEmitterVoiceID(&data.mEmitter);
  }

  return -1;
}

uint CAudioSys::S3dFindUnusedHandle() {
  int i = 0;
  do {
    if (!(*mpEmitterDB)[i].mUsed) {
      break;
    }
    ++i;
  } while (i < mMaxNumEmitters);

  if (i < mMaxNumEmitters) {
    return i;
  }

  return -1;
}

uint CAudioSys::S3dFindLowerPriorityHandle(const uint prio) {
  int max = mMaxNumEmitters;
  int i = 0;
  do {
    CEmitterData& data = (*mpEmitterDB)[i];
    if (!data.mUsed) {
      break;
    }

    if (data.mPrio <= prio && !data.mImportant) {
      S3dRemoveEmitter(i);
      break;
    }
    ++i;
  } while (i < max);
  return i < max ? i : -1;
}

SND_SEQID CAudioSys::SeqPlayEx(const ushort gid, const ushort sid, void* arrfile,
                               SND_PLAYPARA* para, const uchar studio) {
  return sndSeqPlayEx(gid, sid, arrfile, para, studio);
}

void CAudioSys::SeqStop(const SND_SEQID seqId) { sndSeqStop(seqId); }

void CAudioSys::SeqVolume(const uchar volume, const ushort time, const SND_SEQID seqId,
                          const uchar mode) {
  sndSeqVolume(volume, time, seqId, mode);
}

void CAudioSys::SetStereoMode(const bool mode) {
  if (mode) {
    sndOutputMode(SND_OUTPUTMODE_STEREO);
    OSSetSoundMode(OS_SOUND_MODE_STEREO);
  } else {
    sndOutputMode(SND_OUTPUTMODE_MONO);
    OSSetSoundMode(OS_SOUND_MODE_MONO);
  }
}

CAudioSys::ESurroundModes CAudioSys::GetSurroundMode() { return mSurroundMode; }

void CAudioSys::SetSurroundMode(const ESurroundModes mode) {
  switch (mode) {
  case kSM_Mono:
    SetStereoMode(false);
    break;
  case kSM_Stereo:
    SetStereoMode(true);
    break;
  case kSM_Surround:
    SetStereoMode(true);
    sndOutputMode(SND_OUTPUTMODE_SURROUND);
    break;
  }

  mSurroundMode = mode;
}
bool CAudioSys::GetVerbose() { return mVerbose; }

void CAudioSys::SetVolumeScale(const short scale) { mVolumeScale = scale; }

void CAudioSys::SetDefaultVolumeScale(const short scale) { mDefaultVolumeScale = scale; }

short CAudioSys::GetDefaultVolumeScale() { return mDefaultVolumeScale; }

void CAudioSys::EnableAICallback(const bool enable) {
  if (mAICallbackEnabled != enable) {
    mAICallbackEnabled = enable;
    if (enable) {
      AIRegisterDMACallback(reinterpret_cast< AIDCallback >(mAICallback));
    } else {
      mAICallback = reinterpret_cast< void* >(AIRegisterDMACallback(nullptr));
    }
  }
}

bool CAudioSys::IsAICallbackEnabled() { return mAICallbackEnabled; }
