#ifndef _CPAKFILE
#define _CPAKFILE
#include "rstl/RstlVersions.h"

#include "types.h"
#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/SObjectTag.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/pair.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"
#if !RSTL_PRIME1
#include "Kyoto/IObjectStore.hpp"
#endif

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
  struct SResInfo {
    CAssetId mId;
#if RSTL_PRIME1
    uchar mData[6];
#else
    uchar mData[7];
#endif

    SResInfo(uint id, uint fourCC, uint offset, uint size, uint flags);
    SResInfo(uint id, uint fourCC, uint offset, uint size, uint flags, uint groupedSize);
    // Guessed name: builds the key that lookups compare against.
    static SResInfo SearchKey(CAssetId id) { return SResInfo(id, 'TXTR', 0, 0, 0, 0); }

    uint GetType() const;
    uint GetOffset() const;
    uint GetSize() const;
    bool IsCompressed() const;
    void SetLookaheadAfterResourceSize(uint size);
    uint GetLookaheadAfterResourceSize() const;

    CAssetId GetId() const { return mId; }
    bool operator<(const SResInfo& other) const { return mId < other.mId; }
  };
#pragma pack(pop)

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
  const SResInfo* GetResInfo(uint id) RSTL_PRIME1_CONST;
  const SResInfo* GetResInfoForLoadDirectionless(uint id);
  const SResInfo* GetResInfoForLoadPreferForward(uint id);
  uint GetFakeStaticSize() const;

private:
  void UpdateFakeStaticSize();
  void
  RebuildResourceLists(const rstl::vector< SResInfo, rstl::aligned_allocator >& sortedResources);
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
  uint mResTableCount;
  void* mAramBase;
#else
  int mResTableCount;
  int mFakeStaticSize;
  const void* mpARAMHeader;
#endif

  rstl::vector< rstl::pair< rstl::string, SObjectTag > > mNameList;
  rstl::vector< CAssetId > mDepList;
#if RSTL_PRIME1
  rstl::vector< SResInfo > mResList;
  int mCurrentSeek;
#else
  rstl::vector< SResInfo, rstl::aligned_allocator > mResInfoBuckets;
  rstl::vector< int > mBucketOffsets;
  mutable int mCurrentSeek;
#endif
};
CHECK_SIZEOF(CPakFile, RSTL_PRIME1 ? 0x88 : 0x9c)
NESTED_CHECK_SIZEOF(CPakFile, SResInfo, RSTL_PRIME1 ? 0xa : 0xb)

#if !RSTL_PRIME1
namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CPakFile::SResInfo)
}
#endif

#endif // _CPAKFILE
