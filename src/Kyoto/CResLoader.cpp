#include "rstl/RstlVersions.h"
#include "rstl/macros.hpp"
#include "Kyoto/CResLoader.hpp"
#include "Kyoto/CPakFile.hpp"
#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Streams/CLZOInputStream.hpp"

#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "Kyoto/Streams/CZipInputStream.hpp"

#include "rstl/StringExtras.hpp"

#if RSTL_KYOTO_RES_LOADER_NETWORK_ASSETS
#include "Kyoto/Network/CBBASupport.hpp"
#include "Kyoto/Streams/CBBAInStream.hpp"
#endif

#include <string.h>

static inline int align_size(const int size) { return (size + 31) & ~31; }

CResLoader::CResLoader()
: mCurPak(mPakLoadedList.end())
, mCachedResId(kInvalidAssetId)
, mCachedResInfo(nullptr)
, mForwardSeek(false) {}

CResLoader::~CResLoader() {
  for (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      AUTO(it, mPakLoadingList.begin());
#else
      PakList::iterator it = mPakLoadingList.begin();
#endif
      it != mPakLoadingList.end(); ++it) {
    CPakFile* pak = it->get();
    while (!pak->IsCompletelyLoaded()) {
      pak->AsyncIdle();
    }
  }
}

void CResLoader::MoveToCorrectLoadedList(const rstl::auto_ptr< CPakFile >& pak) {
  if (pak->IsARAMPak()) {
    mAramList.push_back(pak);
  } else {
    mPakLoadedList.push_back(pak);
  }
}

bool CResLoader::CacheFromPak(RSTL_PRIME1_CONST CPakFile& pak, const CAssetId asset) const {
  const ResInfo* resInfo = pak.GetResInfo(asset);

  if (!resInfo) {
    return false;
  }

  mCachedResId = asset;
  mCachedResInfo = resInfo;

  return true;
}

bool CResLoader::CacheFromPakForLoad(CPakFile& pak, const CAssetId asset) {
  const ResInfo* resInfo = nullptr;

  if (mForwardSeek) {
    resInfo = pak.GetResInfoForLoadPreferForward(asset);
    mForwardSeek = false;
  } else {
    resInfo = pak.GetResInfoForLoadDirectionless(asset);
  }

  if (resInfo == nullptr) {
    return false;
  }

  mCachedResId = asset;
  mCachedResInfo = resInfo;

  return true;
}

CPakFile* CResLoader::FindResourceForLoad(const SObjectTag& tag) {
#if RSTL_KYOTO_DEBUG_BUILD
  CPakFile* pak = FindResourceForLoad(tag.GetId());
  if (pak != nullptr && mCachedResInfo->GetType() != tag.GetType()) {
    char gotType[8];
    char wantedType[8];
    strcpy(gotType, SObjectTag::Type2Text(mCachedResInfo->GetType()));
    strcpy(wantedType, SObjectTag::Type2Text(tag.GetType()));
    RS_VERIFY_FAILURE(189, "false", "false",
                      CBasics::Stringize("Error loading resource %s - Wanted type %s, got type %s",
                                         tag.GetId().ToHexString().c_str(), wantedType, gotType));
  }

  return pak;
#else
  return FindResourceForLoad(tag.GetId());
#endif
}

CPakFile* CResLoader::FindResourceForLoad(const CAssetId asset) {
  PakList::iterator it;

  for (it = mAramList.begin(); it != mAramList.end(); ++it) {
    CPakFile* pak = it->get();
    if (CacheFromPak(*pak, asset)) {
      return pak;
    }
  }

  if (mCurPak != mPakLoadedList.end()) {
    CPakFile* pak = mCurPak->get();
    if (CacheFromPakForLoad(*pak, asset)) {
      return pak;
    }
  }

  for (it = mPakLoadedList.begin(); it != mPakLoadedList.end(); ++it) {
    CPakFile* pak = it->get();
    if (mCurPak != it && CacheFromPakForLoad(*pak, asset)) {
      mCurPak = it;
      return pak;
    }
  }

  return nullptr;
}

#if RSTL_KYOTO_RES_LOADER_NETWORK_ASSETS
void CResLoader::FindResource(const SObjectTag& tag) {
  const CBBASupport::SNetworkAsset& networkAsset = CBBASupport::FindNetworkAsset(tag.GetId());
  if (networkAsset.mTag.GetId() == kInvalidAssetId) {
    mForwardSeek = false;
    FindResourceForLoad(tag);
    mForwardSeek = true;
  }
}
#else
CPakFile* CResLoader::FindResource(const SObjectTag& tag) {
  mForwardSeek = false;
  CPakFile* ret = FindResourceForLoad(tag);
  mForwardSeek = true;
  return ret;
}
#endif

#if RSTL_PRIME1
bool CResLoader::ResourceExists(CAssetId asset) {
  if (mCachedResId == asset) {
    return true;
  }

  for (AUTO(it, mAramList.begin()); it != mAramList.end(); ++it) {
    if (CacheFromPak(**it, asset)) {
      return true;
    }
  }

  if (mCurPak != mPakLoadedList.end()) {
    if (CacheFromPak(**mCurPak, asset)) {
      return true;
    }
  }

  for (AUTO(it, mPakLoadedList.begin()); it != mPakLoadedList.end(); ++it) {
    if (mCurPak != it && CacheFromPak(**it, asset)) {
      return true;
    }
  }

  return false;
}
#endif

void CResLoader::ClearCache() {
  mCurPak = mPakLoadedList.end();
  mCachedResId = kInvalidAssetId;
  mCachedResInfo = nullptr;
}

void CResLoader::AsyncIdlePakLoading() {
  bool skipIdle = false;
  for (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      AUTO(it, mPakLoadingList.begin());
#else
      PakList::iterator it = mPakLoadingList.begin();
#endif
      it != mPakLoadingList.end();) {
    CPakFile* pak = it->get();
    const bool aramPak = pak->IsARAMPak();
    if (aramPak || !skipIdle) {
      pak->AsyncIdle();
    }

    if (pak->IsCompletelyLoaded()) {
      MoveToCorrectLoadedList(*it);

      it = mPakLoadingList.erase(it);
    } else {
      if (!aramPak) {
        skipIdle = true;
      }
      ++it;
    }
  }
}

bool CResLoader::AreAllPaksLoaded() const { return mPakLoadingList.empty(); }

const SObjectTag* CResLoader::GetResourceIdByName(const char* name) const {
  for (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      AUTO(it, mAramList.begin());
#else
      PakList::const_iterator it = mAramList.begin();
#endif
      it != mAramList.end(); ++it) {
    const SObjectTag* id = (*it)->GetResIdByName(name);
    if (id != nullptr) {
      return id;
    }
  }

  for (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      AUTO(it, mPakLoadedList.begin());
#else
      PakList::const_iterator it = mPakLoadedList.begin();
#endif
      it != mPakLoadedList.end(); ++it) {
    const SObjectTag* id = (*it)->GetResIdByName(name);
    if (id != nullptr) {
      return id;
    }
  }

  return nullptr;
}

FourCC CResLoader::GetResourceTypeById(const CAssetId asset) const {
#if RSTL_KYOTO_RES_LOADER_NETWORK_ASSETS
  const CBBASupport::SNetworkAsset& networkAsset = CBBASupport::FindNetworkAsset(asset);
  if (networkAsset.mTag.GetId() != kInvalidAssetId) {
    return networkAsset.mTag.GetType();
  }

#endif
  if (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      const_cast< CResLoader& >(*this).ResourceExists(asset)
#else
      const_cast< CResLoader& >(*this).FindResourceForLoad(asset)
#endif
  ) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1 && RSTL_VERSION >= RSTL_GM8E_02
    const_cast< CResLoader& >(*this).FindResourceForLoad(asset);
#endif
    return mCachedResInfo->GetType();
  }

#if RSTL_KYOTO_RES_LOADER_NETWORK_ASSETS
  const SFilenameResult& result = CBBASupport::GetFilenameForAssetId(asset);
  if (result.mType != -1) {
    return result.mType;
  }

  return -1;
#else
  return 0;
#endif
}

bool CResLoader::ResourceExists(const SObjectTag& tag) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_KYOTO_RES_LOADER_NETWORK_ASSETS
  const CBBASupport::SNetworkAsset& networkAsset = CBBASupport::FindNetworkAsset(tag.GetId());
  if (networkAsset.mTag.GetId() != kInvalidAssetId) {
    return true;
  }

  if (const_cast< CResLoader& >(*this).FindResourceForLoad(tag.GetId()) != nullptr) {
    RS_VERIFY_THROW(413, mCachedResInfo->GetType() == tag.GetType(), false,
                    CBasics::Stringize(
                        "Resource types don't match in CResLoader::ResourceExists. Type:%s\tID:%s",
                        SObjectTag::Type2Text(tag.GetType()), tag.GetId().ToHexString().c_str()));
    return true;
  }

  const SFilenameResult& result = CBBASupport::GetFilenameForAssetId(tag.GetId());
  if (result.mType != -1) {
    return true;
  }
  return false;
#elif RSTL_PRIME1
#if NONMATCHING
  return const_cast< CResLoader* >(this)->ResourceExists(tag.GetId());
#else
  return const_cast< CResLoader* >(this)->ResourceExists(tag.GetId()) != nullptr;
#endif
#else
  return const_cast< CResLoader& >(*this).FindResourceForLoad(tag.GetId()) != nullptr;
#endif
}

uint CResLoader::ResourceSize(const SObjectTag& tag) const {
#if RSTL_KYOTO_RES_LOADER_NETWORK_ASSETS
  const CBBASupport::SNetworkAsset& networkAsset = CBBASupport::FindNetworkAsset(tag.GetId());
  if (networkAsset.mTag.GetId() != kInvalidAssetId) {
    int handle;
    if (CBBASupport::BBAOpen(networkAsset.mFilename.data(), kBOM_Read, &handle) != 0) {
      RS_VERIFY_THROW(
          477, 0, false,
          CBasics::Stringize("Could not open file - '%s'\n", networkAsset.mFilename.data()));
    }

    int size = 0;
    CBBASupport::BBAGetFileSize(handle, &size);
    CBBASupport::BBAClose(handle);
    return size;
  }

#endif
  if (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      const_cast< CResLoader& >(*this).ResourceExists(tag.GetId())
#else
      const_cast< CResLoader& >(*this).FindResourceForLoad(tag.GetId())
#endif
  ) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1 && RSTL_VERSION >= RSTL_GM8E_02
    const_cast< CResLoader& >(*this).FindResourceForLoad(tag.GetId());
#endif
    return mCachedResInfo->GetSize();
  }

#if RSTL_KYOTO_RES_LOADER_NETWORK_ASSETS
  const SFilenameResult& result = CBBASupport::GetFilenameForAssetId(tag.GetId());
  if (result.mType != -1) {
    return result.mSize;
  }

#endif
  return 0;
}

#if !RSTL_PRIME1
uint CResLoader::GetResourceOffset(const SObjectTag& tag) const {
  if (const_cast< CResLoader& >(*this).FindResourceForLoad(tag.GetId())) {
    return mCachedResInfo->GetOffset();
  }

  return 0;
}
#endif

CResLoader::ECompressionType CResLoader::GetResourceCompression(const SObjectTag& tag) const {
#if RSTL_KYOTO_RES_LOADER_NETWORK_ASSETS
  const CBBASupport::SNetworkAsset& networkAsset = CBBASupport::FindNetworkAsset(tag.GetId());
  if (networkAsset.mTag.GetId() != kInvalidAssetId) {
    return kCompressionType_Uncompressed;
  }

#endif
  if (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      const_cast< CResLoader& >(*this).ResourceExists(tag.GetId())
#else
      const_cast< CResLoader& >(*this).FindResourceForLoad(tag.GetId())
#endif
  ) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1 && RSTL_VERSION >= RSTL_GM8E_02
    const_cast< CResLoader& >(*this).FindResourceForLoad(tag.GetId());
#endif

    return mCachedResInfo->IsCompressed() ? kCompressionType_Compressed
                                          : kCompressionType_Uncompressed;
  }

  return kCompressionType_Uncompressed;
}

#if RSTL_KYOTO_RES_LOADER_NETWORK_ASSETS
CDvdRequest* CResLoader::ReadNetworkFileToBuffer(const char* filename, char** buffer, int offset,
                                                 int length) {
  RS_DEBUGGER_PRINT("Network build of filename:%s\n", filename);
  int handle;
  if (CBBASupport::BBAOpen(filename, kBOM_Read, &handle) != 0) {
    RS_VERIFY_THROW(563, 0, false, CBasics::Stringize("Could not open file - '%s'\n", filename));
  }

  CBBASupport::BBASeek(handle, offset, kBSO_Start);
  int size = 0;
  if (length == -1) {
    CBBASupport::BBAGetBytesRemaining(handle, &size);
  } else {
    size = length;
  }

  if (*buffer == nullptr) {
    *buffer = static_cast< char* >(rs_aligned_new_line(align_size(size), 583));
  }

  const int error = CBBASupport::BBARead(handle, *buffer, size);
  RS_VERIFY_THROW(587, ((error == CBBASupport::kFE_OK) || (error == CBBASupport::kFE_PartialRead)),
                  false, "Couldn't read in CBBAInStream::Read.");
  CBBASupport::BBAClose(handle);
  return rs_new_line(591) CDummyDvdRequest();
}

#endif
CDvdRequest* CResLoader::LoadResourceAsync(const SObjectTag& tag, char* extBuf) {
#if RSTL_KYOTO_RES_LOADER_NETWORK_ASSETS
  const CBBASupport::SNetworkAsset& networkAsset = CBBASupport::FindNetworkAsset(tag.GetId());
  if (networkAsset.mTag.GetId() != kInvalidAssetId) {
    char* buffer = extBuf;
    return ReadNetworkFileToBuffer(networkAsset.mFilename.data(), &buffer, 0, -1);
  }

#endif
  CPakFile* curPak = FindResourceForLoad(tag);
#if RSTL_KYOTO_RES_LOADER_NETWORK_ASSETS
  if (curPak == nullptr) {
    const SFilenameResult& result = CBBASupport::GetFilenameForAssetId(tag.GetId());
    if (result.mType != -1) {
      char* buffer = extBuf;
      return ReadNetworkFileToBuffer(result.mFilename.data(), &buffer, 0, -1);
    }

    return nullptr;
  }

#endif
  const ResInfo* info = mCachedResInfo;
  return curPak->DvdFile().AsyncSeekRead(extBuf, align_size(info->GetSize()), kSO_Begin,
                                         info->GetOffset());
}

// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
CBufferedDvdRequest* CResLoader::LoadResourceAsync(const SObjectTag& tag) {
#if RSTL_KYOTO_RES_LOADER_NETWORK_ASSETS
  const CBBASupport::SNetworkAsset& networkAsset = CBBASupport::FindNetworkAsset(tag.GetId());
  if (networkAsset.mTag.GetId() != kInvalidAssetId) {
    char* data = nullptr;
    CDvdRequest* request = ReadNetworkFileToBuffer(networkAsset.mFilename.data(), &data, 0, -1);
    return rs_new_line(650) CBufferedDvdRequest(
        nullptr, request, rstl::auto_ptr< uchar >(reinterpret_cast< uchar* >(data)).release());
  }

#endif
  CPakFile* curPak = FindResourceForLoad(tag);
#if RSTL_KYOTO_RES_LOADER_NETWORK_ASSETS
  if (curPak == nullptr) {
    const SFilenameResult& result = CBBASupport::GetFilenameForAssetId(tag.GetId());
    if (result.mType != -1) {
      char* data = nullptr;
      CDvdRequest* request = ReadNetworkFileToBuffer(result.mFilename.data(), &data, 0, -1);
      return rs_new_line(664) CBufferedDvdRequest(
          nullptr, request, rstl::auto_ptr< uchar >(reinterpret_cast< uchar* >(data)).release());
    }

    return nullptr;
  }

#endif
  const ResInfo* info = mCachedResInfo;
  GroupCacheList::iterator it =
      FindGroupCache(&curPak->DvdFile(), info->GetOffset(), info->GetSize());
  if (it == mGroupCaches.end()) {
    const uint groupedSize = info->GetLookaheadAfterResourceSize();
    if (groupedSize == 0) {
      const int size = align_size(info->GetSize());
      rstl::auto_ptr< uchar > buffer(static_cast< uchar* >(rs_aligned_new_line(size, 691)));
      CDvdRequest* request =
          curPak->DvdFile().AsyncSeekRead(buffer.get(), size, kSO_Begin, info->GetOffset());
      return rs_new_line(696) CBufferedDvdRequest(nullptr, request, buffer.release());
    }
    it = AddGroupCache(&curPak->DvdFile(), info->GetOffset(), groupedSize + info->GetSize());
  }
  return it->MakeRequest(info->GetOffset()).release();
}
#endif

CDvdRequest* CResLoader::LoadResourcePartAsync(const SObjectTag& tag, const int offset,
                                               const int length, char* extBuf) {
#if RSTL_KYOTO_RES_LOADER_NETWORK_ASSETS
  const CBBASupport::SNetworkAsset& networkAsset = CBBASupport::FindNetworkAsset(tag.GetId());
  if (networkAsset.mTag.GetId() != kInvalidAssetId) {
    char* buffer = extBuf;
    return ReadNetworkFileToBuffer(networkAsset.mFilename.data(), &buffer, offset, length);
  }

#endif
  CPakFile* curPak = FindResourceForLoad(tag);
#if RSTL_KYOTO_RES_LOADER_NETWORK_ASSETS
  if (curPak == nullptr) {
    const SFilenameResult& result = CBBASupport::GetFilenameForAssetId(tag.GetId());
    if (result.mType != -1) {
      char* buffer = extBuf;
      return ReadNetworkFileToBuffer(result.mFilename.data(), &buffer, offset, length);
    }

    return nullptr;
  }

#endif
  const ResInfo* info = mCachedResInfo;

  return curPak->DvdFile().AsyncSeekRead(extBuf, length, kSO_Begin, info->GetOffset() + offset);
}

CInputStream* CResLoader::LoadNewResourceSync(const SObjectTag& tag, char* extBuf) {
#if RSTL_KYOTO_RES_LOADER_NETWORK_ASSETS
  const CBBASupport::SNetworkAsset& networkAsset = CBBASupport::FindNetworkAsset(tag.GetId());
  if (networkAsset.mTag.GetId() != kInvalidAssetId) {
    CBBAInStream file(networkAsset.mFilename.data());
    const uint size = file.GetRemainingBytes();
    void* dest = extBuf == nullptr ? rs_aligned_new_line(align_size(size), 769) : extBuf;
    file.Get(dest, size);
    return rs_new_line(772) CMemoryInStream(
        dest, size, extBuf == nullptr ? CMemoryInStream::kOS_Owned : CMemoryInStream::kOS_NotOwned);
  }

#endif
  CPakFile* curPak = FindResourceForLoad(tag);
#if RSTL_KYOTO_RES_LOADER_NETWORK_ASSETS
  if (curPak == nullptr) {
    const SFilenameResult& result = CBBASupport::GetFilenameForAssetId(tag.GetId());
    if (result.mType != -1) {
      CBBAInStream file(result.mFilename.data());
      const uint size = file.GetRemainingBytes();
      void* dest = extBuf == nullptr ? rs_aligned_new_line(align_size(size), 789) : extBuf;
      file.Get(dest, size);
      return rs_new_line(792) CMemoryInStream(dest, size,
                                              extBuf == nullptr ? CMemoryInStream::kOS_Owned
                                                                : CMemoryInStream::kOS_NotOwned);
    }

    return nullptr;
  }

#endif
  const ResInfo* info = mCachedResInfo;

  uint len = align_size(info->GetSize());
  void* dest = extBuf ? extBuf : rs_aligned_new_line(len, 805);

  curPak->DvdFile().SyncSeekRead(dest, len, kSO_Begin, info->GetOffset());
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CInputStream* input = rs_new CMemoryInStream(dest, info->GetSize(),
                                               extBuf == nullptr ? CMemoryInStream::kOS_Owned
                                                                 : CMemoryInStream::kOS_NotOwned);
#else
  rstl::auto_ptr< CInputStream > input(rs_new_line(815) CMemoryInStream(
      dest, info->GetSize(),
      extBuf == nullptr ? CMemoryInStream::kOS_Owned : CMemoryInStream::kOS_NotOwned));
#endif

  if (info->IsCompressed()) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    input->Get< uint >();
    return rs_new CZipInputStream(input);
#else
    const int length = input->ReadInt32();
    return rs_new_line(824)
        CLZOInputStream(input, info->GetSize() - input->GetReadPosition(), length);
#endif
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return input;
#else
  return input.release();
#endif
}

#if RSTL_KYOTO_RES_LOADER_NETWORK_ASSETS
bool CResLoader::LoadMemResourceSync(const SObjectTag& tag, char** bufOut, int* lenOut) {
  const CBBASupport::SNetworkAsset& networkAsset = CBBASupport::FindNetworkAsset(tag.GetId());
  if (networkAsset.mTag.GetId() != kInvalidAssetId) {
    CBBAInStream file(networkAsset.mFilename.data());
    *lenOut = file.GetRemainingBytes();
    *bufOut = static_cast< char* >(rs_aligned_new_line(align_size(*lenOut), 841));
    file.Get(*bufOut, *lenOut);
    return true;
  }

  CPakFile* curPak = FindResourceForLoad(tag);
  if (curPak == nullptr) {
    const SFilenameResult& result = CBBASupport::GetFilenameForAssetId(tag.GetId());
    if (result.mType != -1) {
      CBBAInStream file(result.mFilename.data());
      *lenOut = file.GetRemainingBytes();
      *bufOut = static_cast< char* >(rs_aligned_new_line(align_size(*lenOut), 858));
      file.Get(*bufOut, *lenOut);
      return true;
    }

    return false;
  }

  const ResInfo* info = mCachedResInfo;
  uint len = align_size(info->GetSize());
  char* buf = static_cast< char* >(rs_aligned_new_line(len, 871));
  curPak->DvdFile().SyncSeekRead(buf, len, kSO_Begin, info->GetOffset());
  *bufOut = buf;
  *lenOut = info->GetSize();
  return true;
}

#endif
CInputStream* CResLoader::LoadResourceFromMemorySync(const SObjectTag& tag, const void* extBuf) {
#if RSTL_KYOTO_RES_LOADER_NETWORK_ASSETS
  if (FindResourceForLoad(tag) == nullptr) {
    return nullptr;
  }

#else
  FindResourceForLoad(tag);
#endif
  const ResInfo* info = mCachedResInfo;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CInputStream* input = rs_new CMemoryInStream(extBuf, info->GetSize());
#else
  rstl::auto_ptr< CInputStream > input(rs_new_line(956) CMemoryInStream(extBuf, info->GetSize()));
#endif

  if (info->IsCompressed()) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    input->Get< uint >();
    return rs_new CZipInputStream(input);
#else
    const int length = input->ReadInt32();
    return rs_new_line(961)
        CLZOInputStream(input, info->GetSize() - input->GetReadPosition(), length);
#endif
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return input;
#else
  return input.release();
#endif
}

#if !RSTL_KYOTO_RES_LOADER_NETWORK_ASSETS
void CResLoader::LoadMemResourceSync(const SObjectTag& tag, char** bufOut, int* lenOut) {
  CPakFile* curPak = FindResourceForLoad(tag);
  const ResInfo* info = mCachedResInfo;

  uint len = align_size(info->GetSize());
  char* buf = static_cast< char* >(CMemory::Alloc(len, IAllocator::kHI_RoundUpLen));
  curPak->DvdFile().SyncSeekRead(buf, len, kSO_Begin, info->GetOffset());
  *bufOut = buf;
  *lenOut = info->GetSize();
}
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CInputStream* CResLoader::LoadNewResourcePartSync(const SObjectTag& tag, int offset, int length,
                                                  char* extBuf) {
  CPakFile* curPak = FindResourceForLoad(tag);
  const ResInfo* info = mCachedResInfo;

  void* dest = extBuf ? extBuf : CMemory::Alloc(length, IAllocator::kHI_RoundUpLen);
  curPak->DvdFile().SyncSeekRead(dest, length, kSO_Begin, info->GetOffset() + offset);

  CInputStream* input = rs_new CMemoryInStream(
      dest, length, extBuf == nullptr ? CMemoryInStream::kOS_Owned : CMemoryInStream::kOS_NotOwned);
  return input;
}
#endif

void CResLoader::AddPakFileAsync(const rstl::string& filePath, RSTL_PRIME1_CONST bool a,
                                 RSTL_PRIME1_CONST bool b) {
  const rstl::string pathWithExt(filePath + ".pak");

  if (CDvdFile::FileExists(pathWithExt.data())) {
    mPakLoadingList.push_back(rs_new_line(990) CPakFile(pathWithExt, a, b));
  } else {
    RS_VERIFY_FAILURE(994, "false", "false",
                      CBasics::Stringize("Couldn't find/add PAK file %s", pathWithExt.data()));
  }
}

void CResLoader::RemovePakFile(const rstl::string& filePath) {
  rstl::string pathWithExt(filePath + ".pak");
  ClearCache();
  PakList* lists[] = {&mAramList, &mPakLoadedList};

  for (int i = 0; i < ARRAY_SIZE(lists); ++i) {
    PakList& list = *lists[i];
    for (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        AUTO(it, list.begin());
#else
        PakList::iterator it = list.begin();
#endif
        it != list.end(); ++it) {
      const CPakFile* pak = it->get();
      if (CStringExtras::CompareCaseInsensitive(pak->GetDvdFile().GetFilename(), pathWithExt) ==
          0) {
        list.erase(it);
        return;
      }
    }
  }

  for (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
      AUTO(it, mPakLoadingList.begin());
#else
      PakList::iterator it = mPakLoadingList.begin();
#endif
      it != mPakLoadingList.end(); ++it) {
    const CPakFile* pak = it->get();
    if (CStringExtras::CompareCaseInsensitive(pak->GetDvdFile().GetFilename(), pathWithExt) == 0) {
      while (!pak->IsCompletelyLoaded()) {
        AsyncIdlePakLoading();
      }
      mPakLoadingList.erase(it);
      return;
    }
  }
}

#if RSTL_PRIME1
const rstl::vector< CAssetId >* CResLoader::GetTagListForFile(const rstl::string& filePath) const {
  rstl::string pathWithExt(filePath + ".pak");

  const rstl::list< rstl::auto_ptr< CPakFile > >* lists[] = {&mAramList, &mPakLoadedList};

  for (int i = 0; i < ARRAY_SIZE(lists); ++i) {
    const rstl::list< rstl::auto_ptr< CPakFile > >& list = *lists[i];
    for (AUTO(it, list.begin()); it != list.end(); ++it) {
      const CPakFile* pak = it->get();
      if (CStringExtras::CompareCaseInsensitive(pak->GetDvdFile().GetFilename(), pathWithExt) == 0) {
        return pak->GetDepList();
      }
    }
  }

  return nullptr;
}
#endif

rstl::vector< rstl::pair< rstl::string, SObjectTag > > CResLoader::GetResourceIdToNameList() const {
  const PakList* lists[] = {&mAramList, &mPakLoadedList};

  int nameCount = 0;
  for (int i = 0; i < ARRAY_SIZE(lists); ++i) {
    const PakList& list = *lists[i];
    for (
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        AUTO(it, list.begin());
#else
        PakList::const_iterator it = list.begin();
#endif
        it != list.end(); ++it) {
      const CPakFile* pak = it->get();
      if (!pak->IsStashedInARAM()) {
        nameCount += pak->GetStringToObjectList().size();
      }
    }
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::vector< rstl::pair< rstl::string, SObjectTag > > ret(nameCount);
#else
  rstl::vector< rstl::pair< rstl::string, SObjectTag > > ret;
  ret.reserve(nameCount);
#endif

  for (int i = 0; i < ARRAY_SIZE(lists); ++i) {
    const PakList& list = *lists[i];
    for (PakList::const_iterator it = list.begin(); it != list.end(); ++it) {
      const CPakFile* pak = it->get();
      if (!pak->IsStashedInARAM()) {
        const rstl::vector< rstl::pair< rstl::string, SObjectTag > >& tagList =
            pak->GetStringToObjectList();
        ret.insert(ret.end(), tagList.begin(), tagList.end());
      }
    }
  }

  return ret;
}

#if RSTL_KYOTO_DEBUG_BUILD
const rstl::vector< rstl::pair< rstl::string, SObjectTag > >*
CResLoader::GetNameListForFile(const rstl::string& filePath) const {
  rstl::string pathWithExt(filePath + ".pak");

  const PakList* lists[] = {&mAramList, &mPakLoadedList};

  for (int i = 0; i < ARRAY_SIZE(lists); ++i) {
    const PakList& list = *lists[i];
    for (PakList::const_iterator it = list.begin(); it != list.end(); ++it) {
      const CPakFile* pak = it->get();
      if (CStringExtras::CompareCaseInsensitive(pak->GetDvdFile().GetFilename(), pathWithExt) ==
              0 &&
          !pak->IsStashedInARAM()) {
        return &pak->GetStringToObjectList();
      }
    }
  }

  return nullptr;
}

bool CResLoader::ContainsResourceInAramPaks(const SObjectTag& tag) const {
  for (PakList::const_iterator it = mAramList.begin(); it != mAramList.end(); ++it) {
    if ((*it)->GetResInfo(tag.GetId()) != nullptr) {
      return true;
    }
  }

  return false;
}

#endif
int CResLoader::GetPakCount() const { return mAramList.size() + mPakLoadedList.size(); }
RSTL_ECHOES_CONST CPakFile* CResLoader::GetPakFile(const int idx) const {
  int numAramPaks = mAramList.size();
  if (idx < numAramPaks) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    AUTO(it, mAramList.begin());
#else
    PakList::const_iterator it = mAramList.begin();
#endif
    for (int i = 0; i < idx; ++it, ++i) {
    }
    return it->get();
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  AUTO(it, mPakLoadedList.begin());
#else
  PakList::const_iterator it = mPakLoadedList.begin();
#endif
  for (int i = 0; i < idx - numAramPaks; ++it, ++i) {
  }
  return it->get();
}

#if !RSTL_PRIME1
CPakFile* CResLoader::GetPakFile(const int idx) {
  int numAramPaks = mAramList.size();
  if (idx < numAramPaks) {
    PakList::const_iterator it = mAramList.begin();
    for (int i = 0; i < idx; ++it, ++i) {
    }
    return it->get();
  }
  PakList::const_iterator it = mPakLoadedList.begin();
  for (int i = 0; i < idx - numAramPaks; ++it, ++i) {
  }
  return it->get();
}

void CResLoader::KillLookahead(const CLookaheadRes* cache) {
  for (GroupCacheList::iterator it = mGroupCaches.begin(); it != mGroupCaches.end(); ++it) {
    if (&*it == cache) {
      mGroupCaches.erase(it);
      return;
    }
  }

  RS_VERIFY_FAILURE(1215, "false", "false",
                    "CResLoader::KillLookahead couldn't find a lookahead entry to kill.");
}

CResLoader::GroupCacheList::iterator CResLoader::FindGroupCache(const CDvdFile* file, uint offset,
                                                                uint size) {
  GroupCacheList::iterator it = mGroupCaches.end();
  while (it != mGroupCaches.begin()) {
    --it;
    const CLookaheadRes& cache = *it;
    if (!cache.IsInvalid() && cache.Contains(file, offset, size)) {
      return it;
    }
  }
  return mGroupCaches.end();
}

CResLoader::GroupCacheList::iterator CResLoader::AddGroupCache(CDvdFile* file, uint offset,
                                                               uint size) {
  rstl::auto_ptr< uchar > buffer(static_cast< uchar* >(rs_aligned_new_line(size, 1252)));
  rstl::auto_ptr< CDvdRequest > request(file->AsyncSeekRead(buffer.get(), size, kSO_Begin, offset));
  return mGroupCaches.insert(mGroupCaches.end(),
                             CLookaheadRes(buffer.release(), file, offset, size, request, this));
}
#endif
