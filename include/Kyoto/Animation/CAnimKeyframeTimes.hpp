#ifndef _CANIMKEYFRAMETIMES
#define _CANIMKEYFRAMETIMES

#include "rstl/static_assert.hpp"
#include "rstl/types.h"

// Guessed name
class CAnimKeyframeTimes {
public:
  CAnimKeyframeTimes(const float* times, ushort numTimes);
  ~CAnimKeyframeTimes();

  ushort GetNumTimes() const { return mNumTimes; } // Guessed name
  const float* GetTimes() const { return mTimes; } // Guessed name

private:
  bool mOwnsData;      // Guessed name
  ushort mNumTimes;    // Guessed name
  const float* mTimes; // Guessed name
};
CHECK_SIZEOF(CAnimKeyframeTimes, 0x8)

#endif // _CANIMKEYFRAMETIMES
