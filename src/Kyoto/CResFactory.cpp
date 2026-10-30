#include "rstl/RstlVersions.h"
#include "rstl/list.hpp" // IWYU pragma: keep
#include "Kyoto/CResFactory.hpp" // IWYU pragma: keep
#include "Kyoto/CDvdRequest.hpp"
#include "Kyoto/Alloc/CMemory.hpp"

#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Basics/CStopwatch.hpp"
#include "Kyoto/Streams/CLZOSupport.hpp"

// TODO: verify that this Prime/Echoes difference is real
#include "Kyoto/Streams/CZipSupport.hpp"
#include "rstl/math.hpp"
#if RSTL_PRIME1
#else
#include "dolphin/os.h"
#endif

#if RSTL_PRIME1
#include "zlib.h"

static const int kDecompChunkSize = 4096;
#endif

// Written by CMFGameLoader; its purpose is not yet identified.
int gResFactoryUnknown = 2;

CResFactory::CResFactory() {}

CResFactory::~CResFactory() {}

void CResFactory::AddToLoadList(const SLoadingData& data) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  AUTO(pos, mLoadList.end());
#else
  LoadList::iterator pos = mLoadList.end();
#endif

  if (data.mDvdReq->GetMediaType() == 0) {
    pos = mLoadList.begin();
    while (pos != mLoadList.end()) {
      if (pos->mDvdReq->GetMediaType() == 1) {
        break;
      }
      ++pos;
    }
  }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  AUTO(it, mLoadList.insert(pos, data));
#else
  LoadList::iterator it = mLoadList.insert(pos, data);
#endif

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

#if !RSTL_PRIME1
// Guessed; stands in for a function stripped from the link.
static bool StrippedIsLoading(
    const rstl::map< SObjectTag, rstl::list< CResFactory::SLoadingData >::iterator >& map,
    const SObjectTag& tag) {
  return map.find(tag) != map.end();
}
#endif

rstl::auto_ptr< IObj > CResFactory::Build(const SObjectTag& tag, const CVParamTransfer& params) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  AUTO(it, FindInLoadList(tag));
#else
  LoadList::iterator it = FindInLoadList(tag);
#endif

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return mFactoryMgr.MakeObjectFromMemory(tag, buffer, size,
                                               mResLoader.GetResourceCompression(tag) !=
                                                   CResLoader::kCompressionType_Uncompressed,
#else
    return mFactoryMgr.MakeObjectFromMemory(
        tag, rstl::auto_ptr< uchar >(reinterpret_cast< uchar* >(buffer)), size,
        mResLoader.GetResourceCompression(tag) != CResLoader::kCompressionType_Uncompressed,
#endif

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
// TODO: verify that this Prime/Echoes difference is real
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

void CResFactory::CancelBuild(const SObjectTag& tag) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  AUTO(it, FindInLoadList(tag));
#else
  LoadList::iterator it = FindInLoadList(tag);
#endif

  if (it != mLoadList.end()) {
    SLoadingData& data = *it;
    data.mDvdReq->PostCancelRequest();
    mCancelledList.push_back(data);
    EraseFromLoadList(it);
  }
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CResFactory::AsyncIdle(uint time) {
  bool work = true;
#else
void CResFactory::AsyncIdle(uint time, bool keepPumping) {
#endif

  CStopwatch timer;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  AUTO(it, mCancelledList.begin());
#else
  LoadList::iterator it = mCancelledList.begin();
#endif

  while (it != mCancelledList.end()) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    AUTO(current, it);
#else
    LoadList::iterator current = it;
#endif

    ++it;
    if (current->mDvdReq->IsComplete()) {
      mCancelledList.erase(current);
    }
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  while (work) {
    work = false;
    uint elapsed = timer.GetElapsedMicros();
    AUTO(it, mLoadList.begin());
    while (it != mLoadList.end() && elapsed < time) {
      if (PumpResource(it, time - elapsed)) {
        work = true;
        break;
#else
  bool done = false;
  while (!done && mLoadList.size() != 0) {
    done = true;
    const uint elapsed = timer.GetElapsedMicros();
    if (elapsed < time && mLoadList.size() != 0) {
      done = !PumpResource(mLoadList.begin(), time - elapsed);
      if (keepPumping) {
        done = false;
#endif

      }
#if RSTL_PRIME1
      ++it;
      elapsed = timer.GetElapsedMicros();
#endif

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
            .MakeObjectFromMemory(data.mTag, data.mBuffer.release(), data.mSize,
#else
            .MakeObjectFromMemory(data.mTag, data.mBuffer, data.mSize,
#endif

                                  data.mCompression != CResLoader::kCompressionType_Uncompressed,
                                  data.mParams)
            .release();
    return true;
  }
  return false;
}

CResFactory::SLoadingData::SLoadingData(const SObjectTag& tag, CDvdRequest* request, IObj** target,
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                                        void* buffer, int size,
#else
                                        const rstl::auto_ptr< uchar >& buffer, int size,
#endif

                                        CResLoader::ECompressionType compression,
                                        const CVParamTransfer& params)
: mTag(tag)
, mDvdReq(request)
, mTarget(target)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mBuffer(static_cast< uchar* >(buffer))
#else
, mBuffer(buffer)
, mCompressedPos(0)
, mDecompressedPos(0)
#endif

, mSize(size)
, mCompression(compression)
, mParams(params) {}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CResFactory::SLoadingData::~SLoadingData() {
  if (mZip.owner()) {
    inflateEnd(mZip.get());
  }
}
#else
CResFactory::SLoadingData::~SLoadingData() {}
#endif

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if ((zip = mZip.get()) == nullptr) {
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
#else
  if (mDecompBuffer.null()) {
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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  while ((time == 0 || timer.GetElapsedMicros() < time) && length != zip->total_out) {
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
  while ((time == 0 || timer.GetElapsedMicros() < timeLimit) && length != mDecompressedPos) {
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

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (length == zip->total_out) {
    inflateEnd(zip);
    mZip = rstl::auto_ptr< z_stream_s >();
    mBuffer = rstl::auto_ptr< uchar >(mDecompBuffer.release());
#else
  if (length == mDecompressedPos) {
    mBuffer = mDecompBuffer;
#endif

    mDecompBuffer = rstl::auto_ptr< uchar >();
    mCompression = CResLoader::kCompressionType_Uncompressed;
    mSize = length;
    return true;
  }
  return false;
}

