#include "rstl/RstlVersions.h"
#include "Kyoto/Audio/CDSPStreamManager.hpp"

#include "Kyoto/CDvdFile.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CInterruptGuard.hpp"

#if RSTL_PRIME1
#include "dolphin/dvd.h"
#endif

#include "dolphin/os.h"
#if RSTL_PRIME1
#include "dolphin/os/OSThread.h"
#endif

#include "string.h"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CDSPStreamManager g_Streams[4] = {CDSPStreamManager(), CDSPStreamManager(), CDSPStreamManager(),
                                  CDSPStreamManager()};
static int sHandleCounter;
#else
CDSPStreamManager sStreams[2];
rstl::reserved_vector< SDSPStreamVoice, 4 > sVoices;
static bool sDeferCallbacks;
static int sDeferredCallbacks;
const int kDSPStreamBlockSize = 0x8F00;
#endif

#if !RSTL_PRIME1
CDSPStreamManager::CDSPStreamManager()
: mDataOffset(0), mFileCur(0), mReadsPending(0), mType(1), mState(kSS_Idle) {}

SDSPStreamVoice::SDSPStreamVoice()
: mBufferSize(0), mNumSamples(0), mType(1), mStreamId(-1), mUpperHalf(false), mFree(true) {}
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CDSPStreamManager::CDSPStreamManager(const rstl::string& fileName, int handle, int volume,
                                     bool oneshot)
: mFileName(fileName)
, mUnclaimed(false)
, mHeaderReadCancelled(false)
, mHeaderReadState(kHRS_Unread)
, mCompanionRight(-1)
, mCompanionLeft(-1)
, mVolume(volume)
, mOneshot(oneshot)
, mHandleId(handle)
, mStreamId(-1) {
  if (!CDvdFile::FileExists(fileName.data())) {
    mUnclaimed = true;
    return;
#else
void CDSPStreamManager::Initialize() {
  for (int i = 0; i < 4; ++i) {
    sVoices.push_back(SDSPStreamVoice());
  }

  sStreams[0].mType = 0;
  for (int i = 0; i < 2; ++i) {
    sVoices[i].mBuffer = rstl::auto_ptr< uchar >(
        static_cast< uchar* >(CMemory::Alloc(0x11E00, IAllocator::kHI_RoundUpLen)));
    sVoices[i].mType = 0;
  }
  CMemory::OffsetFakeStatics(0x23C00);
}

void CDSPStreamManager::Shutdown() {
  for (int i = 0; i < 2; ++i) {
    if (sStreams[i].mState != kSS_Idle) {
      sStreams[i].Deallocate();
#endif

  }
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CDSPStreamManager::CDSPStreamManager(int unused)
: mFileName(rstl::string_l(""))
, mUnclaimed(true)
, mHeaderReadCancelled(false)
, mHeaderReadState(kHRS_Unread) {
  mCompanionRight = -1;
  mCompanionLeft = -1;
  mVolume = 0;
  mHandleId = -1;
  mStreamId = -1;
#else
  for (int i = 0; i < 4; ++i) {
    if (sVoices[i].mType == 0) {
      sVoices[i].mBuffer = rstl::auto_ptr< uchar >();
    }
  }
  sVoices.clear();
  CMemory::OffsetFakeStatics(-0x23C00);
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
bool CDSPStreamManager::StartHeaderRead(DVDCallback callback) {
  CInterruptGuard interrupts;
  if (mHeaderReadState != 0 || mUnclaimed) {
    return false;
  }
  if (DVDOpen(const_cast< char* >(mFileName.data()), &mDvdFile) == 0) {
    return false;
  }
  DVDReadAsyncPrio(&mDvdFile, &mHeader, 0x60, 0, callback, 1);
  mHeaderReadState = kHRS_Reading;
  return true;
#else
int CDSPStreamManager::StartStreaming(const rstl::string& fileName, int volume, int type) {
  CInterruptGuard guard;
  int i;
  for (i = 0; i < 2; ++i) {
    CDSPStreamManager& stream = sStreams[i];
    if (stream.mState != kSS_Idle || stream.mType != type) {
      continue;
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
bool CDSPStreamManager::HasSupportedSampleRate() { return mHeader.mSampleRate == 32000; }

void CDSPStreamManager::WaitForReadCompletion() {
  BOOL ints = OSEnableInterrupts();
  while (mHeaderReadState == 1) {
    OSYieldThread();
#else
    rstl::string path(fileName);
    if (strncmp("mem:", fileName.data(), strlen("mem:")) == 0) {
      path = fileName.substr(4, -1);
      if (CDvdFile::FileExists(path.data())) {
        stream.mPreload = CStreamPreloadedToken(path);
        if (!stream.mPreload->IsReady()) {
          stream.mPreload = rstl::optional_object< CStreamPreloadedToken >();
#endif

  }
#if RSTL_PRIME1
  OSRestoreInterrupts(ints);
}

CDSPStreamManager& CDSPStreamManager::operator=(const CDSPStreamManager& other) {
  this->~CDSPStreamManager();
  new (this) CDSPStreamManager(other);
  return *this;
}

void CDSPStreamManager::Initialize() {
  CDSPStream::Initialize();
  for (int i = 0; i < 4; ++i) {
    g_Streams[i] = CDSPStreamManager();
#endif

  }
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CDSPStreamManager::Shutdown() {
  CDSPStream::FreeAllStreams();
  for (int i = 0; i < 4; ++i) {
    g_Streams[i] = CDSPStreamManager();
#else
    if (DVDOpen(const_cast< char* >(path.data()), &stream.mFiles[0])) {
      stream.mDataOffset = sizeof(SRSFHeader);
      stream.mState = kSS_ReadingHeader;
      stream.mFileCur = 0;
      stream.mVolume = volume;
      for (int j = 1; j < 10; ++j) {
        DVDOpen(const_cast< char* >(path.data()), &stream.mFiles[j]);
      }
      stream.mVoices.clear();
      stream.mReadsPending = 1;
      if (stream.mPreload) {
        stream.mPreload->Read(&stream.mHeader, 0, sizeof(SRSFHeader));
        DVDCallback(0, &stream.mFiles[0]);
      } else {
        DVDReadAsyncPrio(&stream.mFiles[0], &stream.mHeader, sizeof(SRSFHeader), 0, DVDCallback, 1);
      }
      break;
#endif

  }
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
int CDSPStreamManager::StartStreaming(const rstl::string& fileName, int volume, int oneshot) {
  CInterruptGuard interrupts;
  const bool isOneshot = oneshot;
  const int sep = fileName.find('|', 0);
  if (sep == -1) {
    const int idx = FindUnclaimedStreamIdx();
    if (idx == -1) {
      return -1;
#else
  return i < 2 ? i : -1;
#endif

    }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CDSPStreamManager tmpStream(fileName, GetFreeHandleId(), volume, isOneshot);
    if (!tmpStream.mUnclaimed) {
      CDSPStreamManager& stream = g_Streams[idx];
      stream = tmpStream;
      if (!stream.StartHeaderRead(HeaderReadComplete)) {
        stream = CDSPStreamManager();
        return -1;
#else
int CDSPStreamManager::ActivateVoices() {
  CInterruptGuard guard;
  for (int* it = mVoices.begin(); it != mVoices.end(); ++it) {
    sndStreamActivate(sVoices[*it].mStreamId);
#endif

      }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      return tmpStream.mHandleId;
#else
  return 0;
}

int CDSPStreamManager::AllocateVoice(int volume, uchar pan, SND_ADPCMSTREAM_INFO* info,
                                     bool primary) {
  int idx = 0;
  CInterruptGuard guard;
  for (SDSPStreamVoice* it = sVoices.begin(); it != sVoices.end(); ++it, ++idx) {
    if (!it->mFree || mType != it->mType) {
      continue;
    }

    it->mFree = false;
    it->mUpperHalf = false;
    it->mBufferSize = 0x11E00;
    it->mPan = pan;
    it->mNumSamples = (it->mBufferSize / 8) * 14;
    if (it->mType != 0) {
      it->mBuffer = rstl::auto_ptr< uchar >(
          static_cast< uchar* >(CMemory::Alloc(it->mBufferSize, IAllocator::kHI_RoundUpLen)));
    }
    const SDSPStreamVoice& voice = *it;
    it->mStreamId = sndStreamAllocEx(0xFF, voice.mBuffer.get(), voice.mNumSamples,
                                     mHeader.mSampleRate, volume, voice.mPan, 0, 0, 0, 0, 0x30001,
                                     primary ? UpdateStream : UpdateSecondaryStream,
                                     reinterpret_cast< u32 >(this), nullptr);
    sndStreamADPCMParameter(it->mStreamId, info);
    return idx;
#endif

    }
    return -1;
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int leftIdx = 0;
  int rightIdx = 0;
  if (!FindUnclaimedStereoPair(leftIdx, rightIdx)) {
    return -1;
#else
void CDSPStreamManager::DVDCallback(s32 result, DVDFileInfo* fileInfo) {
  static volatile bool sBusy = false;
  static volatile int sCount = 0;
  static volatile int sReadIdx = 0;
  static volatile int sWriteIdx = 0;
  static DVDFileInfo* volatile sQueue[8];

  {
    CInterruptGuard guard;
    sQueue[sWriteIdx] = fileInfo;
    sWriteIdx = (sWriteIdx + 1) & 7;
    ++sCount;
    if (sBusy) {
      return;
    }
    sBusy = true;
#endif

  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::string leftFile(fileName.data(), sep);
  rstl::string rightFile(fileName.data() + sep + 1, -1);
  CDSPStreamManager tmpLeft(leftFile, GetFreeHandleId(), volume, isOneshot);
  CDSPStreamManager tmpRight(rightFile, GetFreeHandleId(), volume, isOneshot);
  if (tmpLeft.mUnclaimed || tmpRight.mUnclaimed) {
    return -1;
#else
  for (;;) {
    DVDFileInfo* next;
    {
      CInterruptGuard guard;
      if (sCount == 0) {
        sBusy = false;
        return;
      }
      --sCount;
      next = sQueue[sReadIdx];
      sReadIdx = (sReadIdx + 1) & 7;
    }
    ReadCompleted(result, fileInfo);
  }
#endif

  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  tmpLeft.mCompanionRight = rightIdx;
  tmpRight.mCompanionLeft = leftIdx;
  g_Streams[leftIdx] = tmpLeft;
  g_Streams[rightIdx] = tmpRight;

  const bool rightOk = g_Streams[rightIdx].StartHeaderRead(HeaderReadComplete);
  const bool leftOk = g_Streams[leftIdx].StartHeaderRead(HeaderReadComplete);
  if (!leftOk || !rightOk) {
    CDSPStreamManager& left = g_Streams[leftIdx];
    CDSPStreamManager& right = g_Streams[rightIdx];
    left.mHeaderReadCancelled = true;
    right.mHeaderReadCancelled = true;
    left.WaitForReadCompletion();
    right.WaitForReadCompletion();
    left = CDSPStreamManager();
    right = CDSPStreamManager();
    return -1;
  }
  return tmpLeft.mHandleId;
}

int CDSPStreamManager::FindUnclaimedStreamIdx() {
  for (int i = 0; i < 4; ++i) {
    if (g_Streams[i].mUnclaimed) {
      return i;
    }
  }
  return -1;
}

bool CDSPStreamManager::FindUnclaimedStereoPair(int& left, int& right) {
  const int idx = FindUnclaimedStreamIdx();
  for (int i = 0; i < 4; ++i) {
    if (g_Streams[i].mUnclaimed && idx != i) {
      left = idx;
      right = i;
      return true;
    }
  }
  return false;
}

int CDSPStreamManager::GetFreeHandleId() {
  CInterruptGuard interrupts;
  for (;;) {
    ++sHandleCounter;
    bool good = true;
    if (sHandleCounter == -1) {
      good = false;
    } else {
      for (int i = 0; i < 4; ++i) {
        if (!g_Streams[i].mUnclaimed && sHandleCounter == g_Streams[i].mHandleId) {
          good = false;
#else
void CDSPStreamManager::ReadCompleted(s32 result, DVDFileInfo* fileInfo) {
  int i;
  for (i = 0; i < 2; ++i) {
    int j;
    for (j = 0; j < 10; ++j) {
      if (&sStreams[i].mFiles[j] == fileInfo) {
#endif

          break;
        }
      }
#if !RSTL_PRIME1
    if (j != 10) {
      break;
#endif

    }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    if (good) {
      return sHandleCounter;
#else
  }
  if (i >= 2) {
    return;
  }

  CDSPStreamManager& stream = sStreams[i];
  --stream.mReadsPending;
  if (stream.mReadsPending > 0) {
    return;
  }

  if (stream.mPreload || DVDGetCommandBlockStatus(&fileInfo->cb) == 0) {
    switch (stream.mState) {
    case kSS_ReadingHeader:
      DCInvalidateRange(&stream.mHeader, sizeof(SRSFHeader));
      if (stream.mHeader.mMagic != 'RS' || stream.mHeader.mVersion != 3) {
        stream.Deallocate();
        return;
      }
      if (stream.mHeader.mNumChannels == 1) {
        stream.mVoices.push_back(
            stream.AllocateVoice(stream.mVolume, 0x40, &stream.mHeader.mAdpcmInfo[0], true));
      } else if (stream.mHeader.mNumChannels == 2) {
        stream.mVoices.push_back(
            stream.AllocateVoice(stream.mVolume, 0, &stream.mHeader.mAdpcmInfo[0], true));
        stream.mVoices.push_back(
            stream.AllocateVoice(stream.mVolume, 0x7F, &stream.mHeader.mAdpcmInfo[1], false));
      } else {
        stream.Deallocate();
        return;
      }
      stream.mState = kSS_Priming;
      stream.BufferStream();
      break;
    case kSS_Priming:
      stream.UpdateARAM();
      stream.ActivateVoices();
      stream.BufferStream();
      stream.mState = kSS_Playing;
      break;
    case kSS_Playing:
      stream.UpdateARAM();
      break;
    case kSS_Stopping:
      stream.Deallocate();
      break;
    }
  } else {
    stream.Deallocate();
  }
}

void CDSPStreamManager::UpdateARAM() {
  for (int* it = mVoices.begin(); it != mVoices.end(); ++it) {
    SDSPStreamVoice& voice = sVoices[*it];
    sndStreamARAMUpdate(voice.mStreamId, voice.mUpperHalf ? voice.mNumSamples / 2 : 0,
                        voice.mNumSamples / 2, 0, 0);
    voice.mUpperHalf = !voice.mUpperHalf;
  }
}

void CDSPStreamManager::IssueRead(DVDFileInfo* file, void* dest, int length, int offset,
                                  const SDSPStreamVoice& voice, const CDSPStreamManager& stream) {
  if (stream.mPreload) {
    stream.mPreload->Read(dest, offset, length);
    if (sDeferCallbacks) {
      ++sDeferredCallbacks;
    } else {
      DVDCallback(0, file);
    }
  } else {
    DVDReadAsyncPrio(file, dest, length, offset, DVDCallback, 1);
  }
}

void CDSPStreamManager::ReadData(uchar* left, uchar* right, uint length, int dataOffset,
                                 uint fileCur) {
  if (right != nullptr) {
    uint remaining = length;
    for (uint pos = fileCur; remaining != 0 && pos < length + fileCur;) {
      uint block = pos / kDSPStreamBlockSize;
      int blockStart = block * kDSPStreamBlockSize;
      uint blockOffset = pos - blockStart;
      uint avail = kDSPStreamBlockSize - blockOffset;
      uint chunk = remaining < avail ? remaining : avail;
      uint offset = dataOffset + block * 0x11E00 + blockOffset;
      uint channelSize = (block + 1) * kDSPStreamBlockSize >= mHeader.mDataSize
                             ? mHeader.mDataSize - blockStart
                             : kDSPStreamBlockSize;
      IssueRead(&mFiles[mReadsPending], left + pos - fileCur, chunk, offset, sVoices[mVoices[0]],
                *this);
      IssueRead(&mFiles[mReadsPending + 1], right + pos - fileCur, chunk, offset + channelSize,
                sVoices[mVoices[1]], *this);
      pos += chunk;
      remaining -= chunk;
      mReadsPending += 2;
    }
  } else {
    IssueRead(&mFiles[mReadsPending], left, length, dataOffset + fileCur, sVoices[mVoices[0]],
              *this);
    ++mReadsPending;
  }
}

void CDSPStreamManager::BufferStream() {
  bool stereo = mVoices.size() > 1;
  int readLen = kDSPStreamBlockSize;
  int end;
  if (mHeader.mLoopFlag != 0) {
    end = (mHeader.mLoopEnd + 31) & ~31;
  } else {
    end = mHeader.mDataSize;
  }

  int loopLen = 0;
  uchar* right = nullptr;
  uchar* const left = sVoices[mVoices[0]].GetCurrentBuffer();
  if (stereo) {
    right = sVoices[mVoices[1]].GetCurrentBuffer();
  }

  if (mFileCur + kDSPStreamBlockSize > end) {
    if (mFileCur < end) {
      readLen = end - mFileCur;
      if (mHeader.mLoopFlag != 0) {
        loopLen = kDSPStreamBlockSize - readLen;
      } else {
        DCZeroRange(left + readLen, kDSPStreamBlockSize - readLen);
        DCFlushRange(left + readLen, kDSPStreamBlockSize - readLen);
        if (stereo) {
          DCZeroRange(right + readLen, kDSPStreamBlockSize - readLen);
          DCFlushRange(right + readLen, kDSPStreamBlockSize - readLen);
        }
      }
    } else if (mHeader.mLoopFlag != 0) {
      mFileCur = mHeader.mLoopStart;
    } else {
      readLen = 0;
      DCZeroRange(left, kDSPStreamBlockSize);
      DCFlushRange(left, kDSPStreamBlockSize);
      if (stereo) {
        DCZeroRange(right, kDSPStreamBlockSize);
        DCFlushRange(right, kDSPStreamBlockSize);
      }
      UpdateARAM();
    }
  }

  CInterruptGuard guard;
  sDeferCallbacks = true;
  if (readLen > 0) {
    ReadData(left, right, readLen, mDataOffset, mFileCur);
    if (loopLen != 0) {
      mFileCur = mHeader.mLoopStart;
      uchar* loopLeft = left + readLen;
      uchar* loopRight = stereo ? right + readLen : nullptr;
      ReadData(loopLeft, loopRight, loopLen, mDataOffset, mFileCur);
      mFileCur += loopLen;
    } else {
      mFileCur += readLen;
    }
  } else if (mState != kSS_Finishing) {
    mState = kSS_Finishing;
  } else if (mState != kSS_Stopping) {
    StopStream();
  }
  sDeferCallbacks = false;

  for (int i = 0; i < sDeferredCallbacks; ++i) {
    DVDCallback(0, &mFiles[0]);
  }
  sDeferredCallbacks = 0;
}

u32 CDSPStreamManager::UpdateStream(void* buf1, u32 len1, void* buf2, u32 len2, u32 user) {
  CDSPStreamManager* stream = reinterpret_cast< CDSPStreamManager* >(user);
  len1 += len2;
  u32 half = sVoices[stream->mVoices[0]].mNumSamples / 2;
  if (len1 < half) {
    return 0;
  }
  if (stream->mReadsPending > 0) {
    return 0;
  }
  stream->BufferStream();
  return half;
}

u32 CDSPStreamManager::UpdateSecondaryStream(void* buf1, u32 len1, void* buf2, u32 len2, u32 user) {
  return 0;
}

void CDSPStreamManager::StopStream() {
  if (mReadsPending > 0) {
    mState = kSS_Stopping;
  } else {
    Deallocate();
  }
}

void CDSPStreamManager::Deallocate() {
  CInterruptGuard guard;
  for (int* it = mVoices.begin(); it != mVoices.end(); ++it) {
    SDSPStreamVoice& voice = sVoices[*it];
    sndStreamDeactivate(voice.mStreamId);
    sndStreamFree(voice.mStreamId);
    voice.mStreamId = -1;
    if (voice.mType != 0) {
      voice.mBuffer = rstl::auto_ptr< uchar >();
    }
    voice.mFree = true;
  }

  for (int i = 0; i < 10; ++i) {
    DVDClose(&mFiles[i]);
  }
  mVoices.clear();
  mPreload = rstl::optional_object< CStreamPreloadedToken >();
  mState = kSS_Idle;
}

void CDSPStreamManager::StopStreaming(int handle) {
  if (handle >= 0 && handle < 2) {
    CInterruptGuard guard;
    if (sStreams[handle].mState != kSS_Idle) {
      sStreams[handle].StopStream();
#endif

    }
  }
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
int CDSPStreamManager::FindClaimedStreamIdx(int handle) {
  for (int i = 0; i < 4; ++i) {
    if (!g_Streams[i].mUnclaimed && handle == g_Streams[i].mHandleId) {
      return i;
#else
void CDSPStreamManager::UpdateVolume(int handle, int volume) {
  if (handle >= 0 && handle < 2) {
    CInterruptGuard guard;
    CDSPStreamManager& stream = sStreams[handle];
    if (stream.mState == kSS_Playing) {
      for (int* it = stream.mVoices.begin(); it != stream.mVoices.end(); ++it) {
        const SDSPStreamVoice& voice = sVoices[*it];
        sndStreamMixParameter(voice.mStreamId, volume, voice.mPan, 0, 0);
#endif

    }
  }
#if RSTL_PRIME1
  return -1;
}

void CDSPStreamManager::StopStreaming(int handle) {
  CInterruptGuard interrupts;
  int idx = FindClaimedStreamIdx(handle);
  if (idx == -1) {
    return;
  }

  CDSPStreamManager& stream = g_Streams[idx];
  if (stream.mUnclaimed) {
    return;
  }

  if (static_cast< EHeaderReadState >(stream.mHeaderReadState) == 1) {
    stream.mHeaderReadCancelled = true;
    return;
  }

  const char companion = stream.CompanionRight();
  if (companion != -1) {
    g_Streams[companion] = CDSPStreamManager();
  }
  CDSPStream::Silence(stream.mStreamId);
  g_Streams[idx] = CDSPStreamManager();
}

SStreamInfo MakeDSPStreamInfo(const CDSPStreamManager& stream) {
  SStreamInfo info;
  info.mFileName = stream.mFileName.data();
  info.mSampleRate = stream.mHeader.mSampleRate;
  info.mAdpcmBytes = (stream.mHeader.mNumNibbles / 2) & 0x7FFFFFE0;
  info.mHeaderSize = 0x60;
  if (stream.mHeader.mLoopFlag != 0) {
    info.mLoopFlag = true;
    info.mLoopStartByte = (stream.mHeader.mLoopStartNibble / 2) & 0x7FFFFFE0;
    const uint loopEnd = (stream.mHeader.mLoopEndNibble / 2) & 0x7FFFFFE0;
    if (loopEnd > info.mAdpcmBytes) {
      info.mLoopEndByte = info.mAdpcmBytes;
    } else {
      info.mLoopEndByte = loopEnd;
    }
  } else {
    info.mLoopFlag = false;
    info.mLoopStartByte = 0;
    info.mLoopEndByte = 0;
  }
  memcpy(&info.mAdpcmInfo, stream.mHeader.mCoef, sizeof(info.mAdpcmInfo));
  return info;
}

void CDSPStreamManager::UpdateVolume(int handle, int volume) {
  CInterruptGuard interrupts;
  int idx = FindClaimedStreamIdx(handle);
  if (idx == -1) {
    return;
  }
  g_Streams[idx].mVolume = volume;
  if (g_Streams[idx].mStreamId != -1) {
    CDSPStream::UpdateVolume(g_Streams[idx].mStreamId, volume);
#endif

  }
}

bool CDSPStreamManager::IsStreamAvailable(int handle) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CInterruptGuard interrupts;
  int idx = FindClaimedStreamIdx(handle);
  if (idx == -1) {
    return false;
#else
  if (handle < 0 && handle >= 2) {
    return true;
#endif

  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (static_cast< EHeaderReadState >(g_Streams[idx].mHeaderReadState) == 1) {
    return false;
  }
  if (g_Streams[idx].mStreamId == -1) {
    return false;
  }
  return CDSPStream::IsStreamAvailable(g_Streams[idx].mStreamId);
#else
  CInterruptGuard guard;
  CDSPStreamManager& stream = sStreams[handle];
  int state = stream.mState;
  return state != kSS_ReadingHeader && state != kSS_Priming;
#endif

}

bool CDSPStreamManager::CanStop(int handle) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CInterruptGuard interrupts;
  int idx = FindClaimedStreamIdx(handle);
  if (idx == -1) {
#else
  if (handle < 0 && handle >= 2) {
#endif

    return true;
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (static_cast< EHeaderReadState >(g_Streams[idx].mHeaderReadState) == 1) {
    return false;
  }
  if (g_Streams[idx].mStreamId == -1) {
    return true;
  }
  return !CDSPStream::IsStreamActive(g_Streams[idx].mStreamId);
#else
  CInterruptGuard guard;
  CDSPStreamManager& stream = sStreams[handle];
  return stream.mState == kSS_Idle;
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CDSPStreamManager::EState CDSPStreamManager::GetStreamState(int handle) {
  CInterruptGuard interrupts;
  int idx = FindClaimedStreamIdx(handle);
  if (idx == -1) {
    return kCDSPSM_Oneshot;
#else
int CDSPStreamManager::GetStreamState(int handle) {
  if (handle < 0 && handle >= 2) {
    return kS_Oneshot;
#endif

  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  switch (static_cast< int >(g_Streams[idx].mHeaderReadState)) {
  case 0:
    return kCDSPSM_Oneshot;
  case 2:
    return g_Streams[idx].mHeader.mLoopFlag ? kCDSPSM_Looping : kCDSPSM_Oneshot;
  default:
    return kCDSPSM_Preparing;
#else
  CInterruptGuard guard;
  CDSPStreamManager& stream = sStreams[handle];
  if (stream.mHeader.mLoopFlag != 0) {
    return kS_Looping;
#endif

  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
}

void CDSPStreamManager::HeaderReadComplete(s32 result, DVDFileInfo* fileInfo) {
  DVDClose(fileInfo);

  for (int idx = 0; idx < 4; ++idx) {
    CDSPStreamManager* stream = &g_Streams[idx];
    if (&stream->mDvdFile == fileInfo && !stream->mUnclaimed) {
      CInterruptGuard interrupts;
      if (result <= 0 || !stream->HasSupportedSampleRate()) {
        *stream = CDSPStreamManager();
        return;
      }

      stream->mHeaderReadState = kHRS_Read;
      int companion = -1;
      if (stream->mCompanionLeft != -1) {
        companion = stream->mCompanionLeft;
      } else if (stream->mCompanionRight != -1) {
        companion = stream->mCompanionRight;
      }

      if (companion != -1) {
        CDSPStreamManager& other = g_Streams[companion];
        const EHeaderReadState compState =
            static_cast< EHeaderReadState >(other.mHeaderReadState);
        if (other.mUnclaimed || compState == 0 ||
            (idx != other.mCompanionRight && idx != other.mCompanionLeft)) {
          *stream = CDSPStreamManager();
          return;
        }
        if (compState == 1) {
          return;
        }
        if (other.mCompanionRight != -1) {
          AllocateStream(companion);
          return;
        }
      }

      AllocateStream(idx);
      return;
    }
  }
}

void CDSPStreamManager::AllocateStream(int idx) {
  CDSPStreamManager& stream = g_Streams[idx];
  SStreamInfo info = MakeDSPStreamInfo(stream);
  if (stream.mCompanionRight == -1) {
    if (!stream.mHeaderReadCancelled) {
      stream.mStreamId =
          CDSPStream::AllocateMono(info, stream.mVolume, 0x40, stream.mOneshot);
    }
    if (stream.mStreamId == -1) {
      stream = CDSPStreamManager();
    }
  } else {
    CDSPStreamManager& rstream = g_Streams[stream.mCompanionRight];
    SStreamInfo rinfo = MakeDSPStreamInfo(rstream);
    if (!stream.mHeaderReadCancelled) {
      stream.mStreamId =
          CDSPStream::AllocateStereo(info, rinfo, stream.mVolume, stream.mOneshot);
    }
    if (stream.mStreamId == -1) {
      stream = CDSPStreamManager();
      rstream = CDSPStreamManager();
    }
  }
#else
  return kS_Oneshot;
#endif

}

