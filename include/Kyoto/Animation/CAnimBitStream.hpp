#ifndef _CANIMBITSTREAM
#define _CANIMBITSTREAM

#include "rstl/static_assert.hpp"
#include "rstl/types.h"

// Guessed name
class CAnimBitStream {
public:
  CAnimBitStream(const void* data, uint size);
  ~CAnimBitStream();

  uint GetNumChannels() const { return mNumChannels; }
  int ReadNextChannelValue(); // Guessed name
  void ResetReadPosition();   // Guessed name

private:
  uint GetChannelBitCountsByteSize() const; // Guessed name
  uint ReadBits(uchar bits);      // Guessed name
  int ReadSignedBits(uchar bits); // Guessed name
  uint ReadBit();                 // Guessed name

  uint mNumChannels;              // Guessed name
  uint mWordCount;                // Guessed name
  uint x8_;
  const uchar* mChannelBitCounts; // Guessed name
  const uint* mWords;             // Guessed name
  uint mCurrentChannel;           // Guessed name
  uchar mBitOffset;               // Guessed name
  uint mWordOffset;               // Guessed name
  bool mOwnsData;                 // Guessed name
};
CHECK_SIZEOF(CAnimBitStream, 0x24)

#endif // _CANIMBITSTREAM
