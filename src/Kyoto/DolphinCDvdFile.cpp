#include "rstl/RstlVersions.h"
#if RSTL_PRIME1
#include "GameVersions.h"

#include "Kyoto/CARAMToken.hpp"
#endif

#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/CDvdRequest.hpp"

#include "Kyoto/CARAMManager.hpp"

#if !RSTL_PRIME1
#include "rstl/math.hpp"
#endif

#include "dolphin/os/OSCache.h"
#if RSTL_PRIME1
#include "rstl/math.hpp"
#endif

#include "dolphin/arq.h"
#include "dolphin/dvd.h"
#include "dolphin/os.h"

#include "string.h"

static CDvdFile* sFirstARAM = nullptr;
#if !RSTL_PRIME1
// Guessed name. Never set in retail; when set, transfers are driven by polling
// IsARAMFileLoaded instead of from interrupt callbacks.
static bool sPollingMode = false;
bool gDvdActivity = false;
#endif

struct CDvdFileARAM {
  CDvdFileARAM()
  : mGotARAMInterrupt(false)
  , mGotDvdInterrupt(false)
  , mFileSize1(0)
  , mFileSize2(0)
  , mCurBufferLen(0)
  , mAramOffset(0)
  , mBufferLen(0)
  , mBufferIndex(0) {}
#if !RSTL_PRIME1
  ~CDvdFileARAM() {}
#endif

  ARQRequest mARQRequest;
  struct SDvdInfo {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    SDvdInfo() : mDvdFileInfo(), mDvdFile(nullptr), mNextfile(nullptr) {}
#else
    SDvdInfo() : mDvdFile(nullptr), mNextfile(nullptr) {}
#endif

    DVDFileInfo mDvdFileInfo;
    CDvdFile* mDvdFile;
    CDvdFile* mNextfile;
  } mInfo;
  rstl::reserved_vector< rstl::auto_ptr< uchar >, 2 > mBuffers;
  bool mGotARAMInterrupt;
  bool mGotDvdInterrupt;
  uint mFileSize1;
  uint mFileSize2;
  int mCurBufferLen;
  uint mAramOffset;
  int mBufferLen;
  uint mBufferIndex;
};

#if !RSTL_PRIME1
CHECK_SIZEOF(CDvdFileARAM, 0x94)
#endif

const char* DecodeARAMFile(const char* filename) {
  if (!strncmp(filename, "aram:", 5)) {
    return filename + 5;
  }

  return filename;
}

void CDvdFile::DVDARAMXferCallback(s32 result, DVDFileInfo* info) {
  CDvdFileARAM::SDvdInfo* ptr = reinterpret_cast< CDvdFileARAM::SDvdInfo* >(info);
#if !RSTL_PRIME1
  if (!sPollingMode) {
#endif

  DVDClose(&ptr->mDvdFileInfo);
#if !RSTL_PRIME1
  }
#endif

  ptr->mDvdFile->HandleDVDInterrupt();
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CDvdFile::ARAMARAMXferCallback(uintptr_t addr) {
#else
void CDvdFile::ARAMARAMXferCallback(u32 addr) {
#endif

  reinterpret_cast< CDvdFileARAM* >(addr)->mInfo.mDvdFile->HandleARAMInterrupt();
}

void CDvdFile::HandleARAMInterrupt() {
  BOOL enabled = OSDisableInterrupts();
  CDvdFileARAM* arFile = mARAMFile.get();

  arFile->mGotARAMInterrupt = true;

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (arFile->mGotARAMInterrupt && arFile->mGotDvdInterrupt) {
#else
  if (!sPollingMode && arFile->mGotARAMInterrupt && arFile->mGotDvdInterrupt) {
#endif

    PingARAMTransfer();
  }

  OSRestoreInterrupts(enabled);
}

void CDvdFile::HandleDVDInterrupt() {
  BOOL enabled = OSDisableInterrupts();
  CDvdFileARAM* arFile = mARAMFile.get();

  arFile->mGotDvdInterrupt = true;

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (arFile->mGotARAMInterrupt && arFile->mGotDvdInterrupt) {
#else
  if (!sPollingMode && arFile->mGotARAMInterrupt && arFile->mGotDvdInterrupt) {
#endif

    PingARAMTransfer();
  }

  OSRestoreInterrupts(enabled);
}

void CDvdFile::PingARAMTransfer() {
  CDvdFileARAM* aramFile = mARAMFile.get();

  if (aramFile->mBufferLen == 0) {
    PopARAMFileLoad();
    return;
  }

  int length = rstl::min_val(65536, aramFile->mBufferLen);
  ARQPostRequest(&aramFile->mARQRequest, 0, ARQ_TYPE_MRAM_TO_ARAM, ARQ_PRIORITY_HIGH,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                 reinterpret_cast< uintptr_t >(aramFile->mBuffers[aramFile->mBufferIndex].get()),
                 reinterpret_cast< uintptr_t >(mARAMBuffer + aramFile->mAramOffset), length,
#else
                 reinterpret_cast< u32 >(aramFile->mBuffers[aramFile->mBufferIndex].get()),
                 reinterpret_cast< u32 >(mARAMBuffer + aramFile->mAramOffset), length,
#endif

                 ARAMARAMXferCallback);

  aramFile->mBufferLen -= length;
  aramFile->mAramOffset += length;
  aramFile->mGotARAMInterrupt = false;
  aramFile->mBufferIndex ^= 1;

  if (aramFile->mCurBufferLen != 0) {
    int length2 = rstl::min_val(65536, aramFile->mCurBufferLen);
#if !RSTL_PRIME1
    aramFile->mGotDvdInterrupt = false;
#endif

    DVDFastOpen(mFileEntry, &aramFile->mInfo.mDvdFileInfo);
    DVDReadAsync(&aramFile->mInfo.mDvdFileInfo, aramFile->mBuffers[aramFile->mBufferIndex].get(),
                 length2, aramFile->mFileSize2, DVDARAMXferCallback);
#if !RSTL_PRIME1
    gDvdActivity = true;
#endif

    aramFile->mFileSize2 += length2;
    aramFile->mCurBufferLen -= length2;
#if RSTL_PRIME1
    aramFile->mGotDvdInterrupt = false;
#endif

  }
}

void CDvdFile::TryARAMFile() {
  mARAMBuffer = static_cast< uchar* >(CARAMManager::Alloc(mSize));
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (CARAMManager::GetInvalidAlloc() == mARAMBuffer) {
#else
  if (!CARAMManager::IsValidAlloc(mARAMBuffer)) {
#endif

    return;
  }
  mARAMFile = rs_new CDvdFileARAM();
  CDvdFileARAM* arfile = mARAMFile.get();
  arfile->mInfo.mDvdFile = this;
  arfile->mGotARAMInterrupt = true;
  arfile->mFileSize1 = arfile->mCurBufferLen = arfile->mBufferLen = GetFileSize();
  mARAMAllocated = true;
  PushARAMFileLoad();
}

void CDvdFile::PushARAMFileLoad() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  BOOL enabled = OSDisableInterrupts();
#else
  BOOL enabled = true;
  if (!sPollingMode) {
    enabled = OSDisableInterrupts();
  }
#endif

  CDvdFile* file = sFirstARAM;
  if (file == NULL) {
    sFirstARAM = this;
    StartARAMFileLoad();
  } else {
    for (CDvdFile* p = file; p != nullptr; p = p->mARAMFile->mInfo.mNextfile) {
      if (p->mARAMFile->mInfo.mNextfile == nullptr) {
        p->mARAMFile->mInfo.mNextfile = this;
        break;
      }
    }
  }
#if !RSTL_PRIME1
  if (!sPollingMode) {
#endif

  OSRestoreInterrupts(enabled);
#if !RSTL_PRIME1
  }
#endif

}

void CDvdFile::PopARAMFileLoad() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  BOOL enabled = OSDisableInterrupts();
#else
  BOOL enabled = true;
  if (!sPollingMode) {
    enabled = OSDisableInterrupts();
  }
#endif

  CDvdFile* file = mARAMFile->mInfo.mNextfile;
  mARAMPopped = true;
  sFirstARAM = file;
  if (file != nullptr) {
    file->StartARAMFileLoad();
  }

#if !RSTL_PRIME1
  if (!sPollingMode) {
#endif

  OSRestoreInterrupts(enabled);
#if !RSTL_PRIME1
  }
#endif

}

bool CDvdFile::IsARAMFileLoaded() {
  if (!mARAMAllocated) {
    return true;
  }

  if (!mARAMPopped) {
#if !RSTL_PRIME1
    if (sPollingMode && mARAMFile->mGotARAMInterrupt && mARAMFile->mGotDvdInterrupt) {
      PingARAMTransfer();
    }
#endif

    return false;
  }

  mARAMFile = nullptr;

  return true;
}

void CDvdFile::StartARAMFileLoad() {
  CDvdFileARAM* aramFile = mARAMFile.get();
  aramFile->mBuffers.push_back(
      static_cast< uchar* >(CMemory::Alloc(0x10000, IAllocator::kHI_RoundUpLen)));
  aramFile->mBuffers.push_back(
      static_cast< uchar* >(CMemory::Alloc(0x10000, IAllocator::kHI_RoundUpLen)));

  int len = rstl::min_val(mSize, 65536);
  aramFile->mCurBufferLen -= len;
  aramFile->mFileSize2 = len;
#if !RSTL_PRIME1
  if (!sPollingMode) {
#endif

  DVDFastOpen(mFileEntry, &aramFile->mInfo.mDvdFileInfo);
#if !RSTL_PRIME1
  } else {
    DVDOpen(const_cast< char* >(DecodeARAMFile(mFilename.data())), &aramFile->mInfo.mDvdFileInfo);
  }
#endif

  DVDReadAsync(&aramFile->mInfo.mDvdFileInfo, aramFile->mBuffers[0].get(), len, 0,
               DVDARAMXferCallback);
#if !RSTL_PRIME1
  gDvdActivity = true;
#endif

}

void CDvdFile::StallForARAMFile() {
  while (mARAMFile.get() != nullptr) {
    OSYieldThread();
  }
}

CDvdFile::CDvdFile(const char* filename)
: mFileEntry(-1)
, mARAMBuffer(0)
, mARAMAllocated(false)
, mARAMPopped(false)
, mARAMFile(nullptr)
, mOffset(0)
, mSize(0)
, mFilename(filename, -1) {
  const char* decodedName = DecodeARAMFile(filename);
  mFileEntry = DVDConvertPathToEntrynum(const_cast< char* >(decodedName));
  DVDFileInfo fileInfo;
  if (mFileEntry != -1) {
    DVDFastOpen(mFileEntry, &fileInfo);
  }

  mSize = fileInfo.length;
  DVDClose(&fileInfo);

  if (filename != decodedName) {
    TryARAMFile();
  }
}

CDvdFile::~CDvdFile() { CloseFile(); }

CDvdRequest* CDvdFile::SyncRead(void* dest, uint len) {
  return AsyncSeekRead(dest, len, kSO_Cur, 0);
}

void CDvdFile::SyncSeekRead(void* dest, uint len, ESeekOrigin origin, int offset) {
  StallForARAMFile();
  CalcFileOffset(offset, origin);

  if (mARAMAllocated) {
    uint roundedLen = (len + 31) & ~31;
    DCFlushRange(dest, roundedLen);
    CARAMManager::WaitForDMACompletion(CARAMManager::DMAToMRAM(
        mARAMBuffer + mOffset, dest, roundedLen, CARAMManager::kDMAPrio_One));
  } else {
    DVDFileInfo info;
#if !RSTL_PRIME1
    if (!sPollingMode) {
#endif

    DVDFastOpen(mFileEntry, &info);
#if !RSTL_PRIME1
    } else {
      DVDOpen(const_cast< char* >(DecodeARAMFile(mFilename.data())), &info);
    }
#endif

    DVDReadAsync(&info, dest, (len + 31) & ~31, mOffset, internalCallback);
#if !RSTL_PRIME1
    gDvdActivity = true;
#endif

    while (DVDGetCommandBlockStatus(&info.cb) != DVD_STATE_END) {
    }
    DVDClose(&info);
  }

  UpdateFilePos(len);
}

CDvdRequest* CDvdFile::AsyncSeekRead(void* dest, uint len, ESeekOrigin origin, int offset) {
  StallForARAMFile();
  CalcFileOffset(offset, origin);
  CDvdRequest* request;
  if (mARAMAllocated) {
    const int roundedLen = (len + 31) & ~31;
    DCFlushRange(dest, roundedLen);
    request = rs_new CARAMDvdRequest(CARAMManager::DMAToMRAM(
        mARAMBuffer + mOffset, dest, roundedLen, CARAMManager::kDMAPrio_One));
  } else {
    CRealDvdRequest* req = rs_new CRealDvdRequest();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    DVDFileInfo* info = req->FileInfo();
#else
    DVDFileInfo* info = &req->FileInfo();
#endif

    DVDFastOpen(mFileEntry, info);
    DVDReadAsync(info, dest, (len + 31) & ~31, mOffset, internalCallback);
#if !RSTL_PRIME1
    gDvdActivity = true;
#endif

    request = req;
  }

  UpdateFilePos(len);

  return request;
}

void CDvdFile::CloseFile() {
  if (!mARAMAllocated) {
    return;
  }

  StallForARAMFile();
  CARAMManager::Free(mARAMBuffer);
}

bool CDvdFile::FileExists(const char* filename) {
  return DVDConvertPathToEntrynum(const_cast< char* >(DecodeARAMFile(filename))) != -1;
}

void CDvdFile::internalCallback(s32 res, DVDFileInfo* info) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= VERSION_GM8E_01
  DCInvalidateRange((void*)info->cb.addr, info->cb.length);
#endif
#else
  if (res != DVD_STATE_CANCELED) {
    DCInvalidateRange(info->cb.addr, info->cb.length);
    gDvdActivity = true;
  }
#endif

}

void CDvdFile::CalcFileOffset(int offset, ESeekOrigin origin) {
  switch (origin) {
  case kSO_Begin:
    mOffset = offset;
    break;
  case kSO_Cur:
    mOffset += offset;
    break;
  case kSO_End:
    mOffset = offset + mSize;
    break;
  }
}

void CDvdFile::UpdateFilePos(int pos) {
  mOffset += (pos + 31) & ~31;
  int filesize = GetFileSize();
  if (mOffset > filesize) {
    mOffset = filesize;
  }
}

