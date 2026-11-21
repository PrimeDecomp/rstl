#include "Kyoto/Animation/CAnimChannelMask.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"

const uchar CAnimChannelMask::skRotation = 1;
const uchar CAnimChannelMask::skTranslation = 2;
const uchar CAnimChannelMask::skScale = 4;

CAnimChannelMask::CAnimChannelMask(const void* data, uint size)
: mOwnsData(false), mNumChannels(0), mMasks(nullptr) {
  CMemoryInStream in(data, size, CMemoryInStream::kOS_NotOwned);
  mNumChannels = in.ReadUint8();
  mMasks = static_cast< const uchar* >(data) + in.GetReadPosition();
}

CAnimChannelMask::~CAnimChannelMask() {
  if (mOwnsData) {
    delete[] mMasks;
  }
}

