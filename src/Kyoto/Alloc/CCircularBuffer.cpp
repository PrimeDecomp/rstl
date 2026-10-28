#include "Kyoto/Alloc/CCircularBuffer.hpp"

#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Basics/CBasics.hpp"

CCircularBuffer::CCircularBuffer(void* buf, const int len, const EOwnership ownership)
: mPtr(static_cast< char* >(buf)), mMemorySize(len), xc_(0), mNextFreeAddr(0), x14_(-1) {
  if (ownership == kOS_NotOwned) {
    (void)mPtr.release();
  }
}

bool CCircularBuffer::IsWrappedMemory(const int offset, const int len) {
  if (x14_ > -1 && x14_ >= offset && x14_ < (offset + len)) {
    return true;
  }

  return false;
}

void* CCircularBuffer::Alloc(const int size) {
  RS_VERIFY_THROW_DIRECT(
      28, size <= mMemorySize, false,
      CBasics::Stringize("The size %d is larger than can be allocated %d", size, mMemorySize));

  if ((mMemorySize - mNextFreeAddr) >= size && !IsWrappedMemory(mNextFreeAddr, size)) {
    const int offset = mNextFreeAddr;
    uchar* ptr = reinterpret_cast< uchar* >(mPtr.get());
    mNextFreeAddr = offset + size;
    return ptr + offset;
  }
  if (xc_ >= size && !IsWrappedMemory(0, size)) {
    uint previousStart = xc_;
    xc_ = 0;
    mNextFreeAddr = size;
    x14_ = previousStart;
    return mPtr.get();
  }

  return nullptr;
}

void CCircularBuffer::Free(void* ptr, const int len) {
  if (x14_ > -1) {
    if (ptr == mPtr.get()) {
      x14_ = -1;
      xc_ = len;
    } else {
      x14_ += len;
    }
  } else {
    xc_ += len;
  }

  if (x14_ == -1 && xc_ == mNextFreeAddr) {
    mNextFreeAddr = 0;
    xc_ = 0;
  }
}

#if RSTL_KYOTO_CIRCULAR_BUFFER_ALLOCATED_AMOUNT
int CCircularBuffer::GetAllocatedAmount() const {
  const int previousStart = x14_;
  int amount = mNextFreeAddr - xc_;
  if (previousStart != -1) {
    amount += mMemorySize - previousStart;
  }

  return amount;
}
#endif
