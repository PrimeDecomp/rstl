#ifndef _CAUDIOSYS
#define _CAUDIOSYS
#include "rstl/RstlVersions.h"

#if RSTL_PRIME1
#include "musyx/musyx.h"
#endif

#include "types.h"
#if RSTL_PRIME1
#include "dolphin/dtk.h"
#endif

#include "rstl/map.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

#include "Kyoto/Math/CVector3f.hpp"
#if RSTL_PRIME1
#include "Kyoto/TToken.hpp"
#endif

#if !RSTL_PRIME1
#include "dolphin/dtk.h"
#include "musyx/musyx.h"
#endif

#if RSTL_PRIME1
class CAudioGroupSet;
class CSimplePool;
#endif

enum ETRKSampleRate {
  kTSR_Zero,
  kTSR_One,
#if RSTL_PRIME1
  // TODO
};

enum ETRKPlayState {
  kTPS_Stopped,
  kTPS_Playing,
  kTPS_Paused,
  kTPS_Busy,
  kTPS_Preparing,
#endif

};

enum ETRKRepeatMode {
  kTRM_NoRepeat,
  kTRM_RepeatOne,
  kTRM_RepeatAll,
};

#if !RSTL_PRIME1
enum ETRKPlayState {
  kTPS_Stopped,
  kTPS_Playing,
  kTPS_Paused,
  kTPS_Busy,
};
#endif

class CAudioSys {
  static const rstl::string mpDefaultInvalidString;
#if RSTL_PRIME1
  static void* SampleDataUploadCallback(u32 address, u32 bytes);
  static void* mpSampleDataUploadBuffer;
#endif

public:
  enum ESurroundModes { kSM_Mono, kSM_Stereo, kSM_Surround };
  class CEmitterData {
  public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CEmitterData() : mUsed(0), mImportant(0), mPrio(kEmitterMedPriority) {}
#else
    CEmitterData() : mUsed(false), mImportant(false), mPriority(kEmitterMedPriority) {}
#endif

    SND_EMITTER mEmitter;
    bool mUsed;
    bool mImportant;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    uchar mPrio;
#else
    uchar mPriority;
#endif

  };

  struct C3DEmitterParmData {
    C3DEmitterParmData(const float maxDist = 150.f, const float distComp = 0.1f,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                       const uint flags = 1, const uchar maxVol = 127, const uchar minVol = 0)
    : mPos(0.f, 0.f, 0.f)
    , mDir(0.f, 0.f, 0.f)
    , mMaxDist(maxDist)
    , mDistComp(distComp)
    , mFlags(flags)
    , mSfxId(0)
    , mMaxVol(maxVol)
    , mMinVol(minVol)
    , mImportant(false)
    , mPrio(CAudioSys::kEmitterMedPriority) {}
#else
                       const uint flags = 1, uchar maxVol = 127, uchar minVol = 0);
#endif

    CVector3f mPos;
    CVector3f mDir;
    float mMaxDist;
    float mDistComp;
    uint mFlags;
    ushort mSfxId;
    uchar mMaxVol;
    uchar mMinVol;
    bool mImportant; // Can't be allocated over, regardless of priority
    uchar mPrio;
#if !RSTL_PRIME1
    uchar mStudio;
#endif

  };

  class CTrkData {
  public:
    CTrkData(const rstl::string& name) : mInUse(true), mName(name) {}

    bool GetIsTrackInUse() const { return mInUse; }
    void SetIsTrackInUse(bool v) { mInUse = v; }
    DTKTrack* GetTrack() { return &mTrack; }
    char* GetFileName() { return const_cast< char* >(mName.data()); }

    DTKTrack mTrack;
    bool mInUse;
    rstl::string mName;
  };

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CAudioSys(uchar, uchar, uchar, uchar, uint);
#else
  CAudioSys(uchar numVoices, uchar numMusic, uchar numSfx, uchar maxNumEmitters, uint aramSize);
#endif

  ~CAudioSys();

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static void SysSetVolume(uchar, ushort, uchar);
  static void SysSetSfxVolume(uchar, ushort, uchar, uchar);
  static bool SysLoadGroupSet(CSimplePool*, uint);
  static bool SysLoadGroupSet(const CToken&, const rstl::string&, uint);
  static const rstl::string& SysGetGroupSetName(uint);
  static bool SysPushGroupIntoARAM(const rstl::string& name, uchar);
  static void SysPopGroupFromARAM();
  static bool SysUnloadGroupSet(const rstl::string& name);
  static bool SysUnloadSampleData(const rstl::string& name);
  static bool SysIsGroupSetLoaded(const rstl::string& name);
  static rstl::ncrc_ptr< CAudioGroupSet > FindGroupSet(const rstl::string& name);
#else
  static void SysSetVolume(uchar volume, uint time, uchar group);
  static void SysSetSfxVolume(uchar volume, ushort time, uchar music, uchar fx);

  static SND_VOICEID SfxStart(const ushort sfxId, uchar vol, uchar pan, const uchar prio);
  static void SfxStop(SND_VOICEID handle);
  static SND_VOICEID SfxCheck(SND_VOICEID handle);
  static void SfxPan(SND_VOICEID handle, uchar pan);
  static void SfxSpan(SND_VOICEID handle, uchar span);
  static void SfxVolume(SND_VOICEID handle, uchar vol);
  static void SfxPitchBend(SND_VOICEID handle, ushort pitch);
  static void SfxCtrl(SND_VOICEID handle, uchar ctrl, uchar val);
  static bool SfxSetFilter(SND_VOICEID handle, uint filter, uint param);

  static int TrkQueueTrack(const rstl::string& name, void (*callback)(u32), uint eventMask);
  static void TrkRemoveTrack(const rstl::string& name);
  static void TrkFlushTracks();
  static void TrkSetSampleRate(ETRKSampleRate rate);
  static void TrkSetRepeatMode(ETRKRepeatMode mode);
  static void TrkSetState(ETRKPlayState state);
  static ETRKPlayState TrkGetState();
  static void TrkSetVolume(uchar left, uchar right);
  static void TrkNextTrack();
#endif

  static rstl::ncrc_ptr< CTrkData > FindTrack(const rstl::string& name);

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static void SetDefaultVolumeScale(short);
  static void SetVolumeScale(short);
  static void SetVerbose(bool verbose) { mVerbose = verbose; }
  static void EnableProLogic2(bool enabled) { mProLogic2 = enabled; }
  static void TrkRemoveTrack(const rstl::string& name);
  static void TrkFlushTracks();
  static void TrkSetState(ETRKPlayState);
  static ETRKPlayState TrkGetState();
  static void TrkSetVolume(uchar, uchar);
  static void TrkSetRepeatMode(ETRKRepeatMode);
  static void TrkNextTrack();
  static int TrkQueueTrack(const rstl::string&, void (*)(u32), uint);

  static void SetStereoMode(const bool mode);
  static void SetSurroundMode(const ESurroundModes mode);
#if RSTL_VERSION == VERSION_GM8E_02 || RSTL_VERSION == VERSION_GM8J_00
  static ESurroundModes GetSurroundMode();
#else
  static ESurroundModes GetSurroundMode() { return mSurroundMode; }
#endif
  static void TrkSetSampleRate(ETRKSampleRate);

  static short GetDefaultVolumeScale();
  static bool GetVerbose();

  static SND_VOICEID SfxStart(const SND_FXID, const uchar vol, const uchar pan, const uchar prio);
  static void SfxStop(SND_VOICEID handle);
  static void SfxCtrl(const SND_VOICEID handle, const uchar ctrl, const uchar val);
  static SND_VOICEID SfxCheck(SND_VOICEID handle);
  static void SfxVolume(SND_VOICEID handle, u8 vol);
  static void SfxSpan(SND_VOICEID handle, uchar vol);
  static void SfxPitchBend(SND_VOICEID handle, const ushort pitch);
  static uint S3dFindLowerPriorityHandle(uint prio);
  static uint S3dFindUnusedHandle();
  static uint S3dAddEmitterParaEx(const C3DEmitterParmData& params, ushort handle,
#else
  static void S3dAddListener(const CVector3f& pos, const CVector3f& dir, const CVector3f& heading,
                             const CVector3f& up, float frontSur, float backSur,
                             float soundSpeed, uint flags, uchar volume);
  static bool S3dUpdateListener(const CVector3f& pos, const CVector3f& dir,
                                const CVector3f& heading, const CVector3f& up, uchar volume);
  static bool S3dRemoveListener();
  static uint S3dAddEmitterParaEx(const C3DEmitterParmData& params, ushort groupId,
#endif

                                  SND_PARAMETER_INFO* paraInfo);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static const bool S3dUpdateEmitter(const uint a, const CVector3f& b, const CVector3f& c,
                                     const uchar d);
  static const bool S3dRemoveEmitter(uint handle);
  static const bool S3dCheckEmitter(uint handle);
  static uint S3dEmitterVoiceID(uint handle);

  static void S3dAddListener(const CVector3f& pos, const CVector3f& dir, const CVector3f& heading,
                             const CVector3f& up, const float frontSur, const float backSur,
                             const float soundSpeed, const uint flags, const uchar volume);
  static bool S3dRemoveListener();
  static bool S3dUpdateListener(const CVector3f& pos, const CVector3f& dir,
                                const CVector3f& heading, const CVector3f& up, const uchar volume);

  static void S3dAddEmitter(SND_FXID fxid, const CVector3f& pos, const CVector3f& dir,
                            const bool b1, const bool b2, short, int);
#else
  static bool S3dUpdateEmitter(uint handle, const CVector3f& pos, const CVector3f& dir,
                               const uchar maxVol);
  static bool S3dRemoveEmitter(uint handle);
#endif

  static void S3dFlushAllEmitters();
  static void S3dFlushUnusedEmitters();
#if !RSTL_PRIME1
  static bool S3dCheckEmitter(uint handle);
  static uint S3dEmitterVoiceID(uint handle);
  static uint S3dFindUnusedHandle();
  static uint S3dFindLowerPriorityHandle(uint prio);
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static uint SeqPlayEx(const ushort gid, const ushort sid, void* arrfile, SND_PLAYPARA* para,
                        const uchar studio);
  static void SeqStop(u32 seqId);
  static void SeqVolume(u8, u16, u32, u8);
#else
  static SND_SEQID SeqPlayEx(ushort gid, ushort sid, void* arrfile, SND_PLAYPARA* para,
                             uchar studio);
  static void SeqStop(SND_SEQID seqId);
  static void SeqVolume(uchar volume, ushort time, SND_SEQID seqId, uchar mode);

  static void SetStereoMode(bool mode);
  static ESurroundModes GetSurroundMode();
  static void SetSurroundMode(ESurroundModes mode);
  static void SetVolumeScale(short scale);
  static void SetDefaultVolumeScale(short scale);
  static short GetDefaultVolumeScale();
  static void EnableAICallback(bool enable);
  static bool IsAICallbackEnabled();

  static short GetScaledVolume(int vol) { return kVolumeTable[vol]; }
  static const ushort kVolumeTable[];
#endif

  static bool mInitialized;
  static bool mIsListenerActive;
#if RSTL_PRIME1
  static bool mVerbose;
  static uint mUnusedEmitterHandle;
#endif

  static uchar mMaxNumEmitters;
#if RSTL_PRIME1
  static rstl::map< rstl::string, rstl::ncrc_ptr< CAudioGroupSet > >* mpGroupSetDB;
  static rstl::map< uint, rstl::string >* mpGroupSetResNameDB;
#endif

  static rstl::map< rstl::string, rstl::ncrc_ptr< CTrkData > >* mpDVDTrackDB;
  static rstl::vector< CEmitterData >* mpEmitterDB;
  static SND_LISTENER* mpListener;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static short GetScaledVolume(const int vol) { return kVolumeTable[vol]; }

  static void EnableAICallback(bool enable);
  static bool IsAICallbackEnabled();

  /* TODO: Remaining globals */
#else
  static uint mUnusedEmitterHandle;
  static ESurroundModes mSurroundMode;
  static uint mMaxAramUsage;
#endif

  static void* mAICallback;
#if RSTL_PRIME1
  static bool mAICallbackEnabled;
  static ESurroundModes mSurroundMode;
  static int mMaxAramUsage;
  static int mCurrentAramUsage;
#endif

  static bool mProLogic2;
  static short mVolumeScale;
  static short mDefaultVolumeScale;
#if !RSTL_PRIME1
  static bool mAICallbackEnabled;
#endif

  static const uchar kMaxVolume;
  static const uchar kEmitterMedPriority;
#if RSTL_PRIME1
  static const ushort kVolumeTable[];
#endif

};

#if RSTL_PRIME1
CHECK_SIZEOF(CAudioSys, 0x1)
NESTED_CHECK_SIZEOF(CAudioSys, CEmitterData, 0x54)
#endif

NESTED_CHECK_SIZEOF(CAudioSys, C3DEmitterParmData, 0x2c)
#if RSTL_PRIME1
#if RSTL_VERSION < RSTL_R3IJ
NESTED_CHECK_SIZEOF(CAudioSys, CTrkData, 0x64)
#endif
#endif

#endif // _CAUDIOSYS
