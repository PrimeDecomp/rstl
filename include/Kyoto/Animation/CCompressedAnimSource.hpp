#ifndef _CCOMPRESSEDANIMSOURCE
#define _CCOMPRESSEDANIMSOURCE

#include "rstl/static_assert.hpp"
#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "rstl/single_ptr.hpp"

class CInputStream;

// Guessed name
class CCompressedAnimSourceHeader {
public:
  explicit CCompressedAnimSourceHeader(CInputStream& in);

  ushort GetVersion() const { return mVersion; }                           // Guessed name
  uint GetBufferSize() const { return mBufferSize; }                       // Guessed name
  const CVector3f& GetAverageVelocity() const { return mAverageVelocity; } // Guessed name
  float GetDuration() const { return mDuration; }                          // Guessed name
  uint GetFlags() const { return mFlags; }                                // Guessed name
  float GetOffsetScale() const { return mOffsetScale; }                   // Guessed name
  float GetScaleScale() const { return mScaleScale; }                     // Guessed name
  const ushort GetNumKeyframes() const { return mNumKeyframes; }                // Guessed name

  static const uint skHasOffsets; // Guessed name
  static const uint skHasScales;  // Guessed name

private:
  // Guessed name
  ushort mVersion;
  uint mFlags; // Guessed name
  float x8_;
  float mOffsetScale; // Guessed name
  float mScaleScale; // Guessed name
  CVector3f mAverageVelocity;
  float mDuration;
  ushort mNumKeyframes; // Guessed name
  uint mBufferSize;
  signed char x2c_;
};
CHECK_SIZEOF(CCompressedAnimSourceHeader, 0x30)

class CCompressedAnimSource {
public:
  explicit CCompressedAnimSource(CInputStream& in);

  // Guessed name
  CVector3f GetRootOffset() const;
  // Guessed name
  float GetAverageVelocity() const;
  bool HasScaleData() const; // Guessed name
  CCharAnimTime GetAnimationDuration() const { // Guessed name
    return CCharAnimTime(mHeader.GetDuration());
  }

  // Guessed names
  const CCompressedAnimSourceHeader& GetHeader() const { return mHeader; }
  const float* GetKeyframeTimes() const { return reinterpret_cast< const float* >(x48_); }
  const void* GetChannelMaskData() const { return x4c_; }
  uint GetChannelMaskSize() const { return x34_; }
  short* GetBaseFrameData() const { return reinterpret_cast< short* >(x50_); }
  const void* GetBitStreamData() const { return x54_; }
  uint GetBitStreamSize() const { return x3c_; }
  const void* GetVisibilityData() const { return x58_; }
  uint GetVisibilitySize() const { return x40_; }

private:
  // Guessed name
  static uchar* ReadOwnedBuffer(CInputStream& in, uint size);
  // Guessed name
  void RebaseSectionPointers();
  // Guessed name
  void CalcAverageVelocity();

  CCompressedAnimSourceHeader mHeader;
  uint x30_;
  uint x34_;
  uint x38_;
  uint x3c_;
  uint x40_;
  rstl::single_ptr< uchar > mBuffer; // Guessed name
  uchar* x48_;
  uchar* x4c_;
  uchar* x50_;
  uchar* x54_;
  uchar* x58_;
};
CHECK_SIZEOF(CCompressedAnimSource, 0x5c)

#endif // _CCOMPRESSEDANIMSOURCE
