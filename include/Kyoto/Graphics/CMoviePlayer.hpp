#ifndef _CMOVIEPLAYER
#define _CMOVIEPLAYER

#include "rstl/RstlVersions.h"

#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/CDvdRequest.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/vector.hpp"
#include "Kyoto/Platform.h"

#if !defined(TARGET_PC)
#include "dolphin/thp/THPPlayer.h"
#else

typedef struct THPHeader {
  char mMagic[4];
  u32 mVersion;
  u32 mBufferSize;
  u32 mAudioMaxSamples;
  f32 mFrameRate;
  u32 mNumFrames;
  u32 mFirstFrameSize;
  u32 mMovieDataSize;
  u32 mCompInfoDataOffsets;
  u32 mOffsetDataOffsets;
  u32 mMovieDataOffsets;
  u32 mFinalFrameDataOffsets;
} THPHeader;
CHECK_SIZEOF(THPHeader, 0x30)

typedef struct THPVideoInfo {
  u32 mXSize;
  u32 mYSize;
  u32 mVideoType;
} THPVideoInfo;
CHECK_SIZEOF(THPVideoInfo, 0xC)

typedef struct THPVideoInfoOld {
  u32 mXSize;
  u32 mYSize;
} THPVideoInfoOld;
CHECK_SIZEOF(THPVideoInfoOld, 0x8)

typedef struct THPAudioInfo {
  u32 mSndChannels;
  u32 mSndFrequency;
  u32 mSndNumSamples;
  u32 mSndNumTracks;
} THPAudioInfo;
CHECK_SIZEOF(THPAudioInfo, 0x10)

typedef struct THPAudioInfoOld {
  u32 mSndChannels;
  u32 mSndFrequency;
  u32 mSndNumSamples;
} THPAudioInfoOld;
CHECK_SIZEOF(THPAudioInfoOld, 0xC)

typedef struct THPFrameCompInfo {
  u32 mNumComponents;
  u8 mFrameComp[16];
} THPFrameCompInfo;
CHECK_SIZEOF(THPFrameCompInfo, 0x14)

#endif

class CMoviePlayer {
public:
  enum EPlayMode {
    kPM_Stopped,
    kPM_Playing,
  };

  class CTHPTextureSet {
  public:
    CTHPTextureSet(void* y, void* u, void* v, void* audio)
    : mY(static_cast< uchar* >(y))
    , mU(static_cast< uchar* >(u))
    , mV(static_cast< uchar* >(v))
    , mAudio(static_cast< uchar* >(audio))
    , mAudioSamples(0)
    , mAudioSamplesConsumed(0) {}
    ~CTHPTextureSet() {}

    void* Y() { return mY.get(); }

    void* U() { return mU.get(); }

    void* V() { return mV.get(); }

    void* Audio() { return mAudio.get(); }

    uint GetAudioSamples() const { return mAudioSamples; }

    void SetAudioSamples(uint samples) { mAudioSamples = samples; }
    uint GetAudioSamplesConsumed() const { return mAudioSamplesConsumed; }

    void SetAudioSamplesConsumed(uint samples) { mAudioSamplesConsumed = samples; }

  private:
    rstl::auto_ptr< uchar > mY;
    rstl::auto_ptr< uchar > mU;
    rstl::auto_ptr< uchar > mV;
    rstl::auto_ptr< uchar > mAudio;
    uint mAudioSamples;
    uint mAudioSamplesConsumed;
  };

  static void SetGlobalMovieVolume(uchar); // Original Wii export; also used by GameCube Echoes.
  static void SetAudioEnabled(bool enabled);
  static bool GetAudioEnabled();

  CMoviePlayer(const char* path, const float preLoadSeconds, const bool loop,
               const bool deinterlace);
  ~CMoviePlayer();

  void Update(float dt);
  void DecodeFromRead(const void* ptr);

  bool DrawVideo() const;
  void DrawFrame(int left, int right, int bottom, int top);
  void DrawFrame(const CVector3f&, const CVector3f&, const CVector3f&, const CVector3f&);
  void MixAudio(short* out, const short* in, unsigned long samples);

  EPlayMode GetPlayMode() const;

  void SetPlayMode(EPlayMode mode);
  float GetTotalSeconds() const;
  float GetPlayedSeconds() const;
  uint GetWidth() const;
  uint GetHeight() const;
  bool CanDrawVideo() const { return mIndexLoad.null(); }
  bool ContinueLoading();
  void Rewind();
  bool GetIsFullyCached() const;
  bool GetIsMovieFinishedPlaying() const;
  void DisableLoop() { mLoop = false; }
#if !RSTL_PRIME1
  void SetVolume(uchar volume) { mVolume = volume; }
#endif

  bool IsLooping() const { return mLoop; }

private:
  struct SIndexLoad;

  static void VerifyCallbackStatus();
  static void StaticMyAudioCallback(IF_TARGET_PC(short* output, size_t frames, u32 rate, u32 channels));
  void InitializeTextures();
  void ReadCompleted();
  void PostDVDReadRequestIfNeeded();
  void PrefetchNextFrame();
  void HandleDVDInterrupt(DVDFileInfo* info);
  static void DVDCallback(s32 result, DVDFileInfo* info);
  void CancelReadRequests();

  CDvdFile mDvdFile;
  THPHeader mHeader;
  THPFrameCompInfo mThpComponents;
#if RSTL_PRIME1
  THPVideoInfoOld mVideoInfo;
  THPAudioInfoOld mAudioInfo IF_TARGET_PC({});
#else
  THPVideoInfo mVideoInfo;
  THPAudioInfo mAudioInfo IF_TARGET_PC({});
#endif

  rstl::vector< CTHPTextureSet > mTextures;
  rstl::auto_ptr< uchar > mRequestBuffer;
#if RSTL_PRIME1
  rstl::single_ptr< CDvdRequest > mRequest;
#else
  rstl::single_ptr< CRealDvdRequest > mRequest;
#endif

  rstl::vector< rstl::auto_ptr< uchar > > mRequestQueue;
  rstl::single_ptr< SIndexLoad > mIndexLoad;
#if RSTL_PRIME1
  uint mNextReadSize;
  uint mNextReadOff;
  uint mReadSizeWrapped;
  uint mReadOffWrapped;
#else
  int mNextReadSize;
  int mNextReadOff;
  int mReadSizeWrapped;
  int mReadOffWrapped;
#endif

  int mCurLoadFrame;
  int mRequestFrameWrapped;
  int mCurFrame;
  int mDecodedTexSlot;
  int mDrawTexSlot;
  int mAudioSlot;
  int mDecodedTexCount;
  float mFrameRem;
#if !RSTL_PRIME1
  int mPrefetchFrame;
  int mPrefetchOff;
  int mPrefetchSize;
  rstl::single_ptr< uchar > mPrefetchBuffer;
  rstl::single_ptr< CRealDvdRequest > mPrefetchRequest;
#endif

  EPlayMode mPlayMode;
  float mTotalSeconds;
  float mCurSeconds;
  float mPreLoadSeconds;
  int mPreLoadFrames;
  bool mLoop : 1;
  bool mDeinterlace : 1;
#if !RSTL_PRIME1
  bool mIs60Hz : 1;
#endif

  bool mHasAudio : 1;
  bool mFieldFlip : 1;
  uint mCachedBytes;
  int mFieldIndex;
#if !RSTL_PRIME1
  uchar mVolume;
#endif
#if defined(TARGET_PC)
  u32 mAudioPhase = 0;
  short mAudioHistory[2][2] = {};
  bool mAudioPrimed = false;
#endif
};
CHECK_SIZEOF(CMoviePlayer, RSTL_PRIME1 ? 0x100 : 0x120)

#if RSTL_PRIME1
inline CMoviePlayer::EPlayMode CMoviePlayer::GetPlayMode() const { return mPlayMode; }
#endif

#endif // _CMOVIEPLAYER
