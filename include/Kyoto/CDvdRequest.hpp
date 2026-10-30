#ifndef _CDVDREQUEST
#define _CDVDREQUEST
#include "rstl/RstlVersions.h"

#include "dolphin/dvd.h"
#include "types.h"

class CDvdRequest {
public:
  virtual ~CDvdRequest() = 0;           // 8
  virtual void WaitUntilComplete() = 0; // c
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
#if RSTL_VERSION >= RSTL_R32J
  virtual bool IsReady() = 0; // Guessed name; native predicate is END or BUSY.
#endif
#endif
  virtual bool IsComplete() = 0;        // GC 10, Wii 14
  virtual void PostCancelRequest() = 0; // GC 14, Wii 18
  virtual int GetMediaType() const = 0; // GC 18, Wii 1c
};
#if !RSTL_PRIME1
DECLARE_FULL_SIZE_FOR(CDvdRequest, 0x4)
CHECK_SIZEOF(CDvdRequest, CDvdRequest_FULL_SIZE)
#endif

inline CDvdRequest::~CDvdRequest() {}

class CRealDvdRequest : public CDvdRequest {
public:
  ~CRealDvdRequest();
  void WaitUntilComplete() override;
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  bool IsReady(); // Guessed name.
#endif
  bool IsComplete() override;
  void PostCancelRequest() override;
  int GetMediaType() const override;

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  DVDFileInfo* FileInfo() { return &mFileInfo; }
#else
  DVDFileInfo& FileInfo() { return mFileInfo; }
#endif

private:
  DVDFileInfo mFileInfo;
};

#if !RSTL_PRIME1
CHECK_CHILD_SIZEOF(CRealDvdRequest, CDvdRequest, 0x3c)
#endif

class CARAMDvdRequest : public CDvdRequest {
public:
  CARAMDvdRequest(uint i) : mDmaReq(i) {}
  void WaitUntilComplete() override;
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  bool IsReady(); // Guessed name.
#endif
  bool IsComplete() override;
  void PostCancelRequest() override;
  int GetMediaType() const override;

private:
  uint mDmaReq;
};

#if !RSTL_PRIME1
CHECK_CHILD_SIZEOF(CARAMDvdRequest, CDvdRequest, 0x4)
#endif

#endif // _CDVDREQUEST
