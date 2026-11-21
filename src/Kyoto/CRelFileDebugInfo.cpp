#include "rstl/RstlVersions.h"
#if RSTL_KYOTO_PROTO_UNIT
#include "Kyoto/CRelFileDebugInfo.hpp"

#include "Kyoto/Alloc/CMemory.hpp"

CRelFileDebugInfo* CRelFileDebugInfo::mspHead = nullptr;

CRelFileDebugInfo::CRelFileDebugInfo()
: mName(nullptr), mStart(0), mSize(0), mNext(nullptr), mPrev(nullptr) {}

CRelFileDebugInfo::~CRelFileDebugInfo() { Unregister(); }

void CRelFileDebugInfo::Register(const char* name, const void* start, uint size) {
  if (mNext != nullptr) {
    return;
  }
  mName = name;
#if RSTL_CORRUPTION
  mStart = reinterpret_cast< uint >(start);
#else
  mStart = reinterpret_cast< TRelFileDebugStart >(start);
#endif
  mSize = size;
  mNext = mspHead;
  if (mNext != nullptr) {
    mNext->mPrev = this;
  }
  mspHead = this;
}

void CRelFileDebugInfo::Unregister() {
  if (mspHead == this) {
    mspHead = mNext;
  }
  if (mNext != nullptr) {
    mNext->mPrev = mPrev;
  }
  if (mPrev != nullptr) {
    mPrev->mNext = mNext;
  }
  mNext = nullptr;
  mPrev = nullptr;
}

#if RSTL_CORRUPTION
bool CRelFileDebugInfo::Contains(int address) const { return address - mStart < mSize; }
#else
bool CRelFileDebugInfo::Contains(TRelFileDebugAddress address) const { return address - mStart < mSize; }
#endif

#if RSTL_CORRUPTION
CRelFileDebugInfo* CRelFileDebugInfo::FindByAddress(int address) {
#else
CRelFileDebugInfo* CRelFileDebugInfo::FindByAddress(TRelFileDebugAddress address) {
#endif
  for (CRelFileDebugInfo* info = mspHead; info != nullptr; info = info->mNext) {
    if (info->Contains(address)) {
      return info;
    }
  }
  return nullptr;
}
#endif
