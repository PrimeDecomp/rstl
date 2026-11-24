#include "Kyoto/CDvdRequest.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
// Prototype-only: Prime and Echoes have no dummy request.
#if RSTL_CORRUPTION
int CDummyDvdRequest::GetMediaType() const { return 1; }

void CDummyDvdRequest::PostCancelRequest() {}

bool CDummyDvdRequest::IsComplete() { return true; }

void CDummyDvdRequest::WaitUntilComplete() {}

CDummyDvdRequest::~CDummyDvdRequest() {}
#endif
