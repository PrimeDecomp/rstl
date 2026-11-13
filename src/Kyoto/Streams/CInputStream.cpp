#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Basics/CBasics.hpp"

#include "string.h"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Streams/StreamSupport.hpp"

#if RSTL_ECHOES_DEMO
// Guessed name: the demo discs have an empty function taking an exception name.
void ThrowException(int, const char*);
#endif

CInputStream::CInputStream(int len)
: mBlockOffset(0)
, mBlockLen(0)
, mLen(len)
, mPtr(rs_new uchar[len])
, mOwned(true)
, mReadPosition(0)
, mBitWord(0)
, mBitOffset(0) {}

CInputStream::CInputStream(const void* ptr, int len, bool owned)
: mBlockOffset(0)
, mBlockLen(len)
, mLen(len)
, mPtr(const_cast< uchar* >(reinterpret_cast< const uchar* >(ptr)))
, mOwned(owned)
, mReadPosition(0)
, mBitWord(0)
, mBitOffset(0) {}

CInputStream::~CInputStream() {
  if (mOwned) {
    delete[] mPtr;
  }
}

bool CInputStream::InternalReadNext() {
  mBlockLen = Read(mPtr, mLen);
  mBlockOffset = 0;
  return mBlockLen != 0;
}

#if RSTL_ECHOES_DEMO
void CInputStream::GrabAnotherBlock() {
  if (!InternalReadNext()) {
    ThrowException(0, "kException_EndOfStream");
  }
}
#else
bool CInputStream::GrabAnotherBlock() { return InternalReadNext(); }
#endif

void CInputStream::Get(void* dest, unsigned long len) {
  uint remain = len;
  uint readCount = 0;
  mBitOffset = 0;

  while (remain != 0) {
    uint blockLen = mBlockLen - mBlockOffset;
    blockLen = remain < blockLen ? remain : blockLen;

    if (blockLen != 0) {
      memcpy(reinterpret_cast< uchar* >(dest) + readCount, mPtr + mBlockOffset, blockLen);
      remain -= blockLen;
      readCount += blockLen;
      mBlockOffset += blockLen;
    } else if (remain > 256) {
      uint readLen = Read(reinterpret_cast< uchar* >(dest) + readCount, remain);
      remain -= readLen;
      readCount += readLen;
    } else {
      GrabAnotherBlock();
    }
  }

  mReadPosition += readCount;
}

size_t CInputStream::ReadBytes(void* dest, size_t len) {
  if (len == 0) {
    return 0;
  }

  if (mBlockOffset == mBlockLen) {
    GrabAnotherBlock();
  }

  uint curLen = len;
  uint curReadLen = 0;

  while (curReadLen < len) {
    uint remain = mBlockLen - mBlockOffset;
    if (remain == 0) {
      if (InternalReadNext()) {
        continue;
      } else {
#if NONMATCHING
        mReadPosition += curReadLen;
#endif
        return curReadLen;
      }
    }

    uint sz = curLen < remain ? curLen : remain;
#if RSTL_ECHOES_DEMO
    if (dest != nullptr) {
      memcpy(reinterpret_cast< uchar* >(dest) + curReadLen, mPtr + mBlockOffset, sz);
    }
#else
    memcpy(reinterpret_cast< uchar* >(dest) + curReadLen, mPtr + mBlockOffset, sz);
#endif
    curReadLen += sz;
    curLen -= sz;
    mBlockOffset += sz;
  }

  mReadPosition += curReadLen;
  return curReadLen;
}

static inline uint GetBitMask(const uint& bits) {
  return bits != 32 ? (1 << bits) - 1 : 0xffffffff;
}

uint CInputStream::ReadBits(uint bitCount) {
#if NONMATCHING
  uint result = 0;
  for (uint i = 0; i < bitCount; ++i) {
    if (mBitOffset == 0) {
      uchar byte;
      Get(&byte, 1);
      mBitWord = uint(byte) << 24;
      mBitOffset = 8;
    }
    result = (result << 1) | (mBitWord >> 31);
    mBitWord <<= 1;
    --mBitOffset;
  }
  return result;
#else
  if (mBitOffset >= bitCount) {
    uint bwShift = 32 - bitCount;
    uint mask = GetBitMask(bitCount);
    uint ret = mask & (mBitWord >> bwShift);

    mBitOffset -= bitCount;
    mBitWord <<= bitCount;
    return ret;
  }

  uint shiftAmt = bitCount - mBitOffset;

  uint ret = 0;
  {
    uint bwShift = 32 - mBitOffset;
    uint mask = GetBitMask(mBitOffset);
    ret = (mask & (mBitWord >> bwShift)) << shiftAmt;
  }

  uint len = min_containing_bytes(shiftAmt);
  mBitOffset = 0;
  Get(&mBitWord, len);

  {
    uint bwShift = 32 - shiftAmt;
    uint mask = GetBitMask(shiftAmt);
    ret |= ((mask & (mBitWord >> bwShift)) << mBitOffset);
  }

  mBitOffset = len * 8;
  mBitOffset -= shiftAmt;
  mBitWord <<= shiftAmt;
  return ret;
#endif
}

// The Prime build reads into function-local statics; the demos use plain locals.
#if RSTL_ECHOES_DEMO
#define RSTL_INPUT_STREAM_TEMP
#else
#define RSTL_INPUT_STREAM_TEMP static
#endif

char CInputStream::ReadChar() {
  RSTL_INPUT_STREAM_TEMP char c;
  Get(&c, sizeof(char));
  return c;
}

bool CInputStream::ReadBool() { return static_cast< uchar >(ReadChar()) != 0; }

ushort CInputStream::ReadShort() {
  RSTL_INPUT_STREAM_TEMP ushort s;
  Get(&s, sizeof(ushort));
  return CBasics::SwapBytes(s);
}

uint CInputStream::ReadLong() {
  RSTL_INPUT_STREAM_TEMP uint l;
  Get(&l, sizeof(uint));
  return CBasics::SwapBytes(l);
}

u64 CInputStream::ReadLongLong() {
  RSTL_INPUT_STREAM_TEMP u64 ll;
  Get(&ll, sizeof(u64));
  return CBasics::SwapBytes(ll);
}

float CInputStream::ReadFloat() {
  RSTL_INPUT_STREAM_TEMP float f;
  Get(&f, sizeof(float));
  return CBasics::SwapBytes(f);
}

#if !RSTL_ECHOES_DEMO
double CInputStream::ReadDouble() {
  RSTL_INPUT_STREAM_TEMP double d;
  Get(&d, sizeof(double));
  return d;
}
#endif

#if RSTL_ECHOES_DEMO
void ThrowException(int, const char*) {}

// Guessed name; stripped from the link, but its string literal stays in the pool.
static void ThrowNoException() { ThrowException(0, "(no exceptions)"); }
#endif
