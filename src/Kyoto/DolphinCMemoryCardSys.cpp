#include "rstl/RstlVersions.h"
#include "Kyoto/CMemoryCardSys.hpp"

#include "Kyoto/CCrc32.hpp"

#include "Kyoto/Graphics/CGraphicsPalette.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Basics/CBasics.hpp"

#include "dolphin/os/OSCache.h"

#include "Kyoto/MemoryCopy.hpp"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
bool CMemoryCardSys::mIsInitialized;
bool CMemoryCardSys::mIsCardSysExists;
TCardWorkArea CMemoryCardSys::mWorkAreaA;
TCardWorkArea CMemoryCardSys::mWorkAreaB;
#else
static uint RoundUp(uint value, uint align) { return (value + align - 1) & ~(align - 1); }
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
ECardResult SMemoryCardFileInfo::FileRead() {
  rstl::vector< uchar >& saveData = mSaveData;
  saveData = rstl::vector< uchar >();
  const uint size = mSaveFileData.size();
  const void* data = mSaveFileData.data();
#if NONMATCHING
  const uint crc = CBasics::SwapBytes(*static_cast< const uint* >(data));
#else
  const uint crc = *static_cast< const uint* >(data);
#endif
  if (crc == CCRC32::Calculate(static_cast< const uchar* >(data) + 4, size - 4)) {
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
  }
}

static int RoundUpCardSize(int size, int align) { return (size + align - 1) & ~(align - 1); }

#endif
CMemoryCardSys::CCardFileInfo::Icon::Icon(CAssetId id, int speed, CSimplePool& pool)
: mId(id)
, mSpeed(speed)
, mTex(pool.GetObj(SObjectTag('TXTR', id))) {}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CMemoryCardSys::EMemoryCardPort SMemoryCardFileInfo::GetFileCardPort() {
  return static_cast< CMemoryCardSys::EMemoryCardPort >(mFileInfo.chan);
}

int SMemoryCardFileInfo::GetFileNo() const { return mFileInfo.fileNo; }

CMemoryCardSys::CCardFileInfo::SSaveBlock::SSaveBlock()
: mSequence(0)
, mCorrupted(false) {}

void CMemoryCardSys::CCardFileInfo::SSaveBlock::Check() {
  const uint* data = reinterpret_cast< const uint* >(mData.data());
  const uint crc = data[0];
  mCorrupted = crc != CCRC32::Calculate(data + 1, mData.size() - 4);
  mSequence = data[1];
}

#if RSTL_VERSION >= RSTL_GM8P_00
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
  const EMemoryCardPort port = GetCardPort();
  ECardResult result = static_cast< ECardResult >(CARDOpen(port, mFileName.data(), &mFileInfo));
  mFileInfo.chan = port;
  if (result == kCR_READY) {
    CardStat stat;
    ECardResult statusResult = CMemoryCardSys::GetStatus(GetCardPort(), GetFileNo(), stat);
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
      CARDCreateAsync(GetCardPort(), mFileName.data(), size, &mFileInfo, nullptr));
}

ECardResult CMemoryCardSys::CCardFileInfo::CloseFile() {
  const EMemoryCardPort port = GetCardPort();
  ECardResult result = static_cast< ECardResult >(CARDClose(&mFileInfo));
  mFileInfo.chan = port;
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::FileRead() {
  CardStat stat;
  ECardResult result = CMemoryCardSys::GetStatus(GetCardPort(), GetFileNo(), stat);
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
  ECardResult result = CMemoryCardSys::GetStatus(GetCardPort(), GetFileNo(), stat);
  if (result != kCR_READY) {
    return result;
  }
  const uint fileSize = stat.GetFileLength();
  mSaveBuffer = rstl::vector< uchar >();
  mHeaderBuffer.assign(8192);
  mSaveBlocks[0].mData.assign(mBlockSize);
  mSaveBlocks[1].mData.assign(mBlockSize);
  result = static_cast< ECardResult >(
      CARDRead(&mFileInfo, mHeaderBuffer.data(), 8192, 0));
  if (result == kCR_READY) {
    mStatus = kS_Transferring;
  }
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::TryFileRead() {
  if (mStatus == kS_Standby) {
    return kCR_READY;
  }
  ECardResult result = GetResultCode(GetCardPort());
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
  return static_cast< ECardResult >(
      CARDWriteAsync(&mFileInfo, data, mBlockSize, offset, nullptr));
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
  ECardResult result = GetResultCode(GetCardPort());
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
    result = SetStatus(GetCardPort(), GetFileNo(), stat);
    if (result == kCR_READY) {
      return kCR_BUSY;
    }
    return result;
  }
  mStatus = kS_Standby;
  return kCR_READY;
}
#endif
#else
CMemoryCardSys::CCardFileInfo::SSaveSlot::SSaveSlot() : mGeneration(0), mCorrupt(false) {}

void CMemoryCardSys::CCardFileInfo::SSaveSlot::CheckCrc() {
  const uint* data = reinterpret_cast< const uint* >(mData.data());
  const uint crc = BE_SWAP(data[0]);
  mCorrupt = CCRC32::Calculate(data + 1, mData.size() - 4, 0xFFFFFFFF) != crc;
  mGeneration = BE_SWAP(data[1]);
}

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
      return statResult;
    }
    const int blocks = stat.GetFileLength() / 0x2000;
    mSlotBlocks = (blocks - 1) / 2;
    mAlignedUserDataSize = mSlotBlocks * 0x2000;
  }
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::CreateFile() {
  mNewFile = true;
  mAlignedUserDataSize = RoundUp(mUserDataWrite.size(), 0x2000);
  mSlotBlocks = mAlignedUserDataSize / 0x2000;
  const int size = GetTotalNumBlocks() * 0x2000;
  return static_cast< ECardResult >(
      CARDCreateAsync(GetPort(), mFileName.data(), size, &mFileInfo, nullptr));
}

ECardResult CMemoryCardSys::CCardFileInfo::StartRead() {
  CardStat stat;
  ECardResult result = CMemoryCardSys::GetStatus(GetPort(), GetFileNo(), stat);
  if (result != kCR_READY) {
    return result;
  }
  const uint size = stat.GetFileLength();
  mLoadedData = rstl::vector< uchar >();
  mHeaderBuffer.assign(0x2000);
  mSlots[0].mData.assign(mAlignedUserDataSize);
  mSlots[1].mData.assign(mAlignedUserDataSize);
  result = static_cast< ECardResult >(CARDRead(&mFileInfo, mHeaderBuffer.data(), 0x2000, 0));
  if (result == kCR_READY) {
    mState = kS_ReadHeader;
  }
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::PumpCardRead() {
  if (mState == kS_Standby) {
    return kCR_READY;
  }
  ECardResult result = GetResultCode(GetPort());
  if (result != kCR_READY) {
    return result;
  }
  if (mState == kS_ReadHeader || mState == kS_RepairHeader) {
    if (mState == kS_ReadHeader && CheckHeaderCrc() != kCR_READY) {
      BuildHeaderBuffer();
      mState = kS_RepairHeader;
      void* data = mHeaderBuffer.data();
      DCStoreRange(data, 0x2000);
      result = static_cast< ECardResult >(CARDWriteAsync(&mFileInfo, data, 0x2000, 0, nullptr));
      if (result == kCR_READY) {
        return kCR_BUSY;
      }
      return result;
    } else {
      mHeaderBuffer = rstl::vector< uchar, rstl::aligned_allocator >();
      mState = kS_ReadSlotA;
      result = static_cast< ECardResult >(
          CARDRead(&mFileInfo, mSlots[0].mData.data(), mAlignedUserDataSize, 0x2000));
      if (result == kCR_READY) {
        return kCR_BUSY;
      }
      return result;
    }
  } else if (mState == kS_ReadSlotA) {
    mState = kState_ReadingData1;
    result = static_cast< ECardResult >(CARDRead(
        &mFileInfo, mSlots[1].mData.data(), mAlignedUserDataSize, mAlignedUserDataSize + 0x2000));
    if (result == kCR_READY) {
      return kCR_BUSY;
    }
    return result;
  } else {
    return SelectSaveSlot();
  }
}

ECardResult CMemoryCardSys::CCardFileInfo::WriteSaveSlot(int slot) {
  const int offset = slot * mAlignedUserDataSize + 0x2000;
  void* data = mSlots[mSlot].mData.data();
  DCStoreRange(data, mAlignedUserDataSize);
  return static_cast< ECardResult >(
      CARDWriteAsync(&mFileInfo, data, mAlignedUserDataSize, offset, nullptr));
}

ECardResult CMemoryCardSys::CCardFileInfo::WriteFile() {
  ECardResult result;
  BuildSaveSlot();
  if (mNewFile) {
    BuildHeaderBuffer();
    void* data = mHeaderBuffer.data();
    DCStoreRange(data, 0x2000);
    result = static_cast< ECardResult >(CARDWriteAsync(&mFileInfo, data, 0x2000, 0, nullptr));
    mState = kS_WriteHeader;
  } else {
    result = WriteSaveSlot(mSlot);
    mState = kS_WriteSlot;
  }
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::PumpCardTransfer() {
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
    if (result == kCR_READY) {
      return kCR_BUSY;
    }
    return result;
  } else if (mState == kS_WriteFirstSlot) {
    mState = kS_WriteSlot;
    result = WriteSaveSlot(1 - mSlot);
    if (result == kCR_READY) {
      return kCR_BUSY;
    }
    return result;
  } else if (mState == kS_WriteSlot) {
    mState = kState_WritingStatus;
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
    mState = kS_Standby;
    return kCR_READY;
  }
}
#endif

CMemoryCardSys::EMemoryCardPort CMemoryCardSys::CCardFileInfo::
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
GetCardPort
#else
GetPort
#endif
() {
  return static_cast< EMemoryCardPort >(mFileInfo.chan);
}

int CMemoryCardSys::CCardFileInfo::GetFileNo() { return mFileInfo.fileNo; }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
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

#if RSTL_VERSION >= RSTL_GM8P_00
int CMemoryCardSys::CCardFileInfo::GetTotalBlockCount() { return 2 * mBlockCount + 1; }
#endif
#else
int CMemoryCardSys::CCardFileInfo::GetTotalNumBlocks() { return mSlotBlocks * 2 + 1; }
#endif

void CMemoryCardSys::CCardFileInfo::
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
ResetBannerAndIcons
#else
ResetHeaderInfo
#endif
() {
  mComment = rstl::string();
  mBannerTex = kInvalidAssetId;
  mBannerTok = rstl::optional_object< TLockedToken< CTexture > >();
  mIconToks = rstl::reserved_vector< Icon, 8 >();
}

// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1 || RSTL_VERSION >= RSTL_GM8P_00
void CMemoryCardSys::CCardFileInfo::SetComment(const rstl::string& comment) { mComment = comment; }

void CMemoryCardSys::CCardFileInfo::LockBannerToken(CAssetId bannerTxtr, CSimplePool& pool) {
  mBannerTex = bannerTxtr;
  mBannerTok = TLockedToken< CTexture >(pool.GetObj(SObjectTag('TXTR', mBannerTex)));
}

void CMemoryCardSys::CCardFileInfo::LockIconToken(CAssetId iconTxtr, int speed, CSimplePool& pool) {
  mIconToks.push_back(Icon(iconTxtr, speed, pool));
}

ECardResult CMemoryCardSys::CCardFileInfo::GetStatus(CardStat& stat) {
  ECardResult result = CMemoryCardSys::GetStatus(
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      GetCardPort(),
#else
      GetPort(),
#endif
      GetFileNo(), stat);
  if (result != kCR_READY) {
    return result;
  }
  stat.SetCommentAddr(4);
  stat.SetIconAddr(68);
  int bannerFormat;
  if (mBannerTex == kInvalidAssetId) {
    bannerFormat = CARD_STAT_BANNER_NONE;
  } else if (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      (*mBannerTok)->GetTexelFormat()
#else
      (*mBannerTok.data())->GetTexelFormat()
#endif
      == kTF_RGB5A3) {
    bannerFormat = CARD_STAT_BANNER_RGB5A3;
  } else {
    bannerFormat = CARD_STAT_BANNER_C8;
  }
  stat.SetBannerFormat(bannerFormat);
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  const int count = mIconToks.size();
#endif
  int i = 0;
  for (; i <
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
              mIconToks.size()
#else
              count
#endif
       ; ++i) {
    int format = CARD_STAT_ICON_C8;
    if (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        mIconToks[i].mTex->GetTexelFormat()
#else
        (*mIconToks[i].mTex)->GetTexelFormat()
#endif
        == kTF_RGB5A3) {
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

void CMemoryCardSys::CCardFileInfo::WriteBannerData(COutputStream& out) {
  if (mBannerTex != kInvalidAssetId) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const CTexture& texture = ***mBannerTok;
#else
    const CTexture& texture = **mBannerTok.data();
#endif
    const ETexelFormat format = texture.GetTexelFormat();
    const void* data = texture.GetConstBitMapData(0);
    const uint size = format == kTF_RGB5A3 ? 6144 : 3072;
    out.Put(data, size);
    if (format == kTF_C8) {
      out.Put(texture.GetPalette()->GetPaletteData(), 512);
    }
  }
}

void CMemoryCardSys::CCardFileInfo::WriteIconData(COutputStream& out) {
  const void* palette = nullptr;
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  const int count = mIconToks.size();
#endif
  for (int i = 0; i <
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                    mIconToks.size()
#else
                    count
#endif
       ; ++i) {
    const CTexture& texture = **mIconToks[i].mTex;
    const ETexelFormat format = texture.GetTexelFormat();
    const void* data = texture.GetConstBitMapData(0);
    const uint size = format == kTF_RGB5A3 ? 2048 : 1024;
    out.Put(data, size);
    if (format == kTF_C8) {
      palette = texture.GetPalette()->GetPaletteData();
    }
  }
  if (palette != nullptr) {
    out.Put(palette, 512);
  }
}

void CMemoryCardSys::CCardFileInfo::
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
BuildCardBuffer
#else
BuildHeaderBuffer
#endif
() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const uint bannerSize = CalculateBannerDataSize();
  const uint totalSize = (bannerSize + mSaveBuffer.size() + 8191) & ~8191;
  mCardBuffer.assign(totalSize);
  void* data = mCardBuffer.data();
#else
  mHeaderBuffer.assign(0x2000);
  uint* data = reinterpret_cast< uint* >(mHeaderBuffer.data());
#endif
  {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CMemoryStreamOut out(data, totalSize);
#else
    CMemoryStreamOut out(data, 0x2000);
#endif
    out.WriteInt32(0);
    char comment[64];
    strncpy(comment, mComment.data(), sizeof(comment));
    out.Put(comment, sizeof(comment));
    WriteBannerData(out);
    WriteIconData(out);
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  (memcpy)(mCardBuffer.data() + bannerSize, mSaveBuffer.data(), mSaveBuffer.size());
  *static_cast< uint* >(data) =
      CCRC32::Calculate(static_cast< const uchar* >(data) + 4, totalSize - 4);
#if NONMATCHING
  *static_cast< uint* >(data) = CBasics::SwapBytes(*static_cast< uint* >(data));
#endif
  mSaveBuffer = rstl::vector< uchar >();
#else
  data[0] = BE_SWAP(CCRC32::Calculate(data + 1, 0x2000 - 4, 0xFFFFFFFF));
#endif
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
uint CMemoryCardSys::CCardFileInfo::CalculateTotalDataSize() {
  const uint saveSize = mSaveBuffer.size();
  uint totalSize = CalculateBannerDataSize();
  totalSize += saveSize;
  return (totalSize + 8191) & ~8191;
}

uint CMemoryCardSys::CCardFileInfo::CalculateBannerDataSize() {
  uint size = 68;
  if (mBannerTex != kInvalidAssetId) {
    if ((*mBannerTok)->GetTexelFormat() == kTF_RGB5A3) {
      size = 6212;
    } else {
      size = 3652;
    }
  }
  bool palette = false;
  for (int i = 0; i < mIconToks.size(); ++i) {
    if (mIconToks[i].mTex->GetTexelFormat() == kTF_RGB5A3) {
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
#if RSTL_VERSION >= RSTL_GM8P_00
void CMemoryCardSys::CCardFileInfo::BuildHeader() {
  mHeaderBuffer.assign(8192);
  void* data = mHeaderBuffer.data();
  {
    CMemoryStreamOut out(data, 8192);
    out.WriteInt32(0);
    char comment[64];
    strncpy(comment, mComment.data(), sizeof(comment));
    out.Put(comment, sizeof(comment));
    WriteBannerData(out);
    WriteIconData(out);
  }
  *static_cast< uint* >(data) =
      CCRC32::Calculate(static_cast< char* >(data) + 4, 8188);
}

void CMemoryCardSys::CCardFileInfo::BuildSaveBlock() {
  SSaveBlock& block = mSaveBlocks[mWriteBlock];
  block.mData.assign(mBlockSize);
  uint* data = reinterpret_cast< uint* >(block.mData.data());
  (memcpy)(data + 2, mCardBuffer.data(), mCardBuffer.size());
  data[1] = mSequence;
  data[0] = CCRC32::Calculate(data + 1, mBlockSize - 4);
  mCardBuffer = rstl::vector< uchar >();
}

ECardResult CMemoryCardSys::CCardFileInfo::FinishRead() {
  mSaveBlocks[0].Check();
  mSaveBlocks[1].Check();
  SSaveBlock& first = mSaveBlocks[0];
  SSaveBlock& second = mSaveBlocks[1];
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
    mSequence = selected == 0 ? 1 - first.mSequence : second.mSequence;
  }

  rstl::vector< uchar >& saveBuffer = SaveBuffer();
  saveBuffer = rstl::vector< uchar >();
  if (selected != -1) {
    mWriteBlock = 1 - selected;
    const int size = mBlockSize - 8;
    saveBuffer.assign(size);
    (memcpy)(saveBuffer.data(), mSaveBlocks[selected].mData.data() + 8, size);
  }
  first.mData = rstl::vector< uchar, rstl::aligned_allocator >();
  second.mData = rstl::vector< uchar, rstl::aligned_allocator >();
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::CheckHeader() {
  const uint* data = reinterpret_cast< const uint* >(mHeaderBuffer.data());
  const uint crc = data[0];
  return crc == CCRC32::Calculate(data + 1, 8188) ? kCR_READY : kCR_CRC_MISMATCH;
}
#endif
#else
void CMemoryCardSys::CCardFileInfo::BuildSaveSlot() {
  SSaveSlot& slot = mSlots[mSlot];
  slot.mData.assign(mAlignedUserDataSize);
  uint* data = reinterpret_cast< uint* >(slot.mData.data());
  (memcpy)(data + 2, mUserDataWrite.data(), mUserDataWrite.size());
  data[1] = BE_SWAP(mGeneration);
  data[0] = BE_SWAP(CCRC32::Calculate(data + 1, mAlignedUserDataSize - 4, 0xFFFFFFFF));
  mUserDataWrite = rstl::vector< uchar >();
}

ECardResult CMemoryCardSys::CCardFileInfo::SelectSaveSlot() {
  SSaveSlot& slotA = mSlots[0];
  SSaveSlot& slotB = mSlots[1];
  slotA.CheckCrc();
  slotB.CheckCrc();

  ECardResult result = kCR_READY;
  int slot = -1;
  if (slotA.mCorrupt) {
    if (slotB.mCorrupt) {
      result = kCR_CRC_MISMATCH;
    } else {
      slot = 1;
      mGeneration = slotB.mGeneration;
    }
  } else if (slotB.mCorrupt) {
    slot = 0;
    mGeneration = 1 - slotA.mGeneration;
  } else {
    slot = slotA.mGeneration ^ slotB.mGeneration;
    mGeneration = slot != 0 ? slotB.mGeneration : 1 - slotA.mGeneration;
  }
  mLoadedData = rstl::vector< uchar >();
  if (slot != -1) {
    mSlot = 1 - slot;
    const int size = mAlignedUserDataSize - 8;
    mLoadedData.resize(size);
    (memcpy)(mLoadedData.data(), mSlots[slot].mData.data() + 8, size);
  }
  slotA.mData = rstl::vector< uchar, rstl::aligned_allocator >();
  slotB.mData = rstl::vector< uchar, rstl::aligned_allocator >();
  return result;
}

ECardResult CMemoryCardSys::CCardFileInfo::CheckHeaderCrc() {
  const uint* data = reinterpret_cast< const uint* >(mHeaderBuffer.data());
  const uint crc = BE_SWAP(data[0]);
  return crc == CCRC32::Calculate(data + 1, 0x2000 - 4, 0xFFFFFFFF) ? kCR_READY : kCR_CRC_MISMATCH;
}
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
int
#else
uint
#endif
CardStat::GetFileLength() { return mStat.length; }

int CardStat::GetTime() const { return mStat.time; }
#if RSTL_PRIME1
int CardStat::GetBannerFormat() { return CARDGetBannerFormat(&mStat); }

int CardStat::GetIconFormat(int idx) { return CARDGetIconFormat(&mStat, idx); }
#endif

int CardStat::GetCommentAddr() const { return mStat.commentAddr; }

void CardStat::SetBannerFormat(int format) { CARDSetBannerFormat(&mStat, format); }

void CardStat::SetIconFormat(int format, int idx) { CARDSetIconFormat(&mStat, idx, format); }

void CardStat::SetIconSpeed(int speed, int idx) { CARDSetIconSpeed(&mStat, idx, speed); }

void CardStat::SetIconAddr(int addr) { CARDSetIconAddress(&mStat, addr); }

void CardStat::SetCommentAddr(int addr) { CARDSetCommentAddress(&mStat, addr); }

#if RSTL_PRIME1 && RSTL_VERSION < RSTL_GM8P_00
void CMemoryCardSys::CCardFileInfo::SetComment(const rstl::string& comment) { mComment = comment; }
#endif

CMemoryCardSys::CMemoryCardSys() {
  Initialize();
  mIsCardSysExists = true;
}

// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1 || !TARGET_PC
void CMemoryCardSys::Initialize() {
  if (!mIsInitialized) {
    CARDInit();
    mIsInitialized = true;
  }
}
#endif

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

#if RSTL_PRIME1
ECardResult CMemoryCardSys::MountCardSync(EMemoryCardPort port) {
  return static_cast< ECardResult >(CARDMount(port, AllocCardWorkArea(port), nullptr));
}

#endif

ECardResult CMemoryCardSys::MountCard(EMemoryCardPort port) {
  return static_cast< ECardResult >(
      CARDMountAsync(port, AllocCardWorkArea(port), nullptr, nullptr));
}

ECardResult CMemoryCardSys::UnmountCard(EMemoryCardPort port) {
  ECardResult result = static_cast< ECardResult >(CARDUnmount(port));
  FreeCardWorkArea(port);
  return result;
}

ECardResult CMemoryCardSys::FormatCard(EMemoryCardPort port) {
  return static_cast< ECardResult >(CARDFormatAsync(port, nullptr));
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

#if RSTL_PRIME1
#if RSTL_VERSION < RSTL_GM8P_00
SMemoryCardFileInfo::SMemoryCardFileInfo(int cardPort, const rstl::string& name) : mName(name) {
  mFileInfo.chan = cardPort;
  mFileInfo.fileNo = -1;
}
#endif

#if RSTL_VERSION < RSTL_GM8P_00
CMemoryCardSys::CCardFileInfo::CCardFileInfo(EMemoryCardPort port, const rstl::string& name)
: mStatus(kS_Standby)
, mFileName(name)
, x38_(0)
, mBannerTex(kInvalidAssetId) {
  mFileInfo.chan = port;
  mFileInfo.fileNo = -1;
}
#endif

#if RSTL_VERSION < RSTL_GM8P_00
void CMemoryCardSys::CCardFileInfo::LockBannerToken(CAssetId bannerTxtr, CSimplePool& pool) {
  mBannerTex = bannerTxtr;
  mBannerTok = TLockedToken< CTexture >(pool.GetObj(SObjectTag('TXTR', mBannerTex)));
}
#endif

#if RSTL_VERSION < RSTL_GM8P_00
void CMemoryCardSys::CCardFileInfo::LockIconToken(CAssetId iconTxtr, int speed, CSimplePool& pool) {
  mIconToks.push_back(Icon(iconTxtr, speed, pool));
}
#endif

ECardResult SMemoryCardFileInfo::Open() {
  return static_cast< ECardResult >(CARDOpen(GetFileCardPort(), mName.data(), &mFileInfo));
}

#if RSTL_VERSION < RSTL_GM8P_00
ECardResult CMemoryCardSys::CCardFileInfo::CreateFile() {
  const uint size = CalculateTotalDataSize();
  return static_cast< ECardResult >(
      CARDCreateAsync(GetCardPort(), mFileName.data(), size, &mFileInfo, nullptr));
}
#endif

ECardResult CMemoryCardSys::DeleteFile(EMemoryCardPort port, const rstl::string& name) {
  return static_cast< ECardResult >(CARDDeleteAsync(port, name.data(), nullptr));
}
#endif

ECardResult CMemoryCardSys::FastDeleteFile(EMemoryCardPort port, int fileNo) {
  return static_cast< ECardResult >(CARDFastDeleteAsync(port, fileNo, nullptr));
}

#if RSTL_PRIME1
ECardResult SMemoryCardFileInfo::Close() {
  const CMemoryCardSys::EMemoryCardPort port = GetFileCardPort();
  ECardResult result = static_cast< ECardResult >(CARDClose(&mFileInfo));
  mFileInfo.chan = port;
  return result;
}

#if RSTL_VERSION < RSTL_GM8P_00
ECardResult CMemoryCardSys::CCardFileInfo::CloseFile() {
  const EMemoryCardPort port = GetCardPort();
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
}

#if RSTL_PRIME1
#if RSTL_VERSION < RSTL_GM8P_00
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
#endif

#if RSTL_VERSION < RSTL_GM8P_00
ECardResult CMemoryCardSys::CCardFileInfo::PumpCardTransfer() {
  if (mStatus == kS_Standby) {
    return kCR_READY;
  } else if (mStatus == kS_Transferring) {
    ECardResult result = GetResultCode(GetCardPort());
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
    result = SetStatus(GetCardPort(), GetFileNo(), stat);
    if (result == kCR_READY) {
      return kCR_BUSY;
    }
    return result;
  } else {
    ECardResult result = GetResultCode(GetCardPort());
    if (result != kCR_READY) {
      return result;
    }
    mStatus = kS_Standby;
    return kCR_READY;
  }
}
#endif

#if RSTL_VERSION < RSTL_GM8P_00
ECardResult CMemoryCardSys::CCardFileInfo::GetStatus(CardStat& stat) {
  ECardResult result = CMemoryCardSys::GetStatus(GetCardPort(), GetFileNo(), stat);
  if (result != kCR_READY) {
    return result;
  }
  stat.SetCommentAddr(4);
  stat.SetIconAddr(68);
  int bannerFormat;
  if (mBannerTex == kInvalidAssetId) {
    bannerFormat = CARD_STAT_BANNER_NONE;
  } else if ((*mBannerTok)->GetTexelFormat() == kTF_RGB5A3) {
    bannerFormat = CARD_STAT_BANNER_RGB5A3;
  } else {
    bannerFormat = CARD_STAT_BANNER_C8;
  }
  stat.SetBannerFormat(bannerFormat);
  int i = 0;
  for (; i < mIconToks.size(); ++i) {
    int format = CARD_STAT_ICON_C8;
    if (mIconToks[i].mTex->GetTexelFormat() == kTF_RGB5A3) {
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

#if RSTL_VERSION < RSTL_GM8P_00
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
}
#endif

ECardResult CMemoryCardSys::GetSerialNo(EMemoryCardPort port, long long& serialOut) {
  return static_cast< ECardResult >(CARDGetSerialNo(port, reinterpret_cast< u64* >(&serialOut)));
}

ECardResult CMemoryCardSys::GetStatus(EMemoryCardPort port, int fileNo, CardStat& statOut) {
  CARDStat stat;
  ECardResult result = static_cast< ECardResult >(CARDGetStatus(port, fileNo, &stat));
  memcpy(&statOut.mStat, &stat, sizeof(stat));
  return result;
}

ECardResult CMemoryCardSys::SetStatus(EMemoryCardPort port, int fileNo, const CardStat& stat) {
  return static_cast< ECardResult >(
      CARDSetStatusAsync(port, fileNo, const_cast< CARDStat* >(&stat.mStat), nullptr));
}

// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
bool CMemoryCardSys::mIsInitialized;
bool CMemoryCardSys::mIsCardSysExists;
bool CMemoryCardSys::mIsCardBusy;
TCardWorkArea CMemoryCardSys::mWorkAreaA;
TCardWorkArea CMemoryCardSys::mWorkAreaB;

#endif
TCardWorkArea& CMemoryCardSys::WorkAreaVector(EMemoryCardPort port) {
  switch (port) {
  case kCS_SlotA:
    return mWorkAreaA;
  case kCS_SlotB:
    return mWorkAreaB;
  default:
    return mWorkAreaA;
  }
}

char* CMemoryCardSys::AllocCardWorkArea(EMemoryCardPort port) {
  TCardWorkArea& area = WorkAreaVector(port);
  area.resize(0xa000);
  char* data = area.data();
  DCInvalidateRange(data, area.size());
  return data;
}

void CMemoryCardSys::FreeCardWorkArea(EMemoryCardPort port) {
  TCardWorkArea& area = WorkAreaVector(port);
  area = TCardWorkArea();
}