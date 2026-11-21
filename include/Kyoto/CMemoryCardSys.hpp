#ifndef _CMEMORYCARDSYS
#define _CMEMORYCARDSYS
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#endif

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#endif
#include "string.h"
#include "rstl/types.h"

#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Streams/CMemoryStreamOut.hpp"
#include "Kyoto/TToken.hpp"
#if !RSTL_CORRUPTION
#include "Kyoto/Platform.h"
#endif

#include "dolphin/card.h"
#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

#if !RSTL_CORRUPTION
#define RSTL_CARD_SAVE_BLOCKS (RSTL_VERSION >= RSTL_GM8P_00)
#endif

#if !RSTL_CORRUPTION
#if !RSTL_CARD_SAVE_BLOCKS
typedef rstl::vector< uchar, rstl::aligned_allocator > TCardFileBuffer;
#endif

typedef rstl::vector< char, rstl::aligned_allocator > TCardWorkArea;
#endif

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

  uint GetFileLength();
  int GetTime() const;
  int GetCommentAddr() const;
#if !RSTL_CORRUPTION
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
#if RSTL_CORRUPTION
  enum EMemoryCardPort { kCardPort_Invalid = -1, kCS_SlotA, kCS_SlotB };
#else
  enum EMemoryCardPort { kCS_SlotA, kCS_SlotB };
#endif

#if RSTL_CORRUPTION
  static const int kSupportedSectorSize = 0x2000;

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

    enum { kHeaderSize = 8 };

    /* 0x000 */ int mSlotBlocks;
    /* 0x004 */ int mAlignedUserDataSize;
    /* 0x008 */ int mSlot;
    /* 0x00C */ int mGeneration;
    /* 0x010 */ EStatus mState;
    /* 0x014 */ bool mNewFile;
    CARDFileInfo mFileInfo;
    rstl::string mFileName;
    rstl::string mComment;
    /* 0x04C */ int x4c_;
    CAssetId mBannerTex;
    rstl::optional_object< TLockedToken< CTexture > > mBannerTok;
    rstl::reserved_vector< Icon, 8 > mIconToks;
    rstl::vector< uchar, rstl::aligned_allocator > mHeaderBuffer;
    /* 0x13C */ rstl::reserved_vector< SSaveSlot, 2 > mSlots;
    /* 0x170 */ rstl::vector< uchar > mLoadedData;
    /* 0x180 */ rstl::vector< uchar > mUserDataWrite;

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
    ECardResult ReadSelfAsync();
    ECardResult TryCompleteRead();
    ECardResult WriteSelfAsync();
    ECardResult TryCompleteWrite();
    ECardResult CreateFileSync(); // Guessed name
    ECardResult WriteFileSync(); // Guessed name
    ECardResult ReadFileSync(); // Guessed name
    ECardResult Close(); // Guessed name
    ECardResult GetStatus(CardStat& stat);
    EMemoryCardPort GetPort();
    int GetFileNo();
    int GetTotalNumBlocks();
    bool IsRepairingHeader() const { return mState == kS_RepairHeader; }

    rstl::vector< uchar >& LoadedData() { return mLoadedData; }
    rstl::vector< uchar >& SaveBuffer() { return mUserDataWrite; }
#else
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

#if RSTL_CARD_SAVE_BLOCKS
    typedef rstl::vector< uchar > TCardFileBuffer;

    int mBlockCount;
    int mBlockSize;
    int mWriteBlock;
    uint mSequence;
#endif
    EStatus mStatus;
#if RSTL_CARD_SAVE_BLOCKS
    bool mWriteBothBlocks;
#endif
    CARDFileInfo mFileInfo;
    rstl::string mFileName;
    rstl::string mComment;
    int x38_;
    CAssetId mBannerTex;
    rstl::optional_object< TLockedToken< CTexture > > mBannerTok;
    rstl::reserved_vector< Icon, 8 > mIconToks;
#if RSTL_CARD_SAVE_BLOCKS
    rstl::vector< uchar, rstl::aligned_allocator > mHeaderBuffer;
    rstl::reserved_vector< SSaveBlock, 2 > mSaveBlocks;
#endif
    rstl::vector< uchar > mSaveBuffer;
    TCardFileBuffer mCardBuffer;

  public:
    CCardFileInfo(EMemoryCardPort port, const rstl::string& name);
    ~CCardFileInfo();

    void SetComment(const rstl::string& comment);
    void ResetBannerAndIcons();
    void LockBannerToken(CAssetId bannerTxtr, CSimplePool& pool);
    void LockIconToken(CAssetId iconTxtr, int speed, CSimplePool& pool);

    ECardResult Open();
    ECardResult CreateFile();
    ECardResult CloseFile();
    ECardResult StartRead();
    ECardResult TryFileRead();
    ECardResult FileRead();
    ECardResult FinishRead();
    ECardResult CheckHeader();
    ECardResult WriteFile();
    ECardResult WriteBlock(int index);
    ECardResult PumpCardTransfer();
    ECardResult GetStatus(CardStat& stat);
    EMemoryCardPort GetPort();
    int GetFileNo();
    int GetTotalBlockCount();
    uint CalculateBannerDataSize();
    uint CalculateTotalDataSize();
    void BuildCardBuffer();
    void BuildHeader();
    void BuildSaveBlock();
    void WriteBannerData(COutputStream& out);
    void WriteIconData(COutputStream& out);

    bool IsRepairingHeader() const { return mStatus == kS_Done; }
    rstl::vector< uchar >& SaveBuffer() { return mSaveBuffer; }
    TCardFileBuffer& CardBuffer() { return mCardBuffer; }
    CMemoryStreamOut BeginMemoryOut(uint sz);
  };
#endif
#endif

#if RSTL_CORRUPTION
  private:
    ECardResult CheckHeaderCrc();
    ECardResult ExtractUserData();
    void BuildWriteBuffers();
    void WriteBannerData(COutputStream& out) const;
    void WriteIconData(COutputStream& out) const;
    ECardResult WriteSaveSlot(int slot);
  };

  CMemoryCardSys();
#else
  CMemoryCardSys(IF_TARGET_PC(const char* game, const char* maker));
#endif
  ~CMemoryCardSys();

#if RSTL_CORRUPTION
  static ECardResult GetResultCode(int);
  static ECardResult MountCard(EMemoryCardPort port);
  static ECardResult CheckCard(EMemoryCardPort port);
  static ECardResult GetStatus(EMemoryCardPort port, int fileNo, CardStat& statOut);
  static ECardResult SetStatus(EMemoryCardPort port, int fileNo, const CardStat& stat);
  static ECardResult SetStatusSync(EMemoryCardPort port, int fileNo, const CardStat& stat); // Guessed name
  static ECardResult FastDeleteFile(EMemoryCardPort port, int fileNo);
  static ECardResult FormatCard(EMemoryCardPort port);
  static ECardResult MountSync(EMemoryCardPort port); // Guessed name
  static ECardResult FormatSync(EMemoryCardPort port); // Guessed name
  static ECardResult DeleteNamedFile(EMemoryCardPort port, const rstl::string& name); // Guessed name

  static ProbeResults IsMemoryCardInserted(EMemoryCardPort);
  static ECardResult GetSerialNo(EMemoryCardPort port, long long& serialOut);
  static ECardResult UnmountCard(EMemoryCardPort);
  static ECardResult GetNumFreeBytes(EMemoryCardPort port, uint& freeBytes, uint& freeFiles);
  static rstl::vector< char, rstl::aligned_allocator >& WorkAreaVector(EMemoryCardPort port);
  static char* AllocCardWorkArea(EMemoryCardPort port);
  static void FreeCardWorkArea(EMemoryCardPort port);
  inline void Initialize();

  static bool mIsCardBusy;
#else
#if RSTL_VERSION == RSTL_GM8EAB_00
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
  static ECardResult MountCardSync(EMemoryCardPort port);
  static ECardResult CheckCard(EMemoryCardPort port);
  static ECardResult GetStatus(EMemoryCardPort port, int fileNo, CardStat& statOut);
  static ECardResult SetStatus(EMemoryCardPort port, int fileNo, const CardStat& stat);
  static ECardResult DeleteFile(EMemoryCardPort port, const rstl::string& name);
  static ECardResult FastDeleteFile(EMemoryCardPort port, int fileNo);
  static ECardResult FormatCard(EMemoryCardPort port);

  static ProbeResults IsMemoryCardInserted(EMemoryCardPort);
  static ECardResult GetSerialNo(EMemoryCardPort port, long long& serialOut);
  static ECardResult UnmountCard(EMemoryCardPort);
  static ECardResult Rename(EMemoryCardPort, const rstl::string&, const rstl::string&);
  static ECardResult GetNumFreeBytes(EMemoryCardPort port, uint& freeBytes, uint& freeFiles);
  static TCardWorkArea& WorkAreaVector(EMemoryCardPort port);
  static char* AllocCardWorkArea(EMemoryCardPort port);
  static void FreeCardWorkArea(EMemoryCardPort port);
  inline void Initialize(IF_TARGET_PC(const char* game, const char* maker));

  static bool mIsCardBusy;
#endif
#endif

private:
  static bool mIsInitialized;
  static bool mIsCardSysExists;
#if RSTL_CORRUPTION
  static rstl::vector< char, rstl::aligned_allocator > mWorkAreaA;
  static rstl::vector< char, rstl::aligned_allocator > mWorkAreaB;
#else
  static TCardWorkArea mWorkAreaA;
  static TCardWorkArea mWorkAreaB;
#endif
};

#if RSTL_CORRUPTION
NESTED_CHECK_SIZEOF(CMemoryCardSys, CCardFileInfo, 0x190)


#else
#if RSTL_VERSION != RSTL_GM8EAB_00
NESTED_CHECK_SIZEOF(CMemoryCardSys, CCardFileInfo, RSTL_CARD_SAVE_BLOCKS ? 0x16c : 0x114)

#if !RSTL_CARD_SAVE_BLOCKS
inline CMemoryCardSys::CCardFileInfo::~CCardFileInfo() {}
#endif

#if RSTL_PRIME1
inline CMemoryStreamOut CMemoryCardSys::CCardFileInfo::BeginMemoryOut(uint sz) {
  mSaveBuffer.resize(sz);
  return CMemoryStreamOut(mSaveBuffer.data(), sz);
}
#endif
#endif // RSTL_VERSION != RSTL_GM8EAB_00

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

extern CMemoryCardSys* gpMemoryCardSys;

#endif
#endif // _CMEMORYCARDSYS
