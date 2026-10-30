#ifndef _CMOVIEPLAYER
#define _CMOVIEPLAYER
#include "rstl/RstlVersions.h"

#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/CDvdRequest.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "dolphin/thp/THPInfo.h"
#include "rstl/vector.hpp"
#include "types.h"

#include "dolphin/thp/THPFile.h"

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
#if !RSTL_PRIME1
    ~CTHPTextureSet() {}
#endif

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

  static void SetSfxVolume(uchar);
  static void SetAudioEnabled(bool enabled);
  static bool GetAudioEnabled();

  CMoviePlayer(const char* path, const float preLoadSeconds, const bool loop,
               const bool deinterlace);
  ~CMoviePlayer();

  void Update(float dt);
  void DecodeFromRead(const void* ptr);

  bool DrawVideo() const;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_GM8P_00
  void DrawFrame(int left, int right, int top, int bottom);
#endif
#else
  void DrawFrame(int left, int right, int bottom, int top);
#endif

  void DrawFrame(const CVector3f&, const CVector3f&, const CVector3f&, const CVector3f&);
  void MixAudio(short* out, const short* in, unsigned long samples);

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  EPlayMode GetPlayMode() const { return mPlayMode; }
#else
  EPlayMode GetPlayMode() const;
#endif

  void SetPlayMode(EPlayMode mode);
  float GetTotalSeconds() const;
  float GetPlayedSeconds() const;
  uint GetWidth() const;
  uint GetHeight() const;
  bool CanDrawVideo() const { return mIndexLoad.null(); }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  bool PumpIndexLoad();
#else
  bool ContinueLoading();
#endif

  void Rewind();
  bool GetIsFullyCached() const;
  bool GetIsMovieFinishedPlaying() const;
  void DisableLoop() { mLoop = false; }
#if !RSTL_PRIME1
  void SetVolume(uchar volume) { mVolume = volume; }
#endif

  bool IsLooping() const { return mLoop; }

private:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  struct SMovieReadState;
#else
  struct SIndexLoad;
#endif

  static void VerifyCallbackStatus();
  static void StaticMyAudioCallback();
  void InitializeTextures();
  void ReadCompleted();
  void PostDVDReadRequestIfNeeded();
#if !RSTL_PRIME1
  void PrefetchNextFrame();
  void HandleDVDInterrupt(DVDFileInfo* info);
  static void DVDCallback(s32 result, DVDFileInfo* info);
  void CancelReadRequests();
#endif

  CDvdFile mDvdFile;
  THPHeader mHeader;
  THPFrameCompInfo mThpComponents;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  THPVideoInfoOld mVideoInfo;
  THPAudioInfoOld mAudioInfo;
#else
  THPVideoInfo mVideoInfo;
  THPAudioInfo mAudioInfo;
#endif

  rstl::vector< CTHPTextureSet > mTextures;
  rstl::auto_ptr< uchar > mRequestBuffer;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::single_ptr< CDvdRequest > mRequest;
#else
  rstl::single_ptr< CRealDvdRequest > mRequest;
#endif

  rstl::vector< rstl::auto_ptr< uchar > > mRequestQueue;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::single_ptr< SMovieReadState > mIndexLoad;
  uint mNextReadSize;
  uint mNextReadOff;
  uint mReadSizeWrapped;
  uint mReadOffWrapped;
#else
  rstl::single_ptr< SIndexLoad > mIndexLoad;
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

};
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
// This is the GameCube instance layout; Trilogy currently uses only the static audio API.
#if RSTL_VERSION < RSTL_R3IJ
CHECK_SIZEOF(CMoviePlayer, 0x100)
#endif
#else
CHECK_SIZEOF(CMoviePlayer, 0x120)
#endif

#endif // _CMOVIEPLAYER
