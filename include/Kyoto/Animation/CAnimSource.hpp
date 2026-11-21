#ifndef _CANIMSOURCE
#define _CANIMSOURCE
#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#include "rstl/RstlVersions.h"
#endif

#if RSTL_CORRUPTION
#include "rstl/static_assert.hpp"
#else
#include "Kyoto/Animation/CAnimPOIData.hpp"

#include "Kyoto/Animation/CCharAnimTime.hpp"
#endif
#include "Kyoto/Animation/CSegId.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Animation/CVisibilityAnimationDataList.hpp"
#else
#include "Kyoto/Animation/CSteadyStateAnimInfo.hpp"
#endif
#include "Kyoto/Animation/IAnimReader.hpp"
#if RSTL_CORRUPTION
#include "Kyoto/Animation/CCharAnimTime.hpp"
#endif
#include "Kyoto/Math/CQuaternion.hpp"
#if !RSTL_CORRUPTION
#include "Kyoto/SObjectTag.hpp"

#endif
#include "rstl/auto_ptr.hpp"
#if !RSTL_CORRUPTION
#include "rstl/pair.hpp"
#include "rstl/set.hpp"
#include "rstl/string.hpp"
#endif
#include "rstl/math.hpp"

#include "rstl/vector.hpp"

#if !RSTL_CORRUPTION
class CInputStream;
#endif

#if !RSTL_CORRUPTION
class IObjectStore;
class CCharLayoutInfo;
class CJointData_LinearStorage;
#endif

#if RSTL_CORRUPTION
class CCharLayoutInfo;
class CInputStream;
class CJointData_LinearStorage;
#endif
class CSegIdList;
class CSegStatement;

class CSegStatementSet;

class RotationAndOffsetStorage {
public:
  struct CRotationAndOffsetVectors {
    explicit CRotationAndOffsetVectors(CInputStream& in);

    rstl::vector< CQuaternion > mRotations;
    rstl::vector< CVector3f > mOffsets;
  };

  RotationAndOffsetStorage(const CRotationAndOffsetVectors& vectors, uint numFrames);
  static uint DataSizeInBytes(uint rotationsPerFrame, uint offsetsPerFrame, uint numFrames);

  rstl::auto_ptr< uint > GetRotationsAndOffsets(const rstl::vector< CQuaternion >& rotations,
                                                const rstl::vector< CVector3f >& offsets,
                                                uint numFrames);
  static void CopyRotationsAndOffsets(const rstl::vector< CQuaternion >& rotations,
                                      const rstl::vector< CVector3f >& offsets, uint numFrames,
                                      float* buffer);

  uint GetFrameSizeInBytes() const;

  const uint* StartForFrame(uint frame) const {
#if RSTL_CORRUPTION
    return mStorage.get() +
           (mRotationsPerFrame * 4 + mOffsetsPerFrame * 3) * rstl::min_val(frame, mNumFrames - 1);
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
    return mStorage.get() + frame * (mRotationsPerFrame * 4 + mOffsetsPerFrame * 3);
#else
    return mStorage.get() +
           (mRotationsPerFrame * 4 + mOffsetsPerFrame * 3) * rstl::min_val(frame, mNumFrames - 1);
#endif
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
#if RSTL_CORRUPTION
  uint GetRotationCount() const { return mRotationsPerFrame; } // Guessed name
  uint GetOffsetCount() const { return mOffsetsPerFrame; }
#else
#if !RSTL_PRIME1
  uint GetOffsetCount() const { return mOffsetsPerFrame; }
#endif
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
#if RSTL_CORRUPTION
  explicit CAnimSource(CInputStream& in);
#else
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  CAnimSource(CInputStream& in, IObjectStore& store);
#else
  explicit CAnimSource(CInputStream& in);
#endif

#endif
  ~CAnimSource();

  bool HasRotation(const CSegId& seg) const;

  bool HasOffset(const CSegId& seg) const;
  bool HasScale(const CSegId& seg) const;
  CVector3f GetOffset(const CSegId& seg, const CCharAnimTime& time) const;
  CQuaternion GetRotation(const CSegId& seg, const CCharAnimTime& time) const;
#if !RSTL_CORRUPTION
#if RSTL_PRIME1
  bool HasPOIData() const { return !mEventData.null(); }
  const rstl::vector< CBoolPOINode >& GetBoolPOIStream() const;
  const rstl::vector< CInt32POINode >& GetInt32POIStream() const;
  const rstl::vector< CParticlePOINode >& GetParticlePOIStream() const;
  const rstl::vector< CSoundPOINode >& GetSoundPOIStream() const;
  rstl::set< rstl::pair< rstl::string, int > > GetUniqueBoolPOIs() const;
#endif

#endif
  CVector3f GetOverallOffset(const CCharAnimTime& time) const {
#if RSTL_CORRUPTION
    return GetOffset(GetPrimaryOffsetChannel(), time);
#else
    CSegId root = GetPrimaryOffsetChannel();
    return GetOffset(root, time);
#endif
  }
  CSteadyStateAnimInfo GetSteadyStateAnimInfo(const CCharAnimTime& time) const {
    return CSteadyStateAnimInfo(false, GetAnimationDuration(), GetOverallOffset(time));
  }
  void CalcAverageVelocity();
  void GetSegStatementSet(const CSegIdList& list, CSegStatementSet& set,
                          const CCharAnimTime& time) const;
  // Guessed name.
  void GetJointData_Linear(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                           const CCharAnimTime& time) const;
#if RSTL_CORRUPTION
  // Guessed name
  const CVisibilityAnimationDataReader* GetVisibilityDataAtTime(const CCharAnimTime& time) const;
#endif

  const CCharAnimTime& GetAnimationDuration() const { return mDuration; }
  const CCharAnimTime& GetTimePerFrame() const { return mInterval; }
  CSegId GetPrimaryOffsetChannel() const { return mRoot; }
#if RSTL_CORRUPTION
  const CVector3f& GetRootOffset() const { return mRootDisplacement; } // Guessed name
  float GetAverageVelocity() const;
  // Guessed name.
  bool HasScaleData() const { return !mScales.empty(); }
#else

  float GetAverageVelocity() const { return mAverageVelocity; }
#if !RSTL_PRIME1
  // Guessed name.
  bool HasScaleData() const { return !mScales.empty(); }
#endif
#endif

private:
  // Guessed names.
  void GetSegStatement(const CSegId& seg, uint frame, uint nextFrame, float weight,
                       CSegStatement& statement) const;
  uint GetSize() const;

#if RSTL_CORRUPTION
  ushort mVersion; // Guessed name
#endif
  CCharAnimTime mDuration;
  CCharAnimTime mInterval;
  uint mFrameCount;
#if !RSTL_CORRUPTION
#if RSTL_PRIME1
  uint x14_;
  uint x18_;
#endif

#endif
  CSegId mRoot;
#if RSTL_CORRUPTION
  rstl::vector< signed char > mSegmentChannels;
  rstl::vector< signed char > mRotationChannels;
  rstl::vector< signed char > mOffsetChannels;
  rstl::vector< signed char > mScaleChannels;
  rstl::vector< CVector3f > mScales;
  uint mScalesPerFrame;
#else
#if !RSTL_PRIME1
  rstl::vector< signed char > mSegmentChannels;
#endif
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  rstl::vector< schar > mRotationChannels;
  rstl::vector< schar > mOffsetChannels;
#else
  rstl::vector< signed char > mRotationChannels;
  rstl::vector< signed char > mOffsetChannels;
  rstl::vector< signed char > mScaleChannels;
  rstl::vector< CVector3f > mScales;
  uint mScalesPerFrame;
#endif

#endif
  RotationAndOffsetStorage mStorage;
#if RSTL_CORRUPTION
  CVector3f mRootDisplacement;                  // Guessed name
  CVisibilityAnimationDataList mVisibilityData; // Guessed name
#else
#if RSTL_PRIME1
  CAssetId mEventId;
  rstl::auto_ptr< TLockedToken< CAnimPOIData > > mEventData;
#endif

  float mAverageVelocity;
#endif
};
#if RSTL_CORRUPTION
CHECK_SIZEOF(CAnimSource, 0x98)
#else
CHECK_SIZEOF(CAnimSource, RSTL_PRIME1 ? 0x64 : 0x84)
#endif

#endif // _CANIMSOURCE
