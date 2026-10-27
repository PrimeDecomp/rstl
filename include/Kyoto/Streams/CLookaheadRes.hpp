#ifndef KYOTO_STREAMS_CLOOKAHEADRES_HPP
#define KYOTO_STREAMS_CLOOKAHEADRES_HPP

#include "types.h"

#include "Corruption/Kyoto/CDvdRequest.hpp"
#include "rstl/auto_ptr.hpp"

class CDvdFile;
class CLookaheadRes;
class CResLoader;

// Guessed name
class CBufferedDvdRequest : public CDvdRequest {
public:
  CBufferedDvdRequest(CLookaheadRes* cache, CDvdRequest* request, uchar* buffer);

  // CDvdRequest
  ~CBufferedDvdRequest();
  void WaitUntilComplete();
  bool IsComplete();
  void PostCancelRequest();
  int GetMediaType() const;

  bool GetRequestOwnsMemory() const { return mBuffer.owner(); }
  uchar* GetBufferPtr() const { return mBuffer.get(); }
  rstl::auto_ptr< uchar >& GetBuffer();

private:
  CLookaheadRes* mCache;
  rstl::auto_ptr< CDvdRequest > mRequest;
  rstl::auto_ptr< uchar > mBuffer;
  int mMediaType;
};
CHECK_SIZEOF(CBufferedDvdRequest, 0x1c)

class CLookaheadRes {
public:
  CLookaheadRes(uchar* buffer, CDvdFile* file, uint offset, uint size,
                const rstl::auto_ptr< CDvdRequest >& request, CResLoader* owner);
  ~CLookaheadRes();

  bool IsInvalid() const { return mInvalid; }
  bool Contains(const CDvdFile* file, uint offset, uint size) const;
  rstl::auto_ptr< CBufferedDvdRequest > MakeRequest(uint offset);
  void RequestHasDied();
  bool Cancel();

private:
  rstl::auto_ptr< uchar > mBuffer;
  CDvdFile* mFile;
  uint mOffset;
  uint mSize;
  uint mReferenceCount : 16;
  uint mInvalid : 1;
  rstl::auto_ptr< CDvdRequest > mRequest;
  CResLoader* mOwner;
};
CHECK_SIZEOF(CLookaheadRes, 0x24)

#endif
