#include "Kyoto/Streams/CBitStreamReader.hpp"
#include "rstl/RstlVersions.h"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/StreamSupport.hpp"

CBitStreamReader::CBitStreamReader(CInputStream& stream)
: mStream(stream), mBitWord(0), mBitOffset(0) {}

CBitStreamReader::~CBitStreamReader() {}

static inline uint GetBitMask(const uint& bits) {
  return bits != 32 ? (1 << bits) - 1 : 0xffffffff;
}

uint CBitStreamReader::ReadBits(uint bitCount) {
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
  mStream.Get(&mBitWord, len);

  {
    uint bwShift = 32 - shiftAmt;
    uint mask = GetBitMask(shiftAmt);
    ret |= ((mask & (mBitWord >> bwShift)) << mBitOffset);
  }

  mBitOffset = len * 8;
  mBitOffset -= shiftAmt;
  mBitWord <<= shiftAmt;
  return ret;
}

void CBitStreamReader::Flush() { mBitOffset = 0; }

CInputStream& CBitStreamReader::GetInputStream() {
  Flush();
  return mStream;
}

#if RSTL_KYOTO_PROTO_STREAM_HELPERS
u64 CBitStreamReader::Read64BitsLowWordFirst() {
  const uint low = ReadBits(32);
  const uint high = ReadBits(32);
  return (static_cast< u64 >(high) << 32) | low;
}
#endif
