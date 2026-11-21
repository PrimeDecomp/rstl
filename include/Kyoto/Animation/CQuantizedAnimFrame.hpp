#ifndef _CQUANTIZEDANIMFRAME
#define _CQUANTIZEDANIMFRAME

#include "rstl/static_assert.hpp"
#include "rstl/types.h"

class CAnimBitStream;
class CAnimChannelMask;

class CQuantizedAnimFrame {
public:
  CQuantizedAnimFrame(short* data, uint numDataChannels);
  ~CQuantizedAnimFrame();

  uint GetPackedDataByteSize() const; // Guessed name
  void GetPackedData(const CAnimChannelMask& channelMask, CAnimBitStream& bitPacker);
  void CopyPackedData(CQuantizedAnimFrame& out) const; // Guessed name
  const short* GetData() const { return mData; }       // Guessed name

  static const uchar skRotationChannels;    // Guessed name
  static const uchar skTranslationChannels; // Guessed name
  static const uchar skScaleChannels;       // Guessed name

private:
  bool mOwnsData; // Guessed name
  uint mNumDataChannels;
  short* mData; // Guessed name
};
CHECK_SIZEOF(CQuantizedAnimFrame, 0xc)

#endif // _CQUANTIZEDANIMFRAME
