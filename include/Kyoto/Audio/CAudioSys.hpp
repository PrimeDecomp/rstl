#ifndef _CAUDIOSYS
#define _CAUDIOSYS
#include "rstl/RstlVersions.h"

#include "musyx/musyx.h"

#include "types.h"
#include "dolphin/dtk.h"

#include "rstl/map.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"

#if RSTL_PRIME1
class CAudioGroupSet;
class CSimplePool;
#endif

enum ETRKSampleRate {
  kTSR_Zero,
  kTSR_One,
};

enum ETRKPlayState {
  kTPS_Stopped,
  kTPS_Playing,
  kTPS_Paused,
  kTPS_Busy,
#if RSTL_PRIME1
  kTPS_Preparing,
#endif
};

enum ETRKRepeatMode {
  kTRM_NoRepeat,
  kTRM_RepeatOne,
  kTRM_RepeatAll,
};

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
    CEmitterData() : mUsed(false), mImportant(false), mPrio(kEmitterMedPriority) {}

    SND_EMITTER mEmitter;
    bool mUsed;
    bool mImportant;
    uchar mPrio;
  };

  struct C3DEmitterParmData {
    // Prime inlines this constructor; Echoes has an out-of-line one (CAudioSys.cpp).
    C3DEmitterParmData(const float maxDist = 150.f, const float distComp = 0.1f,
                       const uint flags = 1, uchar maxVol = 127, uchar minVol = 0)
#if RSTL_PRIME1
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
    ;
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

  CAudioSys(uchar numVoices, uchar numMusic, uchar numSfx, uchar maxNumEmitters, uint aramSize);
  ~CAudioSys();

#if RSTL_PRIME1
  static void SysSetVolume(uchar volume, ushort time, uchar group);
#else
  static void SysSetVolume(uchar volume, uint time, uchar group);
#endif
  static void SysSetSfxVolume(uchar volume, ushort time, uchar music, uchar fx);

#if RSTL_PRIME1
  static bool SysLoadGroupSet(CSimplePool*, uint);
  static bool SysLoadGroupSet(const CToken&, const rstl::string&, uint);
  static const rstl::string& SysGetGroupSetName(uint);
  static bool SysPushGroupIntoARAM(const rstl::string& name, uchar);
  static void SysPopGroupFromARAM();
  static bool SysUnloadGroupSet(const rstl::string& name);
  static bool SysUnloadSampleData(const rstl::string& name);
  static bool SysIsGroupSetLoaded(const rstl::string& name);
  static rstl::ncrc_ptr< CAudioGroupSet > FindGroupSet(const rstl::string& name);
#endif

  // Top-level const on these parameters changes MWCC call-site code, so each game keeps its own.
#if RSTL_PRIME1
  static SND_VOICEID SfxStart(const SND_FXID sfxId, const uchar vol, const uchar pan,
                              const uchar prio);
#else
  static SND_VOICEID SfxStart(const SND_FXID sfxId, uchar vol, uchar pan, const uchar prio);
#endif
  static void SfxStop(SND_VOICEID handle);
  static SND_VOICEID SfxCheck(SND_VOICEID handle);
#if !RSTL_PRIME1
  static void SfxPan(SND_VOICEID handle, uchar pan);
#endif
  static void SfxSpan(SND_VOICEID handle, uchar span);
  static void SfxVolume(SND_VOICEID handle, uchar vol);
  static void SfxPitchBend(SND_VOICEID handle, ushort pitch);
  static void SfxCtrl(SND_VOICEID handle, uchar ctrl, uchar val);
#if !RSTL_PRIME1
  static bool SfxSetFilter(SND_VOICEID handle, uint filter, uint param);
#endif

  static int TrkQueueTrack(const rstl::string& name, void (*callback)(u32), uint eventMask);
  static void TrkRemoveTrack(const rstl::string& name);
  static void TrkFlushTracks();
  static void TrkSetSampleRate(ETRKSampleRate rate);
  static void TrkSetRepeatMode(ETRKRepeatMode mode);
  static void TrkSetState(ETRKPlayState state);
  static ETRKPlayState TrkGetState();
  static void TrkSetVolume(uchar left, uchar right);
  static void TrkNextTrack();
  static rstl::ncrc_ptr< CTrkData > FindTrack(const rstl::string& name);

#if RSTL_PRIME1
  static void S3dAddListener(const CVector3f& pos, const CVector3f& dir, const CVector3f& heading,
                             const CVector3f& up, const float frontSur, const float backSur,
                             const float soundSpeed, const uint flags, const uchar volume);
  static bool S3dUpdateListener(const CVector3f& pos, const CVector3f& dir,
                                const CVector3f& heading, const CVector3f& up, const uchar volume);
#else
  static void S3dAddListener(const CVector3f& pos, const CVector3f& dir, const CVector3f& heading,
                             const CVector3f& up, float frontSur, float backSur,
                             float soundSpeed, uint flags, uchar volume);
  static bool S3dUpdateListener(const CVector3f& pos, const CVector3f& dir,
                                const CVector3f& heading, const CVector3f& up, uchar volume);
#endif
  static bool S3dRemoveListener();
  static uint S3dAddEmitterParaEx(const C3DEmitterParmData& params, ushort groupId,
                                  SND_PARAMETER_INFO* paraInfo);
  static RSTL_PRIME1_CONST bool S3dUpdateEmitter(RSTL_PRIME1_CONST uint handle,
                                                 const CVector3f& pos, const CVector3f& dir,
                                                 const uchar maxVol);
  static RSTL_PRIME1_CONST bool S3dRemoveEmitter(uint handle);
  static void S3dFlushAllEmitters();
  static void S3dFlushUnusedEmitters();
  static RSTL_PRIME1_CONST bool S3dCheckEmitter(uint handle);
  static uint S3dEmitterVoiceID(uint handle);
  static uint S3dFindUnusedHandle();
  static uint S3dFindLowerPriorityHandle(uint prio);
#if RSTL_PRIME1
  // No retail or demo-map symbol; nothing calls it.
  static void S3dAddEmitter(SND_FXID fxid, const CVector3f& pos, const CVector3f& dir,
                            const bool b1, const bool b2, short, int);
#endif

  static SND_SEQID SeqPlayEx(ushort gid, ushort sid, void* arrfile, SND_PLAYPARA* para,
                             uchar studio);
  static void SeqStop(SND_SEQID seqId);
  static void SeqVolume(uchar volume, ushort time, SND_SEQID seqId, uchar mode);

  static void SetStereoMode(bool mode);
  static void SetSurroundMode(ESurroundModes mode);
#if RSTL_PRIME1 && RSTL_VERSION != RSTL_GM8E_02 && RSTL_VERSION != RSTL_GM8J_00
  static ESurroundModes GetSurroundMode() { return mSurroundMode; }
#else
  static ESurroundModes GetSurroundMode();
#endif
  static void SetVolumeScale(short scale);
  static void SetDefaultVolumeScale(short scale);
  static short GetDefaultVolumeScale();
  static void EnableAICallback(bool enable);
  static bool IsAICallbackEnabled();
#if RSTL_PRIME1
  static void SetVerbose(bool verbose) { mVerbose = verbose; }
  static bool GetVerbose();
  static void EnableProLogic2(bool enabled) { mProLogic2 = enabled; }
#endif

  static short GetScaledVolume(int vol) { return kVolumeTable[vol]; }

  static bool mInitialized;
  static bool mIsListenerActive;
  static uchar mMaxNumEmitters;
  static rstl::map< rstl::string, rstl::ncrc_ptr< CTrkData > >* mpDVDTrackDB;
  static rstl::vector< CEmitterData >* mpEmitterDB;
  static SND_LISTENER* mpListener;
  static uint mUnusedEmitterHandle;
  static ESurroundModes mSurroundMode;
  static int mMaxAramUsage;
  static void* mAICallback;
  static bool mAICallbackEnabled;
  static bool mProLogic2;
  static short mVolumeScale;
  static short mDefaultVolumeScale;
#if RSTL_PRIME1
  static bool mVerbose;
  static rstl::map< rstl::string, rstl::ncrc_ptr< CAudioGroupSet > >* mpGroupSetDB;
  static rstl::map< uint, rstl::string >* mpGroupSetResNameDB;
  static int mCurrentAramUsage;
#endif

  static const uchar kMaxVolume;
  static const uchar kEmitterMedPriority;
  static const ushort kVolumeTable[];
};

CHECK_SIZEOF(CAudioSys, 0x1)
NESTED_CHECK_SIZEOF(CAudioSys, CEmitterData, 0x54)
NESTED_CHECK_SIZEOF(CAudioSys, C3DEmitterParmData, 0x2c)
NESTED_CHECK_SIZEOF(CAudioSys, CTrkData, 0x64)

#endif // _CAUDIOSYS
