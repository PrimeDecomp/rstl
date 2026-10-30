#include "rstl/RstlVersions.h"
#include "Kyoto/CDvdRequest.hpp"

#include "Kyoto/CARAMManager.hpp"

#include "Kyoto/Alloc/CMemory.hpp"

CRealDvdRequest::~CRealDvdRequest() {
  if (!IsComplete()) {
    PostCancelRequest();
    WaitUntilComplete();
  }
  DVDClose(&mFileInfo);
}

void CRealDvdRequest::WaitUntilComplete() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  while (!CRealDvdRequest::IsComplete()) {}
#else
  while (!CRealDvdRequest::IsComplete()) {
  }
#endif

}

bool CRealDvdRequest::IsComplete()  {
  s32 status = DVDGetCommandBlockStatus(&mFileInfo.cb);
  bool ret = false;
  if (status == DVD_STATE_END || status == DVD_STATE_CANCELED) {
    ret = true;
  }

  return ret;
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CRealDvdRequest::PostCancelRequest() {
  DVDCancelAsync(&mFileInfo.cb, nullptr);
}
#else
void CRealDvdRequest::PostCancelRequest() { DVDCancelAsync(&mFileInfo.cb, nullptr); }
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
int CRealDvdRequest::GetMediaType() const {
  return 1;
}
#else
int CRealDvdRequest::GetMediaType() const { return 1; }
#endif

void CARAMDvdRequest::WaitUntilComplete() {
  if (CARAMManager::GetInvalidDMAHandle() == mDmaReq) {
    return;
  }

  CARAMManager::WaitForDMACompletion(mDmaReq);
  mDmaReq = CARAMManager::GetInvalidDMAHandle();
}

bool CARAMDvdRequest::IsComplete() {
  if (mDmaReq != CARAMManager::GetInvalidDMAHandle()) {
    if (!CARAMManager::IsDMACompleted(mDmaReq)) {
      return false;
    }

    mDmaReq = CARAMManager::GetInvalidDMAHandle();
  }

  return true;
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
void CARAMDvdRequest::PostCancelRequest() {
#else
void CARAMDvdRequest::PostCancelRequest() {}
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
}

int CARAMDvdRequest::GetMediaType() const {
  return 0;
}
#else
int CARAMDvdRequest::GetMediaType() const { return 0; }
#endif

