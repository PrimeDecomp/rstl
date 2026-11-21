#include "rstl/RstlVersions.h"
#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/CMemoryCardSys.hpp"
#include "rstl/RstlVersions.h"

#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CCrc32.hpp"
#include "Kyoto/Graphics/CGraphicsPalette.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/MemoryCopy.hpp"

#include "dolphin/os/OSCache.h"

#if RSTL_CORRUPTION
#include "Kyoto/MemoryCopy.hpp"
#else
static inline uint CalculateCrc(const void* data, uint length) {
  return CCRC32::Calculate(data, length IF_NOT_RSTL_PRIME1(, 0xFFFFFFFF));
}
#endif

#if RSTL_CORRUPTION


static uint RoundUp(uint value, uint align) { return (value + align - 1) & ~(align - 1); } // Guessed name

CMemoryCardSys::CCardFileInfo::Icon::Icon(CAssetId id, int speed, CSimplePool& pool)
: mId(id), mSpeed(speed), mTex(pool.GetObj(SObjectTag('TXTR', id))) {
  const CTexture* pTexture = *mTex;
  const short width = pTexture->GetWidth();
  const short height = pTexture->GetHeight();
  RS_VERIFY_THROW(
      167, width == 32 && height == 32, false,
      CBasics::Stringize("Need icons size 32x32, got %dx%d for icon %x", width, height, mId));

  const ETexelFormat texFormat = pTexture->GetTexelFormat();
  RS_VERIFY_THROW(174, texFormat == kTexelFormat_RGB5A3 || texFormat == kTexelFormat_CI8, false,
                  CBasics::Stringize("Invalid texel format for icon texture (%d).", texFormat));

  if (texFormat == kTexelFormat_CI8) {
    const CGraphicsPalette* pPalette = pTexture->GetPalette();
    RS_VERIFY_THROW_DIRECT(178, pPalette->GetFormat() == kPaletteFormat_RGB5A3, false,
                           CBasics::Stringize("Invalid palette format for icon texture (%d).",
                                              pPalette->GetFormat()));
#else
ECardResult SMemoryCardFileInfo::FileRead() {
  rstl::vector< uchar >& saveData = mSaveData;
  saveData = rstl::vector< uchar >();
  const uint size = mSaveFileData.size();
  const void* data = mSaveFileData.data();
  const uint crc = BE_SWAP(*static_cast< const uint* >(data));
  if (crc == CalculateCrc(static_cast< const uchar* >(data) + 4, size - 4)) {
    uint offset;
    ECardResult result = GetSaveDataOffset(offset);
    if (result != kCR_READY) {
      mSaveFileData = rstl::vector< uchar, rstl::aligned_allocator >();
      return result;
    }
    const uint saveSize = size - offset;
    saveData.assign(saveSize);
    (memcpy)(saveData.data(), static_cast< const uchar* >(data) + offset, saveSize);
    mSaveFileData = rstl::vector< uchar, rstl::aligned_allocator >();
    return kCR_READY;
  } else {
    mSaveFileData = rstl::vector< uchar, rstl::aligned_allocator >();
    return kCR_CRC_MISMATCH;
#endif
  }
}

#if RSTL_CORRUPTION
CMemoryCardSys::CCardFileInfo::SSaveSlot::SSaveSlot() : mGeneration(0), mCorrupt(false) {}
#else
static int RoundUpCardSize(int size, int align) { return (size + align - 1) & ~(align - 1); }
#endif

#if RSTL_CORRUPTION
void CMemoryCardSys::CCardFileInfo::SSaveSlot::CheckCrc() {
  const uint* data = reinterpret_cast< const uint* >(mData.data());
  const uint crc = data[0];
  mCorrupt = CCRC32::Calculate(data + 1, mData.size() - 4, 0xFFFFFFFF) != crc;
  mGeneration = data[1];
#else
CMemoryCardSys::CCardFileInfo::Icon::Icon(CAssetId id, int speed, CSimplePool& pool)
: mId(id), mSpeed(speed), mTex(pool.GetObj(SObjectTag('TXTR', id))) {}

CMemoryCardSys::EMemoryCardPort SMemoryCardFileInfo::GetFileCardPort() {
  return static_cast< CMemoryCardSys::EMemoryCardPort >(mFileInfo.chan);
#endif
}

#if RSTL_CORRUPTION
CMemoryCardSys::CCardFileInfo::CCardFileInfo(EMemoryCardPort port, const rstl::string& name)
: mSlotBlocks(0)
, mAlignedUserDataSize(0)
, mSlot(0)
, mGeneration(0)
, mState(kS_Standby)
, mNewFile(false)
, mFileName(name)
, x4c_(0)
, mBannerTex(kInvalidAssetId)
, mHeaderBuffer(0x2000, uchar())
, mSlots(2, SSaveSlot()) {
  mFileInfo.chan = port;
  mFileInfo.fileNo = -1;
}

CMemoryCardSys::CCardFileInfo::~CCardFileInfo() {
  if (mFileInfo.fileNo != -1) {
    CARDClose(&mFileInfo);
  }
}

ECardResult CMemoryCardSys::CCardFileInfo::Open() {
  const EMemoryCardPort port = GetPort();
  ECardResult result = static_cast< ECardResult >(CARDOpen(port, mFileName.data(), &mFileInfo));
  mFileInfo.chan = port;
  if (result == kCR_READY) {
    CardStat stat;
    ECardResult statResult = CMemoryCardSys::GetStatus(GetPort(), GetFileNo(), stat);
    if (statResult != kCR_READY) {
      rs_debugger_printf("Could not get file status: error code %d.  Possible programmer error?\n",
                         statResult);
      return statResult;
    }
    const int blocks = stat.GetFileLength() / 0x2000;
    mSlotBlocks = (blocks - 1) / 2;
    mAlignedUserDataSize = mSlotBlocks * 0x2000;
  }
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::CreateFileSync() {
  mNewFile = true;
  mAlignedUserDataSize = RoundUp(mUserDataWrite.size(), kSupportedSectorSize);
  mSlotBlocks = mAlignedUserDataSize / kSupportedSectorSize;
  const int size = GetTotalNumBlocks() * kSupportedSectorSize;
  return static_cast< ECardResult >(CARDCreate(GetPort(), mFileName.data(), size, &mFileInfo));
}

ECardResult CMemoryCardSys::CCardFileInfo::CreateFile() {
  mNewFile = true;
  mAlignedUserDataSize = RoundUp(mUserDataWrite.size(), kSupportedSectorSize);
  mSlotBlocks = mAlignedUserDataSize / kSupportedSectorSize;
  const int size = GetTotalNumBlocks() * kSupportedSectorSize;
  return static_cast< ECardResult >(
      CARDCreateAsync(GetPort(), mFileName.data(), size, &mFileInfo, nullptr));
}

ECardResult CMemoryCardSys::CCardFileInfo::Close() {
  const EMemoryCardPort port = GetPort();
  ECardResult result = static_cast< ECardResult >(CARDClose(&mFileInfo));
  mFileInfo.chan = port;
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::ReadFileSync() {
  RS_VERIFY_THROW(299, GetPort() != kCardPort_Invalid, false,
                  "Tried to read a file without opening it first");

  CardStat stat;
  ECardResult result = CMemoryCardSys::GetStatus(GetPort(), GetFileNo(), stat);
  if (result != kCR_READY) {
    rs_debugger_printf("Could not get file status: error code %d.  Possible programmer error?\n",
                       result);
    return result;
  }

  const uint fileLength = stat.GetFileLength();
  RS_VERIFY_THROW(311, fileLength == CMemoryCardSys::kSupportedSectorSize * GetTotalNumBlocks(),
                  false, "Unexpected file length on opened file!");

  mLoadedData = rstl::vector< uchar >();
  mHeaderBuffer.assign(kSupportedSectorSize, 0);
  mSlots[0].mData.assign(mAlignedUserDataSize, 0);
  mSlots[1].mData.assign(mAlignedUserDataSize, 0);

  result = static_cast< ECardResult >(
      CARDRead(&mFileInfo, mHeaderBuffer.data(), kSupportedSectorSize, 0));
  if (result != kCR_READY) {
    return result;
  }

  CheckHeaderCrc();
  mHeaderBuffer = rstl::vector< uchar, rstl::aligned_allocator >();
  result = static_cast< ECardResult >(
      CARDRead(&mFileInfo, mSlots.data()[0].mData.data(), mAlignedUserDataSize, kSupportedSectorSize));
  if (result != kCR_READY) {
    return result;
  }

  result =
      static_cast< ECardResult >(CARDRead(&mFileInfo, mSlots.data()[1].mData.data(), mAlignedUserDataSize,
                                          mAlignedUserDataSize + kSupportedSectorSize));
  if (result != kCR_READY) {
    return result;
  }

  return ExtractUserData();
}

ECardResult CMemoryCardSys::CCardFileInfo::ReadSelfAsync() {
  RS_VERIFY_THROW(360, GetPort() != kCardPort_Invalid, false,
                  "Tried to read a file without opening it first");

  CardStat stat;
  ECardResult result = CMemoryCardSys::GetStatus(GetPort(), GetFileNo(), stat);
  if (result != kCR_READY) {
    rs_debugger_printf("Could not get file status: error code %d.  Possible programmer error?\n",
                       result);
    return result;
  }

  const uint fileLength = stat.GetFileLength();
  RS_VERIFY_THROW(371, fileLength == CMemoryCardSys::kSupportedSectorSize * GetTotalNumBlocks(),
                  false, "Unexpected file length on opened file!");

  mLoadedData = rstl::vector< uchar >();
  mHeaderBuffer.assign(kSupportedSectorSize, 0);
  mSlots[0].mData.assign(mAlignedUserDataSize, 0);
  mSlots[1].mData.assign(mAlignedUserDataSize, 0);

  result = static_cast< ECardResult >(
      CARDRead(&mFileInfo, mHeaderBuffer.data(), kSupportedSectorSize, 0));
  if (result == kCR_READY) {
    rs_debugger_printf("ReadSelfAsync: Reading System Data.\n");
    mState = kS_ReadHeader;
  }
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::TryCompleteRead() {
  if (mState == kS_Standby) {
    return kCR_READY;
  }
  ECardResult result = GetResultCode(GetPort());
  if (result != kCR_READY) {
    return result;
  }
  if (mState == kS_ReadHeader || mState == kS_RepairHeader) {
    if (mState == kS_ReadHeader && CheckHeaderCrc() != kCR_READY) {
      rs_debugger_printf("TryCompleteRead: Repairing corrupt system data.\n");
      BuildHeaderBuffer();
      mState = kS_RepairHeader;
      void* data = mHeaderBuffer.data();
      DCStoreRange(data, kSupportedSectorSize);
      result = static_cast< ECardResult >(
          CARDWriteAsync(&mFileInfo, data, kSupportedSectorSize, 0, nullptr));
      if (result == kCR_READY) {
        return kCR_BUSY;
      }
      return result;
    } else {
      mHeaderBuffer = rstl::vector< uchar, rstl::aligned_allocator >();
      rs_debugger_printf("TryCompleteRead: Reading First User Data.\n");
      mState = kS_ReadSlotA;
      result = static_cast< ECardResult >(
          CARDRead(&mFileInfo, mSlots[0].mData.data(), mAlignedUserDataSize, kSupportedSectorSize));
    if (result == kCR_READY) {
      return kCR_BUSY;
    }
    return result;
  }
  } else if (mState == kS_ReadSlotA) {
    rs_debugger_printf("TryCompleteRead: Reading Second User Data.\n");
    mState = kState_ReadingData1;
    result = static_cast< ECardResult >(CARDRead(&mFileInfo, mSlots[1].mData.data(),
                                                 mAlignedUserDataSize,
                                                 mAlignedUserDataSize + kSupportedSectorSize));
    if (result == kCR_READY) {
      return kCR_BUSY;
  }
  return result;
  } else {
    RS_VERIFY_THROW_DIRECT(476, mState == kState_ReadingData1, false,
                           CBasics::Stringize("Unexpected state %d", mState));
    return ExtractUserData();
  }
}

ECardResult CMemoryCardSys::CCardFileInfo::WriteFileSync() {
  RS_VERIFY_THROW(486, GetPort() != kCardPort_Invalid, false,
                  "Tried to read a file without opening it first");

  BuildWriteBuffers();
  if (mNewFile) {
    BuildHeaderBuffer();
    void* header = mHeaderBuffer.data();
    DCStoreRange(header, kSupportedSectorSize);
    ECardResult headerResult =
        static_cast< ECardResult >(CARDWrite(&mFileInfo, header, kSupportedSectorSize, 0));
    if (headerResult != kCR_READY) {
      return headerResult;
    }
  }

  const int offset = mSlot * mAlignedUserDataSize + kSupportedSectorSize;
  void* data = mSlots[mSlot].mData.data();
  DCStoreRange(data, mAlignedUserDataSize);
  ECardResult result =
      static_cast< ECardResult >(CARDWrite(&mFileInfo, data, mAlignedUserDataSize, offset));
  mSlots[mSlot].mData = rstl::vector< uchar, rstl::aligned_allocator >();
  if (result == kCR_READY) {
    CardStat stat;
    ECardResult statusResult = GetStatus(stat);
    if (statusResult != kCR_READY) {
      return statusResult;
    }
    result = SetStatusSync(GetPort(), GetFileNo(), stat);
    if (result != kCR_READY) {
      rs_debugger_printf("Could not set file status: error code %d.  Possible programmer error?\n",
                         result);
    }
    return result;
  } else {
    return result;
  }
}

ECardResult CMemoryCardSys::CCardFileInfo::WriteSaveSlot(int slot) {
  const int offset = slot * mAlignedUserDataSize + 0x2000;
  void* data = mSlots[mSlot].mData.data();
  DCStoreRange(data, mAlignedUserDataSize);
  return static_cast< ECardResult >(
      CARDWriteAsync(&mFileInfo, data, mAlignedUserDataSize, offset, nullptr));
}

ECardResult CMemoryCardSys::CCardFileInfo::WriteSelfAsync() {
  RS_VERIFY_THROW(549, GetPort() != kCardPort_Invalid, false,
                  "Tried to read a file without opening it first");

  ECardResult result;
  BuildWriteBuffers();
  if (mNewFile) {
    BuildHeaderBuffer();
    void* data = mHeaderBuffer.data();
    DCStoreRange(data, kSupportedSectorSize);
    result = static_cast< ECardResult >(
        CARDWriteAsync(&mFileInfo, data, kSupportedSectorSize, 0, nullptr));
    mState = kS_WriteHeader;
    rs_debugger_printf("WriteSelfAsync: Writing System Data.\n");
    return result;
  } else {
    result = WriteSaveSlot(mSlot);
    rs_debugger_printf("WriteSelfAsync: Writing User Data.\n");
    mState = kS_WriteSlot;
    return result;
  }
}

ECardResult CMemoryCardSys::CCardFileInfo::TryCompleteWrite() {
  if (mState == kS_Standby) {
    return kCR_READY;
  }
  ECardResult result = GetResultCode(GetPort());
  if (result != kCR_READY) {
    return result;
  }
  if (mState == kS_WriteHeader) {
    mState = mNewFile ? kS_WriteFirstSlot : kS_WriteSlot;
    result = WriteSaveSlot(mSlot);
    rs_debugger_printf("TryCompleteWrite: Writing User Data.\n");
    if (result == kCR_READY) {
      return kCR_BUSY;
    }
    return result;
  } else if (mState == kS_WriteFirstSlot) {
    mState = kS_WriteSlot;
    const ECardResult result = WriteSaveSlot(1 - mSlot);
    rs_debugger_printf("TryCompleteWrite: Writing initial backup user Data.\n");
    if (result == kCR_READY) {
      return kCR_BUSY;
    }
    return result;
  } else if (mState == kS_WriteSlot) {
    mState = kState_WritingStatus;
    rs_debugger_printf("TryCompleteWrite: Writing Status.\n");
    CardStat stat;
    result = GetStatus(stat);
    if (result != kCR_READY) {
      return result;
    }
    result = SetStatus(GetPort(), GetFileNo(), stat);
    if (result == kCR_READY) {
      return kCR_BUSY;
    }
    rs_debugger_printf("Could not set file status: error code %d.  Possible programmer error?\n",
                       result);
    return result;
  } else {
    RS_VERIFY_THROW_DIRECT(689, mState == kState_WritingStatus, false,
                           CBasics::Stringize("Unexpected state %d", mState));
    mState = kS_Standby;
  return kCR_READY;
}
}
#else
int SMemoryCardFileInfo::GetFileNo() const { return mFileInfo.fileNo; }

CMemoryCardSys::CCardFileInfo::SSaveBlock::SSaveBlock() : mSequence(0), mCorrupted(false) {}

void CMemoryCardSys::CCardFileInfo::SSaveBlock::Check() {
  const uint* data = reinterpret_cast< const uint* >(mData.data());
  const uint crc = BE_SWAP(data[0]);
  mCorrupted = crc != CalculateCrc(data + 1, mData.size() - 4);
  mSequence = BE_SWAP(data[1]);
}

#if RSTL_CARD_SAVE_BLOCKS
CMemoryCardSys::CCardFileInfo::CCardFileInfo(EMemoryCardPort port, const rstl::string& name)
: mBlockCount(0)
, mBlockSize(0)
, mWriteBlock(0)
, mSequence(0)
, mStatus(kS_Standby)
, mWriteBothBlocks(false)
, mFileName(name)
, x38_(0)
, mBannerTex(kInvalidAssetId)
, mHeaderBuffer(8192, static_cast< uchar >(0), rstl::aligned_allocator())
, mSaveBlocks(2, SSaveBlock()) {
  mFileInfo.chan = port;
  mFileInfo.fileNo = -1;
}

CMemoryCardSys::CCardFileInfo::~CCardFileInfo() {
  if (mFileInfo.fileNo != -1) {
    CARDClose(&mFileInfo);
  }
}

ECardResult CMemoryCardSys::CCardFileInfo::Open() {
  const EMemoryCardPort port = GetPort();
  ECardResult result = static_cast< ECardResult >(CARDOpen(port, mFileName.data(), &mFileInfo));
  mFileInfo.chan = port;
  if (result == kCR_READY) {
    CardStat stat;
    ECardResult statusResult = CMemoryCardSys::GetStatus(GetPort(), GetFileNo(), stat);
    if (statusResult != kCR_READY) {
      return statusResult;
    }
    mBlockCount = static_cast< int >((static_cast< uint >(stat.GetFileLength()) >> 13) - 1) / 2;
    mBlockSize = mBlockCount * 8192;
  }
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::CreateFile() {
  mWriteBothBlocks = true;
  mBlockSize = RoundUpCardSize(mCardBuffer.size(), 8192);
  mBlockCount = mBlockSize / 8192;
  const uint size = GetTotalBlockCount() * 8192;
  return static_cast< ECardResult >(
      CARDCreateAsync(GetPort(), mFileName.data(), size, &mFileInfo, nullptr));
}

ECardResult CMemoryCardSys::CCardFileInfo::CloseFile() {
  const EMemoryCardPort port = GetPort();
  ECardResult result = static_cast< ECardResult >(CARDClose(&mFileInfo));
  mFileInfo.chan = port;
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::FileRead() {
  CardStat stat;
  ECardResult result = CMemoryCardSys::GetStatus(GetPort(), GetFileNo(), stat);
  if (result != kCR_READY) {
    return result;
  }
  const uint fileSize = stat.GetFileLength();
  mSaveBuffer = rstl::vector< uchar >();
  mHeaderBuffer.assign(8192);
  mSaveBlocks[0].mData.assign(mBlockSize);
  SSaveBlock& second = mSaveBlocks[1];
  second.mData.assign(mBlockSize);
  result = static_cast< ECardResult >(CARDRead(&mFileInfo, mHeaderBuffer.data(), 8192, 0));
  if (result != kCR_READY) {
    return result;
  }
  CheckHeader();
  mHeaderBuffer = rstl::vector< uchar, rstl::aligned_allocator >();
  result = static_cast< ECardResult >(
      CARDRead(&mFileInfo, mSaveBlocks.front().mData.data(), mBlockSize, 8192));
  if (result != kCR_READY) {
    return result;
  }
  result = static_cast< ECardResult >(
      CARDRead(&mFileInfo, second.mData.data(), mBlockSize, mBlockSize + 8192));
  if (result != kCR_READY) {
    return result;
  }
  return FinishRead();
}

ECardResult CMemoryCardSys::CCardFileInfo::StartRead() {
  CardStat stat;
  ECardResult result = CMemoryCardSys::GetStatus(GetPort(), GetFileNo(), stat);
  if (result != kCR_READY) {
    return result;
  }
  const uint fileSize = stat.GetFileLength();
  mSaveBuffer = rstl::vector< uchar >();
  mHeaderBuffer.assign(8192);
  mSaveBlocks[0].mData.assign(mBlockSize);
  mSaveBlocks[1].mData.assign(mBlockSize);
  result = static_cast< ECardResult >(CARDRead(&mFileInfo, mHeaderBuffer.data(), 8192, 0));
  if (result == kCR_READY) {
    mStatus = kS_Transferring;
  }
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::TryFileRead() {
  if (mStatus == kS_Standby) {
    return kCR_READY;
  }
  ECardResult result = GetResultCode(GetPort());
  if (result != kCR_READY) {
    return result;
  }
  if (mStatus == kS_Transferring || mStatus == kS_Done) {
    if (mStatus == kS_Transferring && CheckHeader() != kCR_READY) {
      BuildHeader();
      mStatus = kS_Done;
      void* data = mHeaderBuffer.data();
      DCStoreRange(data, 8192);
      result = static_cast< ECardResult >(CARDWriteAsync(&mFileInfo, data, 8192, 0, nullptr));
      if (result == kCR_READY) {
        return kCR_BUSY;
      }
      return result;
    } else {
      mHeaderBuffer = rstl::vector< uchar, rstl::aligned_allocator >();
      mStatus = kS_ReadingFirstBlock;
      result = static_cast< ECardResult >(
          CARDRead(&mFileInfo, mSaveBlocks[0].mData.data(), mBlockSize, 8192));
    }
    if (result == kCR_READY) {
      return kCR_BUSY;
    }
    return result;
  } else if (mStatus == kS_ReadingFirstBlock) {
    mStatus = kS_ReadingSecondBlock;
    result = static_cast< ECardResult >(
        CARDRead(&mFileInfo, mSaveBlocks[1].mData.data(), mBlockSize, mBlockSize + 8192));
    if (result == kCR_READY) {
      return kCR_BUSY;
    }
    return result;
  }
  return FinishRead();
}

ECardResult CMemoryCardSys::CCardFileInfo::WriteBlock(int index) {
  const int offset = index * mBlockSize + 8192;
  void* data = mSaveBlocks[mWriteBlock].mData.data();
  DCStoreRange(data, mBlockSize);
  return static_cast< ECardResult >(CARDWriteAsync(&mFileInfo, data, mBlockSize, offset, nullptr));
}

ECardResult CMemoryCardSys::CCardFileInfo::WriteFile() {
  BuildSaveBlock();
  ECardResult result;
  if (mWriteBothBlocks) {
    BuildHeader();
    void* data = mHeaderBuffer.data();
    DCStoreRange(data, 8192);
    result = static_cast< ECardResult >(CARDWriteAsync(&mFileInfo, data, 8192, 0, nullptr));
    mStatus = kS_WritingHeader;
  } else {
    result = WriteBlock(mWriteBlock);
    mStatus = kS_WritingBlock;
  }
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::PumpCardTransfer() {
  if (mStatus == kS_Standby) {
    return kCR_READY;
  }
  ECardResult result = GetResultCode(GetPort());
  if (result != kCR_READY) {
    return result;
  }
  if (mStatus == kS_WritingHeader) {
    mStatus = mWriteBothBlocks ? kS_WritingBothBlocks : kS_WritingBlock;
    result = WriteBlock(mWriteBlock);
    if (result == kCR_READY) {
      return kCR_BUSY;
    }
    return result;
  } else if (mStatus == kS_WritingBothBlocks) {
    mStatus = kS_WritingBlock;
    result = WriteBlock(1 - mWriteBlock);
    if (result == kCR_READY) {
      return kCR_BUSY;
    }
    return result;
  } else if (mStatus == kS_WritingBlock) {
    mStatus = kS_SettingStatus;
    CardStat stat;
    result = GetStatus(stat);
    if (result != kCR_READY) {
      return result;
    }
    result = SetStatus(GetPort(), GetFileNo(), stat);
    if (result == kCR_READY) {
      return kCR_BUSY;
    }
    return result;
  }
  mStatus = kS_Standby;
  return kCR_READY;
}
#endif
#endif

CMemoryCardSys::EMemoryCardPort CMemoryCardSys::CCardFileInfo::GetPort() {
  return static_cast< EMemoryCardPort >(mFileInfo.chan);
}

int CMemoryCardSys::CCardFileInfo::GetFileNo() { return mFileInfo.fileNo; }

#if RSTL_CORRUPTION
int CMemoryCardSys::CCardFileInfo::GetTotalNumBlocks() { return mSlotBlocks * 2 + 1; }
#else
ECardResult SMemoryCardFileInfo::GetSaveDataOffset(uint& offOut) {
  CardStat stat;
  ECardResult result = CMemoryCardSys::GetStatus(GetFileCardPort(), GetFileNo(), stat);
  if (result != kCR_READY) {
    offOut = -1;
    return result;
  }
  offOut = 4;
  offOut += 64;
  const int bannerFormat = stat.GetBannerFormat();
  if (bannerFormat == CARD_STAT_BANNER_C8) {
    offOut += 3584;
  } else if (bannerFormat == CARD_STAT_BANNER_RGB5A3) {
    offOut += 6144;
  }
  bool palette = false;
  int idx = 0;
  int format = stat.GetIconFormat(idx);
  while (format != CARD_STAT_ICON_NONE) {
    if (format == CARD_STAT_ICON_C8) {
      palette = true;
      offOut += 1024;
    } else {
      offOut += 2048;
    }
    ++idx;
    format = stat.GetIconFormat(idx);
  }
  if (palette) {
    offOut += 512;
  }
  return kCR_READY;
}
#endif

#if RSTL_CORRUPTION
void CMemoryCardSys::CCardFileInfo::ResetHeaderInfo() {
#else
#if RSTL_CARD_SAVE_BLOCKS
int CMemoryCardSys::CCardFileInfo::GetTotalBlockCount() { return 2 * mBlockCount + 1; }
#endif

void CMemoryCardSys::CCardFileInfo::ResetBannerAndIcons() {
#endif
  mComment = rstl::string();
  mBannerTex = kInvalidAssetId;
  mBannerTok = rstl::optional_object< TLockedToken< CTexture > >();
  mIconToks = rstl::reserved_vector< Icon, 8 >();
}

#if RSTL_CORRUPTION
void CMemoryCardSys::CCardFileInfo::SetComment(const rstl::string& comment) { mComment = comment; }

void CMemoryCardSys::CCardFileInfo::LockBannerToken(CAssetId bannerTxtr, CSimplePool& pool) {
  mBannerTex = bannerTxtr;
  mBannerTok = TLockedToken< CTexture >(pool.GetObj(SObjectTag('TXTR', mBannerTex)));

  const CTexture* pTexture = *mBannerTok.data();
  const short width = pTexture->GetWidth();
  const short height = pTexture->GetHeight();
  RS_VERIFY_THROW(746, width == 96 && height == 32, false,
                  CBasics::Stringize("Need banner size 96x32, got %dx%d for banner %x", width,
                                     height, mBannerTex));

  const ETexelFormat texFormat = pTexture->GetTexelFormat();
  RS_VERIFY_THROW(753, texFormat == kTexelFormat_RGB5A3 || texFormat == kTexelFormat_CI8, false,
                  CBasics::Stringize("Invalid texel format for banner texture (%d).", texFormat));

  if (texFormat == kTexelFormat_CI8) {
    const CGraphicsPalette* pPalette = pTexture->GetPalette();
    RS_VERIFY_THROW_DIRECT(757, pPalette->GetFormat() == kPaletteFormat_RGB5A3, false,
                           CBasics::Stringize("Invalid palette format for banner texture (%d).",
                                              pPalette->GetFormat()));
  }
}

void CMemoryCardSys::CCardFileInfo::LockIconToken(CAssetId iconTxtr, int speed, CSimplePool& pool) {
  mIconToks.push_back(Icon(iconTxtr, speed, pool));
}

ECardResult CMemoryCardSys::CCardFileInfo::GetStatus(CardStat& stat) {
  ECardResult result = CMemoryCardSys::GetStatus(GetPort(), GetFileNo(), stat);
  if (result != kCR_READY) {
    rs_debugger_printf("Could not get file status: error code %d.  Possible programmer error?\n",
                       result);
    return result;
  }
  stat.SetCommentAddr(4);
  stat.SetIconAddr(68);
  int bannerFormat;
  if (mBannerTex == kInvalidAssetId) {
    bannerFormat = CARD_STAT_BANNER_NONE;
  } else if ((*mBannerTok.data())->GetTexelFormat() == kTexelFormat_RGB5A3) {
    bannerFormat = CARD_STAT_BANNER_RGB5A3;
  } else {
    bannerFormat = CARD_STAT_BANNER_C8;
  }
  stat.SetBannerFormat(bannerFormat);
  const int count = mIconToks.size();
  int i = 0;
  for (; i < count; ++i) {
    int format = CARD_STAT_ICON_C8;
    if ((*mIconToks[i].mTex)->GetTexelFormat() == kTexelFormat_RGB5A3) {
      format = CARD_STAT_ICON_RGB5A3;
    }
    stat.SetIconFormat(format, i);
    stat.SetIconSpeed(mIconToks[i].mSpeed, i);
  }
  if (i < 8) {
    stat.SetIconFormat(CARD_STAT_ICON_NONE, i);
    stat.SetIconSpeed(CARD_STAT_SPEED_END, i);
  }
  return kCR_READY;
}
#else
#if RSTL_CARD_SAVE_BLOCKS
void CMemoryCardSys::CCardFileInfo::SetComment(const rstl::string& comment) { mComment = comment; }

void CMemoryCardSys::CCardFileInfo::LockBannerToken(CAssetId bannerTxtr, CSimplePool& pool) {
  mBannerTex = bannerTxtr;
  mBannerTok = TLockedToken< CTexture >(pool.GetObj(SObjectTag('TXTR', mBannerTex)));
}

void CMemoryCardSys::CCardFileInfo::LockIconToken(CAssetId iconTxtr, int speed, CSimplePool& pool) {
  mIconToks.push_back(Icon(iconTxtr, speed, pool));
}

ECardResult CMemoryCardSys::CCardFileInfo::GetStatus(CardStat& stat) {
  ECardResult result = CMemoryCardSys::GetStatus(GetPort(), GetFileNo(), stat);
  if (result != kCR_READY) {
    return result;
  }
  stat.SetCommentAddr(4);
  stat.SetIconAddr(68);
  int bannerFormat;
  if (mBannerTex == kInvalidAssetId) {
    bannerFormat = CARD_STAT_BANNER_NONE;
  } else if ((*mBannerTok)->GetTexelFormat() == kTexelFormat_RGB5A3) {
    bannerFormat = CARD_STAT_BANNER_RGB5A3;
  } else {
    bannerFormat = CARD_STAT_BANNER_C8;
  }
  stat.SetBannerFormat(bannerFormat);
  int i = 0;
#if RSTL_PRIME1
  for (; i < mIconToks.size(); ++i) {
#else
  const int count = mIconToks.size();
  for (; i < count; ++i) {
#endif
    int format = CARD_STAT_ICON_C8;
    if (mIconToks[i].mTex->GetTexelFormat() == kTexelFormat_RGB5A3) {
      format = CARD_STAT_ICON_RGB5A3;
    }
    stat.SetIconFormat(format, i);
    stat.SetIconSpeed(mIconToks[i].mSpeed, i);
  }
  if (i < 8) {
    stat.SetIconFormat(CARD_STAT_ICON_NONE, i);
    stat.SetIconSpeed(CARD_STAT_SPEED_END, i);
  }
  return kCR_READY;
}
#endif
#endif

#if RSTL_CORRUPTION
void CMemoryCardSys::CCardFileInfo::WriteBannerData(COutputStream& out) const {
#else
void CMemoryCardSys::CCardFileInfo::WriteBannerData(COutputStream& out) {
#endif
  if (mBannerTex != kInvalidAssetId) {
#if RSTL_CORRUPTION
    const CTexture& texture = **mBannerTok.data();
#else
    const CTexture& texture = ***mBannerTok;
#endif
    const ETexelFormat format = texture.GetTexelFormat();
    const void* data = texture.GetConstBitMapData(0);
    const uint size = format == kTexelFormat_RGB5A3 ? 6144 : 3072;
    out.Put(data, size);
    if (format == kTexelFormat_CI8) {
      out.Put(texture.GetPalette()->GetPaletteData(), 512);
    }
  }
}

#if RSTL_CORRUPTION
void CMemoryCardSys::CCardFileInfo::WriteIconData(COutputStream& out) const {
#else
void CMemoryCardSys::CCardFileInfo::WriteIconData(COutputStream& out) {
#endif
  const void* palette = nullptr;
#if RSTL_PRIME1
  for (int i = 0; i < mIconToks.size(); ++i) {
#else
  const int count = mIconToks.size();
  for (int i = 0; i < count; ++i) {
#endif
    const CTexture& texture = **mIconToks[i].mTex;
    const ETexelFormat format = texture.GetTexelFormat();
    const void* data = texture.GetConstBitMapData(0);
    const uint size = format == kTexelFormat_RGB5A3 ? 2048 : 1024;
    out.Put(data, size);
    if (format == kTexelFormat_CI8) {
      palette = texture.GetPalette()->GetPaletteData();
    }
  }
  if (palette != nullptr) {
    out.Put(palette, 512);
  }
}

#if RSTL_CORRUPTION
void CMemoryCardSys::CCardFileInfo::BuildHeaderBuffer() {
  mHeaderBuffer.assign(0x2000);
  uint* data = reinterpret_cast< uint* >(mHeaderBuffer.data());
#else
void CMemoryCardSys::CCardFileInfo::BuildCardBuffer() {
  const uint bannerSize = CalculateBannerDataSize();
  const uint totalSize = (bannerSize + mSaveBuffer.size() + 8191) & ~8191;
  mCardBuffer.assign(totalSize);
  void* data = mCardBuffer.data();
#endif
  {
#if RSTL_CORRUPTION
    CMemoryStreamOut out(data, 0x2000);
#else
    CMemoryStreamOut out(data, totalSize);
#endif
    out.WriteInt32(0);
    char comment[64];
    strncpy(comment, mComment.data(), sizeof(comment));
    out.Put(comment, sizeof(comment));
    WriteBannerData(out);
    WriteIconData(out);
  }
#if RSTL_CORRUPTION
  data[0] = CCRC32::Calculate(data + 1, 0x2000 - 4, 0xFFFFFFFF);
#else
  (memcpy)(mCardBuffer.data() + bannerSize, mSaveBuffer.data(), mSaveBuffer.size());
  *static_cast< uint* >(data) = CalculateCrc(static_cast< const uchar* >(data) + 4, totalSize - 4);
#if NONMATCHING
  *static_cast< uint* >(data) = CBasics::SwapBytes(*static_cast< uint* >(data));
#endif
  mSaveBuffer = rstl::vector< uchar >();
#endif
}

#if RSTL_CORRUPTION
void CMemoryCardSys::CCardFileInfo::BuildWriteBuffers() {
  SSaveSlot& slot = mSlots[mSlot];
  slot.mData.assign(mAlignedUserDataSize, 0);
  uint* data = reinterpret_cast< uint* >(slot.mData.data());
  RS_VERIFY_THROW_DIRECT(899, mUserDataWrite.size() <= ( mAlignedUserDataSize - kHeaderSize ), false,
                         "User data too big for allocated mem card space!");

  (memcpy)(data + 2, mUserDataWrite.data(), mUserDataWrite.size());
  data[1] = mGeneration;
  data[0] = CCRC32::Calculate(data + 1, mAlignedUserDataSize - 4, 0xFFFFFFFF);
  rs_debugger_printf("BuildWriteBuffers: Using Write Index %d, Write Toggle Bit %d.\n", mSlot,
                     mGeneration);
  mUserDataWrite = rstl::vector< uchar >();
#else
uint CMemoryCardSys::CCardFileInfo::CalculateTotalDataSize() {
  const uint saveSize = mSaveBuffer.size();
  uint totalSize = CalculateBannerDataSize();
  totalSize += saveSize;
  return (totalSize + 8191) & ~8191;
#endif
}

#if RSTL_CORRUPTION
ECardResult CMemoryCardSys::CCardFileInfo::ExtractUserData() {
  mSlots[0].CheckCrc();
  mSlots[1].CheckCrc();
  SSaveSlot& slotA = mSlots[0];
  SSaveSlot& slotB = mSlots[1];
#else
uint CMemoryCardSys::CCardFileInfo::CalculateBannerDataSize() {
  uint size = 68;
  if (mBannerTex != kInvalidAssetId) {
    if ((*mBannerTok)->GetTexelFormat() == kTexelFormat_RGB5A3) {
      size = 6212;
    } else {
      size = 3652;
    }
  }
  bool palette = false;
  for (int i = 0; i < mIconToks.size(); ++i) {
    if (mIconToks[i].mTex->GetTexelFormat() == kTexelFormat_RGB5A3) {
      size += 2048;
    } else {
      size += 1024;
      palette = true;
    }
  }
  if (palette) {
    size += 512;
  }
  return size;
}
#endif

#if RSTL_CORRUPTION
  ECardResult result = kCR_READY;
  int readIndex = -1;
  if (slotA.mCorrupt) {
    if (slotB.mCorrupt) {
      rs_debugger_printf("ExtractUserData: Both User Areas Corrupted.\n");
      result = kCR_CRC_MISMATCH;
    } else {
      mGeneration = slotB.mGeneration;
      readIndex = 1;
      rs_debugger_printf("ExtractUserData: First User Area Corrupted.  Next Write index %d, next "
                         "Write Toggle Bit %d\n",
                         0, mGeneration);
    }
  } else if (slotB.mCorrupt) {
    mGeneration = 1 - slotA.mGeneration;
    readIndex = 0;
    rs_debugger_printf("ExtractUserData: Second User Area Corrupted.  Next Write index %d, next "
                       "Write Toggle Bit %d\n",
                       1, mGeneration);
  } else {
    readIndex = slotA.mGeneration ^ slotB.mGeneration;
    RS_VERIFY_THROW(970, readIndex == 0 || readIndex == 1, false,
                    CBasics::Stringize("Unexpected readIndex %d", readIndex));
    mGeneration = readIndex == 0 ? 1 - slotA.mGeneration : slotB.mGeneration;
    rs_debugger_printf("ExtractUserData: Both User Areas Okay.  Read Toggle Bits (%d,%d) -> Read "
                       "Index %d.  Next Write index %d, next Write Toggle Bit %d\n",
                       slotA.mGeneration, slotB.mGeneration, readIndex, 1 - readIndex, mGeneration);
  }

  rstl::vector< uchar >& loadedData = LoadedData();
  loadedData = rstl::vector< uchar >();
  if (readIndex != -1) {
    mSlot = 1 - readIndex;
    const int size = mAlignedUserDataSize - kHeaderSize;
    loadedData.assign(size);
    (memcpy)(loadedData.data(), mSlots[readIndex].mData.data() + kHeaderSize, size);
  }
  slotA.mData = rstl::vector< uchar, rstl::aligned_allocator >();
  slotB.mData = rstl::vector< uchar, rstl::aligned_allocator >();
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::CheckHeaderCrc() {
  const uint* data = reinterpret_cast< const uint* >(mHeaderBuffer.data());
  const uint crc = data[0];
  return crc == CCRC32::Calculate(data + 1, 0x2000 - 4, 0xFFFFFFFF) ? kCR_READY : kCR_CRC_MISMATCH;
}
#else
#if RSTL_CARD_SAVE_BLOCKS
void CMemoryCardSys::CCardFileInfo::BuildHeader() {
  mHeaderBuffer.assign(8192);
  uint* data = reinterpret_cast< uint* >(mHeaderBuffer.data());
  {
    CMemoryStreamOut out(data, 8192);
    out.WriteInt32(0);
    char comment[64];
    strncpy(comment, mComment.data(), sizeof(comment));
    out.Put(comment, sizeof(comment));
    WriteBannerData(out);
    WriteIconData(out);
  }
  data[0] = BE_SWAP(CalculateCrc(data + 1, 8188));
}

void CMemoryCardSys::CCardFileInfo::BuildSaveBlock() {
  SSaveBlock& block = mSaveBlocks[mWriteBlock];
  block.mData.assign(mBlockSize);
  uint* data = reinterpret_cast< uint* >(block.mData.data());
  (memcpy)(data + 2, mCardBuffer.data(), mCardBuffer.size());
  data[1] = BE_SWAP(mSequence);
  data[0] = BE_SWAP(CalculateCrc(data + 1, mBlockSize - 4));
  mCardBuffer = TCardFileBuffer();
}

ECardResult CMemoryCardSys::CCardFileInfo::FinishRead() {
#if RSTL_PRIME1
  mSaveBlocks[0].Check();
  mSaveBlocks[1].Check();
  SSaveBlock& first = mSaveBlocks[0];
  SSaveBlock& second = mSaveBlocks[1];
#else
  SSaveBlock& first = mSaveBlocks[0];
  SSaveBlock& second = mSaveBlocks[1];
  first.Check();
  second.Check();
#endif
  ECardResult result = kCR_READY;
  int selected = -1;
  if (first.mCorrupted) {
    if (second.mCorrupted) {
      result = kCR_CRC_MISMATCH;
    } else {
      selected = 1;
      mSequence = second.mSequence;
    }
  } else if (second.mCorrupted) {
    selected = 0;
    mSequence = 1 - first.mSequence;
  } else {
    selected = first.mSequence ^ second.mSequence;
#if RSTL_PRIME1
    mSequence = selected == 0 ? 1 - first.mSequence : second.mSequence;
#else
    mSequence = selected != 0 ? second.mSequence : 1 - first.mSequence;
#endif
  }
#if RSTL_PRIME1
  rstl::vector< uchar >& saveBuffer = SaveBuffer();
  saveBuffer = rstl::vector< uchar >();
#else
  mSaveBuffer = rstl::vector< uchar >();
#endif
  if (selected != -1) {
    mWriteBlock = 1 - selected;
    const int size = mBlockSize - 8;
#if RSTL_PRIME1
    saveBuffer.assign(size);
    (memcpy)(saveBuffer.data(), mSaveBlocks[selected].mData.data() + 8, size);
#else
    mSaveBuffer.resize(size);
    (memcpy)(mSaveBuffer.data(), mSaveBlocks[selected].mData.data() + 8, size);
#endif
  }
  first.mData = rstl::vector< uchar, rstl::aligned_allocator >();
  second.mData = rstl::vector< uchar, rstl::aligned_allocator >();
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::CheckHeader() {
  const uint* data = reinterpret_cast< const uint* >(mHeaderBuffer.data());
  const uint crc = BE_SWAP(data[0]);
  return crc == CalculateCrc(data + 1, 8188) ? kCR_READY : kCR_CRC_MISMATCH;
}
#endif
#endif

uint CardStat::GetFileLength() { return mStat.length; }

int CardStat::GetTime() const { return mStat.time; }
#if !RSTL_CORRUPTION

int CardStat::GetBannerFormat() { return CARDGetBannerFormat(&mStat); }

int CardStat::GetIconFormat(int idx) { return CARDGetIconFormat(&mStat, idx); }
#endif

int CardStat::GetCommentAddr() const { return mStat.commentAddr; }

void CardStat::SetBannerFormat(int format) { CARDSetBannerFormat(&mStat, format); }

void CardStat::SetIconFormat(int format, int idx) { CARDSetIconFormat(&mStat, idx, format); }

void CardStat::SetIconSpeed(int speed, int idx) { CARDSetIconSpeed(&mStat, idx, speed); }

void CardStat::SetIconAddr(int addr) { CARDSetIconAddress(&mStat, addr); }

void CardStat::SetCommentAddr(int addr) { CARDSetCommentAddress(&mStat, addr); }

#if RSTL_CORRUPTION
CMemoryCardSys::CMemoryCardSys() {
  Initialize();
#else
#if !RSTL_CARD_SAVE_BLOCKS
void CMemoryCardSys::CCardFileInfo::SetComment(const rstl::string& comment) { mComment = comment; }
#endif

CMemoryCardSys::CMemoryCardSys(IF_TARGET_PC(const char* game, const char* maker)) {
  Initialize(IF_TARGET_PC(game, maker));
#endif
  mIsCardSysExists = true;
}

void CMemoryCardSys::Initialize(IF_TARGET_PC(const char* game, const char* maker)) {
  if (!mIsInitialized) {
#if TARGET_PC
    CARDSetLoadType(CARD_RAWIMAGE);
    // CARDDetectDolphin(-1);
    CARDInit(game, maker);
#else
    CARDInit();
#endif
    mIsInitialized = true;
  }
}

CMemoryCardSys::~CMemoryCardSys() {
  mIsCardSysExists = false;
  FreeCardWorkArea(kCS_SlotA);
  FreeCardWorkArea(kCS_SlotB);
}

ProbeResults CMemoryCardSys::IsMemoryCardInserted(EMemoryCardPort port) {
  ProbeResults result;
  result.mError =
      static_cast< ECardResult >(CARDProbeEx(port, &result.mCardSize, &result.mSectorSize));
  return result;
}

ECardResult CMemoryCardSys::GetResultCode(int port) {
  return static_cast< ECardResult >(CARDGetResultCode(port));
}

ECardResult CMemoryCardSys::MountCardSync(EMemoryCardPort port) {
  return static_cast< ECardResult >(CARDMount(port, AllocCardWorkArea(port), nullptr));
}

ECardResult CMemoryCardSys::MountCard(EMemoryCardPort port) {
  return static_cast< ECardResult >(
      CARDMountAsync(port, AllocCardWorkArea(port), nullptr, nullptr));
}

ECardResult CMemoryCardSys::UnmountCard(EMemoryCardPort port) {
  ECardResult result = static_cast< ECardResult >(CARDUnmount(port));
  FreeCardWorkArea(port);
  return result;
}

#if RSTL_CORRUPTION
ECardResult CMemoryCardSys::FormatSync(EMemoryCardPort port) {
  return static_cast< ECardResult >(CARDFormat(port));
}

#endif
ECardResult CMemoryCardSys::FormatCard(EMemoryCardPort port) {
#if RSTL_CORRUPTION
  return static_cast< ECardResult >(CARDFormatAsync(port, nullptr));
#else
  return static_cast< ECardResult >(
      IF_TARGET_PC_OR(CARDFormat, CARDFormatAsync)(port IF_NOT_TARGET_PC(, nullptr)));
#endif
}

ECardResult CMemoryCardSys::GetNumFreeBytes(EMemoryCardPort port, uint& freeBytes,
                                            uint& freeFiles) {
  s32 bytes;
  s32 files;
  ECardResult result = static_cast< ECardResult >(CARDFreeBlocks(port, &bytes, &files));
  freeBytes = bytes;
  freeFiles = files;
  return result;
}

#if RSTL_CORRUPTION
ECardResult CMemoryCardSys::DeleteNamedFile(EMemoryCardPort port, const rstl::string& name) {
  return static_cast< ECardResult >(CARDDelete(port, name.data()));
#else
#if !RSTL_CARD_SAVE_BLOCKS
SMemoryCardFileInfo::SMemoryCardFileInfo(int cardPort, const rstl::string& name) : mName(name) {
  mFileInfo.chan = cardPort;
  mFileInfo.fileNo = -1;
}

CMemoryCardSys::CCardFileInfo::CCardFileInfo(EMemoryCardPort port, const rstl::string& name)
: mStatus(kS_Standby), mFileName(name), x38_(0), mBannerTex(kInvalidAssetId) {
  mFileInfo.chan = port;
  mFileInfo.fileNo = -1;
}

void CMemoryCardSys::CCardFileInfo::LockBannerToken(CAssetId bannerTxtr, CSimplePool& pool) {
  mBannerTex = bannerTxtr;
  mBannerTok = TLockedToken< CTexture >(pool.GetObj(SObjectTag('TXTR', mBannerTex)));
}

void CMemoryCardSys::CCardFileInfo::LockIconToken(CAssetId iconTxtr, int speed, CSimplePool& pool) {
  mIconToks.push_back(Icon(iconTxtr, speed, pool));
}
#endif

ECardResult SMemoryCardFileInfo::Open() {
  return static_cast< ECardResult >(CARDOpen(GetFileCardPort(), mName.data(), &mFileInfo));
}

#if !RSTL_CARD_SAVE_BLOCKS
ECardResult CMemoryCardSys::CCardFileInfo::CreateFile() {
  const uint size = CalculateTotalDataSize();
  return static_cast< ECardResult >(
      CARDCreateAsync(GetPort(), mFileName.data(), size, &mFileInfo, nullptr));
}
#endif

ECardResult CMemoryCardSys::DeleteFile(EMemoryCardPort port, const rstl::string& name) {
  return static_cast< ECardResult >(CARDDeleteAsync(port, name.data(), nullptr));
#endif
}

ECardResult CMemoryCardSys::FastDeleteFile(EMemoryCardPort port, int fileNo) {
  return static_cast< ECardResult >(CARDFastDeleteAsync(port, fileNo, nullptr));
}

#if !RSTL_CORRUPTION
ECardResult SMemoryCardFileInfo::Close() {
  const CMemoryCardSys::EMemoryCardPort port = GetFileCardPort();
  ECardResult result = static_cast< ECardResult >(CARDClose(&mFileInfo));
  mFileInfo.chan = port;
  return result;
}

#if !RSTL_CARD_SAVE_BLOCKS
ECardResult CMemoryCardSys::CCardFileInfo::CloseFile() {
  const EMemoryCardPort port = GetPort();
  ECardResult result = static_cast< ECardResult >(CARDClose(&mFileInfo));
  mFileInfo.chan = port;
  return result;
}
#endif

ECardResult CMemoryCardSys::Rename(EMemoryCardPort port, const rstl::string& oldName,
                                   const rstl::string& newName) {
  return static_cast< ECardResult >(CARDRenameAsync(port, oldName.data(), newName.data(), nullptr));
}

#endif
ECardResult CMemoryCardSys::CheckCard(EMemoryCardPort port) {
  return static_cast< ECardResult >(CARDCheckAsync(port, nullptr));
#if !RSTL_CORRUPTION
}

#if !RSTL_CARD_SAVE_BLOCKS
ECardResult CMemoryCardSys::CCardFileInfo::WriteFile() {
  BuildCardBuffer();
  void* data = mCardBuffer.data();
  const int size = mCardBuffer.size();
  DCStoreRange(data, size);
  ECardResult result =
      static_cast< ECardResult >(CARDWriteAsync(&mFileInfo, data, size, 0, nullptr));
  mStatus = kS_Transferring;
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::PumpCardTransfer() {
  if (mStatus == kS_Standby) {
    return kCR_READY;
  } else if (mStatus == kS_Transferring) {
    ECardResult result = GetResultCode(GetPort());
    if (result != kCR_BUSY) {
      mCardBuffer = TCardFileBuffer();
    }
    if (result != kCR_READY) {
      return result;
    }
    mStatus = kS_Done;
    CardStat stat;
    result = GetStatus(stat);
    if (result != kCR_READY) {
      return result;
    }
    result = SetStatus(GetPort(), GetFileNo(), stat);
    if (result == kCR_READY) {
      return kCR_BUSY;
    }
    return result;
  } else {
    ECardResult result = GetResultCode(GetPort());
    if (result != kCR_READY) {
      return result;
    }
    mStatus = kS_Standby;
    return kCR_READY;
  }
}

ECardResult CMemoryCardSys::CCardFileInfo::GetStatus(CardStat& stat) {
  ECardResult result = CMemoryCardSys::GetStatus(GetPort(), GetFileNo(), stat);
  if (result != kCR_READY) {
    return result;
  }
  stat.SetCommentAddr(4);
  stat.SetIconAddr(68);
  int bannerFormat;
  if (mBannerTex == kInvalidAssetId) {
    bannerFormat = CARD_STAT_BANNER_NONE;
  } else if ((*mBannerTok)->GetTexelFormat() == kTexelFormat_RGB5A3) {
    bannerFormat = CARD_STAT_BANNER_RGB5A3;
  } else {
    bannerFormat = CARD_STAT_BANNER_C8;
  }
  stat.SetBannerFormat(bannerFormat);
  int i = 0;
  for (; i < mIconToks.size(); ++i) {
    int format = CARD_STAT_ICON_C8;
    if (mIconToks[i].mTex->GetTexelFormat() == kTexelFormat_RGB5A3) {
      format = CARD_STAT_ICON_RGB5A3;
    }
    stat.SetIconFormat(format, i);
    stat.SetIconSpeed(mIconToks[i].mSpeed, i);
  }
  if (i < 8) {
    stat.SetIconFormat(CARD_STAT_ICON_NONE, i);
    stat.SetIconSpeed(CARD_STAT_SPEED_END, i);
  }
  return kCR_READY;
}

ECardResult SMemoryCardFileInfo::StartRead() {
  CardStat stat;
  ECardResult result = CMemoryCardSys::GetStatus(GetFileCardPort(), GetFileNo(), stat);
  if (result != kCR_READY) {
    return result;
  }
  const uint size = stat.GetFileLength();
  mSaveData = rstl::vector< uchar >();
  mSaveFileData.assign(size);
  return static_cast< ECardResult >(
      CARDReadAsync(&mFileInfo, mSaveFileData.data(), size, 0, nullptr));
}
#endif

ECardResult SMemoryCardFileInfo::TryFileRead() {
  ECardResult result = CMemoryCardSys::GetResultCode(GetFileCardPort());
  if (result != kCR_READY) {
    return result;
  }
  return FileRead();
#endif
}

ECardResult CMemoryCardSys::GetSerialNo(EMemoryCardPort port, long long& serialOut) {
  return static_cast< ECardResult >(CARDGetSerialNo(port, reinterpret_cast< u64* >(&serialOut)));
}

ECardResult CMemoryCardSys::GetStatus(EMemoryCardPort port, int fileNo, CardStat& statOut) {
  CARDStat stat;
  ECardResult result = static_cast< ECardResult >(CARDGetStatus(port, fileNo, &stat));
  memcpy(&statOut.mStat, &stat, sizeof(stat));
  return result;
}

#if RSTL_CORRUPTION
ECardResult CMemoryCardSys::SetStatusSync(EMemoryCardPort port, int fileNo, const CardStat& stat) {
  return static_cast< ECardResult >(
      CARDSetStatus(port, fileNo, const_cast< CARDStat* >(&stat.mStat)));
}

#endif
ECardResult CMemoryCardSys::SetStatus(EMemoryCardPort port, int fileNo, const CardStat& stat) {
  return static_cast< ECardResult >(
      CARDSetStatusAsync(port, fileNo, const_cast< CARDStat* >(&stat.mStat), nullptr));
}

bool CMemoryCardSys::mIsInitialized;
bool CMemoryCardSys::mIsCardSysExists;
bool CMemoryCardSys::mIsCardBusy;
#if RSTL_CORRUPTION
rstl::vector< char, rstl::aligned_allocator > CMemoryCardSys::mWorkAreaA;
rstl::vector< char, rstl::aligned_allocator > CMemoryCardSys::mWorkAreaB;
#else
TCardWorkArea CMemoryCardSys::mWorkAreaA;
TCardWorkArea CMemoryCardSys::mWorkAreaB;
#endif

#if RSTL_CORRUPTION
rstl::vector< char, rstl::aligned_allocator >&
CMemoryCardSys::WorkAreaVector(EMemoryCardPort port) {
#else
TCardWorkArea& CMemoryCardSys::WorkAreaVector(EMemoryCardPort port) {
#endif
  switch (port) {
  case kCS_SlotA:
    return mWorkAreaA;
  case kCS_SlotB:
    return mWorkAreaB;
  default:
    RS_VERIFY_FAILURE(1306, "false", "false", "Invalid memory card port");
    return mWorkAreaA;
  }
}

char* CMemoryCardSys::AllocCardWorkArea(EMemoryCardPort port) {
#if RSTL_CORRUPTION
  rstl::vector< char, rstl::aligned_allocator >& area = WorkAreaVector(port);
#else
  TCardWorkArea& area = WorkAreaVector(port);
#endif
  area.resize(0xa000);
  char* data = area.data();
  DCInvalidateRange(data, area.size());
  return data;
}

void CMemoryCardSys::FreeCardWorkArea(EMemoryCardPort port) {
#if RSTL_CORRUPTION
  rstl::vector< char, rstl::aligned_allocator >& area = WorkAreaVector(port);
  area = rstl::vector< char, rstl::aligned_allocator >();
#else
  TCardWorkArea& area = WorkAreaVector(port);
  area = TCardWorkArea();
#endif
}
