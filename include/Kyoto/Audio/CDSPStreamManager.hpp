#ifndef _CDSPSTREAMMANAGER
#define _CDSPSTREAMMANAGER
#include "rstl/RstlVersions.h"

#include "types.h"

// TODO: verify that this Prime/Echoes difference is real
#include "Kyoto/Streams/CStreamPreloadedToken.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#include "Kyoto/Audio/CDSPStream.hpp"
#endif

#include "rstl/string.hpp"

#if !RSTL_PRIME1
#include "dolphin/dvd.h"
#include "musyx/musyx.h"

// Guessed name. Bytes of one ADPCM channel block in a streamed buffer.
extern const int kDSPStreamBlockSize;

// Header of an Echoes .rsf stream: "RS" version 3, one or two ADPCM channels.
struct SRSFHeader {
  ushort mMagic;
  ushort mVersion;
  uint mNumChannels;
  uint x8_;
  uint mSampleRate;
  uint mDataSize;
  ushort mLoopFlag;
  uint mLoopStart;
  uint mLoopEnd;
  SND_ADPCMSTREAM_INFO mAdpcmInfo[2];
};
CHECK_SIZEOF(SRSFHeader, 0x60)

// One sndStream voice with its ARAM staging buffer.
struct SDSPStreamVoice {
  SDSPStreamVoice();

  uchar* GetCurrentBuffer() const { return mBuffer.get() + (mUpperHalf ? mBufferSize / 2 : 0); }

  rstl::auto_ptr< uchar > mBuffer;
  uint mBufferSize;
  uint mNumSamples;
  uchar mPan;
  int mType;
  SND_STREAMID mStreamId;
  bool mUpperHalf : 1;
  bool mFree : 1;
};
CHECK_SIZEOF(SDSPStreamVoice, 0x20)
#endif

class CDSPStreamManager {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  friend SStreamInfo MakeDSPStreamInfo(const CDSPStreamManager& stream);
#else
public:
  enum EState { kS_Looping, kS_Oneshot };
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
public:
  enum EState { kCDSPSM_Looping, kCDSPSM_Oneshot, kCDSPSM_Preparing };

  CDSPStreamManager(int unused = 0);
  CDSPStreamManager(const rstl::string& fileName, int handle, int volume, bool oneshot);
  CDSPStreamManager& operator=(const CDSPStreamManager& other);
#else
  CDSPStreamManager();
#endif

  static void Initialize();
  static void Shutdown();
  static int StartStreaming(const rstl::string& fileName, int volume, int type);

  static void StopStreaming(int handle);
  static void UpdateVolume(int handle, int volume);
  static bool IsStreamAvailable(int handle);
  static bool CanStop(int handle);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static EState GetStreamState(int handle);
#else
  static int GetStreamState(int handle);
#endif

private:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static int FindUnclaimedStreamIdx();
  static bool FindUnclaimedStereoPair(int& left, int& right);
  static int GetFreeHandleId();
  static int FindClaimedStreamIdx(int handle);
  static void AllocateStream(int idx);
  static void HeaderReadComplete(s32 result, DVDFileInfo* fileInfo);
#else
  enum EStreamState {
    kSS_Idle,
    kSS_ReadingHeader,
    kSS_Priming,
    kSS_Playing,
    kSS_Stopping,
    kSS_Finishing,
  };
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  bool StartHeaderRead(DVDCallback callback);
  bool HasSupportedSampleRate();
  void WaitForReadCompletion();
#else
  static void DVDCallback(s32 result, DVDFileInfo* fileInfo);
  static void ReadCompleted(s32 result, DVDFileInfo* fileInfo);
  // Guessed name
  static void IssueRead(DVDFileInfo* file, void* dest, int length, int offset,
                        const SDSPStreamVoice& voice, const CDSPStreamManager& stream);
  static u32 UpdateStream(void* buf1, u32 len1, void* buf2, u32 len2, u32 user);
  static u32 UpdateSecondaryStream(void* buf1, u32 len1, void* buf2, u32 len2, u32 user);
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
private:
  enum EHeaderReadState { kHRS_Unread, kHRS_Reading, kHRS_Read };
#else
  int AllocateVoice(int volume, uchar pan, SND_ADPCMSTREAM_INFO* info, bool primary);
  int ActivateVoices();
  void UpdateARAM();
  void ReadData(uchar* left, uchar* right, uint length, int dataOffset, uint fileCur);
  void BufferStream();
  void StopStream();
  void Deallocate();
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  char CompanionRight() { return mCompanionRight; }

  ATTRIBUTE_ALIGN_DECL(32, dspadpcm_header mHeader);
  rstl::string mFileName;
  bool mUnclaimed : 1;
  volatile bool mHeaderReadCancelled : 1;
  uchar mHeaderReadState : 2;
  char mCompanionRight;
  char mCompanionLeft;
#else
  ATTRIBUTE_ALIGN_DECL(32, SRSFHeader mHeader);
  DVDFileInfo mFiles[10];
  int mDataOffset;
  int mFileCur;
  int mReadsPending;
#endif

  char mVolume;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  char mOneshot;
  int mHandleId;
  uint mStreamId;
  DVDFileInfo mDvdFile;
#else
  int mType;
  int mState;
  rstl::reserved_vector< int, 2 > mVoices;
  rstl::optional_object< CStreamPreloadedToken > mPreload;
#endif

};
CHECK_SIZEOF(CDSPStreamManager, RSTL_PRIME1 ? 0xC0 : 0x300)

#endif // _CDSPSTREAMMANAGER
