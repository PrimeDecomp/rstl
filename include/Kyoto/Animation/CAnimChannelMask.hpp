#ifndef _CANIMCHANNELMASK
#define _CANIMCHANNELMASK

#include "rstl/static_assert.hpp"
#include "rstl/types.h"

// Guessed name
class CAnimChannelMask {
public:
  CAnimChannelMask(const void* data, uint size);
  ~CAnimChannelMask();

  uchar GetNumChannels() const { return mNumChannels; } // Guessed name
  const uchar* GetMasks() const { return mMasks; }      // Guessed name

  static const uchar skRotation;    // Guessed name
  static const uchar skTranslation; // Guessed name
  static const uchar skScale;       // Guessed name

private:
  bool mOwnsData;      // Guessed name
  uchar mNumChannels;  // Guessed name
  const uchar* mMasks; // Guessed name
};
CHECK_SIZEOF(CAnimChannelMask, 0x8)

#endif // _CANIMCHANNELMASK
