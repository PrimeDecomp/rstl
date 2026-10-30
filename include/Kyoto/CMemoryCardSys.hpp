#ifndef _CMEMORYCARDSYS
#define _CMEMORYCARDSYS
#include "rstl/RstlVersions.h"

#include "string.h"
#include "types.h"

#include "Kyoto/CSimplePool.hpp"
#if !RSTL_PRIME1
#include "Kyoto/TToken.hpp"
#endif

#include "Kyoto/Streams/CMemoryStreamOut.hpp"
#if RSTL_PRIME1
#include "Kyoto/TToken.hpp"
#endif

#include "dolphin/card.h"
#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

#if RSTL_PRIME1 && RSTL_VERSION < RSTL_GM8P_00
typedef rstl::vector< uchar, rstl::aligned_allocator > TCardFileBuffer;
#endif

typedef rstl::vector< char, rstl::aligned_allocator > TCardWorkArea;

class CTexture;

// Dolphin SDK result codes, with Retro's CRC mismatch extension.
enum ECardResult {
  kCR_CRC_MISMATCH = -1003, /* Extension enum for Retro's CRC check */
  kCR_FATAL_ERROR = -128,
  kCR_ENCODING = -13,
  kCR_NAMETOOLONG = -12,
  kCR_INSSPACE = -9,
  kCR_NOENT = -8,
  kCR_EXIST = -7,
  kCR_BROKEN = -6,
  kCR_IOERROR = -5,
  kCR_NOFILE = -4,
  kCR_NOCARD = -3,
  kCR_WRONGDEVICE = -2,
  kCR_BUSY = -1,
  kCR_READY = 0
};

struct ProbeResults {
  ECardResult mError;
  s32 mCardSize;   /* in megabits */
  s32 mSectorSize; /* in bytes */
};

struct CardStat {
  CARDStat mStat;

  CardStat() { memset(this, 0, sizeof(CardStat)); }

#if !RSTL_PRIME1
  uint GetFileLength();
#endif

  int GetTime() const;
  int GetCommentAddr() const;
#if RSTL_PRIME1
  int GetFileLength();
  int GetBannerFormat();
  int GetIconFormat(int idx);
#endif

  void SetBannerFormat(int format);
  void SetIconFormat(int format, int idx);
  void SetIconSpeed(int speed, int idx);
  void SetIconAddr(int addr);
  void SetCommentAddr(int addr);
};
CHECK_SIZEOF(CardStat, 0x6C);

class CMemoryCardSys {
public:
  enum EMemoryCardPort { kCS_SlotA, kCS_SlotB };

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION == RSTL_GM8EAB_00
  struct CCardFileInfo {
    CARDFileInfo mFileInfo;
  };

  struct CCardFileStatus {
    CARDStat mStat;

    CCardFileStatus() { memset(this, 0, sizeof(CCardFileStatus)); }
    uint GetFileLength() const;
  };
#else
  class CCardFileInfo {
    struct Icon {
      CAssetId mId;
      int mSpeed;
      TLockedToken< CTexture > mTex;

      Icon(CAssetId id, int speed, CSimplePool& pool);
    };

    enum EStatus {
      kS_Standby,
      kS_Transferring,
      kS_Done,
      kS_ReadingFirstBlock,
      kS_ReadingSecondBlock,
      kS_WritingHeader,
      kS_WritingBlock,
      kS_WritingBothBlocks,
      kS_SettingStatus
    };

    struct SSaveBlock {
      uint mSequence;
      bool mCorrupted;
      rstl::vector< uchar, rstl::aligned_allocator > mData;

      SSaveBlock();
      void Check();
    };

#if RSTL_VERSION >= RSTL_GM8P_00
    int mBlockCount;
    int mBlockSize;
    int mWriteBlock;
    uint mSequence;
#endif
    EStatus mStatus;
#if RSTL_VERSION >= RSTL_GM8P_00
    bool mWriteBothBlocks;
#endif
    CARDFileInfo mFileInfo;
    rstl::string mFileName;
    rstl::string mComment;
    int x38_;
    CAssetId mBannerTex;
    rstl::optional_object< TLockedToken< CTexture > > mBannerTok;
    rstl::reserved_vector< Icon, 8 > mIconToks;
#if RSTL_VERSION >= RSTL_GM8P_00
    rstl::vector< uchar, rstl::aligned_allocator > mHeaderBuffer;
    rstl::reserved_vector< SSaveBlock, 2 > mSaveBlocks;
#endif
    rstl::vector< u8 > mSaveBuffer;
#if RSTL_VERSION >= RSTL_GM8P_00
    rstl::vector< u8 > mCardBuffer;
#else
    TCardFileBuffer mCardBuffer;
#endif

  public:
    CCardFileInfo(EMemoryCardPort port, const rstl::string& name);
#if RSTL_VERSION >= RSTL_GM8P_00
    ~CCardFileInfo();
#else
    ~CCardFileInfo() {}
#endif

    void SetComment(const rstl::string& name);
    void LockBannerToken(CAssetId bannerTxtr, CSimplePool& sp);
    void LockIconToken(CAssetId iconTxtr, int speed, CSimplePool& sp);

    ECardResult PumpCardTransfer();
    ECardResult CreateFile();
    ECardResult Open();
    ECardResult StartRead();
    ECardResult TryFileRead();
    ECardResult FileRead();
    ECardResult FinishRead();
    ECardResult CheckHeader();
    ECardResult WriteFile();
    ECardResult WriteBlock(int index);
    ECardResult CloseFile();
    ECardResult GetStatus(CardStat& stat);
    EMemoryCardPort GetCardPort();
    int GetFileNo();
    uint CalculateBannerDataSize();
    uint CalculateTotalDataSize();
    void BuildCardBuffer();
    void BuildHeader();
    void BuildSaveBlock();
    int GetTotalBlockCount();
    void ResetBannerAndIcons();
    void WriteBannerData(COutputStream& out);
    void WriteIconData(COutputStream& out);

    rstl::vector< u8 >& SaveBuffer() { return mSaveBuffer; }

    inline CMemoryStreamOut BeginMemoryOut(uint sz) {
      mSaveBuffer.resize(sz);
      return CMemoryStreamOut(mSaveBuffer.data(), sz);
    }
  };
#endif
#else
  // Echoes stores a 0x2000-byte header block followed by two alternating copies of the save data.
  class CCardFileInfo {
    struct Icon {
      CAssetId mId;
      int mSpeed;
      TLockedToken< CTexture > mTex;

      Icon(CAssetId id, int speed, CSimplePool& pool);
    };

    struct SSaveSlot {
      int mGeneration;
      bool mCorrupt;
      rstl::vector< uchar, rstl::aligned_allocator > mData;

      SSaveSlot();
      void CheckCrc();
    };

    enum EStatus {
      kS_Standby,
      kS_ReadHeader,
      kS_RepairHeader,
      kS_ReadSlotA,
      kState_ReadingData1,
      kS_WriteHeader,
      kS_WriteSlot,
      kS_WriteFirstSlot,
      kState_WritingStatus,
    };

    int mSlotBlocks;
    int mAlignedUserDataSize;
    int mSlot;
    int mGeneration;
    EStatus mState;
    bool mNewFile;
    CARDFileInfo mFileInfo;
    rstl::string mFileName;
    rstl::string mComment;
    int x4c_;
    CAssetId mBannerTex;
    rstl::optional_object< TLockedToken< CTexture > > mBannerTok;
    rstl::reserved_vector< Icon, 8 > mIconToks;
    rstl::vector< uchar, rstl::aligned_allocator > mHeaderBuffer;
    rstl::reserved_vector< SSaveSlot, 2 > mSlots;
    rstl::vector< uchar > mLoadedData;
    rstl::vector< uchar > mUserDataWrite;

  public:
    CCardFileInfo(EMemoryCardPort port, const rstl::string& name);
    ~CCardFileInfo();

    void SetComment(const rstl::string& comment);
    void ResetHeaderInfo();
    void LockBannerToken(CAssetId bannerTxtr, CSimplePool& pool);
    void LockIconToken(CAssetId iconTxtr, int speed, CSimplePool& pool);
    void BuildHeaderBuffer();

    ECardResult Open();
    ECardResult CreateFile();
    ECardResult StartRead();
    ECardResult PumpCardRead();
    ECardResult WriteFile();
    ECardResult PumpCardTransfer();
    ECardResult GetStatus(CardStat& stat);
    EMemoryCardPort GetPort();
    int GetFileNo();
    int GetTotalNumBlocks();
    bool IsRepairingHeader() const { return mState == kS_RepairHeader; }

    rstl::vector< uchar >& LoadedData() { return mLoadedData; }
    rstl::vector< uchar >& SaveBuffer() { return mUserDataWrite; }

  private:
    ECardResult CheckHeaderCrc();
    ECardResult SelectSaveSlot();
    void BuildSaveSlot();
    void WriteBannerData(COutputStream& out);
    void WriteIconData(COutputStream& out);
    ECardResult WriteSaveSlot(int slot);
  };
#endif

  CMemoryCardSys();
  ~CMemoryCardSys();

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1 && RSTL_VERSION == RSTL_GM8EAB_00
  ECardResult MountCard(EMemoryCardPort port);
  ECardResult UnmountCard(EMemoryCardPort port);
  ECardResult FormatCard(EMemoryCardPort port);
  bool IsMemoryCardInserted(EMemoryCardPort port);
  ECardResult GetSectorSize(EMemoryCardPort port, uint& sectorSizeOut);
  ECardResult GetNumFreeBytes(EMemoryCardPort port, uint& freeBytesOut);
  ECardResult GetFileStatus(EMemoryCardPort port, int fileNo, CCardFileStatus& statusOut);
  int GetFileNumber(const CCardFileInfo& info);
  ECardResult OpenFile(EMemoryCardPort port, const rstl::string& name, CCardFileInfo& info);
  ECardResult CreateFile(EMemoryCardPort port, const rstl::string& name, uint size,
                         CCardFileInfo& info);
  ECardResult CloseFile(CCardFileInfo& info);
  ECardResult Read(CCardFileInfo& info, void* dest, unsigned long len, unsigned long offset);
  ECardResult Write(CCardFileInfo& info, const void* src, unsigned long len, unsigned long offset);
#else
  static ECardResult GetResultCode(int);
  static ECardResult MountCard(EMemoryCardPort port);
#if RSTL_PRIME1
  static ECardResult MountCardSync(EMemoryCardPort port);
#endif
  static ECardResult CheckCard(EMemoryCardPort port);
  static ECardResult GetStatus(EMemoryCardPort port, int fileNo, CardStat& statOut);
  static ECardResult SetStatus(EMemoryCardPort port, int fileNo, const CardStat& stat);
#if RSTL_PRIME1
  static ECardResult DeleteFile(EMemoryCardPort port, const rstl::string& name);
#endif
  static ECardResult FastDeleteFile(EMemoryCardPort port, int fileNo);
  static ECardResult FormatCard(EMemoryCardPort port);

  static ProbeResults IsMemoryCardInserted(EMemoryCardPort);
  static ECardResult GetSerialNo(EMemoryCardPort port, long long& serialOut);
  static ECardResult UnmountCard(EMemoryCardPort);
#if RSTL_PRIME1
  static ECardResult Rename(EMemoryCardPort, const rstl::string&, const rstl::string&);
#endif
  static ECardResult GetNumFreeBytes(EMemoryCardPort port, uint& freeBytes, uint& freeFiles);
  static TCardWorkArea& WorkAreaVector(EMemoryCardPort port);
  static char* AllocCardWorkArea(EMemoryCardPort port);
  static void FreeCardWorkArea(EMemoryCardPort port);
  inline void Initialize();

#if !RSTL_PRIME1
  static bool mIsCardBusy;
#endif
#endif

private:
  static bool mIsInitialized;
  static bool mIsCardSysExists;
  static TCardWorkArea mWorkAreaA;
  static TCardWorkArea mWorkAreaB;
};

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION != RSTL_GM8EAB_00
NESTED_CHECK_SIZEOF(CMemoryCardSys, CCardFileInfo, RSTL_VERSION >= RSTL_GM8P_00 ? 0x16c : 0x114)
#endif

struct SMemoryCardFileInfo {
  CARDFileInfo mFileInfo;
  rstl::string mName;
  rstl::vector< uchar, rstl::aligned_allocator > mSaveFileData;
  rstl::vector< uchar > mSaveData;

  SMemoryCardFileInfo(int cardPort, const rstl::string& name);
  SMemoryCardFileInfo(const SMemoryCardFileInfo& other)
  : mFileInfo(other.mFileInfo)
  , mName(other.mName)
  , mSaveFileData(other.mSaveFileData)
  , mSaveData(other.mSaveData) {}
  ~SMemoryCardFileInfo() {}

  ECardResult Open();
  ECardResult Close();
  CMemoryCardSys::EMemoryCardPort GetFileCardPort();
  int GetFileNo() const;
  ECardResult StartRead();
  ECardResult TryFileRead();
  ECardResult FileRead();
  ECardResult GetSaveDataOffset(uint& offOut);
};
CHECK_SIZEOF(SMemoryCardFileInfo, 0x44);

#if RSTL_VERSION == RSTL_GM8EAB_00
extern CMemoryCardSys* gpMemoryCardSys;
#endif
#else
NESTED_CHECK_SIZEOF(CMemoryCardSys, CCardFileInfo, 0x16C)
#endif

#endif // _CMEMORYCARDSYS
