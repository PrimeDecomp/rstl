#include "rstl/RstlVersions.h"
#include "Kyoto/Animation/CAnimSource.hpp"

#include "Kyoto/Animation/CAnimMathUtils.hpp"
#include "Kyoto/Animation/CBoolPOINode.hpp"

#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#if !RSTL_PRIME1
#include "Kyoto/Animation/CJointData_LinearStorage.hpp"
#endif

#include "Kyoto/Animation/CSegIdList.hpp"
#include "Kyoto/Animation/CSegStatementSet.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#ifdef __MWERKS__
static float clamp_zero_to_one(register const float v) {
  register float zero = 0.f;
  register float one = 1.f;

  register float res, tmp;

  asm {
    fsel res, v, v, zero;
    fsubs tmp, v, one;
    fsel res, tmp, one, res;
  }

  return res;
}
#else
static float clamp_zero_to_one(const float v) {
  return v >= 1.f ? 1.f : v >= 0.f ? v : 0.f;
}
#endif
#else
// Guessed name, corroborated by Prime and the native frame-sampling consumers.
#ifdef __MWERKS__
static float clamp_zero_to_one(register float value) {
  register float out;
  register float zero = 0.f;
  register float one = 1.f;
  asm {
    fsel out, value, value, zero
    fsubs value, value, one
    fsel out, value, one, out
  }
  return out;
}
#else
static float clamp_zero_to_one(float value) {
  return value >= 1.f ? 1.f : value >= 0.f ? value : 0.f;
}
#endif
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
uint RotationAndOffsetStorage::DataSizeInBytes(uint rotationsCountPerFrame, uint offsetsPerFrame,
                                               uint frameCount) {
  return (rotationsCountPerFrame * sizeof(CQuaternion) + offsetsPerFrame * sizeof(CVector3f)) *
         frameCount;
#else
uint RotationAndOffsetStorage::DataSizeInBytes(uint rotationsPerFrame, uint offsetsPerFrame,
                                               uint numFrames) {
  return (rotationsPerFrame * sizeof(CQuaternion) + offsetsPerFrame * sizeof(CVector3f)) *
         numFrames;
#endif

}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
RotationAndOffsetStorage::RotationAndOffsetStorage(const CRotationAndOffsetVectors& storage,
                                                   const uint numFrames)
: mStorage(GetRotationsAndOffsets(storage.mRotations, storage.mOffsets, numFrames))
#else
RotationAndOffsetStorage::RotationAndOffsetStorage(const CRotationAndOffsetVectors& vectors,
                                                   uint numFrames)
: mStorage(GetRotationsAndOffsets(vectors.mRotations, vectors.mOffsets, numFrames))
#endif

, mNumFrames(numFrames)
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
, mRotationsPerFrame(storage.mRotations.size() / numFrames)
, mOffsetsPerFrame(storage.mOffsets.size() / numFrames) {}
#else
, mRotationsPerFrame(vectors.mRotations.size() / numFrames)
, mOffsetsPerFrame(vectors.mOffsets.size() / numFrames) {}
#endif

rstl::auto_ptr< uint >
RotationAndOffsetStorage::GetRotationsAndOffsets(const rstl::vector< CQuaternion >& rotations,
                                                 const rstl::vector< CVector3f >& offsets,
                                                 uint numFrames) {
  mRotationsPerFrame = rotations.size() / numFrames;
  mOffsetsPerFrame = offsets.size() / numFrames;
  rstl::auto_ptr< uint > storage(rs_new uint[DataSizeInBytes(rotations.size() / numFrames, //
                                                             offsets.size() / numFrames,   //
                                                             numFrames                     //
                                                             ) /
                                                 4 +
                                             1]);
  CopyRotationsAndOffsets(rotations, offsets, numFrames, reinterpret_cast< float* >(storage.get()));

  return storage;
}

void RotationAndOffsetStorage::CopyRotationsAndOffsets(const rstl::vector< CQuaternion >& rotations,
                                                       const rstl::vector< CVector3f >& offsets,
                                                       const uint numFrames, float* buf) {
  const uint rotationsPerFrame = rotations.size() / numFrames;
  const uint offsetsPerFrame = offsets.size() / numFrames;

  for (int frame = 0; frame < numFrames; frame++) {
    int i = 0;
    for (int rotation = 0; i < rotationsPerFrame; rotation += numFrames, i++) {
      const CQuaternion& q = rotations[frame + rotation];
      *(buf++) = q.GetScalar();
      *(buf++) = q.AxisX();
      *(buf++) = q.AxisY();
      *(buf++) = q.AxisZ();
    }
    i = 0;
    for (int offset = 0; offset < offsetsPerFrame; offset++, i += numFrames) {
      const CVector3f& o = offsets[frame + i];
      *(buf++) = o.GetX();
      *(buf++) = o.GetY();
      *(buf++) = o.GetZ();
    }
  }
}

uint RotationAndOffsetStorage::GetFrameSizeInBytes() const {
  return mRotationsPerFrame * sizeof(CQuaternion) + mOffsetsPerFrame * sizeof(CVector3f);
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
inline RotationAndOffsetStorage::CRotationAndOffsetVectors::CRotationAndOffsetVectors(
    CInputStream& in)
#else
RotationAndOffsetStorage::CRotationAndOffsetVectors::CRotationAndOffsetVectors(CInputStream& in)
#endif

: mRotations(in), mOffsets(in) {}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CAnimSource::CAnimSource(CInputStream& in, IObjectStore& store)
#else
CAnimSource::CAnimSource(CInputStream& in)
#endif

: mDuration(in)
, mInterval(in)
, mFrameCount(in.Get< uint >())
, mRoot(in)
#if !RSTL_PRIME1
, mSegmentChannels(in)
#endif

, mRotationChannels(in)
, mOffsetChannels(in)
#if !RSTL_PRIME1
, mScaleChannels(in)
, mScales(in)
, mScalesPerFrame(mScales.size() / mFrameCount)
#endif

, mStorage(RotationAndOffsetStorage::CRotationAndOffsetVectors(in), mFrameCount)
#if RSTL_PRIME1
, mEventId(in.Get< CAssetId >())
#endif

, mAverageVelocity(0.f) {
#if RSTL_PRIME1
  if (mEventId != 0) {
    mEventData =
        rs_new TLockedToken< CAnimPOIData >(store.GetObj(SObjectTag('EVNT', mEventId)));
  }
#endif

  CalcAverageVelocity();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  uint totalSize = mRotationChannels.size() + (sizeof(CQuaternion) + sizeof(CVector3f)) + 1;
  totalSize += mOffsetChannels.size();
  totalSize += mFrameCount * mStorage.GetFrameSizeInBytes();
  CCharAnimMemoryMetrics::AddToTotalSize(totalSize + sizeof(CCharAnimTime),
                                         CCharAnimMemoryMetrics::kASS_Two);
#else
  CCharAnimMemoryMetrics::AddToTotalSize(GetSize(), CCharAnimMemoryMetrics::kASS_Two);
#endif

}

CAnimSource::~CAnimSource() {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  uint totalSize = mRotationChannels.size() + (sizeof(CQuaternion) + sizeof(CVector3f)) + 1;
  totalSize += mOffsetChannels.size();
  totalSize += mFrameCount * mStorage.GetFrameSizeInBytes();
  CCharAnimMemoryMetrics::SubtractFromTotalSize(totalSize + sizeof(CCharAnimTime),
                                                CCharAnimMemoryMetrics::kASS_Two);
#else
  CCharAnimMemoryMetrics::SubtractFromTotalSize(GetSize(), CCharAnimMemoryMetrics::kASS_Two);
}

bool CAnimSource::HasRotation(const CSegId& seg) const {
  return mRotationChannels[mSegmentChannels[seg.val()]] >= 0;
#endif

}

bool CAnimSource::HasOffset(const CSegId& seg) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return mOffsetChannels[mRotationChannels[seg.val()]] >= 0;
#else
  return mOffsetChannels[mSegmentChannels[seg.val()]] >= 0;
}

bool CAnimSource::HasScale(const CSegId& seg) const {
  return mScaleChannels[mSegmentChannels[seg.val()]] >= 0;
#endif

}

CVector3f CAnimSource::GetOffset(const CSegId& seg, const CCharAnimTime& animTime) const {
  const float frameTime = animTime.GetSeconds();
  float interval = mInterval.GetSeconds();
#ifdef __MWERKS__
  const float invTime = __fres(interval);
#else
  const float invTime = 1.f / interval;
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const uint frame = static_cast< uint >(frameTime * invTime);
#else
  const uint frame = CCast::ToUint32(frameTime * invTime);
#endif

  float time = interval * frame;
  time = frameTime - time;

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (fabs(time) < Real32::Epsilon()) {
#else
  if (CMath::AbsF(time) < Real32::Epsilon()) {
#endif

    time = 0.f;
  }

  time = clamp_zero_to_one(time * invTime);

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int channel = mRotationChannels[seg.val()];
  if (channel >= 0) {
#else
  int channel = mSegmentChannels[seg.val()];
  if (channel >= 0 && HasOffset(seg)) {
    const u32 nextFrame = frame == mFrameCount - 1 ? 0 : frame + 1;
#endif

    channel = mOffsetChannels[channel];
#if RSTL_PRIME1
    const uint nextFrame = frame == mFrameCount - 1 ? 0 : frame + 1;
#endif

    const CVector3f& a = mStorage.GetOffset(channel, frame);
    const CVector3f& b = mStorage.GetOffset(channel, nextFrame);
    return CVector3f::Lerp(a, b, time);
  }

  return CVector3f::Zero();
}

CQuaternion CAnimSource::GetRotation(const CSegId& seg, const CCharAnimTime& animTime) const {
  const float interval = GetTimePerFrame().GetSeconds();
#ifdef __MWERKS__
  const float invTime = __fres(interval);
#else
  const float invTime = 1.f / interval;
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const int channel = mRotationChannels[seg.val()];
  if (channel >= 0) {
#else
  const int channel = mSegmentChannels[seg.val()];
  if (channel >= 0 && HasRotation(seg)) {
#endif

    const float frameTime = animTime.GetSeconds();
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const uint frame = static_cast< uint >(frameTime * invTime);
#else
    const uint frame = CCast::ToUint32(frameTime * invTime);
#endif

    float time = interval * frame;
    time = frameTime - time;

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    if (fabs(time) < Real32::Epsilon()) {
#else
    if (CMath::AbsF(time) < Real32::Epsilon()) {
#endif

      time = 0.f;
    }

    time = clamp_zero_to_one(time * invTime);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const uint nextFrame = frame == mFrameCount - 1 ? 0 : frame + 1;
#else
    const u32 nextFrame = frame == mFrameCount - 1 ? 0 : frame + 1;
#endif

    const CQuaternion& a = mStorage.GetRotation(channel, frame);
    const CQuaternion& b = mStorage.GetRotation(channel, nextFrame);
    return CAnimMathUtils::Slerp(a, b, time);
  }

  return CQuaternion::NoRotation();
}

void CAnimSource::CalcAverageVelocity() {
  const float invDuration = 1.f / mDuration.GetSeconds();
  float distance = 0.f;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const uint channel = mOffsetChannels[mRotationChannels[CSegId::Root().val()]];
#else
  const uint channel = mOffsetChannels[mSegmentChannels[0]];
#endif

  for (uint frame = 1; frame < mFrameCount; ++frame) {
    const CVector3f delta =
        mStorage.GetOffset(channel, frame) - mStorage.GetOffset(channel, frame - 1);
    const float magnitude = delta.Magnitude();
    if (!close_enough(magnitude, 0.f)) {
      distance += magnitude;
    }
  }

  distance *= invDuration;
  mAverageVelocity = distance;
}

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
const rstl::vector< CBoolPOINode >& CAnimSource::GetBoolPOIStream() const {
  return (*mEventData)->GetBoolPOIStream();
#else
void CAnimSource::GetSegStatement(const CSegId& seg, uint frame, uint nextFrame, float weight,
                                  CSegStatement& statement) const {
  const int channel = mSegmentChannels[seg.val()];
  const float inverseWeight = 1.f - weight;
  if (inverseWeight < CAnimMathUtils::kInterpolationThreshold) {
    if (HasRotation(seg)) {
      statement.Set(mStorage.GetRotation(channel, nextFrame));
#endif

}
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
const rstl::vector< CInt32POINode >& CAnimSource::GetInt32POIStream() const {
  return (*mEventData)->GetInt32POIStream();
#else
    if (HasOffset(seg)) {
      const uint offsetChannel = mOffsetChannels[channel];
      statement.Set(mStorage.GetOffset(offsetChannel, nextFrame));
#endif

}
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
const rstl::vector< CParticlePOINode >& CAnimSource::GetParticlePOIStream() const {
  return (*mEventData)->GetParticlePOIStream();
#else
  } else if (weight < CAnimMathUtils::kInterpolationThreshold) {
    if (HasRotation(seg)) {
      statement.Set(mStorage.GetRotation(channel, frame));
#endif

}
#if RSTL_PRIME1
const rstl::vector< CSoundPOINode >& CAnimSource::GetSoundPOIStream() const {
  return (*mEventData)->GetSoundPOIStream();
}

rstl::set< rstl::pair< rstl::string, int > > CAnimSource::GetUniqueBoolPOIs() const {
  const rstl::vector< CBoolPOINode >& nodes = GetBoolPOIStream();
  int count = nodes.size();
  rstl::set< rstl::pair< rstl::string, int > > ret;
  for (int i = 0; i < count; ++i) {
    const CBoolPOINode& node = nodes[i];
    ret.insert(rstl::pair< rstl::string, int >(node.GetString(), node.GetIndex()));
  }
  return ret;
}

void CAnimSource::GetSegStatementSet(const CSegIdList& list, CSegStatementSet& set,
                                     const CCharAnimTime& animTime) const {
  const float frameTime = animTime.GetSeconds();
  const float interval = GetTimePerFrame().GetSeconds();
#ifdef __MWERKS__
  const float invTime = __fres(interval);
#else
  const float invTime = 1.f / interval;
#endif
  const uint frame = static_cast< uint >(frameTime * invTime);
  float time = interval * frame;
  time = frameTime - time;
  if (fabs(time) < Real32::Epsilon()) {
    time = 0.f;
  }
  time = clamp_zero_to_one(time * invTime);
  const uint nextFrame = frame == mFrameCount - 1 ? 0 : frame + 1;
  const int count = list.size();

  for (int i = 0; i < count; ++i) {
    const CSegId& seg = list[i];
    const int channel = mRotationChannels[seg.val()];
    if (channel >= 0) {
      if (1.f - time < CAnimMathUtils::kInterpolationThreshold) {
        set.Set(seg, mStorage.GetRotation(channel, nextFrame));
#endif

        if (HasOffset(seg)) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
          set.Set(seg, mStorage.GetOffset(mOffsetChannels[mRotationChannels[seg.val()]],
                                             nextFrame));
        }
      } else if (time < CAnimMathUtils::kInterpolationThreshold) {
        set.Set(seg, mStorage.GetRotation(channel, frame));
        if (HasOffset(seg)) {
          set.Set(seg, mStorage.GetOffset(mOffsetChannels[mRotationChannels[seg.val()]],
                                             frame));
#else
      const uint offsetChannel = mOffsetChannels[channel];
      statement.Set(mStorage.GetOffset(offsetChannel, frame));
#endif

        }
      } else {
#if !RSTL_PRIME1
    if (HasRotation(seg)) {
#endif

        const CQuaternion& a = mStorage.GetRotation(channel, frame);
        const CQuaternion& b = mStorage.GetRotation(channel, nextFrame);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
        set.Set(seg, CAnimMathUtils::Slerp(a, b, time));
#else
      statement.Set(CAnimMathUtils::Slerp(a, b, weight));
    }
#endif

        if (HasOffset(seg)) {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
          const uint offsetChannel = mOffsetChannels[mRotationChannels[seg.val()]];
#else
      const uint offsetChannel = mOffsetChannels[channel];
#endif

          const CVector3f& a = mStorage.GetOffset(offsetChannel, frame);
          const CVector3f& b = mStorage.GetOffset(offsetChannel, nextFrame);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
          set.Set(seg, CVector3f::Lerp(a, b, time));
        }
      }
    } else {
      set.Set(seg, CQuaternion::NoRotation());
#else
      statement.Set(CVector3f::Lerp(a, b, weight));
#endif

    }
  }
}

#if !RSTL_PRIME1
void CAnimSource::GetSegStatementSet(const CSegIdList& list, CSegStatementSet& set,
                                     const CCharAnimTime& time) const {
  const float frameTime = time.GetSeconds();
  const float interval = GetTimePerFrame().GetSeconds();
#ifdef __MWERKS__
  const float inverseInterval = __fres(interval);
#else
  const float inverseInterval = 1.f / interval;
#endif
  uint frame = CCast::ToUint32(frameTime * inverseInterval);
  float remainder = interval * frame;
  remainder = frameTime - remainder;
  if (CMath::AbsF(remainder) < Real32::Epsilon()) {
    remainder = 0.f;
  }
  float weight = clamp_zero_to_one(remainder * inverseInterval);
  uint nextFrame = frame == mFrameCount - 1 ? 0 : frame + 1;
  const int count = list.Size();
  const float inverseWeight = 1.f - weight;
  for (int i = 0; i < count; ++i) {
    const CSegId& seg = list[i];
    const int channel = mSegmentChannels[seg.val()];
    if (channel >= 0) {
      CSegStatement& statement = set[seg];
      GetSegStatement(seg, frame, nextFrame, weight, statement);
      if (HasScale(seg)) {
        const uint scaleChannel = mScaleChannels[channel];
        if (inverseWeight < CAnimMathUtils::kInterpolationThreshold) {
          statement.SetScale(mScales[nextFrame * mScalesPerFrame + scaleChannel]);
        } else if (weight < CAnimMathUtils::kInterpolationThreshold) {
          statement.SetScale(mScales[frame * mScalesPerFrame + scaleChannel]);
        } else {
          const CVector3f& a = mScales[frame * mScalesPerFrame + scaleChannel];
          const CVector3f& b = mScales[nextFrame * mScalesPerFrame + scaleChannel];
          statement.SetScale(CVector3f::Lerp(a, b, weight));
        }
      }
    } else {
      set[seg].Set(CQuaternion::NoRotation());
    }
  }
}
#endif

#if !RSTL_PRIME1
void CAnimSource::GetJointData_Linear(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                                      const CCharAnimTime& time) const {
  const float frameTime = time.GetSeconds();
  const float interval = GetTimePerFrame().GetSeconds();
#ifdef __MWERKS__
  const float inverseInterval = __fres(interval);
#else
  const float inverseInterval = 1.f / interval;
#endif
  uint frame = static_cast< uint >(frameTime * inverseInterval);
  float remainder = interval * frame;
  remainder = frameTime - remainder;
  if (CMath::AbsF(remainder) < Real32::Epsilon()) {
    remainder = 0.f;
  }
  float weight = clamp_zero_to_one(remainder * inverseInterval);
  const uint nextFrame = frame == mFrameCount - 1 ? 0 : frame + 1;
  const bool hasOffsets = mStorage.GetOffsetCount() != 0;
  const bool hasScales = !mScales.empty();
  const uint count = mRotationChannels.size();
  if (!hasScales && data.HasScales()) {
    data.ResetScales();
  }

  u32 copyFrame = ~0u;
  const float inverseWeight = 1.f - weight;
  UJointCursor rotations = data.GetRotations();
  UJointCursor offsets = data.GetTranslations();
  UJointCursor scales = data.GetScales();
  const int stride = data.GetStride();
  if (inverseWeight < CAnimMathUtils::kInterpolationThreshold) {
    copyFrame = nextFrame;
  } else if (weight < CAnimMathUtils::kInterpolationThreshold) {
    copyFrame = frame;
  }
  const signed char* rotationChannels = mRotationChannels.data();
  const signed char* offsetChannels = mOffsetChannels.data();
  const signed char* scaleChannels = mScaleChannels.data();
  const CVector3f* referenceOffsets = layout.GetLinearParentOffsets().data();

  if (copyFrame != ~0u) {
    const CQuaternion* sourceRotations =
        reinterpret_cast< const CQuaternion* >(mStorage.StartForFrame(copyFrame));
    const CVector3f* sourceOffsets = &mStorage.GetOffset(0, copyFrame);
    if (!hasScales && !hasOffsets) {
      for (uint i = 0; i < count; ++i) {
        if (*rotationChannels != -1) {
          *rotations.mRotation = *sourceRotations++;
        } else {
          *rotations.mRotation = CQuaternion::NoRotation();
        }
        rotations.mBytes += stride;
        ++rotationChannels;
      }
    } else if (!hasScales && hasOffsets) {
      data.SetHasOffsets(true);
      for (uint i = 0; i < count; ++i) {
        if (*rotationChannels != -1) {
          *rotations.mRotation = *sourceRotations++;
        } else {
          *rotations.mRotation = CQuaternion::NoRotation();
        }
        rotations.mBytes += stride;
        ++rotationChannels;
        if (*offsetChannels != -1) {
          *offsets.mVector = *sourceOffsets++;
        } else if (data.UsesZeroOffsets()) {
          *offsets.mVector = CVector3f::Zero();
        } else {
          *offsets.mVector = *referenceOffsets;
        }
        offsets.mBytes += stride;
        ++referenceOffsets;
        ++offsetChannels;
      }
    } else {
      const CVector3f* sourceScales = mScales.data() + mScalesPerFrame * copyFrame;
      data.SetHasOffsets(true);
      data.SetHasScales(true);
      for (uint i = 0; i < count; ++i) {
        if (*rotationChannels != -1) {
          *rotations.mRotation = *sourceRotations++;
        } else {
          *rotations.mRotation = CQuaternion::NoRotation();
        }
        rotations.mBytes += stride;
        ++rotationChannels;
        if (*offsetChannels != -1) {
          *offsets.mVector = *sourceOffsets++;
        } else if (data.UsesZeroOffsets()) {
          *offsets.mVector = CVector3f::Zero();
        } else {
          *offsets.mVector = *referenceOffsets;
        }
        offsets.mBytes += stride;
        ++referenceOffsets;
        ++offsetChannels;
        if (*scaleChannels != -1) {
          *scales.mVector = *sourceScales++;
        } else {
          *scales.mVector = CVector3f::One();
        }
        scales.mBytes += stride;
        ++scaleChannels;
      }
    }
  } else {
    const CQuaternion* priorRotations =
        reinterpret_cast< const CQuaternion* >(mStorage.StartForFrame(frame));
    const CVector3f* priorOffsets = &mStorage.GetOffset(0, frame);
    const CQuaternion* nextRotations =
        reinterpret_cast< const CQuaternion* >(mStorage.StartForFrame(nextFrame));
    const CVector3f* nextOffsets = &mStorage.GetOffset(0, nextFrame);
    if (!hasScales && !hasOffsets) {
      for (uint i = 0; i < count; ++i) {
        if (*rotationChannels != -1) {
          *rotations.mRotation = CAnimMathUtils::Slerp(*priorRotations++, *nextRotations++, weight);
        } else {
          *rotations.mRotation = CQuaternion::NoRotation();
        }
        rotations.mBytes += stride;
        ++rotationChannels;
      }
    } else if (!hasScales && hasOffsets) {
      data.SetHasOffsets(true);
      for (uint i = 0; i < count; ++i) {
        if (*rotationChannels != -1) {
          *rotations.mRotation = CAnimMathUtils::Slerp(*priorRotations++, *nextRotations++, weight);
        } else {
          *rotations.mRotation = CQuaternion::NoRotation();
        }
        rotations.mBytes += stride;
        ++rotationChannels;
        if (*offsetChannels != -1) {
          *offsets.mVector = CVector3f::Lerp(*priorOffsets++, *nextOffsets++, weight);
        } else if (data.UsesZeroOffsets()) {
          *offsets.mVector = CVector3f::Zero();
        } else {
          *offsets.mVector = *referenceOffsets;
        }
        offsets.mBytes += stride;
        ++referenceOffsets;
        ++offsetChannels;
      }
    } else {
      data.SetHasOffsets(true);
      data.SetHasScales(true);
      const CVector3f* priorScales = mScales.data() + frame * mScalesPerFrame;
      const CVector3f* nextScales = mScales.data() + nextFrame * mScalesPerFrame;
      for (uint i = 0; i < count; ++i) {
        if (*rotationChannels != -1) {
          *rotations.mRotation = CAnimMathUtils::Slerp(*priorRotations++, *nextRotations++, weight);
        } else {
          *rotations.mRotation = CQuaternion::NoRotation();
        }
        rotations.mBytes += stride;
        ++rotationChannels;
        if (*offsetChannels != -1) {
          *offsets.mVector = CVector3f::Lerp(*priorOffsets++, *nextOffsets++, weight);
        } else if (data.UsesZeroOffsets()) {
          *offsets.mVector = CVector3f::Zero();
        } else {
          *offsets.mVector = *referenceOffsets;
        }
        offsets.mBytes += stride;
        ++referenceOffsets;
        ++offsetChannels;
        if (*scaleChannels != -1) {
          *scales.mVector = CVector3f::Lerp(*priorScales++, *nextScales++, weight);
        } else {
          *scales.mVector = CVector3f::One();
        }
        scales.mBytes += stride;
        ++scaleChannels;
      }
    }
  }
}

uint CAnimSource::GetSize() const {
  // The original accounting omits the scale-channel map and scale keys.
  uint size = sizeof(CAnimSource) + mSegmentChannels.size();
  size += mRotationChannels.size();
  size += mOffsetChannels.size();
  size += mFrameCount * mStorage.GetFrameSizeInBytes();
  return size;
}
#endif
