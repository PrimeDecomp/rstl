#include "Kyoto/CPakFile.hpp"
#include "rstl/macros.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CARAMManager.hpp"
#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Platform.h"
#include "Kyoto/Streams/CMemoryInStream.hpp"

#include "rstl/StringExtras.hpp"
#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"
#include "rstl/reserved_vector.hpp"

#include "stdio.h"

#if RSTL_PRIME1
#define IF_PRIME1(A, B) A
#else
#define IF_PRIME1(A, B) B
#endif

static inline int round_up_32(int val) { return (val + 31) & ~31; }

CPakFile::CResInfo::CResInfo(uint id, uint fourCC, uint offset, uint size,
                             uint flags IF_NOT_RSTL_PRIME1(, uint groupedSize))
: mId(id) {
  const uint typeIdx = CFactoryMgr::FourCCToTypeIdx(fourCC);
  mData[0] = static_cast< uchar >(typeIdx | (flags != 0 ? 0x80 : 0));
  mData[1] = static_cast< uchar >(offset >> 5);
  mData[2] = static_cast< uchar >(offset >> 13);
  mData[3] = static_cast< uchar >((offset >> 21) | ((size << 2) & 0x80));
  mData[4] = static_cast< uchar >(size >> 6);
  mData[5] = static_cast< uchar >(size >> 14);
#if !RSTL_PRIME1
  mData[6] = static_cast< uchar >(groupedSize >> 5);
#endif
}

uint CPakFile::CResInfo::GetType() const { return CFactoryMgr::TypeIdxToFourCC(mData[0] & 0x7f); }

uint CPakFile::CResInfo::GetOffset() const {
  return ((mData[1] | (mData[2] << 8) | (mData[3] << 16)) & 0x7fffff) << 5;
}

uint CPakFile::CResInfo::GetSize() const {
  return ((mData[3] >> 7) | (mData[4] << 1) | (mData[5] << 9)) << 5;
}

bool CPakFile::CResInfo::IsCompressed() const { return (mData[0] & ~0x7f) != 0; }

#if !RSTL_PRIME1
uint CPakFile::CResInfo::GetLookaheadAfterResourceSize() const { return mData[6] << 5; }

void CPakFile::CResInfo::SetLookaheadAfterResourceSize(uint size) {
  mData[6] = static_cast< uchar >(size >> 5);
}
#endif

CPakFile::CPakFile(const rstl::string& filename, bool buildDepList, bool worldPak)
: mFile(filename.data())
, mBuildDepList(buildDepList)
, mAramFile(mFile.IsARAMFile())
, mWorldPak(worldPak)
, mStashedInARAM(false)
, mAsyncLoadPhase(kAP_Warmup)
, mResTableOffset(0)
, mResTableCount(0)
#if RSTL_PRIME1
#if !defined(TARGET_PC)
, mpARAMHeader(CARAMManager::GetInvalidAlloc())
#endif
#else
, mFakeStaticSize(0)
, mpARAMHeader(CARAMManager::GetInvalidAlloc())
#endif
, mCurrentSeek(-1) {
}

CPakFile::~CPakFile() {
  while (mAsyncLoadPhase != kAP_Loaded) {
    AsyncIdle();
  }
  CMemory::OffsetFakeStatics(-IF_PRIME1(GetFakeStaticSize(), mFakeStaticSize));
#if !defined(TARGET_PC)
  CARAMManager::Free(mpARAMHeader);
#endif
}

void CPakFile::AsyncIdle() {
#if RSTL_PRIME1
  if (mAsyncLoadPhase == kAP_Loaded) {
    return;
  }
  if (!mFile.IsARAMFileLoaded()) {
    return;
  }
  if (!mDvdReq.null() && !mDvdReq->IsComplete()) {
    return;
  }

  switch (mAsyncLoadPhase) {
  case kAP_Warmup:
    Warmup();
    break;
  case kAP_InitialHeaderLoad:
    InitialHeaderLoad();
    break;
  case kAP_DataLoad:
    DataLoad();
    break;
  default:
    break;
  }
#else
  if (mAsyncLoadPhase != kAP_Loaded && mFile.IsARAMFileLoaded() &&
      (mDvdReq.null() || mDvdReq->IsComplete())) {
    switch (mAsyncLoadPhase) {
    case kAP_Warmup:
      Warmup();
      break;
    case kAP_InitialHeaderLoad:
      InitialHeaderLoad();
      break;
    case kAP_DataLoad:
      DataLoad();
      break;
    default:
      break;
    }
  }
#endif
}

void CPakFile::Warmup() {
  const int length = rstl::min_val< int >(mFile.Length(), 8192);
  mHeaderData.resize(length);
  mDvdReq = rstl::auto_ptr< CDvdRequest >(mFile.SyncRead(&mHeaderData[0], length));
  mAsyncLoadPhase = kAP_InitialHeaderLoad;
}

void CPakFile::InitialHeaderLoad() {
  CMemoryInStream in(mHeaderData.data(), mHeaderData.size());
  mDvdReq = rstl::auto_ptr< CDvdRequest >();

  const int version = in.ReadInt32();
  if (version != 0x30005) {
    char buf[248];
#if NONMATCHING
    snprintf(buf, sizeof(buf),
             "%s: Incompatible pak file version -- Current version is %x, you're using %x",
             mFile.GetFilename().data(), 0x30005, version);
#else
    sprintf(buf, "%s: Incompatible pak file version -- Current version is %x, you're using %x",
            mFile.GetFilename().data(), 0x30005, version);
#endif
    return;
  }

  in.ReadInt32();
  const int nameCount = in.ReadInt32();
  mNameList.reserve(nameCount);

  for (int i = 0; i < nameCount; ++i) {
    const FourCC type = in.Get< FourCC >();
    const CAssetId id = in.Get< CAssetId >();
    const rstl::string name = CStringExtras::ReadString(in);
    IF_PRIME1(mNameList.push_back, mNameList.push_back_unsafe)(
        rstl::pair< rstl::string, SObjectTag >(name, SObjectTag(type, id)));
  }

  mResTableCount = in.ReadInt32();
  mResTableOffset = in.GetReadPosition();
  mAsyncLoadPhase = kAP_DataLoad;

  const int origSize = mHeaderData.size();
  const uint resDataSize = mResTableCount * 20;
  const int newSize = (resDataSize + mResTableOffset + 31) & ~31;
  if (newSize > origSize) {
    mHeaderData.resize(newSize);
    mDvdReq = rstl::auto_ptr< CDvdRequest >(mFile.AsyncSeekRead(
        mHeaderData.data() + origSize, mHeaderData.size() - origSize, kSO_Begin, origSize));
  } else {
    DataLoad();
  }
}

void CPakFile::DataLoad() {
  mDvdReq = rstl::auto_ptr< CDvdRequest >();
  CMemoryInStream in(&mHeaderData[mResTableOffset], mHeaderData.size() - mResTableOffset);

  LoadResourceTable(in);
  mAsyncLoadPhase = kAP_Loaded;
#if !defined(TARGET_PC)
  if (mWorldPak) {
    uint size = round_up_32(mResTableCount * sizeof(CResInfo));
    mpARAMHeader = CARAMManager::Alloc(size);
    uint handle = CARAMManager::DMAToARAM(mResInfoBuckets.data(), const_cast< void* >(mpARAMHeader), size,
                                          CARAMManager::kDMAPrio_One);
    CARAMManager::WaitForDMACompletion(handle);
  }
#endif
  mHeaderData = rstl::vector< uchar, rstl::aligned_allocator >();
#if RSTL_PRIME1
  CMemory::OffsetFakeStatics(GetFakeStaticSize());
#else
  UpdateFakeStaticSize();
#endif
}

void CPakFile::LoadResourceTable(CMemoryInStream& in) {
#if RSTL_PRIME1
  int reserveBytes = round_up_32(mResTableCount * static_cast< int >(sizeof(CResInfo))) +
                     static_cast< int >(sizeof(CResInfo)) - 1;
  mResInfoBuckets.reserve(static_cast< uint >(rstl::max_val(reserveBytes, 64)) / sizeof(CResInfo));
#else
  ResListType sortedResources;
  sortedResources.reserve(mResTableCount);
#endif
  if (mBuildDepList) {
    mDepList.reserve(mResTableCount);
  }
  for (int i = 0; i < mResTableCount; ++i) {
    uint flags = in.Get< uint >();
    uint fourCC = in.Get< uint >();
    uint id = in.Get< uint >();
    uint size = in.Get< uint >();
    uint offset = in.Get< uint >();

    IF_PRIME1(mResInfoBuckets.push_back,
              sortedResources.push_back_unsafe)(CResInfo(id, fourCC, offset, size, flags));
    if (mBuildDepList) {
      IF_PRIME1(mDepList.push_back, mDepList.push_back_unsafe)(id);
    }
  }

#if !RSTL_PRIME1
  for (int i = 0; i < mResTableCount; ++i) {
    CResInfo& info = sortedResources[i];
    if (info.GetSize() <= 8192) {
      uint groupedSize = 0;
      for (int j = i + 1; j < mResTableCount; ++j) {
        const uint nextSize = sortedResources[j].GetSize();
        if (groupedSize + nextSize >= 8192) {
          break;
        }
        groupedSize += nextSize;
      }
      info.SetLookaheadAfterResourceSize(groupedSize);
    }
  }
#endif

  static rstl::less< CResInfo > compare;
#if RSTL_PRIME1
  rstl::sort(mResInfoBuckets.begin(), mResInfoBuckets.end(), compare);
#else
  rstl::sort(sortedResources.begin(), sortedResources.end(), compare);
  RebuildResourceLists(sortedResources);
#endif
}

int CPakFile::GetFakeStaticSize() const {
  return (mDepList.size() * sizeof(CAssetId) + mResInfoBuckets.size() * sizeof(CResInfo)
#if !RSTL_PRIME1
          + mResTableCount * sizeof(CResInfo) + mBucketOffsets.size() * sizeof(uint)
#endif
  );
}

const CPakFile::CResInfo* CPakFile::GetResInfo(uint id) RSTL_PRIME1_CONST {
  if (!IsCompletelyLoaded())
    return nullptr;
  if (mStashedInARAM)
    return nullptr;

#if RSTL_PRIME1
  CResInfo key(id, 'TXTR', 0, 0, 0);
#else
  const uint bucket = id & 0xff;
  ResListType::const_iterator first = mResInfoBuckets.begin() + mBucketOffsets[bucket];
  ResListType::const_iterator last = mResInfoBuckets.begin() + mBucketOffsets[bucket + 1];
#endif

  static rstl::less< CResInfo > compare;
  ResListType::const_iterator it = rstl::lower_bound(
#if RSTL_PRIME1
      mResInfoBuckets.begin(), mResInfoBuckets.end(), key,
#else
      first, last, CResInfo::SearchKey(id),
#endif
      compare);
  if (it == IF_PRIME1(mResInfoBuckets.end(), last) || it->GetId() != id) {
    return nullptr;
  }

  return &*it;
}

const CPakFile::CResInfo* CPakFile::GetResInfoForLoadDirectionless(uint id) {
  if (mStashedInARAM)
    return nullptr;
#if RSTL_PRIME1
  CResInfo key(id, 'TXTR', 0, 0, 0);
#else
  const uint bucket = id & 0xff;
  ResListType::const_iterator first = mResInfoBuckets.begin() + mBucketOffsets[bucket];
  ResListType::const_iterator last = mResInfoBuckets.begin() + mBucketOffsets[bucket + 1];
#endif

  static rstl::less< CResInfo > compare;
  ResListType::const_iterator it = rstl::lower_bound(
#if RSTL_PRIME1
      mResInfoBuckets.begin(), mResInfoBuckets.end(), key,
#else
      first, last, CResInfo::SearchKey(id),
#endif
      compare);
  if (it == IF_PRIME1(mResInfoBuckets.end(), last) || it->GetId() != id) {
    return nullptr;
  }
  const CResInfo* best = &*it;
  int bestDelta = CMath::AbsI(static_cast< int >(it->GetOffset() - mCurrentSeek));
  ++it;
  while (it != IF_PRIME1(mResInfoBuckets.end(), last)) {
    if (it->GetId() != id)
      break;
    const int delta = CMath::AbsI(static_cast< int >(it->GetOffset() - mCurrentSeek));
    if (delta < bestDelta) {
      best = &*it;
      bestDelta = delta;
    }
    ++it;
  }
  mCurrentSeek = best->GetOffset() + best->GetSize();
  return best;
}

const CPakFile::CResInfo* CPakFile::GetResInfoForLoadPreferForward(uint id) {
  if (mStashedInARAM)
    return nullptr;
#if RSTL_PRIME1
  CResInfo key(id, 'TXTR', 0, 0, 0);
#else
  const uint bucket = id & 0xff;
  ResListType::const_iterator first = mResInfoBuckets.begin() + mBucketOffsets[bucket];
  ResListType::const_iterator last = mResInfoBuckets.begin() + mBucketOffsets[bucket + 1];
#endif

  static rstl::less< CResInfo > compare;
  ResListType::const_iterator it = rstl::lower_bound(
#if RSTL_PRIME1
      mResInfoBuckets.begin(), mResInfoBuckets.end(), key,
#else
      first, last, CResInfo::SearchKey(id),
#endif
      compare);
  if (it == IF_PRIME1(mResInfoBuckets.end(), last) || it->GetId() != id) {
    return nullptr;
  }
  const CResInfo* best = &*it;
  int bestDelta = mCurrentSeek - static_cast< int >(IF_PRIME1(best, it)->GetOffset());
  ++it;
  while (it != IF_PRIME1(mResInfoBuckets.end(), last)) {
    if (it->GetId() != id)
      break;
    const int delta = mCurrentSeek - static_cast< int >(it->GetOffset());
    if ((bestDelta < 0 && (delta > 0 || delta > bestDelta)) ||
        (bestDelta >= 0 && delta > 0 && delta < bestDelta)) {
      best = &*it;
      bestDelta = delta;
    }
    ++it;
  }
  mCurrentSeek = best->GetOffset() + best->GetSize();
  return best;
}

const SObjectTag* CPakFile::GetResIdByName(const char* name) const {
  if (!mStashedInARAM) {
    for (AUTO(it, mNameList.begin()); it != mNameList.end(); ++it) {
      const int cmp = CStringExtras::CompareCaseInsensitive(it->first, rstl::string_l(name));
      if (cmp == 0) {
        return &it->second;
      }
    }
  }
  return nullptr;
}

const rstl::vector< CAssetId >* CPakFile::GetDepList() const {
  if (mDepList.size() != 0)
    return &mDepList;
  return nullptr;
}

void CPakFile::StashWorldPak() {
  if (IS_TARGET_PC) {
    return;
  }

  if (mWorldPak) {
    mStashedInARAM = true;
    mDepList = rstl::vector< CAssetId >();
    mResInfoBuckets = ResListType();
#if !RSTL_PRIME1
    mBucketOffsets = rstl::vector< int >();
    UpdateFakeStaticSize();
#endif
  }
}

#if !RSTL_PRIME1
void CPakFile::RebuildResourceLists(const ResListType& sortedResources) {
  rstl::reserved_vector< uint, 256 > bucketCounts(256, 0);

  const CResInfo emptyInfo = CResInfo(0, 'TXTR', 0, 0, 0, 0);
  mResInfoBuckets.clear();
  mResInfoBuckets.resize(mResTableCount, emptyInfo);
  mBucketOffsets.clear();
  mBucketOffsets.reserve(257);
  for (ResListType::const_iterator it = sortedResources.begin(); it != sortedResources.end(); ++it)
    ++bucketCounts[it->GetId() & 0xff];
  mBucketOffsets.push_back_unsafe(0);
  uint offset = 0;
  for (int i = 0; i < 256; ++i) {
    offset += bucketCounts[i];
    mBucketOffsets.push_back_unsafe(offset);
    bucketCounts[i] = 0;
  }

  for (ResListType::const_iterator it = sortedResources.begin(); it != sortedResources.end();
       ++it) {
    const uint bucket = it->GetId() & 0xff;
    mResInfoBuckets[mBucketOffsets[bucket] + bucketCounts[bucket]] = *it;
    ++bucketCounts[bucket];
  }
}
#endif

void CPakFile::EnsureWorldPakReady() {
  if (IS_TARGET_PC) {
    return;
  }

  if (mWorldPak && mStashedInARAM) {
#if RSTL_PRIME1
    int reserveBytes = round_up_32(mResTableCount * static_cast< int >(sizeof(CResInfo))) +
                       static_cast< int >(sizeof(CResInfo)) - 1;
    mResInfoBuckets.reserve(static_cast< uint >(rstl::max_val(reserveBytes, 64)) / sizeof(CResInfo));

    for (int i = 0; i < static_cast< int >(mResTableCount); ++i) {
      mResInfoBuckets.push_back(CResInfo(i, 'TXTR', 0, 0, 0));
    }

    uint size = round_up_32(mResTableCount * sizeof(CResInfo));
#else
    ResListType resources;
    resources.resize_uninitialized(mResTableCount);
    const uint size = (mResTableCount * sizeof(CResInfo) + 31) & ~31;
#endif

#if !defined(TARGET_PC)
    CARAMManager::WaitForDMACompletion(CARAMManager::DMAToMRAM(
        const_cast< void* >(mpARAMHeader), IF_PRIME1(mResInfoBuckets.data(), resources.data()), size,
        CARAMManager::kDMAPrio_One));
#endif
#if !RSTL_PRIME1
    RebuildResourceLists(resources);
#endif

    if (mBuildDepList) {
      mDepList.reserve(mResTableCount);
      for (int i = 0; i < mResTableCount; ++i) {
#if RSTL_PRIME1
        mDepList.push_back(mResInfoBuckets[i].GetId());
#else
        mDepList.push_back_unsafe(resources[i].GetId());
#endif
      }
    }
    mStashedInARAM = false;
#if !RSTL_PRIME1
    UpdateFakeStaticSize();
#endif
  }
}

#if !RSTL_PRIME1
void CPakFile::UpdateFakeStaticSize() {
  const int newSize = GetFakeStaticSize();
  CMemory::OffsetFakeStatics(newSize - mFakeStaticSize);
  mFakeStaticSize = newSize;
}
#endif
