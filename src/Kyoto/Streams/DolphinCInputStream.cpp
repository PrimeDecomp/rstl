#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/RstlVersions.h"

#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "rstl/math.hpp"
#include "string.h"

CInputStream::CInputStream(const void* ptr, unsigned long len)
: mpBufferStart(const_cast< uchar* >(reinterpret_cast< const uchar* >(ptr)))
, mBufferSize(len)
, mOwner(false) {
  mpBufferAt.p8 = mpBufferStart;
}

CInputStream::CInputStream(const void* ptr, unsigned long len, bool owned)
: mpBufferStart(const_cast< uchar* >(reinterpret_cast< const uchar* >(ptr)))
, mBufferSize(len)
, mOwner(owned) {
  mpBufferAt.p8 = mpBufferStart;
}

CInputStream::CInputStream(const SBufferAndSize& buffer, bool owned)
: mpBufferStart(const_cast< uchar* >(reinterpret_cast< const uchar* >(buffer.mBuffer)))
, mBufferSize(buffer.mSize)
, mOwner(owned) {
  mpBufferAt.p8 = mpBufferStart;
}

CInputStream::~CInputStream() {
  RS_VERIFY_THROW(49, GetReadPosition() <= mBufferSize, false,
                  CBasics::Stringize("CInputStream: Buffer of size %d was overrun by %d",
                                     mBufferSize, GetReadPosition()));
  if (mOwner) {
    delete[] mpBufferStart;
  }
}

void CInputStream::Get(void* dest, unsigned long bufferSize) {
  RS_VERIFY_THROW(62, GetReadPosition() + bufferSize <= mBufferSize, false,
                  CBasics::Stringize("CInputStream: Get trying to overrun buffer of size %d by %d",
                                     mBufferSize, GetReadPosition()));
  memcpy(dest, mpBufferAt.p8, bufferSize);
  mpBufferAt.p8 += bufferSize;
}

const void* CInputStream::Get(unsigned long bufferSize) {
  RS_VERIFY_THROW(71, GetReadPosition() + bufferSize <= mBufferSize, false,
                  CBasics::Stringize("CInputStream: Get trying to overrun buffer of size %d by %d",
                                     mBufferSize, GetReadPosition()));
  const void* result = mpBufferAt.p8;
  mpBufferAt.p8 += bufferSize;
  return result;
}

size_t CInputStream::ReadBytes(void* dest, size_t len) {
  size_t count = mBufferSize - (mpBufferAt.p8 - mpBufferStart);
  if (len < count) {
    count = len;
  }
  count = rstl::max_val(size_t(0), count);
  if (count != 0) {
    if (dest != nullptr) {
      memcpy(dest, mpBufferAt.p8, count);
    }
    mpBufferAt.p8 += count;
  }
  return count;
}

float CInputStream::ReadFloat() {
  static float f;
  *reinterpret_cast< uint* >(&f) = ReadInt32();
  return f;
}

rstl::auto_ptr< uchar > CInputStream::ReleaseBuffer() {
  RS_VERIFY_THROW(111, mOwner, false, "Trying to take ownership of a stream not owned");
  RS_VERIFY_THROW(112, mpBufferStart == mpBufferAt.p8, false,
                  "Trying to take ownership of a stream that's already been read");
  mOwner = false;
  return rstl::auto_ptr< uchar >(mpBufferStart);
}
