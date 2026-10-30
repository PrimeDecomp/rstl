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
#if RSTL_PRIME1
  virtual bool IsComplete() = 0;        // 10
  virtual void PostCancelRequest() = 0; // 14
  virtual int GetMediaType() const = 0; // 18
#else
#if RSTL_VERSION >= RSTL_R32J
  virtual bool IsReady() = 0; // Guessed name; native predicate is END or BUSY.
#endif
  virtual bool IsComplete() = 0;        // GC 10, Wii 14
  virtual void PostCancelRequest() = 0; // GC 14, Wii 18
  virtual int GetMediaType() const = 0; // GC 18, Wii 1c
#endif

};
#if !RSTL_PRIME1
DECLARE_FULL_SIZE_FOR(CDvdRequest, 0x4)
CHECK_SIZEOF(CDvdRequest, CDvdRequest_FULL_SIZE)
#endif

inline CDvdRequest::~CDvdRequest() {}

class CRealDvdRequest : public CDvdRequest {
public:
  ~CRealDvdRequest();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void WaitUntilComplete() override;
  bool IsComplete() override;
  void PostCancelRequest() override;
  int GetMediaType() const override;
#else
  void WaitUntilComplete();
  bool IsReady(); // Guessed name.
  bool IsComplete();
  void PostCancelRequest();
  int GetMediaType() const;
#endif

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
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  void WaitUntilComplete() override;
  bool IsComplete() override;
  void PostCancelRequest() override;
  int GetMediaType() const override;
#else
  void WaitUntilComplete();
  bool IsReady(); // Guessed name.
  bool IsComplete();
  void PostCancelRequest();
  int GetMediaType() const;
#endif

private:
  uint mDmaReq;
};

#if !RSTL_PRIME1
CHECK_CHILD_SIZEOF(CARAMDvdRequest, CDvdRequest, 0x4)
#endif

#endif // _CDVDREQUEST
