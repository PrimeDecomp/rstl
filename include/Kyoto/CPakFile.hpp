#ifndef _CPAKFILE
#define _CPAKFILE
#include "rstl/RstlVersions.h"

#include "types.h"
#if RSTL_PRIME1
#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/SObjectTag.hpp"
#endif

#include "rstl/auto_ptr.hpp"
#include "rstl/pair.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"
#if !RSTL_PRIME1
#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/IObjectStore.hpp"
#endif

class CMemoryInStream;

class CDvdRequest;

class CPakFile {
public:
  enum EAsyncPhase { kAP_Warmup, kAP_InitialHeaderLoad, kAP_DataLoad, kAP_Loaded };
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
// Resource entries store a four-byte ID followed by six packed metadata bytes.
#pragma pack(push, 2)
  struct SResInfo {
#else
#pragma pack(push, 1)
  struct CResInfo {
#endif

    CAssetId mId;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    uchar mData[6];
#else
    uchar mData[7];
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    SResInfo(uint id, uint fourCC, uint offset, uint size, uint flags);
#else
    CResInfo(uint id, uint fourCC, uint offset, uint size, uint flags, uint groupedSize);
    // Guessed name: builds the key that lookups compare against.
    static CResInfo SearchKey(CAssetId id) { return CResInfo(id, 'TXTR', 0, 0, 0, 0); }
#endif

    uint GetType() const;
    uint GetOffset() const;
    uint GetSize() const;
    bool IsCompressed() const;
#if !RSTL_PRIME1
    void SetLookaheadAfterResourceSize(uint size);
    uint GetLookaheadAfterResourceSize() const;
#endif

    CAssetId GetId() const { return mId; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    bool operator<(const SResInfo& other) const { return mId < other.mId; }
#else
    bool operator<(const CResInfo& other) const { return mId < other.mId; }
#endif

  };
#pragma pack(pop)

  CPakFile(const rstl::string& filename, RSTL_PRIME1_CONST bool buildDepList,
           RSTL_PRIME1_CONST bool worldPak);

  ~CPakFile();

  CDvdFile& DvdFile() { return mFile; }
  const CDvdFile& GetDvdFile() const { return mFile; }

  void AsyncIdle();
#if !RSTL_PRIME1
  bool IsARAMPak() const { return mAramFile; }
#endif

  bool IsWorldPak() const { return mWorldPak; }
#if !RSTL_PRIME1
  bool IsStashedInARAM() const { return mStashedInARAM; }
#endif

  bool IsCompletelyLoaded() const { return mAsyncLoadPhase == kAP_Loaded; }
  void EnsureWorldPakReady();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void sub_8036742c();
#else
  // Guessed name.
  void StashWorldPak();
#endif

  rstl::vector< rstl::pair< rstl::string, SObjectTag > >& NameList() { return mNameList; }
#if RSTL_PRIME1
  const rstl::vector< CAssetId >* GetDepList() const;
  const SObjectTag* GetResIdByName(const char* name) const;
  const SResInfo* GetResInfo(uint id) const;
  const SResInfo* GetResInfoForLoadDirectionless(uint id);
  const SResInfo* GetResInfoForLoadPreferForward(uint id);
  uint GetFakeStaticSize() const;

  bool IsARAMPak() const { return mAramFile; }
  bool IsStashedInARAM() const { return mStashedInARAM; }
#endif

  const rstl::vector< rstl::pair< rstl::string, SObjectTag > >& GetStringToObjectList() const {
    return mNameList;
  }
#if !RSTL_PRIME1
  const SObjectTag* GetResIdByName(const char* name) const;
  const CResInfo* GetResInfo(uint id);
  const CResInfo* GetResInfoForLoadDirectionless(uint id);
  const CResInfo* GetResInfoForLoadPreferForward(uint id);
  uint GetFakeStaticSize() const;
#endif

private:
#if !RSTL_PRIME1
  void UpdateFakeStaticSize();
  void
  RebuildResourceLists(const rstl::vector< CResInfo, rstl::aligned_allocator >& sortedResources);
  void LoadResourceTable(CMemoryInStream& in);
#endif

  void Warmup();
  void InitialHeaderLoad();
  void DataLoad();
#if RSTL_PRIME1
  void LoadResourceTable(CMemoryInStream& in);
#endif

  CDvdFile mFile;
  bool mBuildDepList : 1;
  bool mAramFile : 1;
  bool mWorldPak : 1;
  bool mStashedInARAM : 1;
  EAsyncPhase mAsyncLoadPhase;
  rstl::auto_ptr< CDvdRequest > mDvdReq;
  rstl::vector< uchar, rstl::aligned_allocator > mHeaderData;
  uint mResTableOffset;
// TODO: verify that this Prime/Echoes difference is real
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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::vector< SResInfo > mResList;
  int mCurrentSeek;
#else
  rstl::vector< CResInfo, rstl::aligned_allocator > mResInfoBuckets;
  rstl::vector< int > mBucketOffsets;
  mutable int mCurrentSeek;
#endif

};
CHECK_SIZEOF(CPakFile, RSTL_PRIME1 ? 0x88 : 0x9c)
#if RSTL_PRIME1
NESTED_CHECK_SIZEOF(CPakFile, SResInfo, 0xa)
#else
NESTED_CHECK_SIZEOF(CPakFile, CResInfo, 0xb)

namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CPakFile::CResInfo)
}
#endif

#endif // _CPAKFILE
