#include "rstl/RstlVersions.h"
#include "Kyoto/CPakFile.hpp"

#include "Kyoto/Alloc/CMemory.hpp"

#include "Kyoto/CARAMManager.hpp"
#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/Math/CMath.hpp"

#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"
#include "rstl/reserved_vector.hpp"

#include "stdio.h"

static inline int round_up_32(int val) { return (val + 31) & ~31; }

#if RSTL_PRIME1
CPakFile::SResInfo::SResInfo(uint id, uint fourCC, uint offset, uint size, uint flags)
#else
CPakFile::SResInfo::SResInfo(uint id, uint fourCC, uint offset, uint size, uint flags,
                             uint groupedSize)
#endif
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

uint CPakFile::SResInfo::GetType() const { return CFactoryMgr::TypeIdxToFourCC(mData[0] & 0x7f); }

uint CPakFile::SResInfo::GetOffset() const {
  return ((mData[1] | (mData[2] << 8) | (mData[3] << 16)) & 0x7fffff) << 5;
}

uint CPakFile::SResInfo::GetSize() const {
  return ((mData[3] >> 7) | (mData[4] << 1) | (mData[5] << 9)) << 5;
}

bool CPakFile::SResInfo::IsCompressed() const { return (mData[0] & ~0x7f) != 0; }

#if !RSTL_PRIME1
uint CPakFile::SResInfo::GetLookaheadAfterResourceSize() const { return mData[6] << 5; }

void CPakFile::SResInfo::SetLookaheadAfterResourceSize(uint size) {
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
, mAramBase(CARAMManager::GetInvalidAlloc())
#else
, mFakeStaticSize(0)
, mpARAMHeader(CARAMManager::GetInvalidAlloc())
#endif
, mCurrentSeek(-1) {}

CPakFile::~CPakFile() {
  while (mAsyncLoadPhase != kAP_Loaded) {
    AsyncIdle();
  }
#if RSTL_PRIME1
  CMemory::OffsetFakeStatics(-static_cast< int >(GetFakeStaticSize()));
  CARAMManager::Free(mAramBase);
#else
  CMemory::OffsetFakeStatics(-mFakeStaticSize);
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
#if RSTL_PRIME1
  mDvdReq = rstl::auto_ptr< CDvdRequest >(mFile.SyncRead(&mHeaderData[0], length));
#else
  mDvdReq = rstl::auto_ptr< CDvdRequest >(mFile.SyncRead(mHeaderData.data(), length));
#endif
  mAsyncLoadPhase = kAP_InitialHeaderLoad;
}

void CPakFile::InitialHeaderLoad() {
#if RSTL_PRIME1
  CMemoryInStream in(&mHeaderData[0], mHeaderData.size());
#else
  CMemoryInStream in(mHeaderData.data(), mHeaderData.size());
#endif
  mDvdReq = rstl::auto_ptr< CDvdRequest >();

  const int version = in.ReadInt32();
  if (version != 0x30005) {
    char buf[248];
#if RSTL_PRIME1 && NONMATCHING
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
#if RSTL_PRIME1
    int type = in.ReadInt32();
    int id = in.ReadInt32();
    rstl::string name = CStringExtras::ReadString(in);
    mNameList.push_back(rstl::pair< rstl::string, SObjectTag >(name, SObjectTag(type, id)));
#else
    const FourCC type = in.ReadUint32();
    const CAssetId id = in.ReadUint32();
    const rstl::string name = CStringExtras::ReadString(in);
    mNameList.push_back_unsafe(rstl::pair< rstl::string, SObjectTag >(name, SObjectTag(type, id)));
#endif
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
#if RSTL_PRIME1
        &mHeaderData[0] + origSize,
#else
        mHeaderData.data() + origSize,
#endif
        mHeaderData.size() - origSize, kSO_Begin, origSize));
  } else {
    DataLoad();
  }
}

void CPakFile::DataLoad() {
  mDvdReq = rstl::auto_ptr< CDvdRequest >();
  CMemoryInStream in(&mHeaderData[mResTableOffset], mHeaderData.size() - mResTableOffset);

  LoadResourceTable(in);
  mAsyncLoadPhase = kAP_Loaded;
  if (mWorldPak) {
#if RSTL_PRIME1
    uint size = round_up_32(mResTableCount * sizeof(SResInfo));
    mAramBase = CARAMManager::Alloc(size);
    uint handle =
        CARAMManager::DMAToARAM(mResList.data(), mAramBase, size, CARAMManager::kDMAPrio_One);
#else
    const uint size = (mResTableCount * sizeof(SResInfo) + 31) & ~31;
    mpARAMHeader = CARAMManager::Alloc(size);
    const uint handle =
        CARAMManager::DMAToARAM(mResInfoBuckets.data(), const_cast< void* >(mpARAMHeader), size,
                                CARAMManager::kDMAPrio_One);
#endif
    CARAMManager::WaitForDMACompletion(handle);
  }
  mHeaderData = rstl::vector< uchar, rstl::aligned_allocator >();
#if RSTL_PRIME1
  CMemory::OffsetFakeStatics(GetFakeStaticSize());
#else
  UpdateFakeStaticSize();
#endif
}

void CPakFile::LoadResourceTable(CMemoryInStream& in) {
#if RSTL_PRIME1
  int reserveBytes = round_up_32(mResTableCount * static_cast< int >(sizeof(SResInfo))) +
                     static_cast< int >(sizeof(SResInfo)) - 1;
  mResList.reserve(static_cast< uint >(rstl::max_val(reserveBytes, 64)) / sizeof(SResInfo));
  if (mBuildDepList) {
    mDepList.reserve(mResTableCount);
  }

  for (int i = 0; i < static_cast< int >(mResTableCount); ++i) {
    uint flags = in.Get< uint >();
    uint fourCC = in.Get< uint >();
    uint id = in.Get< uint >();
    uint size = in.Get< uint >();
    uint offset = in.Get< uint >();
    mResList.push_back(SResInfo(id, fourCC, offset, size, flags));
    if (mBuildDepList) {
      mDepList.push_back(id);
    }
  }

  static rstl::less< SResInfo > compare;
  rstl::sort(mResList.begin(), mResList.end(), compare);
#else
  rstl::vector< SResInfo, rstl::aligned_allocator > sortedResources;
  sortedResources.reserve(mResTableCount);
  if (mBuildDepList) {
    mDepList.reserve(mResTableCount);
  }
  for (int i = 0; i < static_cast< int >(mResTableCount); ++i) {
    const uint flags = in.ReadInt32();
    const uint type = in.ReadInt32();
    const uint id = in.ReadUint32();
    const uint size = in.Get< uint >();
    const uint offset = in.ReadInt32();
    sortedResources.push_back_unsafe(SResInfo(id, type, offset, size, flags, 0));
    if (mBuildDepList) {
      mDepList.push_back_unsafe(id);
    }
  }

  for (int i = 0; i < static_cast< int >(mResTableCount); ++i) {
    SResInfo& info = sortedResources[i];
    if (info.GetSize() <= 8192) {
      uint groupedSize = 0;
      for (int j = i + 1; j < static_cast< int >(mResTableCount); ++j) {
        const uint nextSize = sortedResources[j].GetSize();
        if (groupedSize + nextSize >= 8192) {
          break;
        }
        groupedSize += nextSize;
      }
      info.SetLookaheadAfterResourceSize(groupedSize);
    }
  }

  static rstl::less< SResInfo > compare;
  rstl::sort(sortedResources.begin(), sortedResources.end(), compare);
  RebuildResourceLists(sortedResources);
#endif
}

uint CPakFile::GetFakeStaticSize() const {
#if RSTL_PRIME1
  return mDepList.size() * sizeof(CAssetId) + mResList.size() * sizeof(SResInfo);
#else
  return mResTableCount * sizeof(SResInfo) + mDepList.size() * sizeof(CAssetId) +
         mResInfoBuckets.size() * sizeof(SResInfo) + mBucketOffsets.size() * sizeof(uint);
#endif
}

const CPakFile::SResInfo* CPakFile::GetResInfo(uint id) RSTL_PRIME1_CONST {
  if (!IsCompletelyLoaded())
    return nullptr;
  if (mStashedInARAM)
    return nullptr;
#if RSTL_PRIME1
  SResInfo key(id, 'TXTR', 0, 0, 0);
  static rstl::less< SResInfo > compare;
  rstl::vector< SResInfo >::const_iterator it =
      rstl::lower_bound(mResList.begin(), mResList.end(), key, compare);
#else
  const uint bucket = id & 0xff;
  rstl::vector< SResInfo, rstl::aligned_allocator >::const_iterator first =
      mResInfoBuckets.begin() + mBucketOffsets[bucket];
  rstl::vector< SResInfo, rstl::aligned_allocator >::const_iterator last =
      mResInfoBuckets.begin() + mBucketOffsets[bucket + 1];
  static rstl::less< SResInfo > compare;
  rstl::vector< SResInfo, rstl::aligned_allocator >::const_iterator it =
      rstl::lower_bound(first, last, SResInfo::SearchKey(id), compare);
#endif
  if (
#if RSTL_PRIME1
      it == mResList.end()
#else
      it == last
#endif
      || it->GetId() != id) {
    return nullptr;
  }

  return &*it;
}

const CPakFile::SResInfo* CPakFile::GetResInfoForLoadDirectionless(uint id)
{
  if (mStashedInARAM)
    return nullptr;
#if RSTL_PRIME1
  SResInfo key(id, 'TXTR', 0, 0, 0);
  static rstl::less< SResInfo > compare;
  rstl::vector< SResInfo >::const_iterator it =
      rstl::lower_bound(mResList.begin(), mResList.end(), key, compare);
#else
  const uint bucket = id & 0xff;
  rstl::vector< SResInfo, rstl::aligned_allocator >::const_iterator first =
      mResInfoBuckets.begin() + mBucketOffsets[bucket];
  rstl::vector< SResInfo, rstl::aligned_allocator >::const_iterator last =
      mResInfoBuckets.begin() + mBucketOffsets[bucket + 1];
  static rstl::less< SResInfo > compare;
  rstl::vector< SResInfo, rstl::aligned_allocator >::const_iterator it =
      rstl::lower_bound(first, last, SResInfo::SearchKey(id), compare);
#endif
  if (
#if RSTL_PRIME1
      it == mResList.end()
#else
      it == last
#endif
      || it->GetId() != id) {
    return nullptr;
  }
  const SResInfo* best = &*it;
  int bestDelta = CMath::AbsI(static_cast< int >(it->GetOffset() - mCurrentSeek));
  ++it;
  while (
#if RSTL_PRIME1
      it != mResList.end()
#else
      it != last
#endif
  ) {
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

const CPakFile::SResInfo* CPakFile::GetResInfoForLoadPreferForward(uint id)
{
  if (mStashedInARAM)
    return nullptr;
#if RSTL_PRIME1
  SResInfo key(id, 'TXTR', 0, 0, 0);
  static rstl::less< SResInfo > compare;
  rstl::vector< SResInfo >::iterator it =
      rstl::lower_bound(mResList.begin(), mResList.end(), key, compare);
#else
  const uint bucket = id & 0xff;
  rstl::vector< SResInfo, rstl::aligned_allocator >::const_iterator first =
      mResInfoBuckets.begin() + mBucketOffsets[bucket];
  rstl::vector< SResInfo, rstl::aligned_allocator >::const_iterator last =
      mResInfoBuckets.begin() + mBucketOffsets[bucket + 1];
  static rstl::less< SResInfo > compare;
  rstl::vector< SResInfo, rstl::aligned_allocator >::const_iterator it =
      rstl::lower_bound(first, last, SResInfo::SearchKey(id), compare);
#endif
  if (
#if RSTL_PRIME1
      it == mResList.end()
#else
      it == last
#endif
      || it->GetId() != id) {
    return nullptr;
  }
#if RSTL_PRIME1
  const SResInfo* best = &*it;
  int bestDelta = mCurrentSeek - static_cast< int >(best->GetOffset());
#else
  const SResInfo* best = &*it;
  int bestDelta = mCurrentSeek - static_cast< int >(it->GetOffset());
#endif
  ++it;
  while (
#if RSTL_PRIME1
      it != mResList.end()
#else
      it != last
#endif
  ) {
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
    for (
#if RSTL_PRIME1
        AUTO(it, mNameList.begin());
#else
        rstl::vector< rstl::pair< rstl::string, SObjectTag > >::const_iterator it =
            mNameList.begin();
#endif
        it != mNameList.end(); ++it) {
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
  if (mWorldPak) {
    mStashedInARAM = true;
    mDepList = rstl::vector< CAssetId >();
#if RSTL_PRIME1
    mResList = rstl::vector< SResInfo >();
#else
    mResInfoBuckets = rstl::vector< SResInfo, rstl::aligned_allocator >();
    mBucketOffsets = rstl::vector< int >();
    UpdateFakeStaticSize();
#endif
  }
}

#if !RSTL_PRIME1
void CPakFile::RebuildResourceLists(
    const rstl::vector< SResInfo, rstl::aligned_allocator >& sortedResources) {
  rstl::reserved_vector< uint, 256 > bucketCounts(0);

  const SResInfo emptyInfo = SResInfo(0, 'TXTR', 0, 0, 0, 0);
  mResInfoBuckets.clear();
  mResInfoBuckets.resize(mResTableCount, emptyInfo);
  mBucketOffsets.clear();
  mBucketOffsets.reserve(257);
  for (rstl::vector< SResInfo, rstl::aligned_allocator >::const_iterator it =
           sortedResources.begin();
       it != sortedResources.end(); ++it)
    ++bucketCounts[it->GetId() & 0xff];
  mBucketOffsets.push_back_unsafe(0);
  uint offset = 0;
  for (int i = 0; i < 256; ++i) {
    offset += bucketCounts[i];
    mBucketOffsets.push_back_unsafe(offset);
    bucketCounts[i] = 0;
  }

  for (rstl::vector< SResInfo, rstl::aligned_allocator >::const_iterator it =
           sortedResources.begin();
       it != sortedResources.end(); ++it) {
    const uint bucket = it->GetId() & 0xff;
    mResInfoBuckets[mBucketOffsets[bucket] + bucketCounts[bucket]] = *it;
    ++bucketCounts[bucket];
  }
}
#endif

void CPakFile::EnsureWorldPakReady() {
  if (mWorldPak && mStashedInARAM) {
#if RSTL_PRIME1
    int reserveBytes = round_up_32(mResTableCount * static_cast< int >(sizeof(SResInfo))) +
                       static_cast< int >(sizeof(SResInfo)) - 1;
    mResList.reserve(static_cast< uint >(rstl::max_val(reserveBytes, 64)) / sizeof(SResInfo));

    for (int i = 0; i < static_cast< int >(mResTableCount); ++i) {
      mResList.push_back(SResInfo(i, 'TXTR', 0, 0, 0));
    }

    uint size = round_up_32(mResTableCount * sizeof(SResInfo));
#else
    rstl::vector< SResInfo, rstl::aligned_allocator > resources;
    resources.resize_uninitialized(mResTableCount);
    const uint size = (mResTableCount * sizeof(SResInfo) + 31) & ~31;
#endif
    CARAMManager::WaitForDMACompletion(CARAMManager::DMAToMRAM(
#if RSTL_PRIME1
        mAramBase, mResList.data(),
#else
        const_cast< void* >(mpARAMHeader), resources.data(),
#endif
        size, CARAMManager::kDMAPrio_One));
#if !RSTL_PRIME1
    RebuildResourceLists(resources);
#endif

    if (mBuildDepList) {
      mDepList.reserve(mResTableCount);
#if RSTL_PRIME1
      for (int i = 0; i < static_cast< int >(mResTableCount); ++i) {
        mDepList.push_back(mResList[i].GetId());
      }
#else
      for (int i = 0; i < mResTableCount; ++i) {
        mDepList.push_back_unsafe(resources[i].GetId());
      }
#endif
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

