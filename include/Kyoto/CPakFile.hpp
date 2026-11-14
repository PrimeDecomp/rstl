#ifndef _CPAKFILE
#define _CPAKFILE
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"

#include "rstl/types.h"
#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/IObjectStore.hpp"

#include "rstl/RstlVersions.h"
#include "rstl/auto_ptr.hpp"
#include "rstl/pair.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CMemoryInStream;

class CDvdRequest;

class CPakFile {
public:
  enum EAsyncPhase { kAP_Warmup, kAP_InitialHeaderLoad, kAP_DataLoad, kAP_Loaded };
#if RSTL_PRIME1
#pragma pack(push, 2)
#else
#pragma pack(push, 1)
#endif
  struct CResInfo {
    CAssetId mId;
#if RSTL_PRIME1
    uchar mData[6];
#else
    uchar mData[7];
#endif

#if !RSTL_KYOTO_PAK_FILE_CORRUPTION
    CResInfo(CAssetId id, uint type, uint offset, uint size, uint flags);
#endif
    CResInfo(CAssetId id, uint type, uint offset, uint size, uint flags,
             uint lookaheadAfterResourceSize);
#if RSTL_KYOTO_PAK_FILE_CORRUPTION
    CResInfo(const CResInfo& other);
    void operator=(const CResInfo& other);
#endif
    // Guessed name: builds the key that lookups compare against.
    static CResInfo SearchKey(CAssetId id) { return CResInfo(id, 'TXTR', 0, 0, 0, 0); }

    uint GetType() const;
    uint GetOffset() const;
    uint GetSize() const;
    bool IsCompressed() const;
    void SetLookaheadAfterResourceSize(uint size);
    uint GetLookaheadAfterResourceSize() const;

    CAssetId GetId() const { return mId; }
    bool operator<(const CResInfo& other) const { return GetId() < other.GetId(); }
  };
#pragma pack(pop)

#if RSTL_PRIME1
  typedef rstl::vector< CResInfo > ResListType;
#else
  typedef rstl::vector< CResInfo, rstl::aligned_allocator > ResListType;
#endif

  CPakFile(const rstl::string& filename, RSTL_PRIME1_CONST bool buildDepList,
           RSTL_PRIME1_CONST bool worldPak);

  ~CPakFile();

  CDvdFile& DvdFile() { return mFile; }
  const CDvdFile& GetDvdFile() const { return mFile; }

  void AsyncIdle();
  bool IsARAMPak() const { return mAramFile; }
  bool IsWorldPak() const { return mWorldPak; }
  bool IsStashedInARAM() const { return mStashedInARAM; }
  bool IsCompletelyLoaded() const { return mAsyncLoadPhase == kAP_Loaded; }
  void EnsureWorldPakReady();
  // Guessed name.
  void StashWorldPak();

  rstl::vector< rstl::pair< rstl::string, SObjectTag > >& NameList() { return mNameList; }
  const rstl::vector< rstl::pair< rstl::string, SObjectTag > >& GetStringToObjectList() const {
    return mNameList;
  }
  const rstl::vector< CAssetId >* GetDepList() const;
  const SObjectTag* GetResIdByName(const char* name) const;
  const CResInfo* GetResInfo(CAssetId id) RSTL_PRIME1_CONST;
  const CResInfo* GetResInfoForLoadDirectionless(CAssetId id);
  const CResInfo* GetResInfoForLoadPreferForward(CAssetId id);
  int GetFakeStaticSize() const;

private:
  static uint GetBucketForId(CAssetId id) { // Guessed name
#if RSTL_KYOTO_ASSET_ID_CLASS
    return static_cast< uint >(id.Value()) & 0xff;
#else
    return id & 0xff;
#endif
  }
  void UpdateFakeStaticSize();
  void
  RebuildResourceLists(const rstl::vector< CResInfo, rstl::aligned_allocator >& sortedResources);
  void LoadResourceTable(CMemoryInStream& in);
  void Warmup();
  void InitialHeaderLoad();
  void DataLoad();

  CDvdFile mFile;
  bool mBuildDepList : 1;
  bool mAramFile : 1;
  bool mWorldPak : 1;
  bool mStashedInARAM : 1;
  EAsyncPhase mAsyncLoadPhase;
  rstl::auto_ptr< CDvdRequest > mDvdReq;
  rstl::vector< uchar, rstl::aligned_allocator > mHeaderData;
  uint mResTableOffset;
#if RSTL_PRIME1
  int mResTableCount;
#else
  int mResTableCount;
  int mFakeStaticSize;
#endif
#if !defined(TARGET_PC)
  RSTL_ECHOES_CONST void* mpARAMHeader;
#endif // !defined(TARGET_PC)

  rstl::vector< rstl::pair< rstl::string, SObjectTag > > mNameList;
  rstl::vector< CAssetId > mDepList;
  ResListType mResInfoBuckets;
#if !RSTL_PRIME1
  rstl::vector< int > mBucketOffsets;
#endif
#if RSTL_KYOTO_PAK_FILE_CORRUPTION
  rstl::vector< rstl::pair< CAssetId, rstl::string > > x98_names; // Guessed name
#endif
#if !RSTL_PRIME1
  mutable
#endif
  int mCurrentSeek;
};
CHECK_SIZEOF(CPakFile, RSTL_PRIME1 ? 0x88 : RSTL_KYOTO_PAK_FILE_CORRUPTION ? 0xac : 0x9c)
NESTED_CHECK_SIZEOF(CPakFile, CResInfo, RSTL_PRIME1 ? 0xa : RSTL_KYOTO_ASSET_ID_CLASS ? 0xf : 0xb)

#if !RSTL_PRIME1 && !RSTL_KYOTO_PAK_FILE_CORRUPTION
namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CPakFile::CResInfo)
}
#endif

#endif // _CPAKFILE
