#include "Kyoto/Animation/CQuantizedAnimFrame.hpp"

#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Animation/CAnimBitStream.hpp"
#include "Kyoto/Animation/CAnimChannelMask.hpp"

#include "string.h"

const uchar CQuantizedAnimFrame::skRotationChannels = 4;
const uchar CQuantizedAnimFrame::skTranslationChannels = 3;
const uchar CQuantizedAnimFrame::skScaleChannels = 3;

CQuantizedAnimFrame::CQuantizedAnimFrame(short* data, uint numDataChannels)
: mOwnsData(false), mNumDataChannels(numDataChannels), mData(data) {}

CQuantizedAnimFrame::~CQuantizedAnimFrame() {
  if (mOwnsData) {
    delete[] mData;
  }
}

uint CQuantizedAnimFrame::GetPackedDataByteSize() const { return mNumDataChannels * sizeof(short); }

void CQuantizedAnimFrame::GetPackedData(const CAnimChannelMask& channelMask,
                                      CAnimBitStream& bitPacker) {
  RS_VERIFY_THROW(76, bitPacker.GetNumChannels() == mNumDataChannels, 0,
                  "CQuantizedAnimFrame::GetPackedData -- Number of data channels between packed data "
                  "and quantized data buffer do not match!");

  const uint numChannels = channelMask.GetNumChannels();
  const uchar* masks = channelMask.GetMasks();
  uint dataChannel = 0;
  for (uchar i = 0; i < numChannels; ++i) {
    const uchar mask = masks[i];
    if (mask & CAnimChannelMask::skRotation) {
      mData[dataChannel] = short(bitPacker.ReadNextChannelValue());
      mData[dataChannel + 1] += short(bitPacker.ReadNextChannelValue());
      mData[dataChannel + 2] += short(bitPacker.ReadNextChannelValue());
      mData[dataChannel + 3] += short(bitPacker.ReadNextChannelValue());
      dataChannel += 4;
    }
    if (mask & CAnimChannelMask::skTranslation) {
      mData[dataChannel] += short(bitPacker.ReadNextChannelValue());
      mData[dataChannel + 1] += short(bitPacker.ReadNextChannelValue());
      mData[dataChannel + 2] += short(bitPacker.ReadNextChannelValue());
      dataChannel += 3;
    }
    if (mask & CAnimChannelMask::skScale) {
      mData[dataChannel] += short(bitPacker.ReadNextChannelValue());
      mData[dataChannel + 1] += short(bitPacker.ReadNextChannelValue());
      mData[dataChannel + 2] += short(bitPacker.ReadNextChannelValue());
      dataChannel += 3;
    }
  }
}

void CQuantizedAnimFrame::CopyPackedData(CQuantizedAnimFrame& out) const {
  memcpy(out.mData, mData, GetPackedDataByteSize());
}

