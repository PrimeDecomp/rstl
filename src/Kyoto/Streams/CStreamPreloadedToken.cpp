#include "Kyoto/Streams/CStreamPreloadedToken.hpp"
#include "rstl/RstlVersions.h"

#if RSTL_KYOTO_CORRUPTION
#include "Corruption/Kyoto/CDvdFile.hpp"
#include "Corruption/Kyoto/CDvdRequest.hpp"
#else
#include "Echoes/Kyoto/CDvdFile.hpp"
#include "Echoes/Kyoto/CDvdRequest.hpp"
#include "EchoesNamespace.h"
#endif
#if RSTL_KYOTO_DEBUG_BUILD
#include "Kyoto/Alloc/Assert.hpp"
#endif
#include "Kyoto/Alloc/CMemory.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"
#include "rstl/vector.hpp"

#include "dolphin/os/OSCache.h"

#include <string.h>

#ifndef ECHOES_NAMESPACE
#define ECHOES_NAMESPACE
#endif

class CStreamPreloadedData {
public:
  CStreamPreloadedData(const rstl::string& path);
  ~CStreamPreloadedData();

  rstl::string GetFilename() const { return mPath; }
  bool IsReady();
  void Read(void* dest, int offset, int length);

  rstl::string mPath;
  int mSize;
  int mRefCount;
  rstl::vector< rstl::auto_ptr< uchar > > mBuffers;
  rstl::vector< rstl::auto_ptr< ECHOES_NAMESPACE CDvdRequest > > mRequests;
};
CHECK_SIZEOF(CStreamPreloadedData, 0x38)

static rstl::list< rstl::auto_ptr< CStreamPreloadedData > > mPreloadedDatas;

static void CopyAndFlush(void* dest, const void* src, int length) {
  memcpy(dest, src, length);
  DCFlushRange(dest, length);
}

CStreamPreloadedData::CStreamPreloadedData(const rstl::string& filename)
: mPath(filename), mSize(0), mRefCount(1), mBuffers(), mRequests() {
#if RSTL_KYOTO_DEBUG_BUILD
  // clang-format off
  RS_VERIFY_THROW(66, CDvdFile::FileExists( filename.c_str() ), false, "CStreamPreloadedData: Invalid file name\n");
  // clang-format on
#endif
  ECHOES_NAMESPACE CDvdFile file(filename.data());
  mSize = file.GetFileSize();
  int offset;
  const int count = (mSize + 0x3fff) / 0x4000;
  mBuffers.reserve(count);
  mRequests.reserve(count);

  int i = 0;
  offset = 0;
  for (; i < count; offset += 0x4000, ++i) {
    int length = 0x4000;
    const int remaining = mSize - offset;
    if (remaining <= 0x4000) {
      length = remaining;
    }
    const uint alignedLength = (length + 31) & ~31;
    rstl::auto_ptr< uchar > buffer(
        static_cast< uchar* >(RSTL_KYOTO_ALIGNED_NEW(alignedLength, 78)));
    rstl::auto_ptr< ECHOES_NAMESPACE CDvdRequest > request(
        file.AsyncSeekRead(buffer.get(), alignedLength, ECHOES_NAMESPACE kSO_Set, offset));
    mBuffers.push_back_unsafe(buffer);
    mRequests.push_back_unsafe(request);
  }
}

CStreamPreloadedData::~CStreamPreloadedData() {
  for (rstl::vector< rstl::auto_ptr< ECHOES_NAMESPACE CDvdRequest > >::iterator it =
           mRequests.begin();
       it != mRequests.end(); ++it) {
    if (!(*it)->IsComplete()) {
      (*it)->PostCancelRequest();
    }
  }
}

bool CStreamPreloadedData::IsReady() {
  if (!mRequests.empty()) {
    if (!mRequests.back()->IsComplete()) {
      return false;
    }
    mRequests = rstl::vector< rstl::auto_ptr< ECHOES_NAMESPACE CDvdRequest > >();
  }
  return true;
}

void CStreamPreloadedData::Read(void* dest, int offset, int length) {
#if RSTL_KYOTO_PROTO_SOURCE_FORMS
  const int chunk = offset / 0x4000;
  int firstLength = (chunk + 1) * 0x4000 - offset;
#else
  int firstLength;
  const int chunk = offset / 0x4000;
  firstLength = (chunk + 1) * 0x4000 - offset;
#endif
  if (length < firstLength) {
    firstLength = length;
  }
  CopyAndFlush(dest, mBuffers[chunk].get() + (offset - chunk * 0x4000), firstLength);

  uchar* output = static_cast< uchar* >(dest) + firstLength;
  int remaining = length - firstLength;
  int nextChunk = chunk + 1;
  while (remaining != 0) {
#if RSTL_KYOTO_PROTO_SOURCE_FORMS
    int count = 0x4000;
    if (remaining <= 0x4000) {
      count = remaining;
    }
#else
    const int count = remaining > 0x4000 ? 0x4000 : remaining;
#endif
    CopyAndFlush(output, mBuffers[nextChunk].get(), count);
    remaining -= count;
    output += count;
    ++nextChunk;
  }
}

static rstl::list< rstl::auto_ptr< CStreamPreloadedData > >::iterator
FindFile(const rstl::string& path) {
  for (rstl::list< rstl::auto_ptr< CStreamPreloadedData > >::iterator it = mPreloadedDatas.begin();
       it != mPreloadedDatas.end(); ++it) {
    const int comparison = CStringExtras::CompareCaseInsensitive((*it)->GetFilename(), path);
    if (comparison == 0) {
      return it;
    }
  }
  return mPreloadedDatas.end();
}

static CStreamPreloadedData* AcquireFile(const rstl::string& path) {
  rstl::list< rstl::auto_ptr< CStreamPreloadedData > >::iterator it = FindFile(path);
  if (it == mPreloadedDatas.end()) {
    rstl::auto_ptr< CStreamPreloadedData > data(RSTL_KYOTO_NEW(184) CStreamPreloadedData(path));
    it = mPreloadedDatas.insert(mPreloadedDatas.end(), data);
  } else {
    ++(*it)->mRefCount;
  }
  return (*it).get();
}

static void ReleaseFile(const rstl::string& path) {
  rstl::list< rstl::auto_ptr< CStreamPreloadedData > >::iterator it = FindFile(path);
#if RSTL_KYOTO_DEBUG_BUILD
  RS_VERIFY_THROW(198, it != mPreloadedDatas.end(), false,
                  "CStreamPreloadedToken: Tried to delref nonexistant data");
#endif
  if (it != mPreloadedDatas.end()) {
    --(*it)->mRefCount;
    if ((*it)->mRefCount == 0) {
#if RSTL_KYOTO_DEBUG_BUILD
      rs_debugger_printf("Killed preloaded data!\n");
#endif
      mPreloadedDatas.erase(it);
    }
  }
}

CStreamPreloadedToken::CStreamPreloadedToken(const rstl::string& path) : mData(AcquireFile(path)) {}

CStreamPreloadedToken::CStreamPreloadedToken(const CStreamPreloadedToken& other)
: mData(other.mData) {
  ++mData->mRefCount;
}

CStreamPreloadedToken::~CStreamPreloadedToken() { ReleaseFile(mData->GetFilename()); }

void CStreamPreloadedToken::operator=(const CStreamPreloadedToken& other) {
  if (mData != other.mData) {
    ReleaseFile(mData->GetFilename());
    mData = other.mData;
    ++mData->mRefCount;
  }
}

bool CStreamPreloadedToken::IsReady() const { return mData->IsReady(); }

void CStreamPreloadedToken::Read(void* dest, int offset, int length) const {
  mData->Read(dest, offset, length);
}

#if RSTL_KYOTO_PROTO_STREAM_HELPERS
int CStreamPreloadedToken::GetReferenceCount() const { return mData->mRefCount; }
#endif
