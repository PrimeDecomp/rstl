#ifndef _CFBSTREAMEDANIMREADER
#define _CFBSTREAMEDANIMREADER
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CAllFormatsAnimSource.hpp"

#include "Kyoto/Animation/CAnimSourceReaderBase.hpp"
#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"
#include "Kyoto/Animation/CFBStreamedCompression.hpp"

#include "Kyoto/Animation/CTimeRemainderAndFraction.hpp"
#include "Kyoto/Basics/CCast.hpp"
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
#include "Kyoto/Animation/TSubAnimTypeToken.hpp"
#endif

#include "rstl/math.hpp"

class CMemoryInputToBitLevelLoader;
class CSegStatement;
class CFBKeyFrameReductionPerChannel_HeaderForAll;
template < typename T >
class CBitLevelLoader;

template < typename T >
class TAnimSourceInfo : public IAnimSourceInfo {
public:
#if !RSTL_PRIME1
  explicit TAnimSourceInfo(const TSubAnimTypeToken< T >& source) : mSource(source) {}

  // IAnimSourceInfo
  CCharAnimTime GetAnimationDuration() const override { return mSource->GetAnimationDuration(); }
  bool HasScaleData() const override { return mSource->HasScaleData(); }
#endif

  ~TAnimSourceInfo() override {}
#if RSTL_PRIME1
  TAnimSourceInfo(const TSubAnimTypeToken< T >& source) : mSource(source) {}
  bool HasPOIData() const override { return mSource->HasPOIData(); }
  CCharAnimTime GetAnimationDuration() const override { return mSource->GetAnimationDuration(); }
  const rstl::vector< CSoundPOINode >& GetSoundPOIStream() const override {
    return mSource->GetSoundPOIStream();
  }
  const rstl::vector< CParticlePOINode >& GetParticlePOIStream() const override {
    return mSource->GetParticlePOIStream();
  }
  const rstl::vector< CInt32POINode >& GetInt32POIStream() const override {
    return mSource->GetInt32POIStream();
  }
  const rstl::vector< CBoolPOINode >& GetBoolPOIStream() const override {
    return mSource->GetBoolPOIStream();
  }
#endif

private:
  TSubAnimTypeToken< T > mSource;
};

class CFBStreamedAnimReaderTotals {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CFBStreamedAnimReaderTotals(const CFBStreamedCompression& source);
#else
  explicit CFBStreamedAnimReaderTotals(const CFBStreamedCompression& source);
#endif

  ~CFBStreamedAnimReaderTotals();
  void CalculateDown();
  void SetToReadStart(const CFBStreamedCompression& source);
  void IncrementInto(CBitLevelLoader< CMemoryInputToBitLevelLoader >& loader,
                     const CFBStreamedCompression& source, CFBStreamedAnimReaderTotals& out);
  uint GetFrameNumber() const { return mCurKey; }
  uint NumEntries() const { return mBoneChanCount; }
  uint GetSegId(uint index) const { return mSegIds[index]; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  bool HasOffset(uint index) const { return mHasTrans[index]; }
#else
  const bool* GetRotationFlags() const { return mHasRotation; } // Guessed name.

  const bool* GetOffsetFlags() const { return mHasOffset; } // Guessed name.

  const bool* GetScaleFlags() const { return mHasScale; } // Guessed name.

  bool HasRotation(uint index) const { return mHasRotation[index]; }
  bool HasOffset(uint index) const { return mHasOffset[index]; }
  bool HasScale(uint index) const { return mHasScale[index]; }
  const bool HasOffsetData() const { return mHasOffsetData; }
  const bool HasScaleData() const { return mHasScaleData; }
#endif

  bool AmCalculatedDown() const { return mCalculated; }
  const CQuaternion& GetQuat(uint index) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return *reinterpret_cast< const CQuaternion* >(mComputedFloats + index * 8);
#else
    return *reinterpret_cast< const CQuaternion* >(mComputedFloats + index * mValuesPerChannel);
#endif
  }
  const CVector3f& GetVector(uint index) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    uint offset = index * 8 + 4;
    return *reinterpret_cast< const CVector3f* >(mComputedFloats + offset);
#else
    return *reinterpret_cast< const CVector3f* >(mComputedFloats +
                                                 (index * mValuesPerChannel + kRotationValueCount));
#endif
  }
#if !RSTL_PRIME1
  const CVector3f& GetScale(uint index) const {
    uint offset = mHasOffsetData ? kRotationValueCount + kOffsetValueCount : kRotationValueCount;
    return *reinterpret_cast< const CVector3f* >(mComputedFloats +
                                                 (offset + index * mValuesPerChannel));
  }
#endif

private:
#if !RSTL_PRIME1
  static const uchar kRotationValueCount; // Guessed name.
  static const uchar kOffsetValueCount;   // Guessed name.
#endif

  void Allocate(uint channelCount);
#if !RSTL_PRIME1
  uchar GetValuesPerChannel() const; // Guessed name.
#endif

  uchar* mBuffer;
#if !RSTL_PRIME1
  uint mBufferSize;
#endif

  short* mCumulativeInts;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  bool* mHasTrans;
#else
  bool* mHasRotation;
  bool* mHasOffset;
  bool* mHasScale;
#endif

  short* mSegIds;
  float* mComputedFloats;
  uint mRotDiv;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  float mTransMult;
#else
  float mOffsetMult;
  float mScaleMult;
#endif

  uint mCurKey;
  bool mCalculated;
  uint mBoneChanCount;
#if !RSTL_PRIME1
  bool mHasOffsetData;
  bool mHasScaleData;
  uchar mValuesPerChannel;
#endif
};
CHECK_SIZEOF(CFBStreamedAnimReaderTotals, RSTL_PRIME1 ? 0x28 : 0x3c)

class CFBFullBodyAspectsForStream {
public:
  CFBFullBodyAspectsForStream(const CFBKeyFrameReductionPerChannel_HeaderForAll& header,
                              const CTimeRemainderAndFraction& time, const CCharAnimTime& duration);
  void SetTime(const CTimeRemainderAndFraction& time);
  uint GetPrevIndex() const { return mPriorKey; }
  float GetT() const { return mT; }

private:
  const CFBKeyFrameReductionPerChannel_HeaderForAll* mHeader;
  uint mPriorFrame;
  uint mNextFrame;
  uint mLastFrame;
  float mSampleTime;
  float mT;
  uint mPriorKey;
  uint mNextKey;
};
CHECK_SIZEOF(CFBFullBodyAspectsForStream, 0x20)

class CFBStreamedPairOfTotals {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CFBStreamedPairOfTotals(const TSubAnimTypeToken< CFBStreamedCompression >& source);
  ~CFBStreamedPairOfTotals() {}
#else
  explicit CFBStreamedPairOfTotals(const TSubAnimTypeToken< CFBStreamedCompression >& source);
#endif

  void SetTime(CMemoryInputToBitLevelLoader& input,
               CBitLevelLoader< CMemoryInputToBitLevelLoader >& loader, const CCharAnimTime& time);
  void DoIncrement(CBitLevelLoader< CMemoryInputToBitLevelLoader >& loader);
  float GetT() const;
  CFBStreamedAnimReaderTotals& Prior() { return mNextSel ? mA : mB; }
  CFBStreamedAnimReaderTotals& Next() { return mNextSel ? mB : mA; }

private:
  TSubAnimTypeToken< CFBStreamedCompression > mSource;
  bool mNextSel;
  CFBStreamedAnimReaderTotals mA;
  CFBStreamedAnimReaderTotals mB;
  CFBFullBodyAspectsForStream mAspects;
  uint mCurKey;
};
CHECK_SIZEOF(CFBStreamedPairOfTotals, RSTL_PRIME1 ? 0x88 : 0xb0)

class CMemoryInputToBitLevelLoader {
  friend class CBitLevelLoader< CMemoryInputToBitLevelLoader >;

public:
#if RSTL_PRIME1 && NONMATCHING
  CMemoryInputToBitLevelLoader(const uint* data) : mData(reinterpret_cast< const uchar* >(data)) {}
#else
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
  explicit
#endif
  CMemoryInputToBitLevelLoader(const uint* data)
  : mData(reinterpret_cast< const uchar* >(data) - sizeof(uint)) {}
#endif

private:
  const uchar* mData;
};
CHECK_SIZEOF(CMemoryInputToBitLevelLoader, 0x4)

template < typename T >
class CBitLevelLoader {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CBitLevelLoader(T& input) : mInput(&input), mWord(Input(*mInput)), mBit(0) {}
#else
  explicit CBitLevelLoader(T& input) : mInput(&input), mWord(Input(*mInput)), mBit(0) {}
#endif

  uint LoadUnsigned(uint bits);
  int LoadSigned(uint bits);

private:
  static uint Input(T& input);

  T* mInput;
  uint mWord;
  uint mBit;
};

template < typename T >
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
NTSC_INLINE
#endif
uint CBitLevelLoader< T >::LoadUnsigned(uint bits) {
  uint remaining = bits;
  uint result = 0;
  uint shift = 0;
  while (remaining != 0) {
#if RSTL_PRIME1 && NONMATCHING
    if (mBit == 32) {
      mBit = 0;
      mWord = Input(*mInput);
    }
#endif
    uint count = rstl::min_val(32 - mBit, remaining);
    uint highShift = 32 - count;
    result |= ((mWord >> mBit) << highShift) >> (highShift - shift);
    mBit += count;
    shift += count;
    remaining -= count;
#if !(RSTL_PRIME1 && NONMATCHING)
    if (mBit == 32) {
      mBit = 0;
      mWord = Input(*mInput);
    }
#endif
  }
  return result;
}

template < typename T >
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
NTSC_INLINE
#endif
int CBitLevelLoader< T >::LoadSigned(uint bits) {
  if (bits == 0) {
    return 0;
  }
  uint value = LoadUnsigned(bits);
#if RSTL_PRIME1 && NONMATCHING
  if (bits < 32 && (value & (1u << (bits - 1)))) {
#else
  if (value & (1 << (bits - 1))) {
#endif
    value |= ~0u << bits;
  }
  return value;
}

template <>
inline uint
CBitLevelLoader< CMemoryInputToBitLevelLoader >::Input(CMemoryInputToBitLevelLoader& input) {
#if RSTL_PRIME1 && NONMATCHING
  uint value = TLoadedVal< uint >::Read(input.mData);
  input.mData += sizeof(uint);
  return value;
#else
  input.mData += sizeof(uint);
  return TLoadedVal< uint >::Read(input.mData);
#endif
}

class CSegIdToIndexConverter {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CSegIdToIndexConverter(const CFBStreamedAnimReaderTotals& totals) {
    for (uint i = 0; i < 100; ++i) {
      mIndices[i] = ~0u;
    }
    uint count = totals.NumEntries();
    for (uint i = 0; i < count; ++i) {
      mIndices[totals.GetSegId(i)] = i;
    }
  }
#else
  explicit CSegIdToIndexConverter(const CFBStreamedAnimReaderTotals& totals);
  ~CSegIdToIndexConverter();
#endif

  uint SegIdToIndex(uint seg) const { return mIndices[seg]; }

private:
  uint mIndices[100];
};
CHECK_SIZEOF(CSegIdToIndexConverter, 0x190)

class CFBStreamedAnimReader : public CAnimSourceReaderBase {
public:
#if !RSTL_PRIME1
  CFBStreamedAnimReader(const TSubAnimTypeToken< CFBStreamedCompression >& source,
                        CCharAnimTime time, const CAnimPOIData* poiData);

  // IAnimReader
#endif

  ~CFBStreamedAnimReader() override;
  SAdvancementResults VAdvanceView(const CCharAnimTime& time) override;

  CCharAnimTime VGetTimeRemaining() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  bool VHasOffset(const CSegId& seg) const override;
  CVector3f VGetOffset(const CSegId& seg) const override;
  CQuaternion VGetRotation(const CSegId& seg) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                           const CCharAnimTime& time) const override;
#if !RSTL_PRIME1
  void VGetJointData_Linear(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                            const CCharAnimTime& time) const override;
  void VGetJointData_Linear(const CCharLayoutInfo& layout,
                            CJointData_LinearStorage& data) const override;
#endif

  rstl::ownership_transfer< IAnimReader > VClone() const override;
  void VSetPhase(float phase) override;
  SAdvancementResults VGetAdvancementResults(const CCharAnimTime& time,
                                             const CCharAnimTime& startOffset) const override;

  virtual bool VSupportsReverseView() const;
  virtual SAdvancementResults VReverseView(const CCharAnimTime& time);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CFBStreamedAnimReader(const TSubAnimTypeToken< CFBStreamedCompression >& source,
                        CCharAnimTime time);
#endif

private:
  void SetReadTime(const CCharAnimTime& time) const;
  void GetSegStatement(CSegStatement& statement, const CSegId& seg) const;

  TSubAnimTypeToken< CFBStreamedCompression > mSource;
  CSteadyStateAnimInfo mSteadyStateInfo;
  mutable CFBStreamedPairOfTotals mTotals;
  mutable CMemoryInputToBitLevelLoader mInput;
  mutable CBitLevelLoader< CMemoryInputToBitLevelLoader > mBitLoader;
  CSegIdToIndexConverter mSegIdToIndex;
};
CHECK_CHILD_SIZEOF(CFBStreamedAnimReader, CAnimSourceReaderBase, RSTL_PRIME1 ? 0x250 : 0x278)

#endif // _CFBSTREAMEDANIMREADER
