#include "Kyoto/Streams/CBitStreamWriter.hpp"
#include "rstl/RstlVersions.h"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Streams/COutputStream.hpp"
#include "Kyoto/Streams/StreamSupport.hpp"

CBitStreamWriter::CBitStreamWriter(COutputStream& stream)
: mStream(stream), mShiftRegister(0), mShiftRegisterOffset(32) {}

CBitStreamWriter::~CBitStreamWriter() { Flush(); }

uint CBitStreamWriter::GetWrittenBits() const {
  return mStream.GetWrittenBytes() * 8 + 32 - mShiftRegisterOffset;
}

void CBitStreamWriter::Flush() {
  if (mShiftRegisterOffset < 32) {
    mShiftRegister = BE_SWAP(mShiftRegister);
    mStream.Put(&mShiftRegister, min_containing_bytes(32 - mShiftRegisterOffset));
    mShiftRegister = 0;
    mShiftRegisterOffset = 32;
  }
}

void CBitStreamWriter::WriteBits(const uint val, const uint bitCount) {

  const uint registerOffset = mShiftRegisterOffset;
  if (registerOffset >= bitCount) {
    const uint off = registerOffset - bitCount;
    mShiftRegister |= (val & (bitCount != 32 ? (1 << bitCount) - 1 : 0xffffffff)) << off;
    mShiftRegisterOffset -= bitCount;
  } else {
    const uint shiftAmt = bitCount - registerOffset;
    const uint shiftA = val >> shiftAmt;

    mShiftRegister |= shiftA & (registerOffset != 0x20 ? (1 << registerOffset) - 1 : 0xffffffff);
    mShiftRegisterOffset = 0;
    Flush();
    const uint shift = 32 - shiftAmt;
    mShiftRegister = (val & (shiftAmt != 32 ? (1 << shiftAmt) - 1 : 0xffffffff)) << shift;
    mShiftRegisterOffset -= shiftAmt;
  }
}

void CBitStreamWriter::FlushAll() {
  Flush();
  mStream.Flush();
}

COutputStream& CBitStreamWriter::GetOutputStream() {
  Flush();
  return mStream;
}

#if RSTL_KYOTO_PROTO_STREAM_HELPERS
void CBitStreamWriter::Write64BitsLowWordFirst(u64 value) {
  WriteBits(static_cast< uint >(value), 32);
  WriteBits(static_cast< uint >(value >> 32), 32);
}
#endif
