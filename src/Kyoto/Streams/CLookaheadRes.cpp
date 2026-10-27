#include "rstl/RstlVersions.h"
#if RSTL_KYOTO_CORRUPTION
#include "Kyoto/Streams/CLookaheadRes.hpp"

#include "Corruption/Kyoto/CResLoader.hpp"
#else
#include "Echoes/Kyoto/CResLoader.hpp"

#include "EchoesNamespace.h"
#endif

#if RSTL_KYOTO_DEBUG_BUILD
#include "Kyoto/Alloc/Assert.hpp"
#endif
#include "Kyoto/Alloc/CMemory.hpp"

// Echoes declares these classes in its CResLoader.hpp, inside its game namespace.
#ifndef START_ECHOES_NAMESPACE
#define START_ECHOES_NAMESPACE
#define END_ECHOES_NAMESPACE
#endif

START_ECHOES_NAMESPACE

// CLookaheadRes is named by a Corruption diagnostic; CBufferedDvdRequest is still a guess.

CLookaheadRes::CLookaheadRes(uchar* buffer, CDvdFile* file, uint offset, uint size,
                             const rstl::auto_ptr< CDvdRequest >& request, CResLoader* owner)
: mBuffer(buffer)
, mFile(file)
, mOffset(offset)
, mSize(size)
, mReferenceCount(0)
, mInvalid(false)
, mRequest(request)
, mOwner(owner) {}

CLookaheadRes::~CLookaheadRes() {}

void CLookaheadRes::RequestHasDied() {
#if RSTL_KYOTO_DEBUG_BUILD
  RS_VERIFY_THROW(32, mReferenceCount > 0, false,
                  "Trying to kill too many references in CLookaheadRes::RequestHasDied");
#endif
  --mReferenceCount;
  if (mReferenceCount == 0) {
    mOwner->KillLookahead(this);
  }
}

rstl::auto_ptr< CBufferedDvdRequest > CLookaheadRes::MakeRequest(uint offset) {
#if RSTL_KYOTO_DEBUG_BUILD
  RS_VERIFY_THROW(52, offset >= mOffset && offset < mOffset + mSize, false,
                  "Trying make an out of range lookahead request");
#endif
  ++mReferenceCount;
  return rstl::auto_ptr< CBufferedDvdRequest >(RSTL_KYOTO_NEW(55) CBufferedDvdRequest(
      this, mRequest.get(), mBuffer.get() + (offset - mOffset)));
}

bool CLookaheadRes::Contains(const CDvdFile* file, uint offset, uint size) const {
  if (file != mFile) {
    return false;
  }
  if (offset >= mOffset && offset < mOffset + mSize) {
#if RSTL_KYOTO_DEBUG_BUILD
    if (offset + size > mOffset + mSize) {
      RS_VERIFY_THROW(73, false, false,
                      "How can there be a partial lookahead allocation in here?!");
      return false;
    }
    return true;
#else
    return offset + size <= mOffset + mSize;
#endif
  }
  return false;
}

bool CLookaheadRes::Cancel() {
  if (mReferenceCount > 1) {
    return true;
  }
  mInvalid = true;
  mRequest->PostCancelRequest();
  return mRequest->IsComplete();
}

CBufferedDvdRequest::CBufferedDvdRequest(CLookaheadRes* cache, CDvdRequest* request, uchar* buffer)
: mCache(cache), mRequest(request), mBuffer(buffer), mMediaType(request->GetMediaType()) {
  if (mCache != nullptr) {
    mRequest.release();
    mBuffer.release();
  }
}

CBufferedDvdRequest::~CBufferedDvdRequest() {
  if (mCache != nullptr) {
    mCache->RequestHasDied();
    mCache = nullptr;
  }
}

rstl::auto_ptr< uchar >& CBufferedDvdRequest::GetBuffer() {
#if RSTL_KYOTO_DEBUG_BUILD
  RS_VERIFY_THROW(142, GetRequestOwnsMemory(), false,
                  "Trying to take ownership of memory you don't know");
#endif
  return mBuffer;
}

void CBufferedDvdRequest::WaitUntilComplete() {
  if (mRequest.get() != nullptr) {
    mRequest->WaitUntilComplete();
  }
}

bool CBufferedDvdRequest::IsComplete() {
  if (mRequest.get() == nullptr) {
    return true;
  }
  return mRequest->IsComplete();
}

void CBufferedDvdRequest::PostCancelRequest() {
  if (mRequest.get() == nullptr) {
    return;
  }
  if (mCache != nullptr) {
    if (mCache->Cancel()) {
      mCache->RequestHasDied();
      mCache = nullptr;
      mRequest = rstl::auto_ptr< CDvdRequest >();
    }
  } else {
    mRequest->PostCancelRequest();
  }
}

int CBufferedDvdRequest::GetMediaType() const { return mMediaType; }

END_ECHOES_NAMESPACE
