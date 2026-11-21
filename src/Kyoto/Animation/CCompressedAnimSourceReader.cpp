#include "Kyoto/Animation/CCompressedAnimSourceReader.hpp"

#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Animation/CAnimMathUtils.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CCompressedAnimSource.hpp"
#include "Kyoto/Animation/CJointData_LinearStorage.hpp"
#include "Kyoto/Animation/CSegStatement.hpp"
#include "Kyoto/Animation/TAnimSourceInfo.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "rstl/math.hpp"



CCompressedAnimSourceReader::CCompressedAnimSourceReader(
    const TSubAnimTypeToken< CCompressedAnimSource >& source, CCharAnimTime time,
    const CAnimPOIData* poiData, ushort currentKey, float decodeTime)
: CAnimSourceReaderBase(rs_new_line(22) TAnimSourceInfo< CCompressedAnimSource >(source), poiData)
, mSource(source)
, mCurrentKey(currentKey)
, mKeyframeTimes(source->GetKeyframeTimes(), source->GetHeader().GetNumKeyframes())
, mChannelMask(source->GetChannelMaskData(), source->GetChannelMaskSize())
, mBitStream(source->GetBitStreamData(), source->GetBitStreamSize())
, mFirstFrameBuffer(rs_new_line(29) short[mBitStream.GetNumChannels()])
, mSecondFrameBuffer(rs_new_line(30) short[mBitStream.GetNumChannels()])
, mFirstFrame(mFirstFrameBuffer, mBitStream.GetNumChannels())
, mSecondFrame(mSecondFrameBuffer, mBitStream.GetNumChannels())
, mSteadyStateInfo(false, CCharAnimTime(source->GetHeader().GetDuration()), CVector3f::Zero())
, mDecodeTime(decodeTime)
, mVisibilityData(source->GetVisibilityData(), source->GetVisibilitySize())
, mOwnsFrameBuffers(true) {
  ResetDecodeState();
  mCurTime = time;
  SeekDecodeTime(time);
}

CCompressedAnimSourceReader::~CCompressedAnimSourceReader() {
  if (mOwnsFrameBuffers) {
    delete[] mFirstFrameBuffer;
    delete[] mSecondFrameBuffer;
  }
}

void CCompressedAnimSourceReader::SeekDecodeTime(const CCharAnimTime& time) const {
  const float duration = mSource->GetHeader().GetDuration();
  const uint numKeys = mSource->GetHeader().GetNumKeyframes();
  const float clampedTime = rstl::min_val(time.GetSeconds(), duration);
  const float* times = mKeyframeTimes.GetTimes();
  if (clampedTime < times[rstl::max_val(0, int(mCurrentKey) - 1)]) {
    ResetDecodeState();
  }

  float nextTime = times[mCurrentKey];
  while (clampedTime >= nextTime && mCurrentKey < numKeys - 1) {
    mSecondFrame.CopyPackedData(mFirstFrame);
    mSecondFrame.GetPackedData(mChannelMask, mBitStream);
    ++mCurrentKey;
    nextTime = times[mCurrentKey];
  }
  mDecodeTime = clampedTime;
}

void CCompressedAnimSourceReader::GetSegStatement(CSegStatement& statement,
                                                  const CSegId& seg) const {
  const uchar segmentIndex = seg.val();
  const uchar* masks = mChannelMask.GetMasks();
  uint dataChannel = 0;
  for (uchar i = 0; i < segmentIndex; ++i) {
    if (masks[i] & CAnimChannelMask::skRotation) {
      dataChannel += CQuantizedAnimFrame::skRotationChannels;
    }
    if (masks[i] & CAnimChannelMask::skTranslation) {
      dataChannel += CQuantizedAnimFrame::skTranslationChannels;
    }
    if (masks[i] & CAnimChannelMask::skScale) {
      dataChannel += CQuantizedAnimFrame::skScaleChannels;
    }
  }
  const float t = GetFrameInterpolantFromTime(mDecodeTime);
  const uchar mask = masks[segmentIndex];
  SetDecodeGQR6();
  const short* first = mFirstFrame.GetData() + dataChannel;
  const short* second = mSecondFrame.GetData() + dataChannel;
  uint offset = 0;
  if (mask & CAnimChannelMask::skRotation) {
    statement.Set(InterpolateQuantizedRotation(first, second, t));
    offset = CQuantizedAnimFrame::skRotationChannels;
  }
  if (mask & CAnimChannelMask::skTranslation) {
    statement.Set(InterpolateQuantizedVector(first + offset, second + offset, t,
                                             mSource->GetHeader().GetOffsetScale()));
    offset += CQuantizedAnimFrame::skTranslationChannels;
  }
  if (mask & CAnimChannelMask::skScale) {
    statement.SetScale(InterpolateQuantizedVector(first + offset, second + offset, t,
                                                  mSource->GetHeader().GetScaleScale()));
  }
}

CQuaternion CCompressedAnimSourceReader::InterpolateQuantizedRotation(register const short* first,
                                                                      register const short* second,
                                                                      float t) const {
  struct SDecodedRotation { // Guessed name
    float sign;
    CVector3f vector;
  };
  SDecodedRotation firstValues;
  SDecodedRotation secondValues;
  register SDecodedRotation* firstOut = &firstValues;
  register SDecodedRotation* secondOut = &secondValues;
  register float positiveScale = 32768.f / 32767.f;
  register float pair0;
  register float pair1;
  register float scaledPair;
  register float firstSign;
  register float secondSign;
  register float firstMagnitude;
  register float secondMagnitude;
  __asm__ {
    psq_l pair0, 0(first), 0, 6
    ps_muls0 scaledPair, pair0, positiveScale
    ps_sel pair0, pair0, scaledPair, pair0
    psq_st pair0, 0(firstOut), 0, 0
    ps_mul firstSign, pair0, pair0
    psq_l pair1, 4(first), 0, 6
    ps_muls0 scaledPair, pair1, positiveScale
    ps_sel pair1, pair1, scaledPair, pair1
    ps_mul scaledPair, pair1, pair1
    psq_st pair1, 8(firstOut), 0, 0
    ps_sum0 firstMagnitude, scaledPair, scaledPair, firstSign
    ps_sum0 firstMagnitude, firstMagnitude, firstMagnitude, firstMagnitude

    psq_l pair0, 0(second), 0, 6
    ps_muls0 scaledPair, pair0, positiveScale
    ps_sel pair0, pair0, scaledPair, pair0
    psq_st pair0, 0(secondOut), 0, 0
    ps_mul secondSign, pair0, pair0
    psq_l pair1, 4(second), 0, 6
    ps_muls0 scaledPair, pair1, positiveScale
    ps_sel pair1, pair1, scaledPair, pair1
    ps_mul scaledPair, pair1, pair1
    psq_st pair1, 8(secondOut), 0, 0
    ps_sum0 secondMagnitude, scaledPair, scaledPair, secondSign
    ps_sum0 secondMagnitude, secondMagnitude, secondMagnitude, secondMagnitude
  }
  const float firstW = CMath::SqrtF(rstl::max_val(1.f - firstMagnitude, 0.f));
  const float secondW = CMath::SqrtF(rstl::max_val(1.f - secondMagnitude, 0.f));
  return CAnimMathUtils::Slerp(CQuaternion(firstSign > 0.f ? -firstW : firstW, firstValues.vector),
                               CQuaternion(secondSign > 0.f ? -secondW : secondW, secondValues.vector),
                               t);
}

CVector3f CCompressedAnimSourceReader::InterpolateQuantizedVector(register const short* first,
                                                                  register const short* second,
                                                                  float t,
                                                                  register float scale) const {
  float firstValues[4];
  float secondValues[4];
  register float* firstOut = firstValues;
  register float* secondOut = secondValues;
  register float pair0;
  register float pair1;
  __asm__ {
    psq_l pair0, 0(first), 0, 5
    ps_muls0 pair0, pair0, scale
    psq_st pair0, 0(firstOut), 0, 0
    psq_l pair1, 4(first), 0, 5
    ps_muls0 pair1, pair1, scale
    psq_st pair1, 8(firstOut), 0, 0
    psq_l pair0, 0(second), 0, 5
    ps_muls0 pair0, pair0, scale
    psq_st pair0, 0(secondOut), 0, 0
    psq_l pair1, 4(second), 0, 5
    ps_muls0 pair1, pair1, scale
    psq_st pair1, 8(secondOut), 0, 0
  }
  const CVector3f firstVector(firstValues[0], firstValues[1], firstValues[2]);
  const CVector3f secondVector(secondValues[0], secondValues[1], secondValues[2]);
  return CVector3f::Lerp(firstVector, secondVector, t);
}

SAdvancementResults CCompressedAnimSourceReader::VAdvanceView(const CCharAnimTime& time) {
  const CCharAnimTime priorTime = mCurTime;
  const CCharAnimTime duration = mSource->GetAnimationDuration();
  if (priorTime == duration) {
    mCurTime = CCharAnimTime::ZeroFlat();
    SeekDecodeTime(mCurTime);
    mPassedBoolCount = 0;
    mPassedParticleCount = 0;
    mPassedSoundCount = 0;
    mPassedAssetSoundCount = 0;
    mPassedAssetCount = 0;
    return SAdvancementResults(time,
                               SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
  }
  if (time.EqualsZero()) {
    return SAdvancementResults(CCharAnimTime::ZeroFlat(),
                               SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
  }
  CSegStatement prior;
  GetSegStatement(prior, CSegId(0));
  mCurTime += time;
  CCharAnimTime remainder = CCharAnimTime::ZeroFlat();
  if (mCurTime > duration) {
    remainder = mCurTime - duration;
    mCurTime = duration;
  }
  if (CMath::AbsF(mCurTime.GetSeconds() - duration.GetSeconds()) < 0.00001f) {
    mCurTime = duration;
  }
  SeekDecodeTime(mCurTime);
  UpdatePOIStates();
  CSegStatement next;
  GetSegStatement(next, CSegId(0));
  const CQuaternion priorRotation = prior.Orientation();
  const CQuaternion nextRotation = next.Orientation();
  const CQuaternion priorInverse = priorRotation.BuildInverted();
  CVector3f offset(0.f, 0.f, 0.f);
  if (prior.OffsetValid() || next.OffsetValid()) {
    offset = next.Offset() - prior.Offset();
    const CQuaternion nextInverse = nextRotation.BuildInverted();
    offset = nextInverse.Transform(offset);
  }
  return SAdvancementResults(remainder, SAdvancementDeltas(offset, nextRotation * priorInverse));
}

CCharAnimTime CCompressedAnimSourceReader::VGetTimeRemaining() const {
  const float remaining = mSource->GetHeader().GetDuration() - mCurTime.GetSeconds();
  return CCharAnimTime(rstl::max_val(0.f, remaining));
}

CSteadyStateAnimInfo CCompressedAnimSourceReader::VGetSteadyStateAnimInfo() const {
  return mSteadyStateInfo;
}

bool CCompressedAnimSourceReader::VHasOffset(const CSegId& seg) const { return false; }

CVector3f CCompressedAnimSourceReader::VGetOffset(const CSegId& seg) const {
  return CVector3f::Zero();
}

CQuaternion CCompressedAnimSourceReader::VGetRotation(const CSegId& seg) const {
  return CQuaternion::NoRotation();
}

rstl::ownership_transfer< IAnimReader > CCompressedAnimSourceReader::VClone() const {
  return rs_new_line(455) CCompressedAnimSourceReader(mSource, mCurTime, mPOIData);
}

bool CCompressedAnimSourceReader::VSupportsReverseView() const { return false; }

SAdvancementResults CCompressedAnimSourceReader::VReverseView(const CCharAnimTime& time) {
  return SAdvancementResults(CCharAnimTime::ZeroFlat(),
                             SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
}

void CCompressedAnimSourceReader::VSetPhase(float phase) {
  mCurTime = CCharAnimTime(phase * mSource->GetHeader().GetDuration());
  ResetDecodeState();
  SeekDecodeTime(mCurTime);
  UpdatePOIStates();
  if (!mCurTime.GreaterThanZero()) {
    mPassedBoolCount = 0;
    mPassedParticleCount = 0;
    mPassedSoundCount = 0;
    mPassedAssetSoundCount = 0;
    mPassedAssetCount = 0;
  }
}

SAdvancementResults
CCompressedAnimSourceReader::VGetAdvancementResults(const CCharAnimTime& time,
                                                    const CCharAnimTime& startOffset) const {
  const CCharAnimTime priorTime = mCurTime + startOffset;
  CCharAnimTime nextTime = mCurTime + startOffset;
  const CCharAnimTime& duration = mSource->GetAnimationDuration();
  if (priorTime >= duration) {
    return SAdvancementResults(time,
                               SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
  }
  if (time.EqualsZero()) {
    return SAdvancementResults(CCharAnimTime::ZeroFlat(),
                               SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
  }
  SeekDecodeTime(priorTime);
  CSegStatement prior;
  GetSegStatement(prior, CSegId(0));
  nextTime += time;
  CCharAnimTime remainder = CCharAnimTime::ZeroFlat();
  if (nextTime > duration) {
    remainder = nextTime - duration;
    nextTime = duration;
  }
  SeekDecodeTime(nextTime);
  CSegStatement next;
  GetSegStatement(next, CSegId(0));
  const CQuaternion priorRotation = prior.Orientation();
  const CQuaternion nextRotation = next.Orientation();
  const CQuaternion priorInverse = priorRotation.BuildInverted();
  CVector3f offset(0.f, 0.f, 0.f);
  if (prior.OffsetValid() || next.OffsetValid()) {
    offset = next.Offset() - prior.Offset();
    const CQuaternion nextInverse = nextRotation.BuildInverted();
    offset = nextInverse.Transform(offset);
  }
  SeekDecodeTime(mCurTime);
  return SAdvancementResults(remainder, SAdvancementDeltas(offset, nextRotation * priorInverse));
}

void CCompressedAnimSourceReader::VGetSegStatementSet(const CSegIdList& list,
                                                      CSegStatementSet& set) const {
  RS_VERIFY_THROW(568, false, 0,
                  "CCompressedAnimSourceReader::VGetSegStatementSet -- unsupported!");
}

void CCompressedAnimSourceReader::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& set,
                                                      const CCharAnimTime& time) const {
  RS_VERIFY_THROW(575, false, 0,
                  "CCompressedAnimSourceReader::VGetSegStatementSet -- unsupported!");
}

void CCompressedAnimSourceReader::VGetJointData_Linear(const CCharLayoutInfo& layout,
                                                       CJointData_LinearStorage& storage,
                                                       const CCharAnimTime& time) const {
  if (time != mCurTime) {
    SeekDecodeTime(time);
  }
  const bool hasOffsets = (mSource->GetHeader().GetFlags() & CCompressedAnimSourceHeader::skHasOffsets) != 0;
  const bool hasScales = (mSource->GetHeader().GetFlags() & CCompressedAnimSourceHeader::skHasScales) != 0;
  const uint numSegmentsInAnimation = mChannelMask.GetNumChannels();
  const uchar* masks = mChannelMask.GetMasks();
  const short* first = mFirstFrame.GetData();
  const short* second = mSecondFrame.GetData();
  uchar* rotations = storage.GetRotations();
  uchar* translations = storage.GetTranslations();
  uchar* scales = storage.GetScales();
  const int stride = storage.GetStride();
  if (numSegmentsInAnimation != uint(layout.GetBodyPartSegIds().size())) {
    RS_VERIFY_FAILURE(
        615, "numSegmentsInAnimation == CCast::ToUint32( layout.GetBodyPartSegIds().size() )",
        "false",
        CBasics::Stringize(
            "CCompressedAnimSourceReader::VGetJointData_Linear -- number of segments "
            "mismatch between animation and skeleton! (Anim = %i, Skeleton = %i)",
            numSegmentsInAnimation, layout.GetBodyPartSegIds().size()));
  }
  const uint numJoints = numSegmentsInAnimation;
  if (numJoints != uint(storage.GetNumJoints())) {
    RS_VERIFY_FAILURE(626, "numJoints == CCast::ToUint32( storage.GetNumJoints() )", "false",
                      CBasics::Stringize(
                          "CCompressedAnimSourceReader::VGetJointData_Linear -- number of segments "
                          "mismatch between animation/skeleton and joint storage! "
                          "(Anim/Skeleton = %i, storage = %i",
                          numJoints, storage.GetNumJoints()));
  }
  if (!hasScales && storage.HasScales()) {
    storage.ResetScales();
  }
  const float t = GetFrameInterpolantFromTime(time.GetSeconds());
  uint dataChannel = 0;
  register uint quantization = 0x0f070f07;
  __asm__ { mtspr 918, quantization }
  if (!hasScales && !hasOffsets) {
    for (uint i = 0; i < numJoints; ++i, ++masks) {
      uchar mask = *masks;
      if (mask & CAnimChannelMask::skRotation) {
        *reinterpret_cast< CQuaternion* >(rotations) =
            InterpolateQuantizedRotation(first + dataChannel, second + dataChannel, t);
        dataChannel += CQuantizedAnimFrame::skRotationChannels;
      } else {
        *reinterpret_cast< CQuaternion* >(rotations) = CQuaternion::NoRotation();
      }
      rotations += stride;
    }
  } else if (!hasScales && hasOffsets) {
    storage.SetHasOffsets(true);
    if (storage.UsesZeroOffsets()) {
      for (uint i = 0; i < numJoints; ++i, ++masks) {
        uchar mask = *masks;
        if (mask & CAnimChannelMask::skRotation) {
          *reinterpret_cast< CQuaternion* >(rotations) =
              InterpolateQuantizedRotation(first + dataChannel, second + dataChannel, t);
          dataChannel += CQuantizedAnimFrame::skRotationChannels;
        } else {
          *reinterpret_cast< CQuaternion* >(rotations) = CQuaternion::NoRotation();
        }
        rotations += stride;
        if (mask & CAnimChannelMask::skTranslation) {
          *reinterpret_cast< CVector3f* >(translations) = InterpolateQuantizedVector(
              first + dataChannel, second + dataChannel, t, mSource->GetHeader().GetOffsetScale());
          dataChannel += CQuantizedAnimFrame::skTranslationChannels;
        } else {
          *reinterpret_cast< CVector3f* >(translations) = CVector3f::Zero();
        }
        translations += stride;
      }
    } else {
      const CVector3f* referenceOffsets = layout.GetLinearParentOffsets().data();
      for (uint i = 0; i < numJoints; ++i, ++masks) {
        uchar mask = *masks;
        if (mask & CAnimChannelMask::skRotation) {
          *reinterpret_cast< CQuaternion* >(rotations) =
              InterpolateQuantizedRotation(first + dataChannel, second + dataChannel, t);
          dataChannel += CQuantizedAnimFrame::skRotationChannels;
        } else {
          *reinterpret_cast< CQuaternion* >(rotations) = CQuaternion::NoRotation();
        }
        rotations += stride;
        if (mask & CAnimChannelMask::skTranslation) {
          *reinterpret_cast< CVector3f* >(translations) = InterpolateQuantizedVector(
              first + dataChannel, second + dataChannel, t, mSource->GetHeader().GetOffsetScale());
          dataChannel += CQuantizedAnimFrame::skTranslationChannels;
        } else {
          *reinterpret_cast< CVector3f* >(translations) = *referenceOffsets;
        }
        translations += stride;
        ++referenceOffsets;
      }
    }
  } else {
    storage.SetHasOffsets(true);
    storage.SetHasScales(true);
    if (storage.UsesZeroOffsets()) {
      for (uint i = 0; i < numJoints; ++i, ++masks) {
        uchar mask = *masks;
        if (mask & CAnimChannelMask::skRotation) {
          *reinterpret_cast< CQuaternion* >(rotations) =
              InterpolateQuantizedRotation(first + dataChannel, second + dataChannel, t);
          dataChannel += CQuantizedAnimFrame::skRotationChannels;
        } else {
          *reinterpret_cast< CQuaternion* >(rotations) = CQuaternion::NoRotation();
        }
        rotations += stride;
        if (mask & CAnimChannelMask::skTranslation) {
          *reinterpret_cast< CVector3f* >(translations) = InterpolateQuantizedVector(
              first + dataChannel, second + dataChannel, t, mSource->GetHeader().GetOffsetScale());
          dataChannel += CQuantizedAnimFrame::skTranslationChannels;
        } else {
          *reinterpret_cast< CVector3f* >(translations) = CVector3f::Zero();
        }
        translations += stride;
        if (mask & CAnimChannelMask::skScale) {
          *reinterpret_cast< CVector3f* >(scales) = InterpolateQuantizedVector(
              first + dataChannel, second + dataChannel, t, mSource->GetHeader().GetScaleScale());
          dataChannel += CQuantizedAnimFrame::skScaleChannels;
        } else {
          *reinterpret_cast< CVector3f* >(scales) = CVector3f::One();
        }
        scales += stride;
      }
    } else {
      const CVector3f* referenceOffsets = layout.GetLinearParentOffsets().data();
      for (uint i = 0; i < numJoints; ++i, ++masks) {
        uchar mask = *masks;
        if (mask & CAnimChannelMask::skRotation) {
          *reinterpret_cast< CQuaternion* >(rotations) =
              InterpolateQuantizedRotation(first + dataChannel, second + dataChannel, t);
          dataChannel += CQuantizedAnimFrame::skRotationChannels;
        } else {
          *reinterpret_cast< CQuaternion* >(rotations) = CQuaternion::NoRotation();
        }
        rotations += stride;
        if (mask & CAnimChannelMask::skTranslation) {
          *reinterpret_cast< CVector3f* >(translations) = InterpolateQuantizedVector(
              first + dataChannel, second + dataChannel, t, mSource->GetHeader().GetOffsetScale());
          dataChannel += CQuantizedAnimFrame::skTranslationChannels;
        } else {
          *reinterpret_cast< CVector3f* >(translations) = *referenceOffsets;
        }
        translations += stride;
        ++referenceOffsets;
        if (mask & CAnimChannelMask::skScale) {
          *reinterpret_cast< CVector3f* >(scales) = InterpolateQuantizedVector(
              first + dataChannel, second + dataChannel, t, mSource->GetHeader().GetScaleScale());
          dataChannel += CQuantizedAnimFrame::skScaleChannels;
        } else {
          *reinterpret_cast< CVector3f* >(scales) = CVector3f::One();
        }
        scales += stride;
      }
    }
  }
  if (time != mCurTime) {
    SeekDecodeTime(mCurTime);
  }
}

void CCompressedAnimSourceReader::VGetJointData_Linear(const CCharLayoutInfo& layout,
                                                       CJointData_LinearStorage& storage) const {
  VGetJointData_Linear(layout, storage, CCharAnimTime(mDecodeTime));
}

const CVisibilityAnimationDataReader*
CCompressedAnimSourceReader::VGetVisibilityDataAtCurrentTime() const {
  return CCompressedAnimSourceReader::VGetVisibilityDataAtTime(mCurTime);
}

const CVisibilityAnimationDataReader*
CCompressedAnimSourceReader::VGetVisibilityDataAtTime(const CCharAnimTime& time) const {
  const int index = mVisibilityData.FindRecordIndexAtTime(time, 0);
  if (index >= 0) {
    return mVisibilityData.GetRecordByIndex(index);
  }
  return nullptr;
}

float CCompressedAnimSourceReader::GetFrameInterpolantFromTime(float time) const {
  if (mCurrentKey != 0) {
    const float firstFrameTime = mKeyframeTimes.GetTimes()[mCurrentKey - 1];
    const float secondFrameTime = mKeyframeTimes.GetTimes()[mCurrentKey];
    RS_VERIFY_THROW(992, time >= firstFrameTime && time <= secondFrameTime, 0,
                    "CCompressedAnimSourceReader::GetFrameInterpolantFromTime -- invalid time!");
    const float interval = secondFrameTime - firstFrameTime;
    return rstl::max_val(0.f, time - firstFrameTime) / interval;
  }
  return 0.f;
}

void CCompressedAnimSourceReader::ResetDecodeState() const {
  CQuantizedAnimFrame base(mSource->GetBaseFrameData(), mBitStream.GetNumChannels());
  base.CopyPackedData(mFirstFrame);
  base.CopyPackedData(mSecondFrame);
  mBitStream.ResetReadPosition();
  mCurrentKey = 0;
}

void CCompressedAnimSourceReader::SetDecodeGQR6() {
  // GQR6 (SPR 918) controls the paired-single quaternion loads above. Both halves
  // select signed 16-bit values (type 7) with scale 15, so psq_l divides by 32768.
  // InterpolateQuantizedRotation then rescales positive values by 32768 / 32767.
  register uint value = 0x0f070f07;
  __asm__ { mtspr 918, value }
}
