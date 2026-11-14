#ifndef _CRESLOADER
#define _CRESLOADER
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#if RSTL_PRIME1
#include "Kyoto/CPakFile.hpp"
#include "Kyoto/SObjectTag.hpp"
#endif

#include "rstl/types.h"

#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"
#include "rstl/pair.hpp"

#include "rstl/string.hpp"
#include "rstl/vector.hpp"

#include "Kyoto/CDvdRequest.hpp"
#include "Kyoto/CPakFile.hpp"
#if !RSTL_PRIME1
#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/Streams/CLookaheadRes.hpp"
#endif

class CDvdRequest;
class CInputStream;

class CBufferedDvdRequest;

class CResLoader {
public:
  typedef rstl::list< rstl::auto_ptr< CPakFile > > PakList;
  typedef CPakFile::CResInfo ResInfo;
#if !RSTL_PRIME1
  typedef rstl::list< CLookaheadRes > GroupCacheList;
#endif

  enum ECompressionType {
    kCompressionType_Uncompressed,
    kCompressionType_Compressed,
  };
  CResLoader();
  ~CResLoader();
  void MoveToCorrectLoadedList(const rstl::auto_ptr< CPakFile >& pak);
  bool CacheFromPak(RSTL_PRIME1_CONST CPakFile& pak, CAssetId asset) const;

  bool CacheFromPakForLoad(CPakFile& pak, CAssetId asset);
  CPakFile* FindResourceForLoad(const SObjectTag& tag);
  CPakFile* FindResourceForLoad(RSTL_PRIME1_CONST CAssetId asset);

#if RSTL_KYOTO_RES_LOADER_NETWORK_ASSETS
  void FindResource(const SObjectTag& tag);
#else
  CPakFile* FindResource(const SObjectTag& tag);
#endif
  bool ResourceExists(CAssetId asset);

  void ClearCache();
  void AsyncIdlePakLoading();
  bool AreAllPaksLoaded() const;
  const SObjectTag* GetResourceIdByName(const char* name) const;
  FourCC GetResourceTypeById(CAssetId asset) const;
  bool ResourceExists(const SObjectTag& tag) const;

  uint ResourceSize(const SObjectTag& tag) const;
  uint GetResourceOffset(const SObjectTag& tag) const;

  ECompressionType GetResourceCompression(const SObjectTag& tag) const;
  CDvdRequest* LoadResourceAsync(const SObjectTag& tag, char* extBuf);
  CBufferedDvdRequest* LoadResourceAsync(const SObjectTag& tag);
  CDvdRequest* LoadResourcePartAsync(const SObjectTag& tag, int offset, int length, char* extBuf);

  CInputStream* LoadNewResourceSync(const SObjectTag& tag, char* extBuf);
  CInputStream* LoadResourceFromMemorySync(const SObjectTag& tag, const void* extBuf);
#if RSTL_KYOTO_RES_LOADER_NETWORK_ASSETS
  bool LoadMemResourceSync(const SObjectTag& tag, char** extBuf, int* len);
#else
  void LoadMemResourceSync(const SObjectTag& tag, char** extBuf, int* len);
#endif
  CInputStream* LoadNewResourcePartSync(const SObjectTag& tag, int offset, int length,
                                        char* extBuf);

  const rstl::vector< CAssetId >* GetTagListForFile(const rstl::string& pak) const;
  void AddPakFileAsync(const rstl::string&, bool, bool);
  void RemovePakFile(const rstl::string&);

  rstl::vector< rstl::pair< rstl::string, SObjectTag > > GetResourceIdToNameList() const;
  // Guessed name
  const rstl::vector< rstl::pair< rstl::string, SObjectTag > >*
  GetNameListForFile(const rstl::string& filePath) const;
  bool ContainsResourceInAramPaks(const SObjectTag& tag) const; // Guessed name

  int GetPakCount() const;
  RSTL_ECHOES_CONST CPakFile* GetPakFile(int idx) const;
#if !RSTL_PRIME1
  CPakFile* GetPakFile(int idx);
  void KillLookahead(const CLookaheadRes* cache);
#endif

private:
#if RSTL_KYOTO_RES_LOADER_NETWORK_ASSETS
  // Guessed name
  static CDvdRequest* ReadNetworkFileToBuffer(const char* filename, char** buffer, int offset,
                                              int length);
#endif
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  GroupCacheList::iterator FindGroupCache(const CDvdFile* file, uint offset, uint size);
  GroupCacheList::iterator AddGroupCache(CDvdFile* file, uint offset, uint size);

  GroupCacheList mGroupCaches;
#endif
  PakList mAramList;
  PakList mPakLoadedList;
  PakList mPakLoadingList;
  PakList::iterator mCurPak;

  mutable CAssetId mCachedResId;
  mutable const ResInfo* mCachedResInfo;

  bool mForwardSeek;
};
#if !RSTL_PRIME1
CHECK_SIZEOF(CResLoader, RSTL_KYOTO_ASSET_ID_CLASS ? 0x78 : 0x70)
#endif

#endif // _CRESLOADER
