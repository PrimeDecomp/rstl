#ifndef _CANIMSOURCE
#define _CANIMSOURCE
#include "rstl/RstlVersions.h"

#include "Kyoto/Animation/CAnimPOIData.hpp"

#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Animation/CSteadyStateAnimInfo.hpp"
#include "Kyoto/Animation/IAnimReader.hpp"

#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/SObjectTag.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/pair.hpp"
#include "rstl/set.hpp"
#include "rstl/string.hpp"
#include "rstl/math.hpp"

#include "rstl/vector.hpp"

class CInputStream;

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
class IObjectStore;
#else
class CCharLayoutInfo;
class CJointData_LinearStorage;
#endif

class CSegIdList;
#if !RSTL_PRIME1
class CSegStatement;
#endif

class CSegStatementSet;

class RotationAndOffsetStorage {
public:
  struct CRotationAndOffsetVectors {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    CRotationAndOffsetVectors(CInputStream& in);
#else
    explicit CRotationAndOffsetVectors(CInputStream& in);
#endif

    rstl::vector< CQuaternion > mRotations;
    rstl::vector< CVector3f > mOffsets;
  };

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  RotationAndOffsetStorage(const CRotationAndOffsetVectors&, uint numFrames);

  static uint DataSizeInBytes(uint, uint, uint);
#else
  RotationAndOffsetStorage(const CRotationAndOffsetVectors& vectors, uint numFrames);
  static uint DataSizeInBytes(uint rotationsPerFrame, uint offsetsPerFrame, uint numFrames);
#endif

  rstl::auto_ptr< uint > GetRotationsAndOffsets(const rstl::vector< CQuaternion >& rotations,
                                                const rstl::vector< CVector3f >& offsets,
                                                uint numFrames);
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  static void CopyRotationsAndOffsets(const rstl::vector< CQuaternion >&,
                                      const rstl::vector< CVector3f >&, uint numFrames, float* buf);
#else
  static void CopyRotationsAndOffsets(const rstl::vector< CQuaternion >& rotations,
                                      const rstl::vector< CVector3f >& offsets, uint numFrames,
                                      float* buffer);
#endif

  uint GetFrameSizeInBytes() const;

  const uint* StartForFrame(uint frame) const {
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return mStorage.get() + frame * (mRotationsPerFrame * 4 + mOffsetsPerFrame * 3);
#else
    return mStorage.get() +
           (mRotationsPerFrame * 4 + mOffsetsPerFrame * 3) * rstl::min_val(frame, mNumFrames - 1);
#endif

  }

  const CQuaternion& GetRotation(uint channel, uint frame) const {
    const uint* start = StartForFrame(frame);
    return *reinterpret_cast< const CQuaternion* >(start + channel * 4);
  }

  const CVector3f& GetOffset(uint channel, uint frame) const {
    const uint* start = StartForFrame(frame);
    const uint offset = mRotationsPerFrame * 4 + channel * 3;
    return *reinterpret_cast< const CVector3f* >(start + offset);
  }
#if !RSTL_PRIME1
  uint GetOffsetCount() const { return mOffsetsPerFrame; }
#endif

private:
  rstl::auto_ptr< uint > mStorage;
  uint mNumFrames;
  uint mRotationsPerFrame;
  uint mOffsetsPerFrame;
};
CHECK_SIZEOF(RotationAndOffsetStorage, 0x14)

class CAnimSource {
public:
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CAnimSource(CInputStream& in, IObjectStore& store);
#else
  explicit CAnimSource(CInputStream& in);
#endif

  ~CAnimSource();

#if !RSTL_PRIME1
  bool HasRotation(const CSegId& seg) const;
#endif

  bool HasOffset(const CSegId& seg) const;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CVector3f GetOffset(const CSegId& seg, const CCharAnimTime& animTime) const;
  CQuaternion GetRotation(const CSegId& seg, const CCharAnimTime& animTime) const;
  void GetSegStatementSet(const CSegIdList& list, CSegStatementSet& set,
                          const CCharAnimTime& time) const;
  const CCharAnimTime& GetAnimationDuration() const { return mDuration; }
  const CCharAnimTime& GetTimePerFrame() const { return mInterval; }
  bool HasPOIData() const { return !mEventData.null(); }
  const rstl::vector< CBoolPOINode >& GetBoolPOIStream() const;
  const rstl::vector< CInt32POINode >& GetInt32POIStream() const;
  const rstl::vector< CParticlePOINode >& GetParticlePOIStream() const;
  const rstl::vector< CSoundPOINode >& GetSoundPOIStream() const;
  rstl::set< rstl::pair< rstl::string, int > > GetUniqueBoolPOIs() const;
  CSegId GetPrimaryOffsetChannel() const { return mRoot; }
#else
  bool HasScale(const CSegId& seg) const;
  CVector3f GetOffset(const CSegId& seg, const CCharAnimTime& time) const;
  CQuaternion GetRotation(const CSegId& seg, const CCharAnimTime& time) const;
#endif

  CVector3f GetOverallOffset(const CCharAnimTime& time) const {
    CSegId root = GetPrimaryOffsetChannel();
    return GetOffset(root, time);
  }
  CSteadyStateAnimInfo GetSteadyStateAnimInfo(const CCharAnimTime& time) const {
    return CSteadyStateAnimInfo(false, GetAnimationDuration(), GetOverallOffset(time));
  }
  void CalcAverageVelocity();
#if !RSTL_PRIME1
  void GetSegStatementSet(const CSegIdList& list, CSegStatementSet& set,
                          const CCharAnimTime& time) const;
  // Guessed name.
  void GetJointData_Linear(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                           const CCharAnimTime& time) const;

  const CCharAnimTime& GetAnimationDuration() const { return mDuration; }
  const CCharAnimTime& GetTimePerFrame() const { return mInterval; }
  CSegId GetPrimaryOffsetChannel() const { return mRoot; }
#endif

  float GetAverageVelocity() const { return mAverageVelocity; }
#if !RSTL_PRIME1
  // Guessed name.
  bool HasScaleData() const { return !mScales.empty(); }
#endif

private:
#if !RSTL_PRIME1
  // Guessed names.
  void GetSegStatement(const CSegId& seg, uint frame, uint nextFrame, float weight,
                       CSegStatement& statement) const;
  uint GetSize() const;
#endif

  CCharAnimTime mDuration;
  CCharAnimTime mInterval;
  uint mFrameCount;
#if RSTL_PRIME1
  uint x14_;
  uint x18_;
#endif

  CSegId mRoot;
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::vector< schar > mRotationChannels;
  rstl::vector< schar > mOffsetChannels;
#else
  rstl::vector< signed char > mSegmentChannels;
  rstl::vector< signed char > mRotationChannels;
  rstl::vector< signed char > mOffsetChannels;
  rstl::vector< signed char > mScaleChannels;
  rstl::vector< CVector3f > mScales;
  uint mScalesPerFrame;
#endif

  RotationAndOffsetStorage mStorage;
#if RSTL_PRIME1
  CAssetId mEventId;
  rstl::auto_ptr< TLockedToken< CAnimPOIData > > mEventData;
#endif

  float mAverageVelocity;
};
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
CHECK_SIZEOF(CAnimSource, 0x64)
#else
CHECK_SIZEOF(CAnimSource, 0x84)
#endif

#endif // _CANIMSOURCE
