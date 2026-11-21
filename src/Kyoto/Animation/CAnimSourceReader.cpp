#include "rstl/RstlVersions.h"
#if !RSTL_CORRUPTION
#include "rstl/RstlVersions.h"
#endif
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Animation/CAnimSourceReader.hpp"
#if RSTL_CORRUPTION

#include "Kyoto/Math/CloseEnough.hpp"


#else
#include "Kyoto/Animation/CCharAnimTime.hpp"
#endif

CAnimSourceReader::CAnimSourceReader(const TSubAnimTypeToken< CAnimSource >& source,
#if RSTL_CORRUPTION
                                     const CCharAnimTime& time, const CAnimPOIData* poiData)
: CAnimSourceReaderBase(rs_new_line(31) CAnimSourceInfo(source), poiData)
#else
                                     const CCharAnimTime& time
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
                                     , const CAnimPOIData* poiData
#endif
                                     )
: CAnimSourceReaderBase(rs_new CAnimSourceInfo(source)
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
                        , poiData
#endif
                        )
#endif
, mSource(source)
, mSteadyStateInfo(mSource->GetSteadyStateAnimInfo(time)) {
  PostConstruct(time);
}

SAdvancementResults CAnimSourceReader::VAdvanceView(const CCharAnimTime& dt) {

  const CCharAnimTime previousTime = mCurTime;
  const CCharAnimTime& duration = mSource->GetAnimationDuration();
  if (previousTime == duration) {
    mCurTime = CCharAnimTime::ZeroFlat();
    mPassedBoolCount = 0;
#if !RSTL_CORRUPTION
    mPassedIntCount = 0;
#endif
    mPassedParticleCount = 0;
    mPassedSoundCount = 0;
#if RSTL_CORRUPTION
    mPassedAssetSoundCount = 0;
    mPassedAssetCount = 0;
    return SAdvancementResults(dt,
                               SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
#else
    return SAdvancementResults(dt, CAdvancementDeltas(
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
            CVector3f::Zero()
            , CQuaternion::NoRotation()
#endif
            ));

#endif
  }
  if (dt.EqualsZero()) {
#if RSTL_CORRUPTION
    return SAdvancementResults(CCharAnimTime::ZeroFlat(),
                               SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
#else
    return SAdvancementResults(CCharAnimTime::ZeroFlat(), CAdvancementDeltas(
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
            CVector3f::Zero()
            , CQuaternion::NoRotation()
#endif
            ));

#endif
  }

  mCurTime += dt;
  CCharAnimTime remainingTime = CCharAnimTime::ZeroFlat();
  if (mCurTime > duration) {
    remainingTime = mCurTime - duration;
    mCurTime = duration;
  }
#if RSTL_CORRUPTION
  if (close_enough(mCurTime.GetSeconds(), duration.GetSeconds())) {
    mCurTime = duration;
#else
#if RSTL_PRIME1
  if (mSource->HasPOIData())
#endif
  {
    UpdatePOIStates();
#endif
  }
#if RSTL_CORRUPTION
  UpdatePOIStates();
#endif

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const CSegId root = CSegId::Root();
#else
  const CSegId root = CSegId(0);
#endif
  const CQuaternion oldRotation = mSource->GetRotation(root, previousTime);
  const CQuaternion newRotation = mSource->GetRotation(root, mCurTime);
  const CQuaternion inverseOldRotation = oldRotation.BuildInverted();
  CVector3f offset(0.f, 0.f, 0.f);
  if (mSource->HasOffset(root)) {
    const CVector3f oldOffset = mSource->GetOffset(root, previousTime);
    const CVector3f newOffset = mSource->GetOffset(root, mCurTime);
    offset = newOffset - oldOffset;
    const CQuaternion inverseNewRotation = newRotation.BuildInverted();
    const CMatrix3f inverseRotation = inverseNewRotation.BuildTransform();
    offset = inverseRotation * offset;
  }
  return SAdvancementResults(remainingTime,
#if RSTL_CORRUPTION
                             SAdvancementDeltas(offset, newRotation * inverseOldRotation));
#else
                             CAdvancementDeltas(offset, newRotation * inverseOldRotation));

#endif
}

CCharAnimTime CAnimSourceReader::VGetTimeRemaining() const {
  return mSource->GetAnimationDuration() - mCurTime;
}

CSteadyStateAnimInfo CAnimSourceReader::VGetSteadyStateAnimInfo() const {
  return mSteadyStateInfo;
}

bool CAnimSourceReader::VHasOffset(const CSegId& seg) const {
  return mSource->HasOffset(seg);
}

CVector3f CAnimSourceReader::VGetOffset(const CSegId& seg) const {
  return mSource->GetOffset(seg, mCurTime);
}

CVector3f CAnimSourceReader::VGetOffset(const CSegId& seg, const CCharAnimTime& time) const {
  return mSource->GetOffset(seg, time);
}

CQuaternion CAnimSourceReader::VGetRotation(const CSegId& seg) const {
  return mSource->GetRotation(seg, mCurTime);
}

void CAnimSourceReader::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& set) const {
  mSource->GetSegStatementSet(list, set, mCurTime);
}

void CAnimSourceReader::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& set,
                                            const CCharAnimTime& time) const {
  mSource->GetSegStatementSet(list, set, time);
}

#if RSTL_CORRUPTION
void CAnimSourceReader::VGetJointData_Linear(const CCharLayoutInfo& layout,
                                             CJointData_LinearStorage& data,
                                             const CCharAnimTime& time) const {
  mSource->GetJointData_Linear(layout, data, time);
}

const CVisibilityAnimationDataReader* CAnimSourceReader::VGetVisibilityDataAtCurrentTime() const {
  return mSource->GetVisibilityDataAtTime(mCurTime);
}

const CVisibilityAnimationDataReader*
CAnimSourceReader::VGetVisibilityDataAtTime(const CCharAnimTime& time) const {
  return mSource->GetVisibilityDataAtTime(time);
}

void CAnimSourceReader::VGetJointData_Linear(const CCharLayoutInfo& layout,
                                             CJointData_LinearStorage& data) const {
  mSource->GetJointData_Linear(layout, data, mCurTime);
}
#else
#if !RSTL_PRIME1

void CAnimSourceReader::VGetJointData_Linear(const CCharLayoutInfo& layout,
                                             CJointData_LinearStorage& data,
                                             const CCharAnimTime& time) const {
  mSource->GetJointData_Linear(layout, data, time);
}

void CAnimSourceReader::VGetJointData_Linear(const CCharLayoutInfo& layout,
                                             CJointData_LinearStorage& data) const {
  mSource->GetJointData_Linear(layout, data, mCurTime);
}
#endif
#endif

rstl::ownership_transfer< IAnimReader > CAnimSourceReader::VClone() const {
#if RSTL_CORRUPTION
  return rstl::ownership_transfer< IAnimReader >(rs_new_line(212) CAnimSourceReader(
      mSource, mPOIData, mCurTime, mSteadyStateInfo, mPassedBoolCount, mPassedParticleCount,
      mPassedSoundCount, mPassedAssetSoundCount, mPassedAssetCount, mBoolStates, mParticleStates));
#else
  return rstl::ownership_transfer< IAnimReader >(rs_new CAnimSourceReader(
      mSource
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
      , mPOIData
#endif
      , mCurTime, mSteadyStateInfo, mPassedBoolCount, mPassedIntCount, mPassedParticleCount,
      mPassedSoundCount, mBoolStates, mInt32States, mParticleStates));
#endif
}

SAdvancementResults CAnimSourceReader::VReverseView(const CCharAnimTime& dt) {

  const CCharAnimTime previousTime = mCurTime;
  const CCharAnimTime& duration = mSource->GetAnimationDuration();
  if (previousTime.EqualsZero()) {
    mCurTime = duration;
#if RSTL_CORRUPTION
    return SAdvancementResults(dt,
                               SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
#else
    return SAdvancementResults(dt, CAdvancementDeltas(
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
            CVector3f::Zero()
            , CQuaternion::NoRotation()
#endif
            ));

#endif
  }
  if (dt.EqualsZero()) {
#if RSTL_CORRUPTION
    return SAdvancementResults(CCharAnimTime::ZeroFlat(),
                               SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
#else
    return SAdvancementResults(CCharAnimTime::ZeroFlat(), CAdvancementDeltas(
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
            CVector3f::Zero()
            , CQuaternion::NoRotation()
#endif
            ));

#endif
  }

  mCurTime -= dt;
  CCharAnimTime remainingTime = CCharAnimTime::ZeroFlat();
  if (mCurTime < CCharAnimTime()) {
    remainingTime = CCharAnimTime() - mCurTime;
    mCurTime = CCharAnimTime();
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const CSegId root = CSegId::Root();
#else
  const CSegId root = CSegId(0);
#endif
  const CQuaternion oldRotation = mSource->GetRotation(root, previousTime);
  const CQuaternion newRotation = mSource->GetRotation(root, mCurTime);
  const CQuaternion inverseOldRotation = oldRotation.BuildInverted();
  CVector3f offset(0.f, 0.f, 0.f);
  if (mSource->HasOffset(root)) {
    const CVector3f oldOffset = mSource->GetOffset(root, previousTime);
    const CVector3f newOffset = mSource->GetOffset(root, mCurTime);
    offset = newOffset - oldOffset;
    const CQuaternion inverseNewRotation = newRotation.BuildInverted();
    const CMatrix3f inverseRotation = inverseNewRotation.BuildTransform();
    offset = inverseRotation * offset;
  }
  return SAdvancementResults(remainingTime,
#if RSTL_CORRUPTION
                             SAdvancementDeltas(offset, newRotation * inverseOldRotation));
#else
                             CAdvancementDeltas(offset, newRotation * inverseOldRotation));

#endif
}

void CAnimSourceReader::VSetPhase(float phase) {
#if RSTL_CORRUPTION
  const CSteadyStateAnimInfo& steadyStateInfo = mSource->GetSteadyStateAnimInfo(mCurTime);
  mCurTime = CCharAnimTime(phase * steadyStateInfo.GetDuration().GetSeconds());
#else
#if RSTL_PRIME1
  mCurTime = CCharAnimTime(
      phase * mSource->GetSteadyStateAnimInfo(mCurTime).GetDuration().GetSeconds());
  if (mSource->HasPOIData())
#else
  const CCharAnimTime duration = mSource->GetSteadyStateAnimInfo(mCurTime).GetDuration();
  mCurTime = CCharAnimTime(phase * duration.GetSeconds());
#endif
  {
#endif
    UpdatePOIStates();
    if (!mCurTime.GreaterThanZero()) {
      mPassedBoolCount = 0;
#if !RSTL_CORRUPTION
      mPassedIntCount = 0;
#endif
      mPassedParticleCount = 0;
      mPassedSoundCount = 0;
#if RSTL_CORRUPTION
    mPassedAssetSoundCount = 0;
    mPassedAssetCount = 0;
#else
    }
#endif
  }
}

bool CAnimSourceReader::VSupportsReverseView() const { return true; }

SAdvancementResults
CAnimSourceReader::VGetAdvancementResults(const CCharAnimTime& dt,
                                          const CCharAnimTime& startOffset) const {
  const CCharAnimTime previousTime = mCurTime + startOffset;
  CCharAnimTime currentTime = mCurTime + startOffset;
  const CCharAnimTime& duration = mSource->GetAnimationDuration();
  if (previousTime >= duration) {
#if RSTL_CORRUPTION
    return SAdvancementResults(dt,
                               SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
#else
    return SAdvancementResults(dt, CAdvancementDeltas(
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
            CVector3f::Zero()
            , CQuaternion::NoRotation()
#endif
            ));

#endif
  }
  if (dt.EqualsZero()) {
#if RSTL_CORRUPTION
    return SAdvancementResults(CCharAnimTime::ZeroFlat(),
                               SAdvancementDeltas(CVector3f::Zero(), CQuaternion::NoRotation()));
#else
    return SAdvancementResults(CCharAnimTime::ZeroFlat(), CAdvancementDeltas(
// TODO: verify that this Prime/Echoes difference is real
#if !RSTL_PRIME1
            CVector3f::Zero()
            , CQuaternion::NoRotation()
#endif
            ));

#endif
  }

  currentTime += dt;
  CCharAnimTime remainingTime = CCharAnimTime::ZeroFlat();
  if (currentTime > duration) {
    remainingTime = currentTime - duration;
    currentTime = duration;
  }

// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
  const CSegId root = CSegId::Root();
#else
  const CSegId root = CSegId(0);
#endif
  CVector3f offset(0.f, 0.f, 0.f);
  const CQuaternion oldRotation = mSource->GetRotation(root, previousTime);
  const CQuaternion newRotation = mSource->GetRotation(root, currentTime);
  const CQuaternion inverseOldRotation = oldRotation.BuildInverted();
  if (mSource->HasOffset(root)) {
    const CVector3f oldOffset = mSource->GetOffset(root, previousTime);
    const CVector3f newOffset = mSource->GetOffset(root, currentTime);
    offset = newOffset - oldOffset;
    const CQuaternion inverseNewRotation = newRotation.BuildInverted();
    const CMatrix3f inverseRotation = inverseNewRotation.BuildTransform();
    offset = inverseRotation * offset;
  }
  return SAdvancementResults(remainingTime,
#if RSTL_CORRUPTION
                             SAdvancementDeltas(offset, newRotation * inverseOldRotation));
#else
                             CAdvancementDeltas(offset, newRotation * inverseOldRotation));

#endif
}

#if !RSTL_PRIME1
CAnimSourceReader::~CAnimSourceReader() {}
#endif
