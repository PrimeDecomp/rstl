#include "Kyoto/Animation/CAnimBitStream.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"

#include "rstl/math.hpp"

CAnimBitStream::CAnimBitStream(const void* data, uint size)
: mNumChannels(0)
, mWordCount(1)
, x8_(0)
, mChannelBitCounts(nullptr)
, mWords(nullptr)
, mCurrentChannel(0)
, mBitOffset(0)
, mWordOffset(0)
, mOwnsData(false) {
  CMemoryInStream in(data, size, CMemoryInStream::kOS_NotOwned);
  mNumChannels = in.ReadInt32();
  mWordCount = in.ReadInt32();
  x8_ = in.ReadInt32();
  const uchar* channelBitCounts = static_cast< const uchar* >(data) + in.GetReadPosition();
  mChannelBitCounts = channelBitCounts;
  mWords = reinterpret_cast< const uint* >(channelBitCounts + GetChannelBitCountsByteSize());
}

CAnimBitStream::~CAnimBitStream() {
  if (mOwnsData) {
    delete[] mChannelBitCounts;
    delete[] mWords;
  }
}

int CAnimBitStream::ReadNextChannelValue() {
  if (mWordOffset < mWordCount) {
    // The native reader skips the leading bit in each channel cycle.
    if (mCurrentChannel == 0) {
      ReadBit();
    }

    const uchar bits = mChannelBitCounts[mCurrentChannel];
    ++mCurrentChannel;
    if (mCurrentChannel >= mNumChannels) {
      mCurrentChannel = 0;
    }
    return ReadSignedBits(bits);
  }
  return 0;
}

void CAnimBitStream::ResetReadPosition() {
  mCurrentChannel = 0;
  mWordOffset = 0;
  mBitOffset = 0;
}

uint CAnimBitStream::ReadBits(uchar bits) {
  uint value = 0;
  uint shift = 0;
  while (bits != 0) {
    const uint count = rstl::min_val< uchar >(32 - mBitOffset, bits);
    value |= ((mWords[mWordOffset] >> mBitOffset) << (32 - count)) >> (32 - count - shift);
    mBitOffset = mBitOffset + count;
    shift += count;
    bits -= count;
    if (mBitOffset == 32) {
      mBitOffset = 0;
      ++mWordOffset;
    }
  }
  return value;
}

int CAnimBitStream::ReadSignedBits(uchar bits) {
  if (bits == 0) {
    return 0;
  }
  uint value = ReadBits(bits);
  if (value & (1 << (bits - 1))) {
    value |= ~0u << bits;
  }
  return value;
}

uint CAnimBitStream::ReadBit() { return ReadBits(1); }

uint CAnimBitStream::GetChannelBitCountsByteSize() const {
  return mNumChannels * sizeof(uchar);
}

