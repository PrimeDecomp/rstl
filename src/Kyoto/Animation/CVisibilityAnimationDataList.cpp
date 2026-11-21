#include "Kyoto/Animation/CVisibilityAnimationDataList.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Animation/CVisibilityAnimationData.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"

CVisibilityAnimationDataList::CVisibilityAnimationDataList(CInputStream& in)
: mNumRecords(in.Get< ushort >()), mRecordSize(in.Get< uchar >()), mOwnsData(true) {
  mData = ReadOwnedBuffer(in, mNumRecords * mRecordSize);
}

CVisibilityAnimationDataList::CVisibilityAnimationDataList(const void* data, uint size)
: mNumRecords(0), mRecordSize(0), mOwnsData(false), mData(nullptr) {
  if (size != 0) {
    CMemoryInStream in(data, size, CMemoryInStream::kOS_NotOwned);
    mNumRecords = in.Get< ushort >();
    mRecordSize = in.Get< uchar >();
    if (mNumRecords != 0 && mRecordSize != 0) {
      mData = const_cast< uchar* >(static_cast< const uchar* >(data)) + in.GetReadPosition();
    }
  }
}

CVisibilityAnimationDataList::~CVisibilityAnimationDataList() {
  if (mOwnsData) {
    delete[] mData;
  }
}

const CVisibilityAnimationDataReader*
CVisibilityAnimationDataList::GetRecordByIndex(uint index) const {
  if (index < mNumRecords) {
    return reinterpret_cast< const CVisibilityAnimationDataReader* >(mData + index * mRecordSize);
  }
  return nullptr;
}

int CVisibilityAnimationDataList::FindRecordIndexAtTime(const CCharAnimTime& time,
                                                        int startIndex) const {
  if (mNumRecords != 0 && static_cast< ushort >(startIndex) < mNumRecords) {
    uint index = startIndex < 0 ? 0 : startIndex;
    float recordTime = GetRecordByIndex(index)->GetTime();
    while ((time.GetSeconds() > recordTime || close_enough(time.GetSeconds(), recordTime)) &&
           index < mNumRecords) {
      ++index;
      if (index < mNumRecords) {
        recordTime = GetRecordByIndex(index)->GetTime();
      }
    }
    return index - 1;
  }
  return -1;
}

uchar* CVisibilityAnimationDataList::ReadOwnedBuffer(CInputStream& in, uint size) {
  if (size != 0) {
    uchar* buffer = rs_new_line(143) uchar[size];
    in.Get(buffer, size);
    return buffer;
  }
  return nullptr;
}

