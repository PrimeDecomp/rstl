#include "rstl/RstlVersions.h"

#include "Kyoto/CARAMManager.hpp"
#include "Kyoto/CARAMToken.hpp"
#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/CDvdRequest.hpp"
#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Platform.h"
#include "rstl/math.hpp"
#ifdef ENABLE_BBA_TOOLS
#include "Kyoto/Network/CBBASupport.hpp"
#endif

#include "dolphin/arq.h"
#include "dolphin/dvd.h"
#include "dolphin/os.h"
#include "dolphin/os/OSCache.h"

#include "string.h"

#if defined(TARGET_PC)
#include <borealis/log.hpp>
#include <memory>

namespace {
constexpr borealis::Log Log{"CDvdFile"};
} // namespace
#endif

static CDvdFile* sFirstARAM = nullptr;
// Guessed name. Never set in retail; when set, transfers are driven by polling
// IsARAMFileLoaded instead of from interrupt callbacks.
static RSTL_PRIME1_CONST bool sPollingMode = false;
#if !RSTL_PRIME1
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
    SDvdInfo() : mDvdFile(nullptr), mNextfile(nullptr) {}

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

CHECK_SIZEOF(CDvdFileARAM, 0x94)

const char* DecodeARAMFile(const char* filename) {
  if (!strncmp(filename, "aram:", 5)) {
    return filename + 5;
  }

  return filename;
}

void CDvdFile::DVDARAMXferCallback(s32 result, DVDFileInfo* info) {
#if !defined(TARGET_PC)
  CDvdFileARAM::SDvdInfo* ptr = reinterpret_cast< CDvdFileARAM::SDvdInfo* >(info);
  if (!sPollingMode) {
    DVDClose(&ptr->mDvdFileInfo);
  }

  ptr->mDvdFile->HandleDVDInterrupt();
#endif // !defined(TARGET_PC)
}

void CDvdFile::ARAMARAMXferCallback(uintptr_t addr) {
#if !defined(TARGET_PC)
  reinterpret_cast< CDvdFileARAM* >(addr)->mInfo.mDvdFile->HandleARAMInterrupt();
#endif // !defined(TARGET_PC)
}

void CDvdFile::HandleARAMInterrupt() {
#if !defined(TARGET_PC)
  BOOL enabled = OSDisableInterrupts();
  CDvdFileARAM* arFile = mARAMFile.get();

  arFile->mGotARAMInterrupt = true;

  if (!sPollingMode && arFile->mGotARAMInterrupt && arFile->mGotDvdInterrupt) {
    PingARAMTransfer();
  }

  OSRestoreInterrupts(enabled);
#endif // !defined(TARGET_PC)
}

void CDvdFile::HandleDVDInterrupt() {
  BOOL enabled = OSDisableInterrupts();
  CDvdFileARAM* arFile = mARAMFile.get();

  arFile->mGotDvdInterrupt = true;

  if (!sPollingMode && arFile->mGotARAMInterrupt && arFile->mGotDvdInterrupt) {
    PingARAMTransfer();
  }

  OSRestoreInterrupts(enabled);
}

void CDvdFile::PingARAMTransfer() {
#if !defined(TARGET_PC)
  CDvdFileARAM* aramFile = mARAMFile.get();

  if (aramFile->mBufferLen == 0) {
    PopARAMFileLoad();
    return;
  }

  int length = rstl::min_val(65536, aramFile->mBufferLen);
  ARQPostRequest(&aramFile->mARQRequest, 0, ARQ_TYPE_MRAM_TO_ARAM, ARQ_PRIORITY_HIGH,
                 reinterpret_cast< u32 >(aramFile->mBuffers[aramFile->mBufferIndex].get()),
                 reinterpret_cast< u32 >(mARAMBuffer + aramFile->mAramOffset), length,
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
#ifdef ENABLE_BBA_TOOLS
    if (sPollingMode) {
      int handle;
      if (CBBASupport::BBAOpen(
              (rstl::string_l("DolRoot:") + rstl::string(DecodeARAMFile(mFilename.data()), -1))
                  .c_str(),
              kBOM_Read, &handle) == 0) {
        CBBASupport::BBASeek(handle, aramFile->mFileSize2, kBSO_Start);
        CBBASupport::BBARead(handle, aramFile->mBuffers[aramFile->mBufferIndex].get(), length2);
        CBBASupport::BBAClose(handle);
        DVDARAMXferCallback(0, &aramFile->mInfo.mDvdFileInfo);
      } else {
        RS_VERIFY_THROW(233, false, false,
                        CBasics::Stringize("BBA file %s open failed.\n", mFilename.data()));
      }
    } else
#endif
    {
      DVDFastOpen(mFileEntry, &aramFile->mInfo.mDvdFileInfo);
      DVDReadAsync(&aramFile->mInfo.mDvdFileInfo,
                   aramFile->mBuffers[aramFile->mBufferIndex].get(), length2,
                   aramFile->mFileSize2, DVDARAMXferCallback);
#if !RSTL_PRIME1
      gDvdActivity = true;
#endif
    }
    aramFile->mFileSize2 += length2;
    aramFile->mCurBufferLen -= length2;
#if RSTL_PRIME1
    aramFile->mGotDvdInterrupt = false;
#endif
  }
#endif // !defined(TARGET_PC)
}

void CDvdFile::TryARAMFile() {
#if !defined(TARGET_PC)
  mARAMBuffer = static_cast< uchar* >(CARAMManager::Alloc(mSize));
  if (
#if RSTL_PRIME1
      CARAMManager::GetInvalidAlloc() == mARAMBuffer
#else
      !CARAMManager::IsValidAlloc(mARAMBuffer)
#endif
  ) {
    RS_DEBUGGER_PRINT("WARNING: Couldn't cast %s (size %d) to ARAM\n", mFilename.data(), mSize);
    return;
  }
  mARAMFile = rs_new_line(264) CDvdFileARAM();
  CDvdFileARAM* arfile = mARAMFile.get();
  arfile->mInfo.mDvdFile = this;
  arfile->mGotARAMInterrupt = true;
  arfile->mFileSize1 = arfile->mCurBufferLen = arfile->mBufferLen = GetFileSize();
  mARAMAllocated = true;
  PushARAMFileLoad();
#endif // !defined(TARGET_PC)
}

void CDvdFile::PushARAMFileLoad() {
#if !defined(TARGET_PC)
  BOOL enabled = true;
  if (!sPollingMode) {
    enabled = OSDisableInterrupts();
  }
  CDvdFile* file = sFirstARAM;
  if (file == NULL) {
    sFirstARAM = this;
    StartARAMFileLoad();
  } else {
    for (CDvdFile* p = file; p != nullptr; p = p->mARAMFile->mInfo.mNextfile) {
      CDvdFileARAM* aram = p->mARAMFile.get();
      RS_VERIFY_THROW(306, aram != NULL, false, "ARAM DVDFile list is broken in push!");
      if (aram->mInfo.mNextfile == nullptr) {
        aram->mInfo.mNextfile = this;
        break;
      }
    }
  }
  if (!sPollingMode) {
    OSRestoreInterrupts(enabled);
  }
#endif // !defined(TARGET_PC)
}

void CDvdFile::PopARAMFileLoad() {
#if !defined(TARGET_PC)
  BOOL enabled = true;
  if (!sPollingMode) {
    enabled = OSDisableInterrupts();
  }
  RS_VERIFY_THROW(342, this == sFirstARAM, false, "ARAM DvdFile list is broken in pop!");
  CDvdFile* file = mARAMFile->mInfo.mNextfile;
  mARAMPopped = true;
  sFirstARAM = file;
  if (file != nullptr) {
    file->StartARAMFileLoad();
  }

  if (!sPollingMode) {
    OSRestoreInterrupts(enabled);
  }
#endif // !defined(TARGET_PC)
}

bool CDvdFile::IsARAMFileLoaded() {
#if !defined(TARGET_PC)
  if (!mARAMAllocated) {
    return true;
  }

  if (!mARAMPopped) {
    if (sPollingMode && mARAMFile->mGotARAMInterrupt && mARAMFile->mGotDvdInterrupt) {
      PingARAMTransfer();
    }
    return false;
  }

  mARAMFile = nullptr;
#endif // !defined(TARGET_PC)

  return true;
}

void CDvdFile::StartARAMFileLoad() {
#if !defined(TARGET_PC)
  CDvdFileARAM* aramFile = mARAMFile.get();
  aramFile->mBuffers.push_back(static_cast< uchar* >(rs_aligned_new_line(0x10000, 387)));
  aramFile->mBuffers.push_back(static_cast< uchar* >(rs_aligned_new_line(0x10000, 388)));

  int len = rstl::min_val(mSize, 65536);
  aramFile->mCurBufferLen -= len;
  aramFile->mFileSize2 = len;
  if (!sPollingMode) {
    DVDFastOpen(mFileEntry, &aramFile->mInfo.mDvdFileInfo);
  } else {
    DVDOpen(const_cast< char* >(DecodeARAMFile(mFilename.data())), &aramFile->mInfo.mDvdFileInfo);
  }
  DVDReadAsync(&aramFile->mInfo.mDvdFileInfo, aramFile->mBuffers[0].get(), len, 0,
               DVDARAMXferCallback);
#if !RSTL_PRIME1
  gDvdActivity = true;
#endif
#endif // !defined(TARGET_PC)
}

void CDvdFile::StallForARAMFile() {
#if !defined(TARGET_PC)
  if (mARAMFile.get() != nullptr) {
    RS_DEBUGGER_PRINT("Stalling for ARAM file load %s - BUG\n", mFilename.data());
  }

  while (mARAMFile.get() != nullptr) {
    OSYieldThread();
  }
#endif // !defined(TARGET_PC)
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
#if defined(TARGET_PC)
  DVDFileInfo fileInfo{};
  if (mFileEntry != -1 && DVDFastOpen(mFileEntry, &fileInfo)) {
    mSize = fileInfo.length;
    DVDClose(&fileInfo);
    mARAMAllocated = filename != decodedName;
  }
#else
  DVDFileInfo fileInfo;
  BOOL openOk = FALSE;
  if (mFileEntry != -1) {
    openOk = DVDFastOpen(mFileEntry, &fileInfo);
  }
  RS_VERIFY_THROW(453, openOk == TRUE, kDvdIoException_CantOpenFile,
                  CBasics::Stringize("CDvdHalFile() can not find file : %s", decodedName));

  mSize = fileInfo.length;
  DVDClose(&fileInfo);

  if (filename != decodedName) {
    TryARAMFile();
  }
#endif
}

CDvdFile::~CDvdFile() { CloseFile(); }

void CDvdFile::BlockingRead(void* dest, uint len) { SyncSeekRead(dest, len, kSO_Cur, 0); }

CDvdRequest* CDvdFile::SyncRead(void* dest, uint len) {
  return AsyncSeekRead(dest, len, kSO_Cur, 0);
}

void CDvdFile::SyncSeekRead(void* dest, uint len, ESeekOrigin origin, int offset) {
#if defined(TARGET_PC)
  std::unique_ptr< CDvdRequest > request{AsyncSeekRead(dest, len, origin, offset)};
  request->WaitUntilComplete();
#else
  StallForARAMFile();
  CalcFileOffset(offset, origin);

  if (mARAMAllocated) {
    uint roundedLen = (len + 31) & ~31;
    DCFlushRange(dest, roundedLen);
    CARAMManager::WaitForDMACompletion(CARAMManager::DMAToMRAM(
        mARAMBuffer + mOffset, dest, roundedLen, CARAMManager::kDMAPrio_One));
  } else {
    DVDFileInfo info;
    if (!sPollingMode) {
      DVDFastOpen(mFileEntry, &info);
    } else {
      DVDOpen(const_cast< char* >(DecodeARAMFile(mFilename.data())), &info);
    }
    DVDReadAsync(&info, dest, (len + 31) & ~31, mOffset, internalCallback);
#if !RSTL_PRIME1
    gDvdActivity = true;
#endif
    while (DVDGetCommandBlockStatus(&info.cb) != DVD_STATE_END) {
    }
    DVDClose(&info);
  }

  UpdateFilePos(len);
#endif
}

CDvdRequest* CDvdFile::AsyncSeekRead(void* dest, uint len, ESeekOrigin origin, int offset) {
#if defined(TARGET_PC)
  CalcFileOffset(offset, origin);
  auto request = std::make_unique< CRealDvdRequest >();
  DVDFileInfo* info = &request->FileInfo();
  if (mFileEntry == -1 || !DVDFastOpen(mFileEntry, info)) {
    Log.fatal("Unable to open DVD file");
  }
  if (!DVDReadAsync(info, dest, len, mOffset, internalCallback)) {
    Log.fatal("Unable to start DVD read");
  }
  UpdateFilePos(len);
  return request.release();
#else
  StallForARAMFile();
  CalcFileOffset(offset, origin);
  CDvdRequest* request = nullptr;
  if (mARAMAllocated) {
    const int roundedLen = (len + 31) & ~31;
    DCFlushRange(dest, roundedLen);
    request = rs_new_line(574) CARAMDvdRequest(CARAMManager::DMAToMRAM(
        mARAMBuffer + mOffset, dest, roundedLen, CARAMManager::kDMAPrio_One));
  }
#ifdef ENABLE_BBA_TOOLS
  else if (sPollingMode) {
    int handle;
    if (CBBASupport::BBAOpen(
            (rstl::string_l("DolRoot:") + rstl::string(DecodeARAMFile(mFilename.data()), -1))
                .c_str(),
            kBOM_Read, &handle) == 0) {
      CBBASupport::BBASeek(handle, mOffset, kBSO_Start);
      CBBASupport::BBARead(handle, dest, len);
      CBBASupport::BBAClose(handle);
      request = rs_new_line(590) CDummyDvdRequest();
    } else {
      RS_VERIFY_THROW(594, false, false,
                      CBasics::Stringize("BBA file %s open failed.\n", mFilename.data()));
    }
  }
#endif
  else {
    CRealDvdRequest* req = rs_new_line(600) CRealDvdRequest();
    DVDFileInfo* info = &req->FileInfo();
    DVDFastOpen(mFileEntry, info);
    DVDReadAsync(info, dest, (len + 31) & ~31, mOffset, internalCallback);
#if !RSTL_PRIME1
    gDvdActivity = true;
#endif
    request = req;
  }

  UpdateFilePos(len);

  return request;
#endif
}

void CDvdFile::CloseFile() {
  if (!mARAMAllocated || IS_TARGET_PC) {
    return;
  }

  StallForARAMFile();
  IF_NOT_TARGET_PC(CARAMManager::Free(mARAMBuffer));
}

bool CDvdFile::FileExists(const char* filename) {
  return DVDConvertPathToEntrynum(const_cast< char* >(DecodeARAMFile(filename))) != -1;
}

void CDvdFile::internalCallback(s32 res, DVDFileInfo* info) {
#if RSTL_PRIME1
#if RSTL_VERSION >= RSTL_GM8E_01
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
