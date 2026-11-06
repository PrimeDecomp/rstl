#include "Kyoto/CResFactory.hpp" // IWYU pragma: keep
#include "rstl/macros.hpp"
#include "rstl/list.hpp"         // IWYU pragma: keep

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Basics/CStopwatch.hpp"
#include "Kyoto/CDvdRequest.hpp"
#include "Kyoto/Streams/CLZOSupport.hpp"
#include "Kyoto/Streams/CZipSupport.hpp"

#include "dolphin/os.h"
#include "rstl/RstlVersions.h"
#include "rstl/math.hpp"

#if RSTL_PRIME1
#include "zlib.h"

static const int kDecompChunkSize = 4096;
#endif

// Written by CMFGameLoader; its purpose is not yet identified.
int gResFactoryUnknown = 2;

CResFactory::CResFactory() {}

CResFactory::~CResFactory() {}

void CResFactory::AddToLoadList(const SLoadingData& data) {
  AUTO(pos, mLoadList.end());
  if (data.mDvdReq->GetMediaType() == 0) {
    pos = mLoadList.begin();
    while (pos != mLoadList.end()) {
      if (pos->mDvdReq->GetMediaType() == 1) {
        break;
      }
      ++pos;
    }
  }
  AUTO(it, mLoadList.insert(pos, data));
  mLoadMap.insert(rstl::pair< SObjectTag, LoadList::iterator >(data.mTag, it));
}

void CResFactory::EraseFromLoadList(const LoadList::iterator& it) {
  mLoadMap.erase(it->mTag);
  mLoadList.erase(it);
}

CResFactory::LoadList::iterator CResFactory::FindInLoadList(const SObjectTag& tag) {
  rstl::map< SObjectTag, LoadList::iterator >::const_iterator it = mLoadMap.find(tag);
  if (it == mLoadMap.end()) {
    return mLoadList.end();
  }
  return it->second;
}

// Guessed; stands in for a function stripped from the link.
static bool StrippedIsLoading(
    const rstl::map< SObjectTag, rstl::list< CResFactory::SLoadingData >::iterator >& map,
    const SObjectTag& tag) {
  return map.find(tag) != map.end();
}

#if !defined(TARGET_PC)
rstl::auto_ptr< IObj > CResFactory::Build(const SObjectTag& tag, const CVParamTransfer& params) {
  AUTO(it, FindInLoadList(tag));
  if (it != mLoadList.end()) {
    IObj** target = it->mTarget;
    while (*target == nullptr) {
      while (!PumpResource(it, 0)) {
      }
    }
    return rstl::auto_ptr< IObj >(*target);
  }
  return BuildSync(tag, params);
}

rstl::auto_ptr< IObj > CResFactory::BuildSync(const SObjectTag& tag,
                                              const CVParamTransfer& params) {
  if (mFactoryMgr.CanMakeMemory(tag)) {
    char* buffer;
    int size;
    mResLoader.LoadMemResourceSync(tag, &buffer, &size);
    return mFactoryMgr.MakeObjectFromMemory(
        tag,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        buffer,
#else
        rstl::auto_ptr< uchar >(reinterpret_cast< uchar* >(buffer)),
#endif
        size, mResLoader.GetResourceCompression(tag) != CResLoader::kCompressionType_Uncompressed,
        params);
  }
  CInputStream* in = mResLoader.LoadNewResourceSync(tag, nullptr);
  rstl::auto_ptr< IObj > result = mFactoryMgr.MakeObject(tag, *in, params);
  delete in;
  return result;
}

void CResFactory::BuildAsync(const SObjectTag& tag, const CVParamTransfer& params, IObj** target) {
  *target = nullptr;
  const uint size = ResourceSize(tag);
#if RSTL_PRIME1
  char* buffer = static_cast< char* >(CMemory::Alloc(size, IAllocator::kHI_RoundUpLen));
  CDvdRequest* request = LoadResourceAsync(tag, buffer);
#else
  CBufferedDvdRequest* request = mResLoader.LoadResourceAsync(tag);
  const bool ownsBuffer = request->OwnsBuffer();
  rstl::auto_ptr< uchar > buffer;
  if (ownsBuffer) {
    buffer = request->GetBuffer();
  } else {
    buffer = rstl::auto_ptr< uchar >(request->GetBufferPtr());
    buffer.release();
  }
#endif

  SLoadingData data(tag, request, target, buffer, size, mResLoader.GetResourceCompression(tag),
                    params);
  AddToLoadList(data);
}
#endif

void CResFactory::CancelBuild(const SObjectTag& tag) {
  AUTO(it, FindInLoadList(tag));
  if (it != mLoadList.end()) {
    SLoadingData& data = *it;
    data.mDvdReq->PostCancelRequest();
    mCancelledList.push_back(data);
    EraseFromLoadList(it);
  }
}

void CResFactory::AsyncIdle(uint time IF_NOT_RSTL_PRIME1(, bool keepPumping)) {
  IF_RSTL_PRIME1(bool keepPumping);
  bool work = true;
  CStopwatch timer;
  AUTO(it, mCancelledList.begin());
  while (it != mCancelledList.end()) {
    AUTO(current, it);
    ++it;
    if (current->mDvdReq->IsComplete()) {
      mCancelledList.erase(current);
    }
  }

  if (RSTL_PRIME1) {
    while (work) {
      work = false;
      uint elapsed = timer.GetElapsedMicros();
      AUTO(it, mLoadList.begin());
      while (it != mLoadList.end() && elapsed < time) {
        if (PumpResource(it, time - elapsed)) {
          work = true;
          break;
        }
        ++it;
        elapsed = timer.GetElapsedMicros();
      }
    }
  } else {
    bool done = false;
    while (!done && mLoadList.size() != 0) {
      done = true;
      const uint elapsed = timer.GetElapsedMicros();
      if (elapsed < time && mLoadList.size() != 0) {
        done = !PumpResource(mLoadList.begin(), time - elapsed);
        if (keepPumping) {
          done = false;
        }
      }
    }
  }
}

bool CResFactory::PumpResource(const LoadList::iterator& it, uint time) {
  if (it->mDvdReq->IsComplete()) {
    if (it->mCompression == CResLoader::kCompressionType_Compressed &&
        !it->PumpDecompression(time)) {
      return false;
    }
    SLoadingData data(*it);
    EraseFromLoadList(it);
    *data.mTarget =
        mFactoryMgr
            .MakeObjectFromMemory(
                data.mTag, IF_RSTL_PRIME1_ELSE(data.mBuffer.release(), data.mBuffer), data.mSize,
                data.mCompression != CResLoader::kCompressionType_Uncompressed, data.mParams)
            .release();
    return true;
  }
  return false;
}

CResFactory::SLoadingData::SLoadingData(const SObjectTag& tag, CDvdRequest* request, IObj** target,
                                        CFactoryMgr::MemoryBufferPtr buffer, int size,
                                        CResLoader::ECompressionType compression,
                                        const CVParamTransfer& params)
: mTag(tag)
, mDvdReq(request)
, mTarget(target)
, mBuffer(IF_RSTL_PRIME1_ELSE(static_cast< uchar* >(buffer), buffer))
#if !RSTL_PRIME1
, mCompressedPos(0)
, mDecompressedPos(0)
#endif
, mSize(size)
, mCompression(compression)
, mParams(params) {
}

CResFactory::SLoadingData::~SLoadingData() {
#if RSTL_PRIME1
  if (mZip.owner()) {
    inflateEnd(mZip.get());
  }
#endif
}

bool CResFactory::SLoadingData::PumpDecompression(uint time) {
  CStopwatch timer;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  z_stream_s* zip;
#else
  uint chunkSize;
  s64 timeLimit;
#endif

  uint* buffer = reinterpret_cast< uint* >(mBuffer.get());
  const uint length = CBasics::SwapBytes(*buffer);
  if (IF_RSTL_PRIME1_ELSE((zip = mZip.get()) == nullptr, mDecompBuffer.null())) {
#if RSTL_PRIME1
    zip = rs_new z_stream_s;
    zip->zalloc = CZipSupport::Alloc;
    zip->zfree = CZipSupport::Free;
    zip->opaque = nullptr;
#if NONMATCHING
    inflateInit(zip);
#else
    inflateInit2(zip);
#endif
    zip->total_in = 0;
    zip->total_out = 0;
    zip->avail_in = 0;
    zip->avail_out = 0;
    zip->next_in = nullptr;
    zip->next_out = nullptr;
    mZip = rstl::auto_ptr< z_stream_s >(zip);
#endif
    mDecompBuffer = rstl::auto_ptr< uchar >(
        static_cast< uchar* >(CMemory::Alloc(length, IAllocator::kHI_RoundUpLen)));
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const int size = mSize;
#else
  timeLimit = time;
#endif
  uchar* compressed = reinterpret_cast< uchar* >(buffer + 1);
  while ((time == 0 || timer.GetElapsedMicros() < IF_RSTL_PRIME1_ELSE(time, timeLimit)) &&
         length != IF_RSTL_PRIME1_ELSE(zip->total_out, mDecompressedPos)) {
#if RSTL_PRIME1
    if (zip->avail_in == 0) {
      const int amount = rstl::min_val< int >(kDecompChunkSize, size - 4 - zip->total_in);
      if (amount == 0) {
        break;
      }
      zip->next_in = compressed + zip->total_in;
      zip->avail_in = amount;
      zip->next_out = mDecompBuffer.get() + zip->total_out;
      zip->avail_out = length - zip->total_out;
    }
    inflate(zip, Z_NO_FLUSH);
#else
    chunkSize = BE_SWAP(*reinterpret_cast< ushort* >(compressed + mCompressedPos));
    mCompressedPos += 2;
    uint outSize = length - mDecompressedPos;
    CLZOSupport::Inflate(compressed + mCompressedPos, chunkSize,
                         mDecompBuffer.get() + mDecompressedPos, outSize);
    DCStoreRange(mDecompBuffer.get() + mDecompressedPos, outSize);
    mCompressedPos += chunkSize;
    mDecompressedPos += outSize;
#endif
  }

  if (length == IF_RSTL_PRIME1_ELSE(zip->total_out, mDecompressedPos)) {
#if RSTL_PRIME1
    inflateEnd(zip);
    mZip = rstl::auto_ptr< z_stream_s >();
    mBuffer = rstl::auto_ptr< uchar >(mDecompBuffer.release());
#else
    mBuffer = mDecompBuffer;
#endif
    mDecompBuffer = rstl::auto_ptr< uchar >();
    mCompression = CResLoader::kCompressionType_Uncompressed;
    mSize = length;
    return true;
  }
  return false;
}
