#ifndef _CVISIBILITYANIMATIONDATA
#define _CVISIBILITYANIMATIONDATA

#include "rstl/types.h"

class CVisibilityAnimationDataReader {
public:
  float GetTime() const;                // Guessed name
  const uint* GetBitFlagsData() const;  // Guessed name
  uint GetVisibilityBitFlags(uchar arrayIndex) const;
  uchar GetBitFlagsArraySize() const { return mVisiblityBitFlagsArraySize; } // Guessed name

private:
  uchar x0_[4];
  uchar mVisiblityBitFlagsArraySize;
};

#endif // _CVISIBILITYANIMATIONDATA
