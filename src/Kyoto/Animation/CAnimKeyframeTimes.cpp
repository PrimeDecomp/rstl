#include "Kyoto/Animation/CAnimKeyframeTimes.hpp"

#include "Kyoto/Alloc/CMemory.hpp"

CAnimKeyframeTimes::CAnimKeyframeTimes(const float* times, ushort numTimes)
: mOwnsData(false), mNumTimes(numTimes), mTimes(times) {}

CAnimKeyframeTimes::~CAnimKeyframeTimes() {
  if (mOwnsData) {
    delete[] mTimes;
  }
}

