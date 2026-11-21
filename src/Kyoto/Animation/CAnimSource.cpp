#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif
#include "Kyoto/Animation/CAnimSource.hpp"

#include "Kyoto/Animation/CAnimMathUtils.hpp"
#if !RSTL_CORRUPTION
#include "Kyoto/Animation/CBoolPOINode.hpp"

#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"
#endif
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Animation/CJointData_LinearStorage.hpp"
#else
#if !RSTL_PRIME1
#include "Kyoto/Animation/CJointData_LinearStorage.hpp"
#endif

#include "Kyoto/Animation/CSegIdList.hpp"
#endif
#include "Kyoto/Animation/CSegStatementSet.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Alloc/Assert.hpp"
#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"
#include "Kyoto/Animation/CSegIdList.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Basics/CCast.hpp"
#endif
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#if !RSTL_CORRUPTION
#include "Kyoto/Basics/CCast.hpp"
#endif
#include "Kyoto/Streams/CInputStream.hpp"
#if RSTL_CORRUPTION
#include <rstl/types.h>


#endif

// Guessed name, corroborated by Prime and the native frame-sampling consumers.
#ifdef __MWERKS__
#if RSTL_CORRUPTION
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
#else
static float clamp_zero_to_one(register RSTL_PRIME1_CONST float v) {
#if RSTL_PRIME1
  register float zero = 0.f;
  register float one = 1.f;

  register float res, tmp;

  asm {
    fsel res, v, v, zero;
    fsubs tmp, v, one;
    fsel res, tmp, one, res;
  }

  return res;
#else
  register float out;
  register float zero = 0.f;
  register float one = 1.f;
  asm {
    fsel out, v, v, zero
    fsubs v, v, one
    fsel out, v, one, out
  }
  return out;
#endif
#endif
}
#else
#if RSTL_CORRUPTION
static float clamp_zero_to_one(float value) {
#else
static float clamp_zero_to_one(RSTL_PRIME1_CONST float value) {
#endif
  return value >= 1.f ? 1.f : value >= 0.f ? value : 0.f;
}
#endif

uint RotationAndOffsetStorage::DataSizeInBytes(uint rotationsPerFrame, uint offsetsPerFrame,
                                               uint numFrames) {
  return (rotationsPerFrame * sizeof(CQuaternion) + offsetsPerFrame * sizeof(CVector3f)) *
         numFrames;
}

RotationAndOffsetStorage::RotationAndOffsetStorage(const CRotationAndOffsetVectors& vectors,
#if RSTL_CORRUPTION
                                                   uint numFrames)
#else
                                                   RSTL_PRIME1_CONST uint numFrames)
#endif
: mStorage(GetRotationsAndOffsets(vectors.mRotations, vectors.mOffsets, numFrames))
, mNumFrames(numFrames)
, mRotationsPerFrame(vectors.mRotations.size() / numFrames)
, mOffsetsPerFrame(vectors.mOffsets.size() / numFrames) {}

rstl::auto_ptr< uint >
RotationAndOffsetStorage::GetRotationsAndOffsets(const rstl::vector< CQuaternion >& rotations,
                                                 const rstl::vector< CVector3f >& offsets,
                                                 uint numFrames) {
  mRotationsPerFrame = rotations.size() / numFrames;
  mOffsetsPerFrame = offsets.size() / numFrames;
  rstl::auto_ptr< uint > storage(rs_new_line(72)
                                     uint[DataSizeInBytes(rotations.size() / numFrames, //
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

#if !RSTL_CORRUPTION
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
inline
#endif
#endif
RotationAndOffsetStorage::CRotationAndOffsetVectors::CRotationAndOffsetVectors(CInputStream& in)
: mRotations(in), mOffsets(in) {}

#if RSTL_CORRUPTION
CAnimSource::CAnimSource(CInputStream& in)
: mVersion(in.Get< ushort >())
, mDuration(in)
#else
CAnimSource::CAnimSource(CInputStream& in
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
                         , IObjectStore& store
#endif
                         )
: mDuration(in)
#endif
, mInterval(in)
, mFrameCount(in.Get< uint >())
, mRoot(in)
#if RSTL_CORRUPTION
, mSegmentChannels(in)
#else
#if !RSTL_PRIME1
, mSegmentChannels(in)
#endif
#endif
, mRotationChannels(in)
, mOffsetChannels(in)
#if RSTL_CORRUPTION
, mScaleChannels(in)
, mScales(in)
, mScalesPerFrame(mScales.size() / mFrameCount)
#else
#if !RSTL_PRIME1
, mScaleChannels(in)
, mScales(in)
, mScalesPerFrame(mScales.size() / mFrameCount)
#endif
#endif
, mStorage(RotationAndOffsetStorage::CRotationAndOffsetVectors(in), mFrameCount)
#if RSTL_CORRUPTION
, mRootDisplacement(CVector3f::Zero())
, mVisibilityData(in) {
#else
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
#endif
  CalcAverageVelocity();
#if RSTL_CORRUPTION
  CCharAnimMemoryMetrics::AddToTotalSize(GetSize(), CCharAnimMemoryMetrics::kASS_Two);
#else
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
#endif
}

CAnimSource::~CAnimSource() {
#if RSTL_CORRUPTION
  CCharAnimMemoryMetrics::SubtractFromTotalSize(GetSize(), CCharAnimMemoryMetrics::kASS_Two);
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  uint totalSize = mRotationChannels.size() + (sizeof(CQuaternion) + sizeof(CVector3f)) + 1;
  totalSize += mOffsetChannels.size();
  totalSize += mFrameCount * mStorage.GetFrameSizeInBytes();
  CCharAnimMemoryMetrics::SubtractFromTotalSize(totalSize + sizeof(CCharAnimTime),
                                                CCharAnimMemoryMetrics::kASS_Two);
#else
  CCharAnimMemoryMetrics::SubtractFromTotalSize(GetSize(), CCharAnimMemoryMetrics::kASS_Two);
#endif
#endif
}

#if RSTL_CORRUPTION
bool CAnimSource::HasRotation(const CSegId& seg) const {
  return mRotationChannels[mSegmentChannels[seg.val()]] >= 0;
}
#else
#if !RSTL_PRIME1
bool CAnimSource::HasRotation(const CSegId& seg) const {
  return mRotationChannels[mSegmentChannels[seg.val()]] >= 0;
}
#endif
#endif

bool CAnimSource::HasOffset(const CSegId& seg) const {
#if RSTL_CORRUPTION
  return mOffsetChannels[mSegmentChannels[seg.val()]] >= 0;
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  return mOffsetChannels[mRotationChannels[seg.val()]] >= 0;
#else
  return mOffsetChannels[mSegmentChannels[seg.val()]] >= 0;
#endif
#endif
}

#if RSTL_CORRUPTION
bool CAnimSource::HasScale(const CSegId& seg) const {
  return mScaleChannels[mSegmentChannels[seg.val()]] >= 0;
}

const CVisibilityAnimationDataReader*
CAnimSource::GetVisibilityDataAtTime(const CCharAnimTime& time) const {
  const int index = mVisibilityData.FindRecordIndexAtTime(time, 0);
  if (index >= 0) {
    return mVisibilityData.GetRecordByIndex(index);
  }
  return nullptr;
}
#else
#if !RSTL_PRIME1
bool CAnimSource::HasScale(const CSegId& seg) const {
  return mScaleChannels[mSegmentChannels[seg.val()]] >= 0;
}
#endif
#endif

CVector3f CAnimSource::GetOffset(const CSegId& seg, const CCharAnimTime& animTime) const {
  const float frameTime = animTime.GetSeconds();
  float interval = mInterval.GetSeconds();
#ifdef __MWERKS__
  const float invTime = __fres(interval);
#else
  const float invTime = 1.f / interval;
#endif
#if RSTL_CORRUPTION
  const uint frame = CCast::ToUint32(frameTime * invTime);
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const uint frame = static_cast< uint >(frameTime * invTime);
#else
  const uint frame = CCast::ToUint32(frameTime * invTime);
#endif
#endif
  float time = interval * frame;
  time = frameTime - time;

#if RSTL_CORRUPTION
  if (CMath::AbsF(time) < Real32::Epsilon()) {
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  if (fabs(time) < Real32::Epsilon()) {
#else
  if (CMath::AbsF(time) < Real32::Epsilon()) {
#endif
#endif
    time = 0.f;
  }

  time = clamp_zero_to_one(time * invTime);

#if RSTL_CORRUPTION
  int channel = mSegmentChannels[seg.val()];
  if (channel >= 0 && HasOffset(seg)) {
    const int nextFrame = frame == mFrameCount - 1 ? 0 : frame + 1;
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  int channel = mRotationChannels[seg.val()];
  if (channel >= 0) {
#else
  int channel = mSegmentChannels[seg.val()];
  if (channel >= 0 && HasOffset(seg)) {
    const u32 nextFrame = frame == mFrameCount - 1 ? 0 : frame + 1;
#endif
#endif
    channel = mOffsetChannels[channel];
#if !RSTL_CORRUPTION
#if RSTL_PRIME1
    const uint nextFrame = frame == mFrameCount - 1 ? 0 : frame + 1;
#endif
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
#if RSTL_CORRUPTION
  const int channel = mSegmentChannels[seg.val()];
  if (channel >= 0 && HasRotation(seg)) {
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const int channel = mRotationChannels[seg.val()];
  if (channel >= 0) {
#else
  const int channel = mSegmentChannels[seg.val()];
  if (channel >= 0 && HasRotation(seg)) {
#endif
#endif
    const float frameTime = animTime.GetSeconds();
#if RSTL_CORRUPTION
    const uint frame = CCast::ToUint32(frameTime * invTime);
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const uint frame = static_cast< uint >(frameTime * invTime);
#else
    const uint frame = CCast::ToUint32(frameTime * invTime);
#endif
#endif
    float time = interval * frame;
    time = frameTime - time;

#if RSTL_CORRUPTION
    if (CMath::AbsF(time) < Real32::Epsilon()) {
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    if (fabs(time) < Real32::Epsilon()) {
#else
    if (CMath::AbsF(time) < Real32::Epsilon()) {
#endif
#endif
      time = 0.f;
    }

    time = clamp_zero_to_one(time * invTime);
#if RSTL_CORRUPTION
    const int nextFrame = frame == mFrameCount - 1 ? 0 : frame + 1;
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    const uint nextFrame = frame == mFrameCount - 1 ? 0 : frame + 1;
#else
    const u32 nextFrame = frame == mFrameCount - 1 ? 0 : frame + 1;
#endif
#endif
    const CQuaternion& a = mStorage.GetRotation(channel, frame);
    const CQuaternion& b = mStorage.GetRotation(channel, nextFrame);
    return CAnimMathUtils::Slerp(a, b, time);
  }

  return CQuaternion::NoRotation();
}

#if RSTL_CORRUPTION
float CAnimSource::GetAverageVelocity() const {
  const float duration = mDuration.GetSeconds();
  return mRootDisplacement.Magnitude() / duration;
}
#else
void CAnimSource::CalcAverageVelocity() {
  const float invDuration = 1.f / mDuration.GetSeconds();
  float distance = 0.f;
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_CORRUPTION
void CAnimSource::CalcAverageVelocity() {
  const uint channel = mOffsetChannels[mSegmentChannels[0]];
  mRootDisplacement = CVector3f::Zero();
#else
#if RSTL_PRIME1
  const uint channel = mOffsetChannels[mRotationChannels[CSegId::Root().val()]];
#else
  const uint channel = mOffsetChannels[mSegmentChannels[0]];
#endif
#endif
  for (uint frame = 1; frame < mFrameCount; ++frame) {
#if RSTL_CORRUPTION
    mRootDisplacement +=
#else
    const CVector3f delta =
#endif
        mStorage.GetOffset(channel, frame) - mStorage.GetOffset(channel, frame - 1);
#if !RSTL_CORRUPTION
    const float magnitude = delta.Magnitude();
    if (!close_enough(magnitude, 0.f)) {
      distance += magnitude;
#endif
    }
  }

#if RSTL_CORRUPTION
void CAnimSource::GetSegStatement(const CSegId& seg, uint frame, uint nextFrame, float weight,
                                  CSegStatement& statement) const {
  const int channel = mSegmentChannels[seg.val()];
  const float inverseWeight = 1.f - weight;
  if (inverseWeight < CAnimMathUtils::kInterpolationThreshold) {
    if (HasRotation(seg)) {
      statement.Set(mStorage.GetRotation(channel, nextFrame));
    }
    if (HasOffset(seg)) {
      const uint offsetChannel = mOffsetChannels[channel];
      statement.Set(mStorage.GetOffset(offsetChannel, nextFrame));
    }
  } else if (weight < CAnimMathUtils::kInterpolationThreshold) {
    if (HasRotation(seg)) {
      statement.Set(mStorage.GetRotation(channel, frame));
    }
    if (HasOffset(seg)) {
      const uint offsetChannel = mOffsetChannels[channel];
      statement.Set(mStorage.GetOffset(offsetChannel, frame));
    }
  } else {
    if (HasRotation(seg)) {
      const CQuaternion& a = mStorage.GetRotation(channel, frame);
      const CQuaternion& b = mStorage.GetRotation(channel, nextFrame);
      statement.Set(CAnimMathUtils::Slerp(a, b, weight));
    }
    if (HasOffset(seg)) {
      const uint offsetChannel = mOffsetChannels[channel];
      const CVector3f& a = mStorage.GetOffset(offsetChannel, frame);
      const CVector3f& b = mStorage.GetOffset(offsetChannel, nextFrame);
      statement.Set(CVector3f::Lerp(a, b, weight));
    }
  }
}
#else
  distance *= invDuration;
  mAverageVelocity = distance;
}

#if RSTL_PRIME1
const rstl::vector< CBoolPOINode >& CAnimSource::GetBoolPOIStream() const {
  return (*mEventData)->GetBoolPOIStream();
}

const rstl::vector< CInt32POINode >& CAnimSource::GetInt32POIStream() const {
  return (*mEventData)->GetInt32POIStream();
}

const rstl::vector< CParticlePOINode >& CAnimSource::GetParticlePOIStream() const {
  return (*mEventData)->GetParticlePOIStream();
}

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
#endif

#if !RSTL_PRIME1
void CAnimSource::GetSegStatement(const CSegId& seg, uint frame, uint nextFrame, float weight,
                                  CSegStatement& statement) const {
  const int channel = mSegmentChannels[seg.val()];
  const float inverseWeight = 1.f - weight;
  if (inverseWeight < CAnimMathUtils::kInterpolationThreshold) {
    if (HasRotation(seg)) {
      statement.Set(mStorage.GetRotation(channel, nextFrame));
    }
    if (HasOffset(seg)) {
      const uint offsetChannel = mOffsetChannels[channel];
      statement.Set(mStorage.GetOffset(offsetChannel, nextFrame));
    }
  } else if (weight < CAnimMathUtils::kInterpolationThreshold) {
    if (HasRotation(seg)) {
      statement.Set(mStorage.GetRotation(channel, frame));
    }
    if (HasOffset(seg)) {
      const uint offsetChannel = mOffsetChannels[channel];
      statement.Set(mStorage.GetOffset(offsetChannel, frame));
    }
  } else {
    if (HasRotation(seg)) {
      const CQuaternion& a = mStorage.GetRotation(channel, frame);
      const CQuaternion& b = mStorage.GetRotation(channel, nextFrame);
      statement.Set(CAnimMathUtils::Slerp(a, b, weight));
    }
    if (HasOffset(seg)) {
      const uint offsetChannel = mOffsetChannels[channel];
      const CVector3f& a = mStorage.GetOffset(offsetChannel, frame);
      const CVector3f& b = mStorage.GetOffset(offsetChannel, nextFrame);
      statement.Set(CVector3f::Lerp(a, b, weight));
    }
  }
}
#endif
#endif

void CAnimSource::GetSegStatementSet(const CSegIdList& list, CSegStatementSet& set,
#if RSTL_CORRUPTION
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
  const int count = list.size();
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
#else
                                     const CCharAnimTime& animTime) const {
#if RSTL_PRIME1
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
        if (HasOffset(seg)) {
          set.Set(seg, mStorage.GetOffset(mOffsetChannels[mRotationChannels[seg.val()]],
                                          nextFrame));
        }
      } else if (time < CAnimMathUtils::kInterpolationThreshold) {
        set.Set(seg, mStorage.GetRotation(channel, frame));
        if (HasOffset(seg)) {
          set.Set(seg, mStorage.GetOffset(mOffsetChannels[mRotationChannels[seg.val()]], frame));
        }
      } else {
        const CQuaternion& a = mStorage.GetRotation(channel, frame);
        const CQuaternion& b = mStorage.GetRotation(channel, nextFrame);
        set.Set(seg, CAnimMathUtils::Slerp(a, b, time));
        if (HasOffset(seg)) {
          const uint offsetChannel = mOffsetChannels[mRotationChannels[seg.val()]];
          const CVector3f& a = mStorage.GetOffset(offsetChannel, frame);
          const CVector3f& b = mStorage.GetOffset(offsetChannel, nextFrame);
          set.Set(seg, CVector3f::Lerp(a, b, time));
        }
      }
    } else {
      set.Set(seg, CQuaternion::NoRotation());
    }
  }
#else
  const float frameTime = animTime.GetSeconds();
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
  const int count = list.size();
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
#endif
#endif
}

#if RSTL_CORRUPTION
void CAnimSource::GetJointData_Linear(const CCharLayoutInfo& layout,
                                      CJointData_LinearStorage& storage,
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
  const uint nextFrame = frame == mFrameCount - 1 ? 0 : frame + 1;
  const uint32 numJoints = mRotationChannels.size();
  // clang-format off
  RS_VERIFY_THROW(507, numJoints == (uint32) layout.GetBodyPartSegIds().size(), false,
                  CBasics::Stringize("Number of seg id's doesn't match in "
                                     "CAnimSource::GetJointData_Linear. (Anim = %i, Skeleton = %i)",
                                     numJoints, layout.GetBodyPartSegIds().size()));
  RS_VERIFY_THROW(508, numJoints == CCast::ToUint32( storage.GetNumJoints() ), false,
                  CBasics::Stringize("Number of joints in animation does not match the joint "
                                     "storage in CAnimSource::GetJointData_Linear.",
                                     numJoints, storage.GetNumJoints()));
  // clang-format on
  const bool hasOffsets = mStorage.GetOffsetCount() != 0;
  const bool hasScales = !mScales.empty();
  const uint count = numJoints;
  if (!hasScales && storage.HasScales()) {
    storage.ResetScales();
  }

  u32 copyFrame = ~0u;
  const float inverseWeight = 1.f - weight;
  uchar* rotations = storage.GetRotations();
  uchar* offsets = storage.GetTranslations();
  uchar* scales = storage.GetScales();
  const int stride = storage.GetStride();
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
          *reinterpret_cast< CQuaternion* >(rotations) = *sourceRotations++;
        } else {
          *reinterpret_cast< CQuaternion* >(rotations) = CQuaternion::NoRotation();
        }
        rotations += stride;
        ++rotationChannels;
      }
    } else if (!hasScales && hasOffsets) {
      storage.SetHasOffsets(true);
      for (uint i = 0; i < count; ++i) {
        if (*rotationChannels != -1) {
          *reinterpret_cast< CQuaternion* >(rotations) = *sourceRotations++;
        } else {
          *reinterpret_cast< CQuaternion* >(rotations) = CQuaternion::NoRotation();
        }
        rotations += stride;
        ++rotationChannels;
        if (*offsetChannels != -1) {
          *reinterpret_cast< CVector3f* >(offsets) = *sourceOffsets++;
        } else if (storage.UsesZeroOffsets()) {
          *reinterpret_cast< CVector3f* >(offsets) = CVector3f::Zero();
        } else {
          *reinterpret_cast< CVector3f* >(offsets) = *referenceOffsets;
        }
        offsets += stride;
        ++referenceOffsets;
        ++offsetChannels;
      }
    } else {
      const CVector3f* sourceScales = mScales.data() + mScalesPerFrame * copyFrame;
      storage.SetHasOffsets(true);
      storage.SetHasScales(true);
      for (uint i = 0; i < count; ++i) {
        if (*rotationChannels != -1) {
          *reinterpret_cast< CQuaternion* >(rotations) = *sourceRotations++;
        } else {
          *reinterpret_cast< CQuaternion* >(rotations) = CQuaternion::NoRotation();
        }
        rotations += stride;
        ++rotationChannels;
        if (*offsetChannels != -1) {
          *reinterpret_cast< CVector3f* >(offsets) = *sourceOffsets++;
        } else if (storage.UsesZeroOffsets()) {
          *reinterpret_cast< CVector3f* >(offsets) = CVector3f::Zero();
        } else {
          *reinterpret_cast< CVector3f* >(offsets) = *referenceOffsets;
        }
        offsets += stride;
        ++referenceOffsets;
        ++offsetChannels;
        if (*scaleChannels != -1) {
          *reinterpret_cast< CVector3f* >(scales) = *sourceScales++;
        } else {
          *reinterpret_cast< CVector3f* >(scales) = CVector3f::One();
        }
        scales += stride;
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
          *reinterpret_cast< CQuaternion* >(rotations) =
              CAnimMathUtils::Slerp(*priorRotations++, *nextRotations++, weight);
        } else {
          *reinterpret_cast< CQuaternion* >(rotations) = CQuaternion::NoRotation();
        }
        rotations += stride;
        ++rotationChannels;
      }
    } else if (!hasScales && hasOffsets) {
      storage.SetHasOffsets(true);
      for (uint i = 0; i < count; ++i) {
        if (*rotationChannels != -1) {
          *reinterpret_cast< CQuaternion* >(rotations) =
              CAnimMathUtils::Slerp(*priorRotations++, *nextRotations++, weight);
        } else {
          *reinterpret_cast< CQuaternion* >(rotations) = CQuaternion::NoRotation();
        }
        rotations += stride;
        ++rotationChannels;
        if (*offsetChannels != -1) {
          *reinterpret_cast< CVector3f* >(offsets) =
              CVector3f::Lerp(*priorOffsets++, *nextOffsets++, weight);
        } else if (storage.UsesZeroOffsets()) {
          *reinterpret_cast< CVector3f* >(offsets) = CVector3f::Zero();
        } else {
          *reinterpret_cast< CVector3f* >(offsets) = *referenceOffsets;
        }
        offsets += stride;
        ++referenceOffsets;
        ++offsetChannels;
      }
    } else {
      storage.SetHasOffsets(true);
      storage.SetHasScales(true);
      const CVector3f* priorScales = mScales.data() + mScalesPerFrame * frame;
      const CVector3f* nextScales = mScales.data() + mScalesPerFrame * nextFrame;
      for (uint i = 0; i < count; ++i) {
        if (*rotationChannels != -1) {
          *reinterpret_cast< CQuaternion* >(rotations) =
              CAnimMathUtils::Slerp(*priorRotations++, *nextRotations++, weight);
        } else {
          *reinterpret_cast< CQuaternion* >(rotations) = CQuaternion::NoRotation();
        }
        rotations += stride;
        ++rotationChannels;
        if (*offsetChannels != -1) {
          *reinterpret_cast< CVector3f* >(offsets) =
              CVector3f::Lerp(*priorOffsets++, *nextOffsets++, weight);
        } else if (storage.UsesZeroOffsets()) {
          *reinterpret_cast< CVector3f* >(offsets) = CVector3f::Zero();
        } else {
          *reinterpret_cast< CVector3f* >(offsets) = *referenceOffsets;
        }
        offsets += stride;
        ++referenceOffsets;
        ++offsetChannels;
        if (*scaleChannels != -1) {
          *reinterpret_cast< CVector3f* >(scales) =
              CVector3f::Lerp(*priorScales++, *nextScales++, weight);
        } else {
          *reinterpret_cast< CVector3f* >(scales) = CVector3f::One();
        }
        scales += stride;
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
#else
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
#endif
