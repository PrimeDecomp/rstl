#include "Kyoto/Animation/CCompressedAnimSource.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Animation/CAnimChannelMask.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

const uint CCompressedAnimSourceHeader::skHasOffsets = 2;
const uint CCompressedAnimSourceHeader::skHasScales = 4;

CCompressedAnimSourceHeader::CCompressedAnimSourceHeader(CInputStream& in)
: mVersion(in.Get< ushort >())
, mFlags(in.Get< uint >())
, x8_(in.ReadFloat())
, mOffsetScale(in.ReadFloat())
, mScaleScale(in.ReadFloat())
, mAverageVelocity(in)
, mDuration(in.ReadFloat())
, mNumKeyframes(in.Get< ushort >())
, mBufferSize(in.Get< uint >())
, x2c_(-1) {
  if (mVersion != 0) {
    x2c_ = in.Get< signed char >();
  }
}

CCompressedAnimSource::CCompressedAnimSource(CInputStream& in)
: mHeader(in)
, x30_(in.Get< uint >())
, x34_(in.Get< uint >())
, x38_(in.Get< uint >())
, x3c_(in.Get< uint >())
, x40_(mHeader.GetVersion() > 1 ? in.Get< uint >() : 0)
, mBuffer(ReadOwnedBuffer(in, mHeader.GetBufferSize()))
, x48_(nullptr)
, x4c_(nullptr)
, x50_(nullptr)
, x54_(nullptr) {
  RebaseSectionPointers();
  CalcAverageVelocity();
}

uchar* CCompressedAnimSource::ReadOwnedBuffer(CInputStream& in, uint size) {
  uchar* buffer = rs_new_line(165) uchar[size];
  in.Get(buffer, size);
  return buffer;
}

void CCompressedAnimSource::RebaseSectionPointers() {
  if (mBuffer.get() == nullptr) {
    return;
  }
  x48_ = mBuffer.get();
  uint offset = x30_;
  x4c_ = mBuffer.get() + offset;
  offset += x34_;
  x50_ = mBuffer.get() + offset;
  offset += x38_;
  x54_ = mBuffer.get() + offset;
  offset += x3c_;
  x58_ = mBuffer.get() + offset;
}

void CCompressedAnimSource::CalcAverageVelocity() {}

bool CCompressedAnimSource::HasScaleData() const {
  CAnimChannelMask channelMask(x4c_, x34_);
  const uchar* masks = channelMask.GetMasks();
  bool hasScale = false;
  const uint numChannels = channelMask.GetNumChannels();
  for (uchar i = 0; i < numChannels; ++i) {
    if (masks[i] & 4) {
      hasScale = true;
    }
  }
  return hasScale;
}

float CCompressedAnimSource::GetAverageVelocity() const {
  return mHeader.GetAverageVelocity().Magnitude();
}

CVector3f CCompressedAnimSource::GetRootOffset() const {
  return mHeader.GetDuration() * mHeader.GetAverageVelocity();
}

