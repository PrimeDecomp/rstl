#ifndef _CDVDREQUEST
#define _CDVDREQUEST
#include "rstl/RstlVersions.h"

#include "dolphin/dvd.h"
#include "types.h"

class CDvdRequest {
public:
  virtual ~CDvdRequest() = 0;           // 8
  virtual void WaitUntilComplete() = 0; // c
#if !RSTL_PRIME1 && RSTL_VERSION >= RSTL_R32J
  virtual bool IsReady() = 0; // Guessed name; native predicate is END or BUSY.
#endif
  virtual bool IsComplete() = 0;        // GC 10, Wii 14
  virtual void PostCancelRequest() = 0; // GC 14, Wii 18
  virtual int GetMediaType() const = 0; // GC 18, Wii 1c
};
DECLARE_FULL_SIZE_FOR(CDvdRequest, 0x4)
CHECK_SIZEOF(CDvdRequest, CDvdRequest_FULL_SIZE)

inline CDvdRequest::~CDvdRequest() {}

class CRealDvdRequest : public CDvdRequest {
public:
  ~CRealDvdRequest();
  void WaitUntilComplete() override;
  bool IsReady(); // Guessed name.
  bool IsComplete() override;
  void PostCancelRequest() override;
  int GetMediaType() const override;

  DVDFileInfo& FileInfo() { return mFileInfo; }

private:
  DVDFileInfo mFileInfo;
};

CHECK_CHILD_SIZEOF(CRealDvdRequest, CDvdRequest, 0x3c)

class CARAMDvdRequest : public CDvdRequest {
public:
  CARAMDvdRequest(uint i) : mDmaReq(i) {}
  void WaitUntilComplete() override;
  bool IsReady(); // Guessed name.
  bool IsComplete() override;
  void PostCancelRequest() override;
  int GetMediaType() const override;

private:
  uint mDmaReq;
};

CHECK_CHILD_SIZEOF(CARAMDvdRequest, CDvdRequest, 0x4)

#endif // _CDVDREQUEST
